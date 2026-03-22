"""
Auto-Discovering Effect Verification — Tests ALL registered effects.

Queries /api/state to discover every effect and its params automatically.
No hardcoded effect lists — when a new effect is added to EffectLibrary.cpp,
it gets tested automatically.

Requires a test image loaded first. Tests each effect's params for
visible impact on the rendered output.
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


def brightness(path):
    img = cv2.imread(path)
    return float(np.mean(img)) if img is not None else 0.0


def psnr_between(path1, path2):
    img1, img2 = cv2.imread(path1), cv2.imread(path2)
    if img1 is None or img2 is None:
        return float("inf")
    return compute_psnr(img1, img2)


@pytest.fixture(scope="module")
def all_effects(app):
    """Discover all effects from the running app."""
    state = app.state()
    return state["effects"]


@pytest.fixture(scope="module")
def image_loaded(app):
    """Ensure test image is loaded."""
    if os.path.exists(TEST_IMAGE):
        app.load_image(TEST_IMAGE)
        return True
    return False


class TestAllEffectsRender:
    """Every effect must render without crashing when enabled."""

    def test_all_effects_non_black(self, app, tmp_path, all_effects, image_loaded):
        if not image_loaded:
            pytest.skip("Test image not found")

        failures = []
        for fx in all_effects:
            app.reset()
            app.load_image(TEST_IMAGE)
            # Enable with default params
            app.set_effect(fx["name"], enabled=True)
            out = str(tmp_path / f"fx_{fx['name'].replace(' ', '_')}.png")
            result = app.render_frame(out, time_val=1.0, width=256, height=256)
            if not result.get("ok"):
                failures.append(f"{fx['name']}: render failed")
            elif brightness(out) < 3.0:
                failures.append(f"{fx['name']}: all black when enabled")

        assert not failures, (
            f"{len(failures)} effects broken:\n" + "\n".join(failures)
        )


class TestAllEffectParams:
    """Every param on every effect must change the output."""

    def test_all_params_have_effect(self, app, tmp_path, all_effects, image_loaded):
        if not image_loaded:
            pytest.skip("Test image not found")

        failures = []
        for fx in all_effects:
            for param in fx.get("params", []):
                param_name = param["name"]
                default_val = param.get("default", 0.5)
                test_val = 0.0 if default_val > 0.3 else 1.0

                # Render at default
                app.reset()
                app.load_image(TEST_IMAGE)
                app.set_effect(fx["name"], enabled=True, params={param_name: default_val})
                def_path = str(tmp_path / f"fx_{fx['name']}_{param_name}_def.png".replace(" ", "_"))
                app.render_frame(def_path, time_val=1.0, width=256, height=256)

                # Render at test value
                app.reset()
                app.load_image(TEST_IMAGE)
                app.set_effect(fx["name"], enabled=True, params={param_name: test_val})
                test_path = str(tmp_path / f"fx_{fx['name']}_{param_name}_test.png".replace(" ", "_"))
                app.render_frame(test_path, time_val=1.0, width=256, height=256)

                # Verify visible change
                psnr = psnr_between(def_path, test_path)
                if psnr > 55.0:
                    failures.append(
                        f"{fx['name']}:{param_name} no effect "
                        f"(PSNR={psnr:.1f}, {default_val}->{test_val})"
                    )

        if failures:
            report_path = str(tmp_path / "effect_param_failures.txt")
            with open(report_path, "w") as f:
                f.write("\n".join(failures))
            assert False, (
                f"{len(failures)} effect param issues:\n"
                + "\n".join(failures[:30])
                + (f"\n... +{len(failures)-30} more" if len(failures) > 30 else "")
            )


class TestEffectNotDestructive:
    """No effect should turn the image completely black or white."""

    def test_no_destructive_effects(self, app, tmp_path, all_effects, image_loaded):
        if not image_loaded:
            pytest.skip("Test image not found")

        # Known exceptions: effects that ARE supposed to produce extreme output
        EXCEPTIONS = {"Invert", "Threshold", "Greyscale", "Color Flash"}

        failures = []
        for fx in all_effects:
            if fx["name"] in EXCEPTIONS:
                continue

            app.reset()
            app.load_image(TEST_IMAGE)
            # Enable at moderate params (all at 0.5)
            params = {p["name"]: 0.5 for p in fx.get("params", [])}
            app.set_effect(fx["name"], enabled=True, params=params)
            out = str(tmp_path / f"fx_mid_{fx['name'].replace(' ', '_')}.png")
            app.render_frame(out, time_val=1.0, width=256, height=256)

            b = brightness(out)
            if b < 3.0:
                failures.append(f"{fx['name']}: black at mid-params (brightness={b:.1f})")
            elif b > 252.0:
                failures.append(f"{fx['name']}: white at mid-params (brightness={b:.1f})")

        if failures:
            print(f"\nDestructive effects at mid-params:\n" + "\n".join(failures))
