#!/usr/bin/env python3
"""
repair_rest.py - make a "segmented" HDDM file (e.g. REST) readable as one stream.

HDDM file layout (after the XML header that ends with "</HDDM>\\n"), as
written by the hddm ostream: a sequence of items, each starting with a
4-byte big-endian length L.
  L == 1    stream-option record, 16 bytes: 1, 8, 0, status, where
            status = compression (0x00 none, 0x10 zlib, 0x20 bzip2)
                   | integrity   (0x01 crc32)
                   | 0x100 (repositioning supported);
  L != 1    a block of L bytes: one record when uncompressed (followed by
            its 4-byte crc32 when crc32 integrity is on), otherwise one
            compressed block.

A segmented file has one XML header followed by several compressed streams,
each beginning with its own stream-option record: the bytes of several
complete files, minus their headers, written one after the other. HDDM
readers (JANA's REST source, gluex.hddm_r, ...) stop at the end of the first
compressed stream: with bzip2 silently, with zlib after printing
"z input stream crapping out, cret is -3". Only the first stream is read;
for the segmented 2019-11 ver01 REST files that is about 10% of the events.
Uncompressed segmented files are read in full and need no repair. A file in
which catenated streams use DIFFERENT compression (e.g. bzip2 then zlib) also
stops the readers; it cannot be repaired by dropping records, and the script
reports it and writes nothing.

The repair walks the framing exactly and drops every stream-option record
that restarts a compressed stream of the same compression type, so all the
compressed blocks follow each other in one stream. Records that really change
the compression are kept. Nothing else is changed: the output is exactly 16
bytes shorter per removed record, which is checked. If the framing walk does
not end exactly at the end of the file, nothing is written.

Usage:
  repair_rest.py --check FILE [FILE ...]   list the stream-option records (no output)
  repair_rest.py SRC DST [--verify]        write the repaired copy to DST

The output goes to DST.part and is renamed to DST only after the size check.
DST must not exist and must not be SRC. Write it under a NEW name or path:
never put the repaired bytes back under the original name in a cached storage
system (dCache, xrootd/OSDF caches), because cached copies of the old bytes
would keep being served under that name.

--verify counts the events of SRC and DST with gluex.hddm_r
(pip install gluex.hddm_r); the repaired file must give more events. This takes
a few minutes per million events. It reads REST files only (hddm_r).

Exit status: 0 on success (with --check: no file needs repair); 1 if a file
needs repair (--check) or on any failure.

R. Jones (UConn), 2026. Tested on synthetic segmented REST files in all six
modes (no/zlib/bzip2 compression x no/crc32 integrity), and earlier (bzip2
only) on 13 segmented 2019-11 ver01 REST files.
"""
import argparse
import os
import struct
import sys

COMP = {0x00: "none", 0x10: "zlib", 0x20: "bzip2"}
REC = 16
END = b"</HDDM>\n"
CH = 1 << 26                 # copy chunk size (64 MB)


def header_end(f):
    head = f.read(1 << 20)
    if not head.startswith(b"<HDDM"):
        raise ValueError("not an HDDM file (no <HDDM header)")
    x = head.find(END)
    if x < 0:
        raise ValueError("end of the XML header not found in the first MB")
    return x + len(END)


def walk(path):
    """Walk the framing. Returns (header length, [(offset, status, action)]), action "keep", "remove" or "bad".
    The writer puts every later stream-option record INSIDE the compressed stream, so an outer record after a
    compressed stream started can only come from catenation: "remove" when it restarts the same compression
    (the repair), "bad" when it switches to another compression (the reader stops there; not repairable here)."""
    size = os.path.getsize(path)
    recs = []
    with open(path, "rb") as f:
        p = header_end(f)
        hlen = p
        cur = 0                      # current status as seen from the outer framing
        f.seek(p)
        while p < size:
            w = f.read(4)
            if len(w) < 4:
                raise ValueError("truncated item at byte %d" % p)
            L = struct.unpack(">I", w)[0]
            if L == 1:
                r = f.read(12)
                if len(r) < 12 or struct.unpack(">II", r[:8]) != (8, 0):
                    raise ValueError("malformed stream-option record at byte %d" % p)
                st = struct.unpack(">I", r[8:])[0]
                c = st & 0xf0
                if c not in COMP:
                    raise ValueError("unknown compression 0x%x at byte %d" % (c, p))
                prev = cur & 0xf0
                action = "keep" if (not recs or prev == 0) else ("remove" if c == prev else "bad")
                recs.append((p, st, action))
                cur = st
                p += REC
            else:
                p += 4 + L
                if (cur & 0xf0) == 0 and (cur & 0x01):
                    p += 4           # crc32 after each uncompressed record
                f.seek(p)
        if p != size:
            raise ValueError("framing walk ended at byte %d, file size %d: layout not understood" % (p, size))
    return hlen, recs


def describe(st):
    return "%s%s" % (COMP[st & 0xf0], "+crc32" if st & 0x01 else "")


def copy_without(src, dst, offsets):

    with open(src, "rb") as fin, open(dst, "wb") as fout:
        pos = 0
        for cut in offsets + [None]:
            n = (cut - pos) if cut is not None else None
            while n is None or n > 0:
                b = fin.read(CH if n is None else min(CH, n))
                if not b:
                    break
                fout.write(b)
                if n is not None:
                    n -= len(b)
            if cut is None:
                break
            fin.seek(cut + REC)
            pos = cut + REC


def count_events(path):
    from gluex import hddm_r
    n = 0
    for rec in hddm_r.istream(path):
        n += len(rec.getReconstructedPhysicsEvents())
    return n


def main():
    ap = argparse.ArgumentParser(description="Make a segmented HDDM (REST) file readable as one stream.")
    ap.add_argument("--check", action="store_true", help="only list the stream-option records of the given files")
    ap.add_argument("--verify", action="store_true", help="count the events of SRC and DST with gluex.hddm_r")
    ap.add_argument("files", nargs="+", help="--check: FILE [FILE ...]; otherwise: SRC DST")
    a = ap.parse_args()

    if a.check:
        bad = 0
        for f in a.files:
            try:
                _, recs = walk(f)
            except (ValueError, OSError) as e:
                print("%s: ERROR %s" % (f, e)); bad += 1; continue
            nrm = sum(r[2] == "remove" for r in recs)
            nbad = [r[0] for r in recs if r[2] == "bad"]
            modes = ", ".join(describe(r[1]) for r in recs) or "none (no stream-option record)"
            print("%s: %d stream-option record%s (%s)%s%s" % (f, len(recs), "" if len(recs) == 1 else "s", modes,
                  ", SEGMENTED: %d to remove" % nrm if nrm else "",
                  ", NOT REPAIRABLE: compression changes between catenated streams at byte %s (readers stop there)"
                  % ", ".join(map(str, nbad)) if nbad else ""))
            bad += (nrm > 0) or bool(nbad)
        return 1 if bad else 0

    if len(a.files) != 2:
        ap.error("give SRC and DST (or --check FILE ...)")
    src, dst = a.files
    if os.path.exists(dst):
        sys.exit("%s exists: refusing to overwrite" % dst)
    if os.path.abspath(dst) == os.path.realpath(src) or os.path.realpath(dst) == os.path.realpath(src):
        sys.exit("DST is SRC")
    try:
        _, recs = walk(src)
    except (ValueError, OSError) as e:
        sys.exit("%s: %s; nothing written" % (src, e))
    nbad = [r[0] for r in recs if r[2] == "bad"]
    if nbad:
        sys.exit("%s: compression changes between catenated streams at byte %s; readers stop there and this script "
                 "cannot merge streams of different compression; nothing written" % (src, ", ".join(map(str, nbad))))
    cut = [r[0] for r in recs if r[2] == "remove"]
    if not cut:
        print("%s: not segmented (%d stream-option records); nothing written" % (src, len(recs)))
        return 0
    part = dst + ".part"
    copy_without(src, part, cut)
    for p in cut:
        print("removed stream-option record at byte %d" % p)
    s1, s2 = os.path.getsize(src), os.path.getsize(part)
    print("%d stream-option records, %d removed (%s); size %d -> %d (difference %d, expected %d)"
          % (len(recs), len(cut), describe(recs[0][1]), s1, s2, s1 - s2, REC * len(cut)))
    if s1 - s2 != REC * len(cut):
        os.remove(part)
        sys.exit("size check FAILED; output removed")
    try:
        _, recs2 = walk(part)
    except ValueError as e:
        os.remove(part)
        sys.exit("framing of the output FAILED (%s); output removed" % e)
    if any(r[2] != "keep" for r in recs2):
        os.remove(part)
        sys.exit("output still segmented; output removed")
    os.rename(part, dst)
    print("wrote %s" % dst)
    if a.verify:
        e1, e2 = count_events(src), count_events(dst)
        print("events read: %s %d, %s %d" % (src, e1, dst, e2))
        if e2 <= e1:
            sys.exit("verify FAILED: the repaired file does not give more events than the original")
    return 0


if __name__ == "__main__":
    sys.exit(main())
