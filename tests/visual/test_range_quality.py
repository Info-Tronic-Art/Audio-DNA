"""
Tier 2: Range Quality Analysis

For each source parameter, renders at 11 positions (0.0 to 1.0 in steps of 0.1)
and produces a quality report:
- Brightness curve (mean pixel value at each position)
- Variety score (average PSNR between adjacent steps)
- Useful range (fraction of positions that are non-black AND visually distinct)
- Discontinuity check (any adjacent pair with PSNR < 8 = jump-cut)

Generates CSV reports in tests/visual/reports/

Usage:
    AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_range_quality.py -v
    AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_range_quality.py -v -k "mandelbrot"
"""

import os
import csv
import pytest
import cv2
import numpy as np
import sys
sys.path.insert(0, os.path.dirname(__file__))
from vision_check import compute_psnr

REPORTS_DIR = os.path.join(os.path.dirname(__file__), "reports")

# All source+param combos to test. Format: (source_id, uniform, name)
# Import from test_fractals to stay in sync
from test_fractals import ALL_PARAM_TESTS


def brightness(path):
    img = cv2.imread(path)
    if img is None:
        return 0.0
    return float(np.mean(img))


def render_sweep(app, tmp_path, source_id, param_uniform, steps=11):
    """Render a source at multiple parameter positions, return list of (value, path) tuples."""
    results = []
    for i in range(steps):
        val = i / (steps - 1)
        app.load_source(source_id)
        app.update_source_params({param_uniform: val})
        out = str(tmp_path / f"{source_id}_{param_uniform}_{i:02d}.png")
        app.render_frame(out, time_val=1.0, width=256, height=256)
        results.append((val, out))
    return results


def analyze_sweep(results):
    """Analyze a parameter sweep for quality metrics."""
    brightnesses = []
    psnrs = []

    for val, path in results:
        brightnesses.append(brightness(path))

    for i in range(len(results) - 1):
        img1 = cv2.imread(results[i][1])
        img2 = cv2.imread(results[i + 1][1])
        if img1 is not None and img2 is not None:
            psnrs.append(compute_psnr(img1, img2))
        else:
            psnrs.append(float("inf"))

    # Metrics
    non_black = sum(1 for b in brightnesses if b > 5.0)
    useful_range = non_black / len(brightnesses)

    distinct_pairs = sum(1 for p in psnrs if p < 55.0)
    variety_score = np.mean([p for p in psnrs if p != float("inf")]) if psnrs else 0.0

    has_discontinuity = any(p < 8.0 for p in psnrs if p != float("inf"))
    has_dead_zone = any(b < 5.0 for b in brightnesses)

    return {
        "brightnesses": brightnesses,
        "psnrs": psnrs,
        "useful_range": useful_range,
        "variety_score": variety_score,
        "has_discontinuity": has_discontinuity,
        "has_dead_zone": has_dead_zone,
        "distinct_pairs": distinct_pairs,
        "total_pairs": len(psnrs),
    }


def save_report(source_id, param_name, param_uniform, results, metrics):
    """Save a CSV report for one parameter sweep."""
    os.makedirs(REPORTS_DIR, exist_ok=True)
    path = os.path.join(REPORTS_DIR, f"{source_id}_{param_uniform}.csv")
    with open(path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["source", "param", "uniform", "useful_range", "variety_score",
                     "has_discontinuity", "has_dead_zone"])
        w.writerow([source_id, param_name, param_uniform,
                     f"{metrics['useful_range']:.2f}",
                     f"{metrics['variety_score']:.1f}",
                     metrics["has_discontinuity"],
                     metrics["has_dead_zone"]])
        w.writerow([])
        w.writerow(["position", "brightness", "psnr_to_next"])
        for i, (val, _path) in enumerate(results):
            psnr = metrics["psnrs"][i] if i < len(metrics["psnrs"]) else ""
            w.writerow([f"{val:.2f}", f"{metrics['brightnesses'][i]:.1f}",
                        f"{psnr:.1f}" if isinstance(psnr, float) and psnr != float("inf") else "inf"])


# Deduplicate: one test per (source, param) combo
UNIQUE_PARAMS = list({(t[0], t[1], t[4]): t for t in ALL_PARAM_TESTS}.values())


class TestRangeQuality:
    """Sweep every parameter and verify quality metrics."""

    @pytest.mark.parametrize(
        "source_id,param_uniform,default_val,test_val,param_name",
        UNIQUE_PARAMS,
        ids=[f"{t[0]}:{t[4]}" for t in UNIQUE_PARAMS],
    )
    def test_param_range_quality(self, app, tmp_path,
                                  source_id, param_uniform, default_val, test_val, param_name):
        """Each parameter must have >70% useful range, no discontinuities, no dead zones."""
        results = render_sweep(app, tmp_path, source_id, param_uniform, steps=11)
        metrics = analyze_sweep(results)
        save_report(source_id, param_name, param_uniform, results, metrics)

        # Quality gates
        assert metrics["useful_range"] >= 0.7, (
            f"{source_id}:{param_name} only {metrics['useful_range']*100:.0f}% useful "
            f"(need 70%). {sum(1 for b in metrics['brightnesses'] if b < 5)} black frames."
        )
        assert not metrics["has_discontinuity"], (
            f"{source_id}:{param_name} has discontinuity (PSNR < 8 between adjacent steps)"
        )
        assert metrics["distinct_pairs"] >= 3, (
            f"{source_id}:{param_name} only {metrics['distinct_pairs']}/{metrics['total_pairs']} "
            f"pairs are distinct. Control may not be working."
        )
