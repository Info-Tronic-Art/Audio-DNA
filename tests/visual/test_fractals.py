"""
Eyes Visual Tests — Comprehensive Fractal Source Validation

Tests EVERY control on EVERY fractal source. Each parameter is tested by:
1. Loading the source with default params
2. Rendering a baseline frame
3. Changing ONE parameter to a different value
4. Rendering a comparison frame
5. Verifying the frame is not all-black (the fractal is visible)
6. Verifying PSNR < threshold (the parameter actually changed the output)
"""

import os
import pytest
import cv2
import numpy as np
import sys
sys.path.insert(0, os.path.dirname(__file__))
from vision_check import compute_psnr


def frame_is_not_black(path, threshold=5.0):
    img = cv2.imread(path)
    if img is None:
        return False
    return float(np.mean(img)) > threshold


def frames_are_different(path1, path2, max_psnr=55.0):
    img1 = cv2.imread(path1)
    img2 = cv2.imread(path2)
    if img1 is None or img2 is None:
        return False
    psnr = compute_psnr(img1, img2)
    return psnr < max_psnr


# ============================================================
# PARAMETER TESTS: (source_id, param_uniform, default, test_val, name)
# ============================================================

KALEIDO_PARAMS = [
    ("kaleido_fractal", "u_src_iterations", 0.5, 0.9, "Iterations"),
    ("kaleido_fractal", "u_src_fold_angle", 0.4, 0.8, "Fold Angle"),
    ("kaleido_fractal", "u_src_zoom", 0.5, 0.2, "Zoom"),
    ("kaleido_fractal", "u_src_rotation", 0.0, 0.5, "Rotation"),
    ("kaleido_fractal", "u_src_color_shift", 0.0, 0.5, "Color Shift"),
    ("kaleido_fractal", "u_src_palette", 0.6, 0.0, "Palette"),
]

MANDELBROT_PARAMS = [
    ("mandelbrot", "u_src_dive_speed", 0.0, 0.3, "Dive Speed"),
    ("mandelbrot", "u_src_location", 0.0, 0.3, "Location"),
    ("mandelbrot", "u_src_zoom", 0.0, 0.3, "Zoom"),
    ("mandelbrot", "u_src_center_x", 0.5, 0.7, "Center X"),
    ("mandelbrot", "u_src_center_y", 0.5, 0.7, "Center Y"),
    ("mandelbrot", "u_src_julia_mix", 0.0, 1.0, "Julia Mix"),
    ("mandelbrot", "u_src_max_iter", 0.3, 0.8, "Max Iterations"),
    ("mandelbrot", "u_src_power", 0.0, 0.5, "Power"),
    ("mandelbrot", "u_src_color_speed", 0.3, 0.8, "Color Speed"),
    ("mandelbrot", "u_src_color_shift", 0.0, 0.5, "Color Shift"),
    ("mandelbrot", "u_src_palette", 0.6, 0.0, "Palette"),
]

JULIA_PARAMS = [
    ("julia_set", "u_src_dive_speed", 0.0, 0.3, "Dive Speed"),
    ("julia_set", "u_src_location", 0.0, 0.5, "Location"),
    ("julia_set", "u_src_cx", 0.5, 0.8, "C Real"),
    ("julia_set", "u_src_cy", 0.5, 0.8, "C Imaginary"),
    ("julia_set", "u_src_zoom", 0.0, 0.4, "Zoom"),
    ("julia_set", "u_src_iterations", 0.3, 0.8, "Iterations"),
    ("julia_set", "u_src_color_speed", 0.3, 0.8, "Color Speed"),
    ("julia_set", "u_src_color_shift", 0.0, 0.5, "Color Shift"),
    ("julia_set", "u_src_palette", 0.6, 0.0, "Palette"),
]

BURNING_SHIP_PARAMS = [
    ("burning_ship", "u_src_dive_speed", 0.0, 0.3, "Dive Speed"),
    ("burning_ship", "u_src_location", 0.0, 0.4, "Location"),
    ("burning_ship", "u_src_center_x", 0.5, 0.3, "Center X"),
    ("burning_ship", "u_src_center_y", 0.5, 0.3, "Center Y"),
    ("burning_ship", "u_src_zoom", 0.0, 0.4, "Zoom"),
    ("burning_ship", "u_src_iterations", 0.3, 0.8, "Iterations"),
    ("burning_ship", "u_src_color_speed", 0.3, 0.8, "Color Speed"),
    ("burning_ship", "u_src_color_shift", 0.0, 0.5, "Color Shift"),
    ("burning_ship", "u_src_palette", 0.6, 0.0, "Palette"),
]

NEWTON_PARAMS = [
    ("newton_fractal", "u_src_dive_speed", 0.0, 0.3, "Dive Speed"),
    ("newton_fractal", "u_src_power", 0.2, 0.7, "Power"),
    ("newton_fractal", "u_src_zoom", 0.0, 0.4, "Zoom"),
    ("newton_fractal", "u_src_damping", 0.5, 0.2, "Damping"),
    ("newton_fractal", "u_src_color_shift", 0.0, 0.5, "Color Shift"),
    ("newton_fractal", "u_src_palette", 0.6, 0.0, "Palette"),
]

SIERPINSKI_PARAMS = [
    ("sierpinski", "u_src_mode", 0.0, 1.0, "Mode"),
    ("sierpinski", "u_src_zoom", 0.0, 0.5, "Zoom"),
    ("sierpinski", "u_src_iterations", 0.5, 0.9, "Iterations"),
    ("sierpinski", "u_src_rotation", 0.5, 0.8, "Rotation"),
    ("sierpinski", "u_src_color_shift", 0.0, 0.5, "Color Shift"),
    ("sierpinski", "u_src_palette", 0.6, 0.0, "Palette"),
]

APOLLONIAN_PARAMS = [
    ("apollonian", "u_src_zoom", 0.0, 0.5, "Zoom"),
    ("apollonian", "u_src_iterations", 0.4, 0.9, "Iterations"),
    ("apollonian", "u_src_rotation", 0.5, 0.8, "Rotation"),
    ("apollonian", "u_src_color_shift", 0.0, 0.5, "Color Shift"),
    ("apollonian", "u_src_palette", 0.6, 0.0, "Palette"),
]

# 3D fractals — now with zoom, speed, trail, slice_count
MANDELBULB_PARAMS = [
    ("mandelbulb", "u_src_power", 0.5, 0.2, "Power"),
    ("mandelbulb", "u_src_iterations", 0.4, 0.8, "Iterations"),
    ("mandelbulb", "u_src_rotation_x", 0.55, 0.8, "Angle X"),
    ("mandelbulb", "u_src_rotation_y", 0.55, 0.8, "Angle Y"),
    ("mandelbulb", "u_src_zoom", 0.3, 0.8, "Zoom"),
    ("mandelbulb", "u_src_speed", 0.55, 0.8, "Speed"),
    ("mandelbulb", "u_src_color_shift", 0.0, 0.5, "Color Shift"),
    ("mandelbulb", "u_src_slice", 0.5, 0.8, "Cross Section"),
    ("mandelbulb", "u_src_slice_count", 0.0, 0.5, "Slice Count"),
    ("mandelbulb", "u_src_glow", 0.0, 0.7, "Glow"),
    ("mandelbulb", "u_src_trail_dist", 0.0, 0.5, "Trail Distance"),
    ("mandelbulb", "u_src_trail_fade", 0.5, 0.1, "Trail Fade"),
    ("mandelbulb", "u_src_slice_dist", 0.3, 0.8, "Slice Distance"),
    ("mandelbulb", "u_src_palette", 0.6, 0.0, "Palette"),
]

MENGER_PARAMS = [
    ("menger_sponge", "u_src_iterations", 0.5, 0.9, "Iterations"),
    ("menger_sponge", "u_src_rotation_x", 0.55, 0.8, "Angle X"),
    ("menger_sponge", "u_src_rotation_y", 0.55, 0.8, "Angle Y"),
    ("menger_sponge", "u_src_zoom", 0.3, 0.8, "Zoom"),
    ("menger_sponge", "u_src_speed", 0.55, 0.8, "Speed"),
    ("menger_sponge", "u_src_twist", 0.0, 0.5, "Twist"),
    ("menger_sponge", "u_src_color_shift", 0.0, 0.5, "Color Shift"),
    ("menger_sponge", "u_src_slice", 0.5, 0.8, "Cross Section"),
    ("menger_sponge", "u_src_slice_count", 0.0, 0.5, "Slice Count"),
    ("menger_sponge", "u_src_trail_dist", 0.0, 0.5, "Trail Distance"),
    ("menger_sponge", "u_src_trail_fade", 0.5, 0.1, "Trail Fade"),
    ("menger_sponge", "u_src_slice_dist", 0.3, 0.8, "Slice Distance"),
    ("menger_sponge", "u_src_palette", 0.6, 0.0, "Palette"),
]

KIFS_PARAMS = [
    ("kifs", "u_src_scale", 0.4, 0.7, "Scale"),
    ("kifs", "u_src_iterations", 0.4, 0.8, "Iterations"),
    ("kifs", "u_src_fold_type", 0.0, 0.5, "Fold Type"),
    ("kifs", "u_src_rotation_x", 0.55, 0.8, "Angle X"),
    ("kifs", "u_src_rotation_y", 0.55, 0.8, "Angle Y"),
    ("kifs", "u_src_zoom", 0.3, 0.8, "Zoom"),
    ("kifs", "u_src_speed", 0.55, 0.8, "Speed"),
    ("kifs", "u_src_offset", 0.5, 0.8, "Offset"),
    ("kifs", "u_src_color_shift", 0.0, 0.5, "Color Shift"),
    ("kifs", "u_src_slice", 0.5, 0.8, "Cross Section"),
    ("kifs", "u_src_slice_count", 0.0, 0.5, "Slice Count"),
    ("kifs", "u_src_trail_dist", 0.0, 0.5, "Trail Distance"),
    ("kifs", "u_src_trail_fade", 0.5, 0.1, "Trail Fade"),
    ("kifs", "u_src_slice_dist", 0.3, 0.8, "Slice Distance"),
    ("kifs", "u_src_palette", 0.6, 0.0, "Palette"),
]

JULIA3D_PARAMS = [
    ("julia_set_3d", "u_src_location", 0.0, 0.5, "Location"),
    ("julia_set_3d", "u_src_cx", 0.35, 0.6, "C Real"),
    ("julia_set_3d", "u_src_cy", 0.6, 0.3, "C Imaginary"),
    ("julia_set_3d", "u_src_rotation_x", 0.55, 0.8, "Angle X"),
    ("julia_set_3d", "u_src_rotation_y", 0.55, 0.8, "Angle Y"),
    ("julia_set_3d", "u_src_zoom", 0.3, 0.8, "Zoom"),
    ("julia_set_3d", "u_src_speed", 0.55, 0.8, "Speed"),
    ("julia_set_3d", "u_src_color_shift", 0.0, 0.5, "Color Shift"),
    ("julia_set_3d", "u_src_slice", 0.5, 0.8, "Cross Section"),
    ("julia_set_3d", "u_src_slice_count", 0.0, 0.5, "Slice Count"),
    ("julia_set_3d", "u_src_glow", 0.0, 0.7, "Glow"),
    ("julia_set_3d", "u_src_trail_dist", 0.0, 0.5, "Trail Distance"),
    ("julia_set_3d", "u_src_trail_fade", 0.5, 0.1, "Trail Fade"),
    ("julia_set_3d", "u_src_slice_dist", 0.3, 0.8, "Slice Distance"),
    ("julia_set_3d", "u_src_palette", 0.6, 0.0, "Palette"),
]

BURNING3D_PARAMS = [
    ("burning_ship_3d", "u_src_power", 0.5, 0.2, "Power"),
    ("burning_ship_3d", "u_src_rotation_x", 0.55, 0.8, "Angle X"),
    ("burning_ship_3d", "u_src_rotation_y", 0.55, 0.8, "Angle Y"),
    ("burning_ship_3d", "u_src_zoom", 0.3, 0.8, "Zoom"),
    ("burning_ship_3d", "u_src_speed", 0.55, 0.8, "Speed"),
    ("burning_ship_3d", "u_src_color_shift", 0.0, 0.5, "Color Shift"),
    ("burning_ship_3d", "u_src_slice", 0.5, 0.8, "Cross Section"),
    ("burning_ship_3d", "u_src_slice_count", 0.0, 0.5, "Slice Count"),
    ("burning_ship_3d", "u_src_glow", 0.0, 0.7, "Glow"),
    ("burning_ship_3d", "u_src_trail_dist", 0.0, 0.5, "Trail Distance"),
    ("burning_ship_3d", "u_src_trail_fade", 0.5, 0.1, "Trail Fade"),
    ("burning_ship_3d", "u_src_slice_dist", 0.3, 0.8, "Slice Distance"),
    ("burning_ship_3d", "u_src_palette", 0.6, 0.0, "Palette"),
]

NEWTON3D_PARAMS = [
    ("newton_3d", "u_src_power", 0.2, 0.6, "Power"),
    ("newton_3d", "u_src_rotation_x", 0.55, 0.8, "Angle X"),
    ("newton_3d", "u_src_rotation_y", 0.55, 0.8, "Angle Y"),
    ("newton_3d", "u_src_zoom", 0.3, 0.8, "Zoom"),
    ("newton_3d", "u_src_speed", 0.55, 0.8, "Speed"),
    ("newton_3d", "u_src_damping", 0.5, 0.2, "Damping"),
    ("newton_3d", "u_src_height", 0.5, 0.9, "Height"),
    ("newton_3d", "u_src_color_shift", 0.0, 0.5, "Color Shift"),
    ("newton_3d", "u_src_trail_dist", 0.0, 0.5, "Trail Distance"),
    ("newton_3d", "u_src_trail_fade", 0.5, 0.1, "Trail Fade"),
    ("newton_3d", "u_src_palette", 0.6, 0.0, "Palette"),
]

SIERPINSKI_TETRA_PARAMS = [
    ("sierpinski_tetra", "u_src_iterations", 0.5, 0.9, "Iterations"),
    ("sierpinski_tetra", "u_src_rotation_x", 0.55, 0.8, "Angle X"),
    ("sierpinski_tetra", "u_src_rotation_y", 0.55, 0.8, "Angle Y"),
    ("sierpinski_tetra", "u_src_zoom", 0.3, 0.8, "Zoom"),
    ("sierpinski_tetra", "u_src_speed", 0.55, 0.8, "Speed"),
    ("sierpinski_tetra", "u_src_color_shift", 0.0, 0.5, "Color Shift"),
    ("sierpinski_tetra", "u_src_slice", 0.5, 0.8, "Cross Section"),
    ("sierpinski_tetra", "u_src_slice_count", 0.0, 0.5, "Slice Count"),
    ("sierpinski_tetra", "u_src_trail_dist", 0.0, 0.5, "Trail Distance"),
    ("sierpinski_tetra", "u_src_trail_fade", 0.5, 0.1, "Trail Fade"),
    ("sierpinski_tetra", "u_src_slice_dist", 0.3, 0.8, "Slice Distance"),
    ("sierpinski_tetra", "u_src_palette", 0.6, 0.0, "Palette"),
]

APOLLONIAN3D_PARAMS = [
    ("apollonian_3d", "u_src_scale", 0.3, 0.7, "Scale"),
    ("apollonian_3d", "u_src_iterations", 0.4, 0.8, "Iterations"),
    ("apollonian_3d", "u_src_rotation_x", 0.55, 0.8, "Angle X"),
    ("apollonian_3d", "u_src_rotation_y", 0.55, 0.8, "Angle Y"),
    ("apollonian_3d", "u_src_zoom", 0.3, 0.8, "Zoom"),
    ("apollonian_3d", "u_src_speed", 0.55, 0.8, "Speed"),
    ("apollonian_3d", "u_src_color_shift", 0.0, 0.5, "Color Shift"),
    ("apollonian_3d", "u_src_slice", 0.5, 0.8, "Cross Section"),
    ("apollonian_3d", "u_src_slice_count", 0.0, 0.5, "Slice Count"),
    ("apollonian_3d", "u_src_trail_dist", 0.0, 0.5, "Trail Distance"),
    ("apollonian_3d", "u_src_trail_fade", 0.5, 0.1, "Trail Fade"),
    ("apollonian_3d", "u_src_slice_dist", 0.3, 0.8, "Slice Distance"),
    ("apollonian_3d", "u_src_palette", 0.6, 0.0, "Palette"),
]

ALL_PARAM_TESTS = (
    KALEIDO_PARAMS + MANDELBROT_PARAMS + JULIA_PARAMS +
    BURNING_SHIP_PARAMS + NEWTON_PARAMS + SIERPINSKI_PARAMS +
    APOLLONIAN_PARAMS + MANDELBULB_PARAMS + MENGER_PARAMS +
    KIFS_PARAMS + JULIA3D_PARAMS + BURNING3D_PARAMS +
    NEWTON3D_PARAMS + SIERPINSKI_TETRA_PARAMS + APOLLONIAN3D_PARAMS
)

ALL_SOURCES = [
    "kaleido_fractal", "mandelbrot", "julia_set", "burning_ship",
    "newton_fractal", "sierpinski", "apollonian",
    "mandelbulb", "menger_sponge", "kifs",
    "julia_set_3d", "burning_ship_3d", "newton_3d",
    "sierpinski_tetra", "apollonian_3d",
]


class TestSourceLoadsAndRenders:
    """Every fractal must render a visible frame at defaults."""

    @pytest.mark.parametrize("source_id", ALL_SOURCES)
    def test_renders_non_black(self, app, tmp_path, source_id):
        app.load_source(source_id)
        out = str(tmp_path / f"{source_id}_default.png")
        result = app.render_frame(out, time_val=1.0, width=512, height=512)
        assert result["ok"] is True, f"Render failed for {source_id}"
        assert frame_is_not_black(out), f"{source_id} renders all-black at defaults"


class TestEveryParamHasEffect:
    """Every parameter must produce a visible change when modified."""

    @pytest.mark.parametrize(
        "source_id,param,default_val,test_val,desc",
        ALL_PARAM_TESTS,
        ids=[f"{t[0]}:{t[4]}" for t in ALL_PARAM_TESTS],
    )
    def test_param_changes_output(self, app, tmp_path, source_id, param, default_val, test_val, desc):
        # Default render
        app.load_source(source_id)
        default_out = str(tmp_path / f"{source_id}_{desc}_def.png")
        app.render_frame(default_out, time_val=1.0, width=512, height=512)

        # Changed render
        app.load_source(source_id)
        app.update_source_params({param: test_val})
        changed_out = str(tmp_path / f"{source_id}_{desc}_chg.png")
        app.render_frame(changed_out, time_val=1.0, width=512, height=512)

        assert frame_is_not_black(default_out), f"{source_id} default is all-black"
        assert frame_is_not_black(changed_out), f"{source_id} with {desc}={test_val} is all-black"
        assert frames_are_different(default_out, changed_out), (
            f"{source_id}:{desc} change {default_val}->{test_val} had no visible effect"
        )


class TestZoomLooping:
    """Zoom at ALL positions (0, 0.25, 0.5, 0.75, 1.0) must be non-black."""

    ZOOM_SOURCES_2D = [
        "mandelbrot", "julia_set", "burning_ship", "newton_fractal",
        "sierpinski", "apollonian",
    ]

    @pytest.mark.parametrize("source_id", ZOOM_SOURCES_2D)
    @pytest.mark.parametrize("zoom_val", [0.0, 0.25, 0.5, 0.75, 1.0])
    def test_2d_zoom_not_black(self, app, tmp_path, source_id, zoom_val):
        app.load_source(source_id)
        app.update_source_params({"u_src_zoom": zoom_val})
        out = str(tmp_path / f"{source_id}_z{zoom_val}.png")
        app.render_frame(out, time_val=0.0, width=512, height=512)
        assert frame_is_not_black(out), f"{source_id} zoom={zoom_val} is all-black"


class TestDiveSpeedNotBlack:
    """Dive speed at various levels must not go black."""

    DIVE_SOURCES = ["mandelbrot", "julia_set", "burning_ship", "newton_fractal"]

    @pytest.mark.parametrize("source_id", DIVE_SOURCES)
    @pytest.mark.parametrize("dive_val", [0.1, 0.3, 0.5, 0.7, 1.0])
    def test_dive_not_black(self, app, tmp_path, source_id, dive_val):
        app.load_source(source_id)
        app.update_source_params({"u_src_dive_speed": dive_val})
        out = str(tmp_path / f"{source_id}_dive{dive_val}.png")
        app.render_frame(out, time_val=5.0, width=512, height=512)
        assert frame_is_not_black(out), f"{source_id} dive={dive_val} at t=5 is all-black"


class TestPowerNotBlack:
    """Power at all positions must not go black."""

    @pytest.mark.parametrize("power_val", [0.0, 0.25, 0.5, 0.75, 1.0])
    def test_mandelbrot_power(self, app, tmp_path, power_val):
        app.load_source("mandelbrot")
        app.update_source_params({"u_src_power": power_val})
        out = str(tmp_path / f"mb_power{power_val}.png")
        app.render_frame(out, time_val=0.0, width=512, height=512)
        assert frame_is_not_black(out), f"Mandelbrot power={power_val} is all-black"


class Test3DZoomRange:
    """3D fractal zoom from outside (0) to inside (1) must be non-black."""

    SOURCES_3D = [
        "mandelbulb", "menger_sponge", "kifs",
        "julia_set_3d", "burning_ship_3d", "sierpinski_tetra", "apollonian_3d",
    ]

    @pytest.mark.parametrize("source_id", SOURCES_3D)
    @pytest.mark.parametrize("zoom_val", [0.0, 0.3, 0.6, 0.9])
    def test_3d_zoom_not_black(self, app, tmp_path, source_id, zoom_val):
        app.load_source(source_id)
        app.update_source_params({"u_src_zoom": zoom_val})
        out = str(tmp_path / f"{source_id}_z{zoom_val}.png")
        app.render_frame(out, time_val=1.0, width=512, height=512)
        assert frame_is_not_black(out), f"{source_id} zoom={zoom_val} is all-black"


class TestPaletteVariety:
    """Palettes must produce different colors."""

    PALETTE_SOURCES = ALL_SOURCES

    @pytest.mark.parametrize("source_id", PALETTE_SOURCES)
    def test_palettes_different(self, app, tmp_path, source_id):
        frames = []
        for pal in [0.0, 0.3, 0.6, 0.9]:
            app.load_source(source_id)
            app.update_source_params({"u_src_palette": pal})
            out = str(tmp_path / f"{source_id}_p{pal}.png")
            app.render_frame(out, time_val=1.0, width=512, height=512)
            frames.append(out)

        diff_count = 0
        for i in range(len(frames)):
            for j in range(i + 1, len(frames)):
                if frames_are_different(frames[i], frames[j]):
                    diff_count += 1
        assert diff_count >= 2, f"{source_id}: only {diff_count}/6 palette pairs differ"


class TestSourceRegistry:
    """All sources registered with correct params."""

    def test_all_registered(self, app):
        data = app.list_sources()
        ids = [s["id"] for s in data["sources"]]
        for src_id in ALL_SOURCES:
            assert src_id in ids, f"'{src_id}' not registered"

    def test_all_have_palette(self, app):
        data = app.list_sources()
        for src in data["sources"]:
            if src["id"] in ALL_SOURCES:
                uniforms = [p["uniform"] for p in src["params"]]
                assert "u_src_palette" in uniforms, f"'{src['id']}' missing palette"

    def test_3d_have_new_controls(self, app):
        data = app.list_sources()
        sources_3d = [
            "mandelbulb", "menger_sponge", "kifs", "julia_set_3d",
            "burning_ship_3d", "newton_3d", "sierpinski_tetra", "apollonian_3d",
        ]
        for src in data["sources"]:
            if src["id"] in sources_3d:
                uniforms = [p["uniform"] for p in src["params"]]
                assert "u_src_zoom" in uniforms, f"'{src['id']}' missing zoom"
                assert "u_src_speed" in uniforms, f"'{src['id']}' missing speed"
                assert "u_src_trail_dist" in uniforms, f"'{src['id']}' missing trail_dist"
                assert "u_src_trail_fade" in uniforms, f"'{src['id']}' missing trail_fade"
