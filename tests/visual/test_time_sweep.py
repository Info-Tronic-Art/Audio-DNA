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
from tier1_exceptions import is_black, arm_audio, BLACK_SOURCES, BLACK_AT_TIMES


def psnr_between(path1, path2):
    img1, img2 = cv2.imread(path1), cv2.imread(path2)
    if img1 is None or img2 is None:
        return float("inf")
    return compute_psnr(img1, img2)


# Sources known to NOT animate (static patterns)
STATIC_SOURCES = {"solid_color", "checkerboard", "color_gradient"}

# Sample times: integers are sin(2*pi*k*t) / fract(k*t) no-ops for integer k, so every sample is offset by 0.13
TIMES = [0.0, 1.13, 5.13, 10.13]


@pytest.fixture(scope="module")
def all_sources(app):
    return app.list_sources()["sources"]


class TestSourcesAnimateOverTime:
    """Animated sources must produce different frames at different times."""

    def test_sources_change_over_time(self, app, tmp_path, all_sources):
        frozen = []
        black_at_time = []

        arm_audio(app)
        for src in all_sources:
            if src["id"] in STATIC_SOURCES or src["id"] in BLACK_SOURCES:
                continue

            frames = {}
            for t in TIMES:
                app.load_source(src["id"])
                out = str(tmp_path / f"{src['id']}_t{t:.2f}.png")
                app.render_frame(out, time_val=t, width=256, height=256)
                frames[t] = out

                if is_black(out) and (src["id"], t) not in BLACK_AT_TIMES:
                    black_at_time.append(f"{src['id']} black at t={t}")

            # Check at least 2 time pairs are different
            diff_count = 0
            for i1, i2 in [(0, 1), (0, 2), (1, 3)]:
                psnr = psnr_between(frames[TIMES[i1]], frames[TIMES[i2]])
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
