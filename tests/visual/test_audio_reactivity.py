"""
Audio Reactivity Verification — Tests that injected audio features change visual output.

Injects synthetic audio features via /api/inject_features and verifies
the rendered output changes compared to zero-feature baseline.
"""

import os
import pytest
import cv2
import numpy as np
import sys
sys.path.insert(0, os.path.dirname(__file__))
from vision_check import compute_psnr

FIXTURES_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "fixtures")
)
TEST_IMAGE = os.path.join(FIXTURES_DIR, "test_card.png")


def psnr_between(path1, path2):
    img1, img2 = cv2.imread(path1), cv2.imread(path2)
    if img1 is None or img2 is None:
        return float("inf")
    return compute_psnr(img1, img2)


# Audio feature pairs: (name, zero_state, active_state)
FEATURE_PAIRS = [
    ("RMS", {"rms": 0.0}, {"rms": 0.9}),
    ("Beat Phase", {"beatPhase": 0.0}, {"beatPhase": 0.5}),
    ("Bass", {"bandEnergies": [0, 0, 0, 0, 0, 0, 0]}, {"bandEnergies": [1, 0, 0, 0, 0, 0, 0]}),
    ("Brilliance", {"bandEnergies": [0, 0, 0, 0, 0, 0, 0]}, {"bandEnergies": [0, 0, 0, 0, 0, 0, 1]}),
    ("Spectral Centroid", {"spectralCentroid": 200.0}, {"spectralCentroid": 8000.0}),
    ("Onset", {"onsetDetected": False, "onsetStrength": 0.0}, {"onsetDetected": True, "onsetStrength": 0.9}),
]

# Sources known to respond to audio
AUDIO_RESPONSIVE_SOURCES = [
    "mandelbrot", "julia_set", "burning_ship", "audio_waveform",
    "mandelbulb", "kifs",
]


class TestAudioFeaturesAffectSources:
    """Audio features should visibly change source output."""

    @pytest.mark.parametrize("feature_name,zero,active", FEATURE_PAIRS[:3],
                             ids=[f[0] for f in FEATURE_PAIRS[:3]])
    def test_rms_bass_beat_affect_sources(self, app, tmp_path, feature_name, zero, active):
        """RMS, bass, and beat phase should affect most sources that use u_rms/u_beatPhase."""
        changed_count = 0
        tested_count = 0

        for src_id in AUDIO_RESPONSIVE_SOURCES:
            tested_count += 1

            # Zero features
            app.load_source(src_id)
            app.inject_features(zero)
            zero_path = str(tmp_path / f"{src_id}_{feature_name}_zero.png")
            app.render_frame(zero_path, time_val=1.0, width=256, height=256)

            # Active features
            app.load_source(src_id)
            app.inject_features(active)
            active_path = str(tmp_path / f"{src_id}_{feature_name}_active.png")
            app.render_frame(active_path, time_val=1.0, width=256, height=256)

            psnr = psnr_between(zero_path, active_path)
            if psnr < 50.0:
                changed_count += 1

        assert changed_count >= 2, (
            f"Feature '{feature_name}' only affected {changed_count}/{tested_count} sources"
        )


class TestAudioFeaturesAffectEffects:
    """Audio features should change effect output when effects use audio uniforms."""

    def test_rms_affects_effects(self, app, tmp_path):
        """Effects that use u_rms should respond to RMS changes."""
        if not os.path.exists(TEST_IMAGE):
            pytest.skip("Test image not found")

        # Effects known to respond to RMS (most use it for brightness)
        effects_to_test = ["Ripple", "Hue Shift", "Chromatic Aberration"]
        changed_count = 0

        for fx_name in effects_to_test:
            app.reset()
            app.load_image(TEST_IMAGE)
            app.set_effect(fx_name, enabled=True, params={"intensity": 0.5})

            # Zero RMS
            app.inject_features({"rms": 0.0})
            zero_path = str(tmp_path / f"fx_{fx_name}_rms0.png".replace(" ", "_"))
            app.render_frame(zero_path, time_val=1.0, width=256, height=256)

            # High RMS
            app.inject_features({"rms": 0.9})
            high_path = str(tmp_path / f"fx_{fx_name}_rms9.png".replace(" ", "_"))
            app.render_frame(high_path, time_val=1.0, width=256, height=256)

            psnr = psnr_between(zero_path, high_path)
            if psnr < 50.0:
                changed_count += 1

        # At least some effects should respond
        assert changed_count >= 1, "No effects responded to RMS injection"
