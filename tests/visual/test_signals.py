"""
Signal Routing Verification — Tests the complete signal→route→parameter→shader pipeline.

Tests at two levels:
1. API-level: verify signal list, route creation, feature injection → parameter change
2. Visual-level: verify that connecting a signal to a parameter changes the rendered output

PREREQUISITES:
- The TestServer needs signal/routing endpoints added (see SIGNAL_TEST_SPEC.md)
- Until those endpoints exist, these tests serve as the specification for what
  to verify and the framework is ready to activate

API ENDPOINTS NEEDED (add to TestServer.h/cpp):
  GET  /api/signals           → list all signals with cached values
  POST /api/add_route         → create a signal→parameter route
  POST /api/remove_route      → remove a route
  GET  /api/routes            → list all active routes
  POST /api/set_macro         → set a macro knob value or source
  GET  /api/macros            → list all macros
"""

import os
import time
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


def brightness(path):
    img = cv2.imread(path)
    return float(np.mean(img)) if img is not None else 0.0


def psnr_between(path1, path2):
    img1, img2 = cv2.imread(path1), cv2.imread(path2)
    if img1 is None or img2 is None:
        return float("inf")
    return compute_psnr(img1, img2)


# ============================================================
# SIGNAL TESTS — Currently runnable via feature injection
# These test the END-TO-END path: inject features → source/effect changes
# ============================================================

class TestFeatureInjectionChangesOutput:
    """Injecting different audio features should produce visually different frames.

    This tests the full pipeline: FeatureBus → (SignalRegistry → RoutingEngine →
    ParameterWriter) → shader uniform → visual change.

    Even without explicit signal routes, sources use u_rms, u_beatPhase etc.
    directly as shader uniforms, so feature injection should change them.
    """

    FEATURE_PAIRS = [
        ("RMS zero vs high",
         {"rms": 0.0}, {"rms": 0.9}),
        ("Beat phase start vs mid",
         {"beatPhase": 0.0}, {"beatPhase": 0.5}),
        ("Bass vs treble",
         {"bandEnergies": [1, 0, 0, 0, 0, 0, 0]},
         {"bandEnergies": [0, 0, 0, 0, 0, 0, 1]}),
        ("Low vs high spectral centroid",
         {"spectralCentroid": 200.0}, {"spectralCentroid": 8000.0}),
        ("No onset vs strong onset",
         {"onsetDetected": False, "onsetStrength": 0.0},
         {"onsetDetected": True, "onsetStrength": 0.9}),
    ]

    # Sources known to use audio uniforms
    SOURCES_WITH_AUDIO = [
        "mandelbrot", "julia_set", "burning_ship", "mandelbulb",
        "audio_waveform", "kifs",
    ]

    @pytest.mark.parametrize("name,feat_a,feat_b",
                             FEATURE_PAIRS,
                             ids=[p[0] for p in FEATURE_PAIRS])
    def test_feature_pair_changes_source(self, app, tmp_path, name, feat_a, feat_b):
        """Different feature values should produce different visual output on sources."""
        changed_sources = 0
        for src_id in self.SOURCES_WITH_AUDIO:
            # Render with feature A
            app.load_source(src_id)
            app.inject_features(feat_a)
            path_a = str(tmp_path / f"{src_id}_{name}_a.png".replace(" ", "_"))
            app.render_frame(path_a, time_val=1.0, width=256, height=256)

            # Render with feature B
            app.load_source(src_id)
            app.inject_features(feat_b)
            path_b = str(tmp_path / f"{src_id}_{name}_b.png".replace(" ", "_"))
            app.render_frame(path_b, time_val=1.0, width=256, height=256)

            psnr = psnr_between(path_a, path_b)
            if psnr < 50.0:
                changed_sources += 1

        assert changed_sources >= 2, (
            f"Feature '{name}' only changed {changed_sources}/{len(self.SOURCES_WITH_AUDIO)} sources"
        )

    def test_rms_affects_effects_on_image(self, app, tmp_path):
        """RMS should change effect output via u_rms uniform."""
        if not os.path.exists(TEST_IMAGE):
            pytest.skip("Test image not found")

        effects = ["Ripple", "Hue Shift", "Brightness"]
        changed = 0
        for fx_name in effects:
            app.reset()
            app.load_image(TEST_IMAGE)
            app.set_effect(fx_name, enabled=True)

            app.inject_features({"rms": 0.0})
            p0 = str(tmp_path / f"fx_{fx_name}_rms0.png".replace(" ", "_"))
            app.render_frame(p0, time_val=1.0, width=256, height=256)

            app.inject_features({"rms": 0.9})
            p1 = str(tmp_path / f"fx_{fx_name}_rms9.png".replace(" ", "_"))
            app.render_frame(p1, time_val=1.0, width=256, height=256)

            if psnr_between(p0, p1) < 50.0:
                changed += 1

        assert changed >= 1, "No effects responded to RMS"


# ============================================================
# SIGNAL ROUTE TESTS — Require API endpoints (specification)
# These define what to test once /api/signals, /api/add_route exist
# ============================================================

class TestSignalRouteEndToEnd:
    """End-to-end: create route from audio signal to effect parameter,
    inject features, verify parameter value changes visual output.

    REQUIRES: /api/signals, /api/add_route, /api/routes endpoints.
    Skip if not available.
    """

    def _has_signal_api(self, app):
        """Check if signal API endpoints are available."""
        try:
            import requests
            r = requests.get(f"{app.base_url}/api/signals", timeout=2)
            return r.status_code == 200
        except Exception:
            return False

    def test_create_route_volume_to_ripple(self, app, tmp_path):
        """Route Volume signal → Ripple intensity → verify RMS changes ripple."""
        if not self._has_signal_api(app):
            pytest.skip("Signal API endpoints not available yet")

        if not os.path.exists(TEST_IMAGE):
            pytest.skip("Test image not found")

        # 1. List signals, find Volume
        # signals = app.list_signals()
        # volume_id = next(s["id"] for s in signals if s["name"] == "Volume")

        # 2. Enable Ripple effect
        # app.load_image(TEST_IMAGE)
        # app.set_effect("Ripple", enabled=True, params={"intensity": 0.0})

        # 3. Create route: Volume → Ripple.intensity, range [0, 0.8]
        # route_id = app.add_route({
        #     "source_signal_id": volume_id,
        #     "target_effect": "Ripple",
        #     "target_param": "intensity",
        #     "output_min": 0.0,
        #     "output_max": 0.8
        # })

        # 4. Inject low RMS → render
        # app.inject_features({"rms": 0.1})
        # low_path = str(tmp_path / "route_low.png")
        # app.render_frame(low_path, time_val=1.0)

        # 5. Inject high RMS → render
        # app.inject_features({"rms": 0.9})
        # high_path = str(tmp_path / "route_high.png")
        # app.render_frame(high_path, time_val=1.0)

        # 6. Verify: high RMS should show more ripple (frames differ)
        # assert psnr_between(low_path, high_path) < 40.0

        # 7. Remove route
        # app.remove_route(route_id)

        pytest.skip("Signal route API not yet implemented — test is specification")

    def test_route_threshold_and_gain(self, app, tmp_path):
        """Route with threshold=0.5 should only activate above 0.5 RMS."""
        if not self._has_signal_api(app):
            pytest.skip("Signal API endpoints not available yet")
        pytest.skip("Signal route API not yet implemented — test is specification")

    def test_route_invert(self, app, tmp_path):
        """Inverted route: high RMS should DECREASE the parameter."""
        if not self._has_signal_api(app):
            pytest.skip("Signal API endpoints not available yet")
        pytest.skip("Signal route API not yet implemented — test is specification")

    def test_route_output_range(self, app, tmp_path):
        """Route with outputMin=0.2, outputMax=0.6 should clamp output."""
        if not self._has_signal_api(app):
            pytest.skip("Signal API endpoints not available yet")
        pytest.skip("Signal route API not yet implemented — test is specification")

    def test_macro_aggregation(self, app, tmp_path):
        """Macro linked to Volume should drive multiple parameters simultaneously."""
        if not self._has_signal_api(app):
            pytest.skip("Signal API endpoints not available yet")
        pytest.skip("Signal route API not yet implemented — test is specification")

    def test_oscillator_modulation(self, app, tmp_path):
        """Oscillator signal connected to parameter should produce time-varying output."""
        if not self._has_signal_api(app):
            pytest.skip("Signal API endpoints not available yet")
        pytest.skip("Signal route API not yet implemented — test is specification")


# ============================================================
# SIGNAL REGISTRY VALIDATION — What should be checkable via API
# ============================================================

class TestSignalRegistrySpec:
    """Specification for signal registry tests.
    These define the minimum signal set per the architecture spec.
    """

    EXPECTED_AUDIO_SIGNALS = [
        "Volume", "Sub Bass", "Bass", "Mid", "Air",
        "Tempo", "Beat Position", "Hit", "Beat In Bar",
        "Bar Position", "Phrase Position", "Energy State",
    ]

    EXPECTED_MODULATION_SIGNALS = [
        "Mod 1", "Mod 2",
    ]

    def test_signal_registry_has_minimum_signals(self, app):
        """Registry should have at least 12 audio + 2 modulation signals."""
        if not hasattr(app, 'list_signals'):
            pytest.skip("list_signals() not available — needs /api/signals endpoint")

        # signals = app.list_signals()
        # signal_names = [s["name"] for s in signals]
        # for expected in self.EXPECTED_AUDIO_SIGNALS:
        #     assert expected in signal_names, f"Missing audio signal: {expected}"
        # for expected in self.EXPECTED_MODULATION_SIGNALS:
        #     assert expected in signal_names, f"Missing modulation signal: {expected}"

        pytest.skip("Signal list API not yet implemented — test is specification")

    def test_signal_values_respond_to_features(self, app):
        """After injecting features, signal cached values should update."""
        # 1. Inject features with known RMS
        # 2. Query /api/signals
        # 3. Find "Volume" signal
        # 4. Verify its cached value ≈ injected RMS
        pytest.skip("Signal API not yet implemented — test is specification")
