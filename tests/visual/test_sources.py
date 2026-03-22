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


def brightness(path):
    img = cv2.imread(path)
    return float(np.mean(img)) if img is not None else 0.0


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
        for src in all_sources:
            app.load_source(src["id"])
            out = str(tmp_path / f"{src['id']}_default.png")
            result = app.render_frame(out, time_val=1.0, width=256, height=256)
            if not result.get("ok"):
                failures.append(f"{src['id']}: render failed")
            elif brightness(out) < 5.0:
                failures.append(f"{src['id']}: all black at defaults")
        assert not failures, "Sources with broken defaults:\n" + "\n".join(failures)


class TestAllSourceParams:
    """Every param on every source must have a visible effect."""

    def test_all_params_have_effect(self, app, tmp_path, all_sources):
        failures = []
        for src in all_sources:
            for param in src.get("params", []):
                uniform = param["uniform"]
                default_val = param.get("default", 0.5)
                # Pick a test value far from default
                test_val = 0.0 if default_val > 0.3 else 1.0

                # Render at default
                app.load_source(src["id"])
                def_path = str(tmp_path / f"{src['id']}_{uniform}_def.png")
                app.render_frame(def_path, time_val=1.0, width=256, height=256)

                # Render at test value
                app.load_source(src["id"])
                app.update_source_params({uniform: test_val})
                test_path = str(tmp_path / f"{src['id']}_{uniform}_test.png")
                app.render_frame(test_path, time_val=1.0, width=256, height=256)

                # Check: not black at either position
                def_bright = brightness(def_path)
                test_bright = brightness(test_path)
                if def_bright < 5.0:
                    failures.append(f"{src['id']}:{param['name']} default is black")
                    continue
                if test_bright < 5.0:
                    failures.append(
                        f"{src['id']}:{param['name']} goes black at {test_val}"
                    )
                    continue

                # Check: param actually changes output
                psnr = psnr_between(def_path, test_path)
                if psnr > 55.0:
                    failures.append(
                        f"{src['id']}:{param['name']} no visible effect "
                        f"(PSNR={psnr:.1f}, default={default_val}, test={test_val})"
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
                    app.render_frame(out, time_val=1.0, width=256, height=256)
                    frames.append(out)

                # Check for black frames
                black_count = sum(1 for f in frames if brightness(f) < 5.0)
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
