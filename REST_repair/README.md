# REST_repair

`repair_rest.py` makes a *segmented* HDDM file (e.g. REST) readable as a single
stream.

## The problem

After its XML header, an HDDM file is a sequence of length-framed items. A
16-byte stream-option record (`1, 8, 0, status`) announces the compression
(none, zlib or bzip2) and integrity (crc32) mode.

A segmented file contains several compressed streams after its one header,
each starting with its own stream-option record, as if several files had been
written one after the other without their headers. Every HDDM reader (the JANA
REST event source, `gluex.hddm_r`, ...) stops at the end of the first
compressed stream, and only the first stream is read:

| compression | segmented file read by hddm_r                                      |
|-------------|--------------------------------------------------------------------|
| bzip2       | first stream only, **no message**                                  |
| zlib        | first stream only, message `z input stream crapping out, cret is -3` |
| none        | read in full: nothing to repair                                    |

Example: about 19,000 files of RunPeriod-2019-11 `recon/ver01/REST` are
segmented into about 10 bzip2 streams each, so analyses that read them see
roughly 10% of the events of those files. The file size looks normal (about
5.8 GB).

Signs that you have hit it:
- a REST file gives roughly 1/10 of the events of its neighbours, or of the
  raw file it came from;
- for the same raw file, a fresh reconstruction gives about 10 times more
  events than the REST file.

## Usage

```
pip install gluex.hddm_r            # only needed for --verify

python3 repair_rest.py --check dana_rest_*.hddm
python3 repair_rest.py dana_rest_071353_025.hddm /new/path/dana_rest_071353_025.hddm --verify
```

- `--check` walks the framing of each file and lists its stream-option
  records. It says whether the file is segmented and how many records must be
  removed. The exit status is 1 if any file needs repair or cannot be read.
  It takes about a second for a 5.8 GB REST file.
- In repair mode the script walks the framing exactly and drops every
  stream-option record that restarts a compressed stream of the same
  compression. All compressed blocks then follow each other in one stream.
  - It checks that the output is exactly 16 bytes shorter per removed record,
    and that the output's own framing walk finds no more segments.
  - It writes `DST.part` first and renames it to `DST` only after those
    checks.
  - It refuses to overwrite an existing `DST`. A file that isn't segmented is
    not copied.
  - If the framing walk does not end exactly at the end of the file, nothing
    is written.
- `--verify` counts the events of SRC and DST with `gluex.hddm_r` (REST
  files). This takes a few minutes per million events.

## Where to put the repaired file

Write the repaired file under a **new path or name**, and then delete the
segmented original. Never write the repaired bytes back under the original
name in dCache or in any xrootd/OSDF-cached area: a cache that already holds
the old bytes keeps serving them under that name, and later jobs silently read
10% of the file again.

## Tests

Synthetic segmented REST files (three copies of a 300-event stream after one
header) in all six modes: no compression, zlib or bzip2, each with and without
crc32.
- **zlib and bzip2, with and without crc32:** the segmented file gives 300
  events. The repaired file gives 900, identical to the original events three
  times in order.
- **Uncompressed:** readers already give 900 events, and the script leaves
  these files alone.
- **Copy step:** the output is byte-identical for copy chunk sizes from 1 byte
  to 64 MB.
- **Earlier use:** the bzip2 repair was used on 13 segmented 2019-11 ver01 REST
  files.

## Limitation

If the catenated streams use **different** compression (e.g. a bzip2 stream
followed by a zlib stream), readers also stop at the boundary. Dropping records
cannot fix that, so `--check` reports the file as `NOT REPAIRABLE` and repair
mode writes nothing.
