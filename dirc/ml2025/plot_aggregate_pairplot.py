#!/usr/bin/env python3
"""
Aggregate alignment parameters from best variations across bar/xbin/momentum combinations
and generate pair plots for MRAOFF or MRAROT alignment studies.
"""

import argparse
import re
from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
import seaborn as sns


ARRAY_NAME_MAP = {
    "mras_x": "MRASXOFFSETS",
    "mras_y": "MRASYOFFSETS",
    "mras_z": "MRASZOFFSETS",
    "mras_rotx": "MRASXROTATIONS",
    "mras_roty": "MRASYROTATIONS",
    "mras_rotz": "MRASZROTATIONS",
    "mran_x": "MRANXOFFSETS",
    "mran_y": "MRANYOFFSETS",
    "mran_z": "MRANZOFFSETS",
    "mran_rotx": "MRANXROTATIONS",
    "mran_roty": "MRANYROTATIONS",
    "mran_rotz": "MRANZROTATIONS",
}

ARRAY_OUTPUT_ORDER = [
    "MRASXOFFSETS",
    "MRASYOFFSETS",
    "MRASZOFFSETS",
    "MRASXROTATIONS",
    "MRASYROTATIONS",
    "MRASZROTATIONS",
    "MRANXOFFSETS",
    "MRANYOFFSETS",
    "MRANZOFFSETS",
    "MRANXROTATIONS",
    "MRANYROTATIONS",
    "MRANZROTATIONS",
]


def compute_best_minus_var0_offsets(df):
    """Compute per-parameter deltas between best-sep row and ivar==0 row."""
    if df.empty or "ivar" not in df.columns:
        return {}

    var0_rows = df[df["ivar"] == 0]
    if var0_rows.empty:
        return {}

    best_row = df.loc[df["sep"].idxmax()]
    var0_row = var0_rows.iloc[0]

    offsets = {}
    for column in ARRAY_NAME_MAP:
        if column not in df.columns:
            continue
        best_value = best_row[column]
        var0_value = var0_row[column]
        if pd.isna(best_value) or pd.isna(var0_value):
            continue
        offsets[column] = float(best_value) - float(var0_value)

    return offsets


def _format_float(value):
    """Format float for compact Python-array text output."""
    if abs(value) < 5e-10:
        value = 0.0
    text = f"{value:.6f}".rstrip("0").rstrip(".")
    if "." not in text:
        text += ".0"
    return text


def write_swif_offset_arrays(offset_values_by_array, output_file, lut_ver, n_csvs):
    """Write swif_LUT_alignment.py-style array assignments to a text file."""
    lines = [
        f"# Auto-generated from optimal rows (max sep) minus var0 for {lut_ver}",
        f"# Source summary_results.csv files used: {n_csvs}",
        "",
    ]

    for array_name in ARRAY_OUTPUT_ORDER:
        values = sorted(offset_values_by_array.get(array_name, set()))
        if not values:
            values = [0.0]
        values_str = ", ".join(_format_float(v) for v in values)
        lines.append(f"{array_name} = [{values_str}]")

    output_file.write_text("\n".join(lines) + "\n", encoding="utf-8")


def get_alignment_params_for_lut(lut_ver, component):
    """
    Get alignment parameter column names for a given LUT version and component.

    Args:
        lut_ver: LUT version string (e.g., "MRAROT_1", "MRAOFF_1")
        component: "mras" or "mran"

    Returns:
        List of alignment parameter column names
    """
    if "OFF" in lut_ver.upper():
        return [f"{component}_x", f"{component}_y", f"{component}_z"]
    elif "ROT" in lut_ver.upper():
        return [f"{component}_rotx", f"{component}_roty", f"{component}_rotz"]
    else:
        raise ValueError(f"Unknown LUT version format: {lut_ver}")


def find_summary_csvs(outputs_root, lut_ver):
    """
    Find all summary_results.csv files for a given LUT version.
    
    Args:
        outputs_root: Root "outputs" directory
        lut_ver: LUT version subdirectory name
    
    Returns:
        List of Path objects pointing to summary_results.csv files
    """
    lut_dir = Path(outputs_root) / lut_ver
    if not lut_dir.exists():
        raise FileNotFoundError(f"LUT directory not found: {lut_dir}")
    
    csv_files = list(lut_dir.glob("*/bar_*/xbin_*/momentum_*/summary_results.csv"))
    return sorted(csv_files)


def extract_indices_from_path(csv_path):
    """
    Extract bar_id, xbin, momentum from the CSV file path.
    
    Args:
        csv_path: Path object to summary_results.csv
    
    Returns:
        Tuple of (bar_id, xbin, momentum_str)
    """
    parts = csv_path.parts
    bar_id = None
    xbin = None
    momentum = None
    
    for i, part in enumerate(parts):
        if part.startswith("bar_"):
            bar_id = int(part.split("_")[1])
        elif part.startswith("xbin_"):
            xbin = int(part.split("_")[1])
        elif part.startswith("momentum_"):
            momentum = part.split("_", 1)[1]  # keep the full formatted string
    
    if bar_id is None or xbin is None or momentum is None:
        raise ValueError(f"Could not extract indices from path: {csv_path}")
    
    return bar_id, xbin, momentum


def parse_momentum_label(momentum_label):
    """Normalize momentum labels from directory names to display strings.

    Supports both legacy encoding (e.g. 3d5, m0d5) and current decimal format (e.g. 3.5, -0.5).
    """
    value = str(momentum_label)
    if "d" in value or value.startswith("m"):
        return value.replace("m", "-").replace("d", ".")
    return value


def extract_truth_from_path(path_value):
    """
    Extract truth offsets/rotations encoded in tokens.

    Supports patterns such as:
        MRASX4.0, MRANY50.0, MRAS_ROTY0.5, MRAN_ROTZ-1.2
        and family-less tokens that inherit the most recent family, e.g.:
        MRASX4.0_ROTY-0.5_MRANY50.0_ROTY0.5

    Returns:
        dict keyed by CSV column names (e.g., mras_x, mran_roty)
    """
    token_pattern = re.compile(r"((?:MRAS|MRAN))?((?:ROT)?[XYZ])([+-]?\d+(?:\.\d+)?)", re.IGNORECASE)

    truth = {}
    current_family = None
    tokens = [tok for tok in re.split(r"[_/]", str(path_value)) if tok]

    for token in tokens:
        # Allow family to be provided as a standalone token, e.g. MRAS_ROTY0.5.
        if token.upper() in {"MRAS", "MRAN"}:
            current_family = token.lower()
            continue

        match = token_pattern.fullmatch(token)
        if not match:
            continue

        family_raw, key_raw, value_raw = match.groups()
        family = family_raw.lower() if family_raw else current_family
        if family is None:
            # Ignore family-less tokens until a family context is known.
            continue

        current_family = family
        key = key_raw.upper()

        if key.startswith("ROT"):
            axis = key[-1].lower()
            col = f"{family}_rot{axis}"
        else:
            axis = key[-1].lower()
            col = f"{family}_{axis}"

        truth[col] = float(value_raw)

    return truth


def extract_truth_from_var0(df, columns):
    """Use variation 0 as the nominal truth when available."""
    var0_rows = df[df["ivar"] == 0]
    if var0_rows.empty:
        return {}

    var0_row = var0_rows.iloc[0]
    return {column: float(var0_row[column]) for column in columns if column in var0_row.index}


def get_best_row(csv_path):
    """
    Read a CSV and return the row with the maximum 'sep' value.
    
    Args:
        csv_path: Path to summary_results.csv
    
    Returns:
        DataFrame row (Series) with highest sep, or None if CSV is empty
    """
    df = pd.read_csv(csv_path)
    if df.empty:
        return None
    return df.loc[df["sep"].idxmax()]


def plot_aggregate_pairplot(outputs_root, lut_ver, output_dir=None, sample=None, offsets_file=None):
    """
    Aggregate best variations from all bar/xbin/momentum combinations
    and create a pair plot of alignment parameters.
    
    Args:
        outputs_root: Root "outputs" directory
        lut_ver: LUT version subdirectory name (e.g., "MRAROT_1", "MRAOFF_1")
        output_dir: Optional output directory for saving the pair plot
    """
    # Find all CSVs for this LUT version.
    # If a sample filter is provided, only keep CSVs whose directory path contains it.
    csv_files = find_summary_csvs(outputs_root, lut_ver)
    if sample is not None:
        csv_files = [
            csv_path for csv_path in csv_files
            if sample in str(csv_path.parent)
        ]

    if not csv_files:
        print(f"No summary_results.csv files found for {lut_ver}")
        return
    
    print(f"Found {len(csv_files)} CSV files for {lut_ver}")

    sample_truth_values = extract_truth_from_path(sample) if sample else {}
    if sample:
        if sample_truth_values:
            print("Extracted truth values from sample name:")
            for key in sorted(sample_truth_values):
                print(f"  {key} = {sample_truth_values[key]}")
        else:
            print("No truth tokens found in sample name; using var0 values only.")
    
    # Collect best rows from each CSV
    best_rows = []
    offset_values_by_array = {name: set() for name in ARRAY_OUTPUT_ORDER}
    for csv_path in csv_files:
        try:
            bar_id, xbin, momentum = extract_indices_from_path(csv_path)
            df_csv = pd.read_csv(csv_path)
            if df_csv.empty:
                continue

            best_row = df_csv.loc[df_csv["sep"].idxmax()]
            if best_row is not None:
                best_row["bar_id"] = bar_id
                best_row["xbin"] = xbin
                best_row["momentum_label"] = momentum
                best_rows.append(best_row)

            best_minus_var0 = compute_best_minus_var0_offsets(df_csv)
            for col_name, delta in best_minus_var0.items():
                array_name = ARRAY_NAME_MAP[col_name]
                offset_values_by_array[array_name].add(round(delta, 6))
        except Exception as e:
            print(f"  Warning: Could not process {csv_path}: {e}")
    
    if not best_rows:
        print("No valid data found to plot")
        return
    
    # Combine into DataFrame
    df_agg = pd.DataFrame(best_rows)
    
    print(f"Aggregated {len(df_agg)} best variations")

    # Determine output directory
    if output_dir is None:
        output_dir = Path(outputs_root) / lut_ver
    else:
        output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    if offsets_file is None:
        offsets_path = output_dir / f"swif_alignment_offsets_{lut_ver}.txt"
    else:
        offsets_path = Path(offsets_file)
        offsets_path.parent.mkdir(parents=True, exist_ok=True)

    write_swif_offset_arrays(offset_values_by_array, offsets_path, lut_ver, len(csv_files))
    print(f"Wrote SWIF offset arrays to {offsets_path}")

    # Generate one pair plot for MRAS (bar < 24) and one for MRAN (bar > 23)
    for component, label, mask in [
        ("mras", "MRAS", df_agg["bar_id"] < 24),
        ("mran", "MRAN", df_agg["bar_id"] > 23),
    ]:
        df_sub = df_agg[mask]
        if df_sub.empty:
            print(f"  No data for {label} — skipping")
            continue

        align_params = get_alignment_params_for_lut(lut_ver, component)
        available_params = [p for p in align_params if p in df_sub.columns]
        if not available_params:
            print(f"  No {label} alignment parameters found in CSV — skipping")
            continue

        print(f"  {label} ({len(df_sub)} entries), parameters: {available_params}")

        # Nominal truth from var0 (first available CSV for this component),
        # then override with sample-token truth values if provided.
        var0_truth_values = {}
        for csv_path in csv_files:
            try:
                candidate_df = pd.read_csv(csv_path)
                var0_truth_values = extract_truth_from_var0(candidate_df, available_params)
                if var0_truth_values:
                    break
            except Exception:
                continue
        truth_values = dict(var0_truth_values)
        truth_values.update(sample_truth_values)
        if truth_values:
            comp_truth = {k: truth_values[k] for k in available_params if k in truth_values}
            if comp_truth:
                print(f"  {label} truth values in use:")
                for key in comp_truth:
                    print(f"    {key} = {comp_truth[key]}")

        df_sub = df_sub.copy().reset_index(drop=True)
        df_sub["point_id"] = df_sub.index.astype(str)
        palette = sns.color_palette("tab20", n_colors=len(df_sub))
        palette_map = {pid: palette[i] for i, pid in enumerate(df_sub["point_id"]) }

        pair_plot = sns.pairplot(
            df_sub,
            vars=available_params,
            hue="point_id",
            palette=palette_map,
            corner=True,
            diag_kind="hist",
            plot_kws={"s": 40, "alpha": 0.8, "edgecolor": "none"},
            diag_kws={
                "multiple": "stack",
                "stat": "count",
                "common_bins": True,
                "alpha": 0.9,
                "edgecolor": "none",
            },
        )
        if pair_plot._legend is not None:
            pair_plot._legend.remove()

        # Overlay truth lines on the 1D histogram panels.
        comp_truth = {k: truth_values[k] for k in available_params if k in truth_values}
        for idx, param_name in enumerate(available_params):
            if param_name in comp_truth:
                ax_diag = pair_plot.axes[idx][idx]
                if ax_diag is not None:
                    ax_diag.axvline(
                        comp_truth[param_name],
                        color="blue",
                        linestyle="--",
                        linewidth=1.6,
                        alpha=0.95,
                        zorder=8,
                    )

        # Overlay truth points (blue crosses) on the 2D scatter panels.
        if len(comp_truth) >= 2:
            n_params = len(available_params)
            for i in range(1, n_params):
                for j in range(i):
                    x_name = available_params[j]
                    y_name = available_params[i]
                    if x_name in comp_truth and y_name in comp_truth:
                        ax = pair_plot.axes[i][j]
                        if ax is not None:
                            ax.scatter(
                                comp_truth[x_name],
                                comp_truth[y_name],
                                marker="x",
                                c="blue",
                                s=80,
                                linewidths=1.8,
                                zorder=7,
                            )

            # Add a legend entry matching single_summary truth styling.
            legend_ax = pair_plot.axes[1][0] if n_params > 1 else pair_plot.axes[0][0]
            truth_handle = Line2D(
                [0], [0],
                marker="x",
                color="blue",
                linestyle="None",
                markersize=8,
                markeredgewidth=1.8,
                label="Truth",
            )
            legend_ax.legend(handles=[truth_handle], loc="best", fontsize=9, framealpha=0.9)

        pair_plot.fig.suptitle(
            f"{label} Alignment Parameter Pair Plot ({lut_ver})",
            y=1.00
        )

        # Add separate table legend in the upper-right with width matching the rightmost plot column.
        right_col_ax = pair_plot.axes[len(available_params) - 1][len(available_params) - 1]
        right_col_bbox = right_col_ax.get_position()
        table_width = right_col_bbox.width
        table_height = min(0.35, 0.028 * (len(df_sub) + 1) + 0.03)
        table_top = 0.94
        table_bottom = max(0.56, table_top - table_height)
        table_ax = pair_plot.fig.add_axes([right_col_bbox.x0, table_bottom, table_width, table_height])
        table_ax.axis("off")

        cell_text = []
        for _, row in df_sub.iterrows():
            cell_text.append([
                " ",
                str(int(row["bar_id"])),
                str(int(row["xbin"])),
                parse_momentum_label(row["momentum_label"]),
            ])

        table = table_ax.table(
            cellText=cell_text,
            colLabels=["color", "bar", "x bin", "p"],
            cellLoc="center",
            loc="center",
            colWidths=[0.16, 0.20, 0.28, 0.16],
        )
        table.auto_set_font_size(False)
        table.set_fontsize(7)
        table.scale(1.0, 1.05)

        # Color the first column cells to match each point color.
        for row_idx, pid in enumerate(df_sub["point_id"], start=1):
            table[(row_idx, 0)].set_facecolor(palette_map[pid])
            table[(row_idx, 0)].set_text_props(text="")

        output_file = output_dir / f"aggregate_pairplot_{lut_ver}_{component}.png"
        pair_plot.fig.savefig(output_file, dpi=150, bbox_inches="tight")
        print(f"  ✓ Saved {label} pair plot to {output_file}")
        plt.close(pair_plot.fig)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Generate aggregate pair plots for alignment parameters"
    )
    parser.add_argument(
        "outputs_root",
        help="Path to outputs root directory"
    )
    parser.add_argument(
        "lut_ver",
        help="LUT version subdirectory name (e.g., MRAROT_1, MRAOFF_1)"
    )
    parser.add_argument(
        "-o", "--output",
        default=None,
        help="Output directory for pair plot (default: outputs_root/lut_ver)"
    )
    parser.add_argument(
        "--sample",
        default=None,
        help=(
            "Optional sample name used to parse truth tokens "
            "(e.g. hd_root_bothp_flat_MRASX4.0_MRAN_ROTY50.0)"
        ),
    )
    parser.add_argument(
        "--offsets-file",
        default=None,
        help=(
            "Output text file for swif_LUT_alignment.py array assignments "
            "(default: output_dir/swif_alignment_offsets_<lut_ver>.txt)"
        ),
    )
    
    args = parser.parse_args()
    plot_aggregate_pairplot(
        args.outputs_root,
        args.lut_ver,
        args.output,
        args.sample,
        args.offsets_file,
    )
