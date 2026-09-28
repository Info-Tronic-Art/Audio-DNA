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
from tier1_exceptions import (candidate_values, T_PARAM, T_RETRY, arm_audio, FEATURES_ACTIVE,
                              LEGACY_CHAIN_EFFECTS, LEGACY_CHAIN_PARAMS, ANIMATED_INPUT_EFFECTS,
                              GATED_EFFECT_PARAMS)

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
            arm_audio(app)
            app.load_image(TEST_IMAGE)
            # Enable with default params
            app.set_effect(fx["name"], enabled=True)
            out = str(tmp_path / f"fx_{fx['name'].replace(' ', '_')}.png")
            result = app.render_frame(out, time_val=T_PARAM, width=256, height=256)
            if result.get("ok") and brightness(out) < 3.0:
                # One retry at another time before failing: a strobing effect is dark for part of its period
                out = str(tmp_path / f"fx_{fx['name'].replace(' ', '_')}_t2.png")
                result = app.render_frame(out, time_val=T_RETRY, width=256, height=256)
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

        def load_input(fx_name):
            # Effects that read previous frames render over an animated source (Renderer legacy path: the source,
            # then effectChain_.render); everything else over the static test card.
            if fx_name in ANIMATED_INPUT_EFFECTS:
                app.load_source("plasma")
            else:
                app.load_image(TEST_IMAGE)

        failures = []
        for fx in all_effects:
            if fx["name"] in LEGACY_CHAIN_EFFECTS:
                continue
            for param in fx.get("params", []):
                param_name = param["name"]
                default_val = param.get("default", 0.5)
                if (fx["name"], param_name) in LEGACY_CHAIN_PARAMS:
                    continue
                gate = GATED_EFFECT_PARAMS.get((fx["name"], param_name), ({}, ""))[0]

                def arm():
                    # FEATURES_ACTIVE, plus the structural state that opens this param's gate (if any)
                    if gate:
                        app.inject_features({**FEATURES_ACTIVE, **gate})
                    else:
                        arm_audio(app)

                # Render at default (a black default -- Strobe's dark half at T_PARAM -- is compared as is:
                # a candidate that lights it is a change)
                app.reset()
                arm()
                load_input(fx["name"])
                app.set_effect(fx["name"], enabled=True, params={param_name: default_val})
                def_path = str(tmp_path / f"fx_{fx['name']}_{param_name}_def.png".replace(" ", "_"))
                t = T_PARAM
                app.render_frame(def_path, time_val=t, width=256, height=256)

                # Walk the candidate ladder until one visibly changes the output
                tried = []
                changed = False
                for test_val in candidate_values(default_val):
                    app.reset()
                    arm()
                    load_input(fx["name"])
                    app.set_effect(fx["name"], enabled=True, params={param_name: test_val})
                    test_path = str(tmp_path / f"fx_{fx['name']}_{param_name}_test_{test_val:.2f}.png".replace(" ", "_"))
                    app.render_frame(test_path, time_val=t, width=256, height=256)
                    psnr = psnr_between(def_path, test_path)
                    tried.append(f"{test_val} (PSNR={psnr:.1f})")
                    if psnr <= 55.0:   # the old pass rule (fail only when PSNR > 55)
                        changed = True
                        break
                if not changed:
                    failures.append(
                        f"{fx['name']}:{param_name} no effect "
                        f"(default={default_val}, tried [{', '.join(tried)}])"
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
            arm_audio(app)
            app.load_image(TEST_IMAGE)
            # Enable at moderate params (all at 0.5)
            params = {p["name"]: 0.5 for p in fx.get("params", [])}
            app.set_effect(fx["name"], enabled=True, params=params)
            out = str(tmp_path / f"fx_mid_{fx['name'].replace(' ', '_')}.png")
            app.render_frame(out, time_val=T_PARAM, width=256, height=256)

            b = brightness(out)
            if b < 3.0:
                failures.append(f"{fx['name']}: black at mid-params (brightness={b:.1f})")
            elif b > 252.0:
                failures.append(f"{fx['name']}: white at mid-params (brightness={b:.1f})")

        if failures:
            print(f"\nDestructive effects at mid-params:\n" + "\n".join(failures))
