#!/usr/bin/env python3
"""
Script to run reco_lut_simple.C for all lut_all_avr.root files in the lut_alignment directory.
Automatically extracts variation number from directory name, finds DIRC_HDDS.xml for MRAS/MRAN values.
"""

import os
import subprocess
import re
import csv
import shutil
import argparse
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

# Default configuration (can be overridden by CLI arguments)
DEFAULT_LUT_VER = "MRAROT_0"
DEFAULT_BAR_ID = 7
DEFAULT_XBIN = 4
DEFAULT_MOMENTUM = 3.5
DEFAULT_SAMPLE = "hd_root_bothp_p3.5_theta4.root"

# Globals?
LUT_VER = DEFAULT_LUT_VER
BAR_ID = DEFAULT_BAR_ID
XBIN = DEFAULT_XBIN
MOMENTUM = DEFAULT_MOMENTUM
SAMPLE = DEFAULT_SAMPLE

MAX_WORKERS = 12 # parallel processes to run reco_lut_simple.C variations; adjust based on CPU cores and memory constraints

SCRIPT_DIR = Path(__file__).resolve().parent
SHARED_LIBRARIES = [
    SCRIPT_DIR / "DrcEvent_cc.so",
    SCRIPT_DIR / "DrcHit_cc.so",
]

# ROOT command template
ROOT_CMD = "root -l -b -q $HD_UTILITIES_HOME/dirc/ml2025/loadlib.C '$HD_UTILITIES_HOME/dirc/ml2025/reco_lut_simple.C(\"{tree}\",\"{lut}\", {bar}, {bin}, {momentum}, {variation})'"

def sanitize_name(value):
    """Sanitize strings to safe directory/file name fragments."""
    return re.sub(r"[^A-Za-z0-9._-]+", "_", value)


def format_numeric_label(value):
    """Format numbers for stable path components using decimal notation (e.g. 3.5)."""
    return sanitize_name(str(value))


def get_output_dir():
    """Build output directory path keyed by LUT version, tree, bar, xbin and momentum."""
    lut_ver_name = sanitize_name(LUT_VER)
    tree_name = sanitize_name(Path(TREE_FILE).stem)
    bar_name = f"bar_{BAR_ID}"
    xbin_name = f"xbin_{XBIN}"
    momentum_name = f"momentum_{format_numeric_label(MOMENTUM)}"
    return SCRIPT_DIR / "outputs" / lut_ver_name / tree_name / bar_name / xbin_name / momentum_name


def copy_optimal_pdfs(optimal_work_dir, output_dir, variation):
    """Copy only lndiff/angle_diff/time_diff PDFs from the best variation to output_dir."""
    plots_dir = Path(optimal_work_dir) / "plots"
    if not plots_dir.is_dir():
        raise FileNotFoundError(f"plots directory not found for optimal variation: {plots_dir}")

    wanted_prefixes = ("lndiff_", "angle_diff_", "time_diff_", "nph_")
    copied = 0
    for plot_file in plots_dir.iterdir():
        if not plot_file.is_file() or plot_file.suffix.lower() != ".pdf":
            continue
        if not plot_file.name.startswith(wanted_prefixes):
            continue

        destination = Path(output_dir) / f"var{variation}_{plot_file.name}"
        shutil.copy2(plot_file, destination)
        copied += 1

    if copied == 0:
        raise FileNotFoundError(f"No matching PDF plots found in {plots_dir}")

    print(f"  ✓ Copied {copied} optimal-variation PDF plots to {output_dir}")

def extract_variation_number(path_str):
    """Extract variation number from path like 'var1', 'var2', etc."""
    match = re.search(r'var(\d+)', path_str)
    if match:
        return int(match.group(1))
    return None

def extract_xml_alignment_values(lut_dir):
    """
    Extract MRAS and MRAN X, Y, Z values and rotation values from DIRC_HDDS.xml in the LUT directory.
    Searches for strings like: volume="MRAS" X_Y_Z="values" and rot="rotx roty rotz"
    Returns a dictionary with keys: mras_x, mras_y, mras_z, mran_x, mran_y, mran_z,
                                     mras_rotx, mras_roty, mras_rotz, mran_rotx, mran_roty, mran_rotz
    Returns dict with 0.0 values if file not found or parsing fails.
    """
    xml_file = os.path.join(lut_dir, "DIRC_HDDS.xml")
    defaults = {
        'mras_x': 0.0, 'mras_y': 0.0, 'mras_z': 0.0,
        'mran_x': 0.0, 'mran_y': 0.0, 'mran_z': 0.0,
        'mras_rotx': 0.0, 'mras_roty': 0.0, 'mras_rotz': 0.0,
        'mran_rotx': 0.0, 'mran_roty': 0.0, 'mran_rotz': 0.0
    }
    
    if not os.path.exists(xml_file):
        print(f"  Warning: DIRC_HDDS.xml not found at {xml_file}")
        return defaults
    
    try:
        with open(xml_file, 'r') as f:
            content = f.read()
        
        # Search for MRAS volume with X_Y_Z attribute
        mras_pattern = r'volume="MRAS"\s+X_Y_Z="([^"]+)"'
        mras_match = re.search(mras_pattern, content)
        if mras_match:
            values = mras_match.group(1).split()
            if len(values) >= 3:
                defaults['mras_x'] = float(values[0])
                defaults['mras_y'] = float(values[1])
                defaults['mras_z'] = float(values[2])
        
        # Search for MRAN volume with X_Y_Z attribute
        mran_pattern = r'volume="MRAN"\s+X_Y_Z="([^"]+)"'
        mran_match = re.search(mran_pattern, content)
        if mran_match:
            values = mran_match.group(1).split()
            if len(values) >= 3:
                defaults['mran_x'] = float(values[0])
                defaults['mran_y'] = float(values[1])
                defaults['mran_z'] = float(values[2])
        
        # Search for MRAS rotation values
        mras_rot_pattern = r'volume="MRAS"[^>]*rot="([^"]+)"'
        mras_rot_match = re.search(mras_rot_pattern, content)
        if mras_rot_match:
            values = mras_rot_match.group(1).split()
            if len(values) >= 3:
                defaults['mras_rotx'] = float(values[0])
                defaults['mras_roty'] = float(values[1])
                defaults['mras_rotz'] = float(values[2])
        
        # Search for MRAN rotation values
        mran_rot_pattern = r'volume="MRAN"[^>]*rot="([^"]+)"'
        mran_rot_match = re.search(mran_rot_pattern, content)
        if mran_rot_match:
            values = mran_rot_match.group(1).split()
            if len(values) >= 3:
                defaults['mran_rotx'] = float(values[0])
                defaults['mran_roty'] = float(values[1])
                defaults['mran_rotz'] = float(values[2])
        
        print(f"  Found alignment values:")
        print(f"    MRAS: X={defaults['mras_x']}, Y={defaults['mras_y']}, Z={defaults['mras_z']}")
        print(f"    MRAS rot: X={defaults['mras_rotx']}, Y={defaults['mras_roty']}, Z={defaults['mras_rotz']}")
        print(f"    MRAN: X={defaults['mran_x']}, Y={defaults['mran_y']}, Z={defaults['mran_z']}")
        print(f"    MRAN rot: X={defaults['mran_rotx']}, Y={defaults['mran_roty']}, Z={defaults['mran_rotz']}")
        
        return defaults
        
    except Exception as e:
        print(f"  Error parsing DIRC_HDDS.xml: {e}")
        return defaults

def find_lut_files(base_dir):
    """Find all lut_all_avr.root files and their variation numbers, plus XML alignment values."""
    lut_files = []
    
    if not os.path.exists(base_dir):
        print(f"Error: Directory {base_dir} not found")
        return lut_files
    
    # Walk through directory structure
    for root, dirs, files in os.walk(base_dir):
        if "lut_all_avr.root" in files:
            full_path = os.path.join(root, "lut_all_avr.root")
            var_num = extract_variation_number(root)
            if var_num is not None:
                # Extract XML alignment values from the variation directory
                xml_values = extract_xml_alignment_values(root)
                lut_files.append((var_num, full_path, xml_values))
                print(f"Found: var{var_num} -> {full_path}")
    
    # Sort by variation number
    lut_files.sort(key=lambda x: x[0])
    return lut_files


def get_csv_header():
    return [
        "ivar", "ibar", "ibin", "momentum_GeV", "nph_pi", "nph_K",
        "spr_pi", "spr_pi_err", "spr_K", "spr_K_err", "sep", "sep_err",
        "mras_x", "mras_y", "mras_z", "mran_x", "mran_y", "mran_z",
        "mras_rotx", "mras_roty", "mras_rotz", "mran_rotx", "mran_roty", "mran_rotz",
    ]


def build_csv_row(result_row, xml_values):
    row = dict(result_row)
    row.update({key: xml_values[key] for key in get_csv_header() if key in xml_values})
    return row


def read_single_result_row(csv_file):
    """Read the single variation result row produced by reco_lut_simple.C."""
    csv_path = Path(csv_file)
    if not csv_path.exists():
        raise FileNotFoundError(f"summary CSV not found: {csv_path}")

    with csv_path.open("r", newline="") as f:
        rows = list(csv.DictReader(f))

    if not rows:
        raise ValueError(f"summary CSV has no data rows: {csv_path}")

    return rows[-1]


def write_merged_summary(rows, csv_file):
    """Write the merged summary CSV after parallel processing completes."""
    csv_path = Path(csv_file)
    csv_path.parent.mkdir(parents=True, exist_ok=True)

    with csv_path.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=get_csv_header())
        writer.writeheader()
        for row in rows:
            writer.writerow(row)


def stage_shared_libraries(work_dir):
    """Copy required ROOT shared libraries into an isolated variation directory."""
    work_dir = Path(work_dir)
    missing_libraries = [lib for lib in SHARED_LIBRARIES if not lib.exists()]
    if missing_libraries:
        missing_str = ", ".join(str(lib) for lib in missing_libraries)
        raise FileNotFoundError(f"missing shared libraries: {missing_str}")

    for lib in SHARED_LIBRARIES:
        shutil.copy2(lib, work_dir / lib.name)


def run_single_variation(task):
    """Run reco_lut_simple.C for one variation in an isolated working directory."""
    var_num, lut_path, xml_values, runs_dir_str = task
    runs_dir = Path(runs_dir_str)
    work_dir = runs_dir / f"var{var_num}"

    if work_dir.exists():
        shutil.rmtree(work_dir)
    work_dir.mkdir(parents=True, exist_ok=True)
    stage_shared_libraries(work_dir)

    cmd = ROOT_CMD.format(
        tree=TREE_FILE,
        lut=lut_path,
        bar=BAR_ID,
        bin=XBIN,
        momentum=MOMENTUM,
        variation=var_num,
    )

    log_path = work_dir / "run.log"
    with log_path.open("w") as log_file:
        completed = subprocess.run(
            cmd,
            shell=True,
            cwd=work_dir,
            stdout=log_file,
            stderr=subprocess.STDOUT,
            text=True,
            check=False,
        )

    if completed.returncode != 0:
        return {
            "variation": var_num,
            "success": False,
            "returncode": completed.returncode,
            "work_dir": str(work_dir),
            "log_path": str(log_path),
            "error": f"ROOT exited with code {completed.returncode}",
        }

    try:
        result_row = read_single_result_row(work_dir / "summary_results.csv")
        merged_row = build_csv_row(result_row, xml_values)
        sep = float(merged_row["sep"])
        sep_err = float(merged_row["sep_err"])
        score = sep
    except Exception as e:
        return {
            "variation": var_num,
            "success": False,
            "returncode": completed.returncode,
            "work_dir": str(work_dir),
            "log_path": str(log_path),
            "error": str(e),
        }

    return {
        "variation": var_num,
        "success": True,
        "returncode": completed.returncode,
        "work_dir": str(work_dir),
        "log_path": str(log_path),
        "row": merged_row,
        "sep": sep,
        "sep_err": sep_err,
        "score": score,
    }

def run_analysis(lut_files):
    """Run reco_lut_simple.C for each LUT file using isolated work dirs and merge results."""
    if not lut_files:
        print("No LUT files found!")
        return
    
    output_dir = get_output_dir()
    runs_dir = output_dir / "variation_runs"
    summary_csv = output_dir / "summary_results.csv"

    print(f"\nFound {len(lut_files)} variations to process with {MAX_WORKERS} workers")
    print(f"Output directory: {output_dir}\n")

    output_dir.mkdir(parents=True, exist_ok=True)
    runs_dir.mkdir(parents=True, exist_ok=True)

    best_sep = float("-inf")
    best_variation = None
    best_work_dir = None

    merged_rows = []
    failures = []

    tasks = [(var_num, lut_path, xml_values, str(runs_dir)) for var_num, lut_path, xml_values in lut_files]

    with ProcessPoolExecutor(max_workers=MAX_WORKERS) as executor:
        future_map = {executor.submit(run_single_variation, task): task[0] for task in tasks}

        for future in as_completed(future_map):
            var_num = future_map[future]
            print(f"\n{'='*80}")
            print(f"Completed variation {var_num}")
            print(f"{'='*80}")

            try:
                result = future.result()
            except Exception as e:
                print(f"✗ Variation {var_num} encountered an executor error: {e}")
                failures.append((var_num, str(e), None))
                continue

            if not result["success"]:
                print(f"✗ Variation {var_num} failed: {result['error']}")
                print(f"  Log: {result['log_path']}")
                failures.append((var_num, result["error"], result["log_path"]))
                continue

            merged_rows.append(result["row"])
            score = result["score"]
            print(f"  sep for var{var_num}: {score:.4f} (sep_err={result['sep_err']:.4f})")
            if score > best_sep:
                best_sep = score
                best_variation = var_num
                best_work_dir = result["work_dir"]
                print(f"  New best sep found at var{var_num}")

    merged_rows.sort(key=lambda row: int(float(row["ivar"])))
    write_merged_summary(merged_rows, summary_csv)

    if best_variation is not None and best_work_dir is not None:
        copy_optimal_pdfs(best_work_dir, output_dir, best_variation)
    
    print(f"\n{'='*80}")
    print("All variations processed!")
    if best_variation is not None:
        print(f"Best sep variation: var{best_variation} ({best_sep:.4f})")
    print(f"Merged summary written to {summary_csv}")
    if failures:
        print(f"Failed variations: {len(failures)}")
        for var_num, error, log_path in failures:
            print(f"  var{var_num}: {error}")
            if log_path:
                print(f"    log: {log_path}")
    print(f"{'='*80}")


def parse_args():
    """Parse optional CLI overrides for analysis configuration."""
    parser = argparse.ArgumentParser(
        description=(
            "Run reco_lut_simple.C across LUT variations, merge summary_results.csv, "
            "and save plots for the best-separation variation."
        ),
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        epilog=(
            "Examples:\n"
            "  python run_lut_variations.py\n"
            "  python run_lut_variations.py --lut-ver MRAROT_1 --bar 15\n"
            "  python run_lut_variations.py --xbin 2 --p 3.0"
        ),
    )
    parser.add_argument(
        "--lut-ver",
        default=LUT_VER,
        help="LUT version suffix used in lut_alignment_<LUT_VER> directory",
    )
    parser.add_argument(
        "--bar",
        type=int,
        default=BAR_ID,
        help="DIRC bar ID used in reco selection and tree path trees/bar_<BAR_ID>/...",
    )
    parser.add_argument(
        "--xbin",
        type=int,
        default=XBIN,
        help="X-bin index passed to reco_lut_simple.C",
    )
    parser.add_argument(
        "--p",
        type=float,
        default=MOMENTUM,
        help="Momentum (GeV/c) passed to reco_lut_simple.C",
    )
    parser.add_argument(
        "--sample",
        default=SAMPLE,
        help="data sample passed to reco_lut_simple.C",
    )
    return parser.parse_args()


def apply_runtime_config(args):
    """Apply CLI overrides and update dependent paths."""
    global LUT_VER, BAR_ID, XBIN, MOMENTUM, LUT_DIR, TREE_FILE, SAMPLE

    LUT_VER = args.lut_ver
    BAR_ID = args.bar
    XBIN = args.xbin
    MOMENTUM = args.p
    SAMPLE = args.sample

    LUT_DIR = f"/work/halld/home/jrsteven/analysisGluexII/dircsim-2019_11-ver05/lut_alignment_{LUT_VER}"
    TREE_FILE = f"/work/halld2/home/jrsteven/analysisGluexII/builds/hd_utilities/dirc/ml2025/trees/bar_{BAR_ID}/{SAMPLE}.root"

    print("Runtime configuration:")
    print(f"  LUT_VER   = {LUT_VER}")
    print(f"  BAR_ID    = {BAR_ID}")
    print(f"  XBIN      = {XBIN}")
    print(f"  MOMENTUM  = {MOMENTUM}")
    print(f"  LUT_DIR   = {LUT_DIR}")
    print(f"  TREE_FILE = {TREE_FILE}")


if __name__ == "__main__":
    args = parse_args()
    apply_runtime_config(args)
    lut_files = find_lut_files(LUT_DIR)
    run_analysis(lut_files)
