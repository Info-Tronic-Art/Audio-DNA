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
from tier1_exceptions import T_PARAM

FIXTURES_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "fixtures")
)
TEST_IMAGE = os.path.join(FIXTURES_DIR, "test_card.png")


def psnr_between(path1, path2):
    img1, img2 = cv2.imread(path1), cv2.imread(path2)
    if img1 is None or img2 is None:
        return float("inf")
    return compute_psnr(img1, img2)


# Sources known to respond to RMS
AUDIO_RESPONSIVE_SOURCES = [
    "mandelbrot", "julia_set", "burning_ship", "audio_waveform",
    "mandelbulb", "kifs",
]

# Sources that read u_beatPhase (tier1-diag E6: PSNR 2.3 / 19.9 / 36.7; none of AUDIO_RESPONSIVE_SOURCES reads it)
BEAT_PHASE_SOURCES = ["twisted_torus", "fermat_spiral", "text_animator"]

# Sources that read u_bass = bandEnergies[1] (ProceduralSource.cpp; tier1-diag: 15.5 / 20.9 / 6.7 / 21.4 / 29.6)
BASS_SOURCES = ["audio_waveform", "perlin_noise", "torus_hole", "geometric_tunnel", "crystal_cavern"]

# Audio feature pairs: (name, zero_state, active_state, sources)
FEATURE_PAIRS = [
    ("RMS", {"rms": 0.0}, {"rms": 0.9}, AUDIO_RESPONSIVE_SOURCES),
    ("Beat Phase", {"beatPhase": 0.0}, {"beatPhase": 0.5}, BEAT_PHASE_SOURCES),
    ("Bass", {"bandEnergies": [0, 0, 0, 0, 0, 0, 0]}, {"bandEnergies": [0, 1, 0, 0, 0, 0, 0]}, BASS_SOURCES),
    ("Brilliance", {"bandEnergies": [0, 0, 0, 0, 0, 0, 0]}, {"bandEnergies": [0, 0, 0, 0, 0, 0, 1]},
     AUDIO_RESPONSIVE_SOURCES),
    ("Spectral Centroid", {"spectralCentroid": 200.0}, {"spectralCentroid": 8000.0}, AUDIO_RESPONSIVE_SOURCES),
    ("Onset", {"onsetDetected": False, "onsetStrength": 0.0}, {"onsetDetected": True, "onsetStrength": 0.9},
     AUDIO_RESPONSIVE_SOURCES),
]


class TestAudioFeaturesAffectSources:
    """Audio features should visibly change source output."""

    @pytest.mark.parametrize("feature_name,zero,active,sources", FEATURE_PAIRS[:3],
                             ids=[f[0] for f in FEATURE_PAIRS[:3]])
    def test_rms_bass_beat_affect_sources(self, app, tmp_path, feature_name, zero, active, sources):
        """RMS, bass, and beat phase should affect most sources that use u_rms/u_bass/u_beatPhase."""
        changed_count = 0
        tested_count = 0

        for src_id in sources:
            tested_count += 1

            # Zero features
            app.load_source(src_id)
            app.inject_features(zero)
            zero_path = str(tmp_path / f"{src_id}_{feature_name}_zero.png")
            app.render_frame(zero_path, time_val=T_PARAM, width=256, height=256)

            # Active features
            app.load_source(src_id)
            app.inject_features(active)
            active_path = str(tmp_path / f"{src_id}_{feature_name}_active.png")
            app.render_frame(active_path, time_val=T_PARAM, width=256, height=256)

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

        # Of the non-source shaders only density_wave and structural_morph read u_rms, and Structural Morph's RMS
        # term is gated by structuralState (tier1-diag E: Density Wave PSNR 9.9)
        effects_to_test = ["Density Wave"]
        changed_count = 0

        for fx_name in effects_to_test:
            app.reset()
            app.load_image(TEST_IMAGE)
            app.set_effect(fx_name, enabled=True)   # defaults ("intensity" never existed: set_effect ignored it)

            # Zero RMS
            app.inject_features({"rms": 0.0})
            zero_path = str(tmp_path / f"fx_{fx_name}_rms0.png".replace(" ", "_"))
            app.render_frame(zero_path, time_val=T_PARAM, width=256, height=256)

            # High RMS
            app.inject_features({"rms": 0.9})
            high_path = str(tmp_path / f"fx_{fx_name}_rms9.png".replace(" ", "_"))
            app.render_frame(high_path, time_val=T_PARAM, width=256, height=256)

            psnr = psnr_between(zero_path, high_path)
            if psnr < 50.0:
                changed_count += 1

        # At least some effects should respond
        assert changed_count >= 1, "No effects responded to RMS injection"
