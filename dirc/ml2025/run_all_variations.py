#!/usr/bin/env python3
"""
Launch run_lut_variations.py for all combinations of bar, xbin, sample, and LUT version,
then run plot_single_summary.py on the resulting summary_results.csv.
Edit the lists below to add or remove items.
"""

import re
import subprocess
import sys
from pathlib import Path
from unittest import result

# --- configurable lists ---------------------------------------------------
BAR_IDS   = [3, 7, 12, 16, 27, 31, 36, 40]
XBINS     = [4, 5]
SAMPLES   = [
    "hd_root_bothp_flat_MRASX4.0_ROTY-0.5_MRANY50.0_ROTY0.5",
    "hd_root_bothm_flat_MRASX4.0_ROTY-0.5_MRANY50.0_ROTY0.5",
]
LUT_VERS  = ["MRAROT_0", "MRAOFF_0"]
MOMENTUM  = 3.5   # must match default in run_lut_variations.py
# -------------------------------------------------------------------------

ML2025_DIR   = Path(__file__).parent
RUN_SCRIPT   = ML2025_DIR / "run_lut_variations.py"
PLOT_SCRIPT  = ML2025_DIR / "plot_single_summary.py"
PYTHON       = sys.executable


def sanitize(value: str) -> str:
    """Mirror sanitize_name() from run_lut_variations.py."""
    return re.sub(r"[^A-Za-z0-9._-]+", "_", value)


def build_csv_path(lut_ver: str, sample: str, bar: int, xbin: int, momentum: float) -> Path:
    """Reconstruct the summary_results.csv path that run_lut_variations.py will write."""
    return (
        ML2025_DIR
        / "outputs"
        / sanitize(lut_ver)
        / sanitize(sample)
        / f"bar_{bar}"
        / f"xbin_{xbin}"
        / f"momentum_{sanitize(str(momentum))}"
        / "summary_results.csv"
    )


total = len(BAR_IDS) * len(XBINS) * len(SAMPLES) * len(LUT_VERS)
count = 0

for lut_ver in LUT_VERS:
    for sample in SAMPLES:
        for bar in BAR_IDS:
            for xbin in XBINS:
                count += 1
                sep = "=" * 72

                # --- run analysis ---
                run_cmd = [
                    PYTHON, str(RUN_SCRIPT),
                    "--lut-ver", lut_ver,
                    "--bar",     str(bar),
                    "--xbin",    str(xbin),
                    "--p", str(MOMENTUM),
                    "--sample",  sample,
                ]
                print(f"\n[{count}/{total}] {' '.join(run_cmd)}\n{sep}")
                result = subprocess.run(run_cmd)
                if result.returncode != 0:
                    print(f"WARNING: run_lut_variations returned code {result.returncode}")

                # --- plot summary ---
                csv_path = build_csv_path(lut_ver, sample, bar, xbin, MOMENTUM)
                if csv_path.exists():
                    plot_cmd = [PYTHON, str(PLOT_SCRIPT), str(csv_path)]
                    print(f"\n  plotting: {' '.join(plot_cmd)}\n{sep}")
                    presult = subprocess.run(plot_cmd)
                    if presult.returncode != 0:
                        print(f"WARNING: plot_single_summary returned code {presult.returncode}")
                else:
                    print(f"WARNING: CSV not found, skipping plot: {csv_path}")
