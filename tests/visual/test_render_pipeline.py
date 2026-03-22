"""
Eyes Visual Tests — Render Pipeline

Tests the core rendering pipeline: image loading, effect application,
feature injection, and frame capture.
"""

import os
import tempfile
import pytest
from vision_check import verify_frame, compute_psnr, compute_ssim


# Path to test fixtures (simple names, no unicode)
FIXTURES_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "fixtures")
)
TEST_IMAGE = os.path.join(FIXTURES_DIR, "test_card.png")


class TestHealth:
    """Verify the test server is responsive."""

    def test_health_endpoint(self, app):
        """Health endpoint should return status and effect count."""
        data = app.health()
        assert data["status"] == "ready"
        assert data["effects_count"] > 0


class TestFrameCapture:
    """Verify basic frame capture works."""

    def test_capture_black_frame(self, app, tmp_path):
        """With no image loaded, capture should produce a frame (may be black)."""
        out = str(tmp_path / "black.png")
        result = app.render_frame(out, time_val=0.0)
        assert result["ok"] is True
        assert os.path.exists(out)

    def test_capture_with_image(self, app, tmp_path):
        """Loading an image and capturing should produce a non-empty PNG."""
        if not os.path.exists(TEST_IMAGE):
            pytest.skip(f"Test image not found: {TEST_IMAGE}")

        app.load_image(TEST_IMAGE)
        out = str(tmp_path / "with_image.png")
        result = app.render_frame(out, time_val=0.0)
        assert result["ok"] is True
        assert os.path.exists(out)
        assert os.path.getsize(out) > 1000  # Should be a real PNG, not empty

    def test_deterministic_renders(self, app, tmp_path):
        """Two renders at the same time should produce identical frames."""
        if not os.path.exists(TEST_IMAGE):
            pytest.skip(f"Test image not found: {TEST_IMAGE}")

        app.load_image(TEST_IMAGE)
        out1 = str(tmp_path / "frame1.png")
        out2 = str(tmp_path / "frame2.png")

        app.render_frame(out1, time_val=1.0)
        app.render_frame(out2, time_val=1.0)

        # These should be identical (same image, same time, no effects)
        import cv2
        import numpy as np

        img1 = cv2.imread(out1)
        img2 = cv2.imread(out2)
        assert img1 is not None and img2 is not None
        assert img1.shape == img2.shape

        psnr = compute_psnr(img1, img2)
        assert psnr > 50.0, f"Frames should be nearly identical, PSNR={psnr:.1f}"


class TestEffects:
    """Verify effects can be enabled and produce visible changes."""

    def test_enable_single_effect(self, app, tmp_path):
        """Enabling an effect should change the rendered output."""
        if not os.path.exists(TEST_IMAGE):
            pytest.skip(f"Test image not found: {TEST_IMAGE}")

        app.load_image(TEST_IMAGE)

        # Capture without effect
        no_fx = str(tmp_path / "no_fx.png")
        app.render_frame(no_fx, time_val=1.0)

        # Enable Invert (strongly changes all pixels)
        app.set_effect("Invert", enabled=True, params={"amount": 1.0})

        with_fx = str(tmp_path / "with_fx.png")
        app.render_frame(with_fx, time_val=1.0)

        # The frames should be different
        import cv2

        img1 = cv2.imread(no_fx)
        img2 = cv2.imread(with_fx)
        assert img1 is not None and img2 is not None

        psnr = compute_psnr(img1, img2)
        # Invert should dramatically change the image
        assert psnr < 100.0, f"Effect should change the image, PSNR={psnr:.1f} (too similar)"
        # Also verify it's NOT identical (PSNR < infinity)
        assert psnr != float("inf"), "Effect had no visible impact"

    def test_effect_chain(self, app, tmp_path):
        """Two effects chained should both apply."""
        if not os.path.exists(TEST_IMAGE):
            pytest.skip(f"Test image not found: {TEST_IMAGE}")

        app.load_image(TEST_IMAGE)

        # Capture with just hue shift
        app.set_effect("Hue Shift", enabled=True, params={"shift": 0.5})
        hue_only = str(tmp_path / "hue_only.png")
        app.render_frame(hue_only, time_val=1.0)

        app.reset()
        app.load_image(TEST_IMAGE)

        # Capture with hue shift + invert
        app.set_effect_chain([
            {"name": "Hue Shift", "params": {"shift": 0.5}},
            {"name": "Invert", "params": {"amount": 1.0}},
        ])
        both = str(tmp_path / "both.png")
        app.render_frame(both, time_val=1.0)

        # The two-effect result should differ from single-effect
        import cv2

        img1 = cv2.imread(hue_only)
        img2 = cv2.imread(both)
        assert img1 is not None and img2 is not None

        psnr = compute_psnr(img1, img2)
        assert psnr < 45.0, f"Second effect should change output, PSNR={psnr:.1f}"


class TestFeatureInjection:
    """Verify audio feature injection works."""

    def test_inject_features(self, app):
        """Injecting features should succeed."""
        result = app.inject_features({
            "rms": 0.8,
            "beatPhase": 0.5,
            "spectralCentroid": 2000.0,
            "bandEnergies": [0.1, 0.3, 0.5, 0.7, 0.5, 0.3, 0.1],
        })
        assert result["ok"] is True


class TestState:
    """Verify state endpoint works."""

    def test_state_returns_effects(self, app):
        """State endpoint should list all effects."""
        data = app.state()
        assert "effects" in data
        assert len(data["effects"]) > 0
        # Each effect should have name, enabled, params
        fx = data["effects"][0]
        assert "name" in fx
        assert "enabled" in fx
        assert "params" in fx


class TestReset:
    """Verify reset clears state properly."""

    def test_reset_disables_effects(self, app):
        """After reset, no effects should be enabled."""
        # Enable an effect
        app.set_effect("Hue Shift", enabled=True, params={"shift": 0.5})

        # Verify it's enabled
        state = app.state()
        hue = next(fx for fx in state["effects"] if fx["name"] == "Hue Shift")
        assert hue["enabled"] is True

        # Reset
        app.reset()

        # Verify all disabled
        state = app.state()
        enabled = [fx for fx in state["effects"] if fx["enabled"]]
        assert len(enabled) == 0, f"Expected 0 enabled effects, got {len(enabled)}"
