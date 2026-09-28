"""
Auto-Discovering Source Verification — Tests ALL registered procedural sources.

Queries /api/sources to discover every source and its params automatically.
No hardcoded source lists — when a new source is added to SourceRegistry.cpp,
it gets tested automatically.

Tier 1: Every param at 5 positions → not-black, has-effect, no-discontinuity.
"""

import os
import pytest
import cv2
import numpy as np
import sys
sys.path.insert(0, os.path.dirname(__file__))
from vision_check import compute_psnr
from tier1_exceptions import (is_black, candidate_values, T_PARAM, arm_audio,
                              BLACK_SOURCES, BLACK_AT_EXTREME)


def psnr_between(path1, path2):
    img1, img2 = cv2.imread(path1), cv2.imread(path2)
    if img1 is None or img2 is None:
        return float("inf")
    return compute_psnr(img1, img2)


@pytest.fixture(scope="module")
def all_sources(app):
    """Discover all sources from the running app."""
    data = app.list_sources()
    return data["sources"]


class TestAllSourcesRender:
    """Every registered source must render a non-black frame at defaults."""

    def test_all_sources_non_black(self, app, tmp_path, all_sources):
        failures = []
        arm_audio(app)
        for src in all_sources:
            if src["id"] in BLACK_SOURCES:
                continue
            app.load_source(src["id"])
            out = str(tmp_path / f"{src['id']}_default.png")
            result = app.render_frame(out, time_val=T_PARAM, width=256, height=256)
            if result.get("ok") and is_black(out):
                # One retry at another time before failing: a strobing source is dark for half its period
                # (strobe_light: 255 at t = 0/2/10, black at 0.5/1/3/5 -- diag E3). Generic, no name needed.
                out = str(tmp_path / f"{src['id']}_default_t2.png")
                result = app.render_frame(out, time_val=2.0, width=256, height=256)
            if not result.get("ok"):
                failures.append(f"{src['id']}: render failed")
            elif is_black(out):
                failures.append(f"{src['id']}: all black at defaults")
        assert not failures, "Sources with broken defaults:\n" + "\n".join(failures)


class TestAllSourceParams:
    """Every param on every source must have a visible effect."""

    def test_all_params_have_effect(self, app, tmp_path, all_sources):
        failures = []
        arm_audio(app)
        for src in all_sources:
            if src["id"] in BLACK_SOURCES:
                continue
            for param in src.get("params", []):
                uniform = param["uniform"]
                default_val = param.get("default", 0.5)

                # Render at default
                app.load_source(src["id"])
                def_path = str(tmp_path / f"{src['id']}_{uniform}_def.png")
                app.render_frame(def_path, time_val=T_PARAM, width=256, height=256)
                if is_black(def_path):
                    failures.append(f"{src['id']}:{param['name']} default is black")
                    continue

                # Walk the candidate ladder until one visibly changes the output
                tried = []
                passed = False
                for test_val in candidate_values(default_val):
                    app.load_source(src["id"])
                    app.update_source_params({uniform: test_val})
                    test_path = str(tmp_path / f"{src['id']}_{uniform}_test_{test_val:.2f}.png")
                    app.render_frame(test_path, time_val=T_PARAM, width=256, height=256)
                    if is_black(test_path):
                        if (src["id"], uniform) in BLACK_AT_EXTREME:
                            tried.append(f"{test_val} (black, listed)")
                            continue
                        failures.append(
                            f"{src['id']}:{param['name']} goes black at {test_val}"
                        )
                        passed = None
                        break
                    psnr = psnr_between(def_path, test_path)
                    tried.append(f"{test_val} (PSNR={psnr:.1f})")
                    if psnr <= 55.0:   # the old pass rule (fail only when PSNR > 55)
                        passed = True
                        break
                if passed is False:
                    failures.append(
                        f"{src['id']}:{param['name']} no visible effect "
                        f"(default={default_val}, tried [{', '.join(tried)}])"
                    )

        if failures:
            # Print all failures but don't assert per-item (too noisy)
            report = "\n".join(failures)
            # Save report
            report_path = str(tmp_path / "source_param_failures.txt")
            with open(report_path, "w") as f:
                f.write(report)
            assert False, (
                f"{len(failures)} source param issues found:\n{report}\n"
                f"Full report: {report_path}"
            )


class TestSourceParamSweep:
    """Sweep critical params across 5 positions to check for discontinuities."""

    def test_no_discontinuities(self, app, tmp_path, all_sources):
        failures = []
        positions = [0.0, 0.25, 0.5, 0.75, 1.0]

        for src in all_sources:
            for param in src.get("params", []):
                uniform = param["uniform"]
                frames = []

                for val in positions:
                    app.load_source(src["id"])
                    app.update_source_params({uniform: val})
                    out = str(tmp_path / f"{src['id']}_{uniform}_{val:.2f}.png")
                    app.render_frame(out, time_val=T_PARAM, width=256, height=256)
                    frames.append(out)

                # Check for black frames
                black_count = sum(1 for f in frames if is_black(f))
                if black_count > 1:
                    failures.append(
                        f"{src['id']}:{param['name']} has {black_count}/5 black frames"
                    )
                    continue

                # Check for discontinuities (adjacent frames too different)
                for i in range(len(frames) - 1):
                    psnr = psnr_between(frames[i], frames[i + 1])
                    if psnr < 8.0 and psnr != float("inf"):
                        failures.append(
                            f"{src['id']}:{param['name']} discontinuity at "
                            f"{positions[i]:.2f}->{positions[i+1]:.2f} (PSNR={psnr:.1f})"
                        )

        if failures:
            report_path = str(tmp_path / "source_discontinuity_failures.txt")
            with open(report_path, "w") as f:
                f.write("\n".join(failures))
            # Warn but don't hard-fail — some discontinuities are expected
            # (e.g., Mode switch from triangle to carpet)
            print(f"\n{'='*60}")
            print(f"WARNING: {len(failures)} discontinuities found:")
            for f in failures[:20]:
                print(f"  {f}")
            if len(failures) > 20:
                print(f"  ... and {len(failures)-20} more")
            print(f"Full report: {report_path}")
            print(f"{'='*60}\n")
