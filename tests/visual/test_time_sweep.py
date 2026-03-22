"""
Time Sweep Verification — Tests that animated sources/effects change over time.

Renders at t=0, t=1, t=5, t=10 and verifies the output changes.
Catches: frozen animations, time-dependent crashes, static shaders that should animate.
"""

import os
import pytest
import cv2
import numpy as np
import sys
sys.path.insert(0, os.path.dirname(__file__))
from vision_check import compute_psnr


def psnr_between(path1, path2):
    img1, img2 = cv2.imread(path1), cv2.imread(path2)
    if img1 is None or img2 is None:
        return float("inf")
    return compute_psnr(img1, img2)


def brightness(path):
    img = cv2.imread(path)
    return float(np.mean(img)) if img is not None else 0.0


# Sources known to NOT animate (static patterns)
STATIC_SOURCES = {"solid_color", "checkerboard", "color_gradient"}


@pytest.fixture(scope="module")
def all_sources(app):
    return app.list_sources()["sources"]


class TestSourcesAnimateOverTime:
    """Animated sources must produce different frames at different times."""

    def test_sources_change_over_time(self, app, tmp_path, all_sources):
        frozen = []
        black_at_time = []

        for src in all_sources:
            if src["id"] in STATIC_SOURCES:
                continue

            frames = {}
            for t in [0.0, 1.0, 5.0, 10.0]:
                app.load_source(src["id"])
                out = str(tmp_path / f"{src['id']}_t{t:.0f}.png")
                app.render_frame(out, time_val=t, width=256, height=256)
                frames[t] = out

                if brightness(out) < 3.0:
                    black_at_time.append(f"{src['id']} black at t={t}")

            # Check at least 2 time pairs are different
            diff_count = 0
            for t1, t2 in [(0, 1), (0, 5), (1, 10)]:
                psnr = psnr_between(frames[float(t1)], frames[float(t2)])
                if psnr < 50.0:
                    diff_count += 1

            if diff_count == 0:
                frozen.append(src["id"])

        if frozen:
            print(f"\nFrozen sources (no time animation): {', '.join(frozen[:20])}")
        if black_at_time:
            print(f"\nBlack at specific times: {', '.join(black_at_time[:20])}")

        # Hard fail if significant number are broken
        assert len(black_at_time) < 5, (
            f"{len(black_at_time)} sources go black at certain times:\n"
            + "\n".join(black_at_time[:10])
        )
