#!/usr/bin/env python3
"""
Plot alignment summary for a single summary_results.csv file in a hierarchical output directory.
Selects alignment parameters based on LUT version (OFF/ROT) and bar ID (MRAS/MRAN).
Saves 4-plot alignment_summary.png in the same directory as the CSV.
"""

import argparse
import re
from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D


def get_alignment_params(lut_ver, bar_id):
    """
    Select alignment parameters based on LUT version and bar ID.
    
    Args:
        lut_ver: LUT version string (e.g., "MRAROT_1", "MRAOFF_1")
        bar_id: Bar ID (< 24 = MRAS, > 23 = MRAN)
    
    Returns:
        List of 3 alignment parameter names to plot
    """
    # Choose MRAS or MRAN based on bar_id
    if bar_id < 24:
        prefix = "mras"
    else:
        prefix = "mran"
    
    # Choose positional offsets or rotations based on LUT_VER
    if "OFF" in lut_ver.upper():
        # Use positional offsets: x, y, z
        return [f"{prefix}_x", f"{prefix}_y", f"{prefix}_z"]
    elif "ROT" in lut_ver.upper():
        # Use rotational parameters: rotx, roty, rotz
        return [f"{prefix}_rotx", f"{prefix}_roty", f"{prefix}_rotz"]
    else:
        raise ValueError(f"Unknown LUT version format: {lut_ver}")


def extract_truth_from_path(path_value):
    """
    Extract truth offsets/rotations encoded in path tokens.

    Supports patterns such as:
        MRASX4.0, MRANY50.0, MRAS_ROTY0.5, MRAN_ROTZ-1.2

    Returns:
        dict keyed by CSV column names (e.g., mras_x, mran_roty)
    """
    pattern = re.compile(r"(MRAS|MRAN)_?((?:ROT)?[XYZ])([+-]?\d+(?:\.\d+)?)", re.IGNORECASE)
    matches = pattern.findall(str(path_value))

    truth = {}
    for family_raw, key_raw, value_raw in matches:
        family = family_raw.lower()  # mras/mran
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


def plot_single_summary(csv_file, output_dir=None):
    """
    Create alignment summary plots for a single CSV file.
    
    Args:
        csv_file: Path to summary_results.csv
        output_dir: Optional output directory. If None, uses parent of csv_file.
    """
    csv_path = Path(csv_file)
    if not csv_path.exists():
        raise FileNotFoundError(f"CSV file not found: {csv_path}")
    
    if output_dir is None:
        output_dir = csv_path.parent
    else:
        output_dir = Path(output_dir)
    
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Read CSV — keep full dataset for axis ranges
    df = pd.read_csv(csv_path)
    df_full = df.copy()
    
    # Filter to top 10% of sep values for colouring the scatter points
    percentile_90 = df["sep"].quantile(0.9)
    df = df[df["sep"] >= percentile_90]
    
    # Extract LUT version and bar ID from the output directory path
    # Expected: outputs/lut_ver/tree/bar_N/xbin_N/momentum_N/summary_results.csv
    parts = csv_path.parts
    if "bar_" in str(csv_path.parent):
        # Find bar_N in path
        for part in parts:
            if part.startswith("bar_"):
                bar_id = int(part.split("_")[1])
                break
        else:
            raise ValueError("Could not extract bar ID from path")
        
        # Find LUT version (should be 1-2 levels up from outputs)
        try:
            outputs_idx = parts.index("outputs")
            lut_ver = parts[outputs_idx + 1]
        except (ValueError, IndexError):
            raise ValueError("Could not extract LUT version from path")
    else:
        raise ValueError("Path does not match expected output directory structure")
    
    # Get alignment parameters to use
    align_params = get_alignment_params(lut_ver, bar_id)
    truth_values = extract_truth_from_var0(df_full, align_params)
    path_truth_values = extract_truth_from_path(csv_path)
    truth_values.update(path_truth_values)

    if truth_values:
        print("Truth values used for plot guides:")
        for key in sorted(truth_values):
            print(f"  {key} = {truth_values[key]}")
    
    # Create 4-subplot figure
    fig, axes = plt.subplots(2, 2, figsize=(14, 10), constrained_layout=True)
    axes = axes.flatten()
    
    # Plot 1-3: Alignment parameter space colored by separation
    pairs = [
        (align_params[0], align_params[1]),
        (align_params[0], align_params[2]),
        (align_params[1], align_params[2]),
    ]
    
    # Find the row with the best (maximum) separation across all variations
    best_row = df_full.loc[df_full["sep"].idxmax()]

    # Sort ascending so highest-sep points are drawn last (on top)
    df_plot = df.sort_values("sep", ascending=True)
    legend_handles = [
        Line2D(
            [0], [0],
            marker="x",
            color="blue",
            linestyle="None",
            markersize=8,
            markeredgewidth=1.8,
            label="Truth",
        ),
        Line2D(
            [0], [0],
            marker="o",
            markerfacecolor="none",
            markeredgecolor="red",
            linestyle="None",
            markersize=8,
            markeredgewidth=1.8,
            label="Optimal",
        ),
    ]

    sc = None
    for ax, (x, y) in zip(axes[:3], pairs):
        sc = ax.scatter(
            df_plot[x],
            df_plot[y],
            c=df_plot["sep"],
            cmap="viridis",
            s=15,
            alpha=0.7
        )
        # Set axis limits to full parameter range across all variations
        margin_x = (df_full[x].max() - df_full[x].min()) * 0.05 or 0.1
        margin_y = (df_full[y].max() - df_full[y].min()) * 0.05 or 0.1
        ax.set_xlim(df_full[x].min() - margin_x, df_full[x].max() + margin_x)
        ax.set_ylim(df_full[y].min() - margin_y, df_full[y].max() + margin_y)
        ax.scatter(
            best_row[x],
            best_row[y],
            edgecolors="red",
            facecolors="none",
            s=120,
            linewidths=1.8,
            zorder=5,
        )
        # Draw truth marker if both coordinates are available from path tokens.
        if x in truth_values and y in truth_values:
            ax.scatter(
                truth_values[x],
                truth_values[y],
                marker="x",
                c="blue",
                s=80,
                linewidths=1.8,
                zorder=6,
            )
        ax.legend(handles=legend_handles, loc="best", fontsize=9, framealpha=0.9)
        ax.set_xlabel(x)
        ax.set_ylabel(y)
    
    # Plot 4: Separation vs variation number (sorted by separation)
    ax = axes[3]
    df_sorted = df.sort_values("sep")
    ivar_sorted = df_sorted["ivar"]
    separations_sorted = df_sorted["sep"].values
    
    ax.plot(
        range(len(df_sorted)), separations_sorted, 'o-',
        linewidth=2, markersize=6, color='tab:blue', label='Separation'
    )
    ax.set_xlabel("Variations (ordered by separation)")
    ax.set_ylabel("Separation (sigma)", color='tab:blue')
    ax.tick_params(axis='y', labelcolor='tab:blue')
    ax.set_xticks(range(len(df_sorted)))
    ax.set_xticklabels(ivar_sorted.astype(int), rotation=45, ha='right')
    ax.grid(True, alpha=0.3)
    
    # Shared colorbar for first 3 plots
    fig.colorbar(sc, ax=axes[:3], label="separation (sigma)")
    
    # Save figure
    output_file = output_dir / "alignment_summary.png"
    fig.savefig(output_file, dpi=150, bbox_inches="tight")
    print(f"✓ Saved alignment summary to {output_file}")
    
    plt.close(fig)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Plot alignment summary for a single summary_results.csv"
    )
    parser.add_argument(
        "csv_file",
        help="Path to summary_results.csv file"
    )
    parser.add_argument(
        "-o", "--output",
        default=None,
        help="Output directory (default: parent directory of CSV file)"
    )
    
    args = parser.parse_args()
    plot_single_summary(args.csv_file, args.output)
