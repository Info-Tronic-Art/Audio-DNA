#include "EffectLibrary.h"

void EffectLibrary::registerEffect(const EffectDef& def)
{
    defs_.push_back(def);
}

void EffectLibrary::registerDefaults()
{
    defs_.clear();

    // === Warp Effects ===

    registerEffect({"Ripple", "warp", "ripple", {
        {"intensity", "u_ripple_intensity", 0.0f},
        {"freq",      "u_ripple_freq",      0.5f},
        {"speed",     "u_ripple_speed",     0.5f}
    }});

    registerEffect({"Bulge", "warp", "bulge", {
        {"amount",   "u_bulge_amount",   0.0f},
        {"center_x", "u_bulge_center_x", 0.5f},
        {"center_y", "u_bulge_center_y", 0.5f}
    }});

    registerEffect({"Wave", "warp", "wave", {
        {"amplitude", "u_wave_amp",       0.0f},
        {"frequency", "u_wave_freq",      0.5f},
        {"direction", "u_wave_direction", 0.0f}
    }});

    registerEffect({"Liquid", "warp", "liquid", {
        {"viscosity",  "u_liquid_visc", 0.5f},
        {"turbulence", "u_liquid_turb", 0.0f}
    }});

    // === Color Effects ===

    registerEffect({"Hue Shift", "color", "hue_shift", {
        {"amount", "u_hue_shift", 0.0f}
    }});

    registerEffect({"Saturation", "color", "saturation", {
        {"amount", "u_saturation", 0.5f}
    }});

    registerEffect({"Brightness", "color", "brightness", {
        {"amount", "u_brightness", 0.5f}
    }});

    registerEffect({"Duotone", "color", "duotone", {
        {"color1_r", "u_duotone_a_r", 0.0f},
        {"color1_g", "u_duotone_a_g", 0.0f},
        {"color1_b", "u_duotone_a_b", 0.5f},
        {"color2_r", "u_duotone_b_r", 1.0f},
        {"color2_g", "u_duotone_b_g", 0.5f},
        {"color2_b", "u_duotone_b_b", 0.0f},
        {"mix",      "u_duotone_mix", 0.0f}
    }});

    registerEffect({"Chromatic Aberration", "color", "chromatic_aberration", {
        {"amount", "u_chroma_amount", 0.0f},
        {"angle",  "u_chroma_angle",  0.0f}
    }});

    // === Glitch Effects ===

    registerEffect({"Pixel Scatter", "glitch", "pixel_scatter", {
        {"amount", "u_scatter_amount", 0.0f},
        {"seed",   "u_scatter_seed",   0.0f}
    }});

    registerEffect({"RGB Split", "glitch", "rgb_split", {
        {"amount", "u_rgb_split", 0.0f},
        {"angle",  "u_rgb_angle", 0.0f}
    }});

    registerEffect({"Block Glitch", "glitch", "block_glitch", {
        {"intensity",  "u_block_glitch_int",  0.0f},
        {"block_size", "u_block_glitch_size", 0.5f}
    }});

    registerEffect({"Scanlines", "glitch", "scanlines", {
        {"intensity", "u_scanline_int",  0.0f},
        {"frequency", "u_scanline_freq", 0.5f}
    }});

    // === Blur/Post Effects ===

    registerEffect({"Gaussian Blur", "blur", "gaussian_blur", {
        {"radius", "u_blur_radius", 0.0f}
    }});

    registerEffect({"Zoom Blur", "blur", "zoom_blur", {
        {"amount",   "u_zoom_blur",       0.0f},
        {"center_x", "u_zoom_center_x",   0.5f},
        {"center_y", "u_zoom_center_y",   0.5f}
    }});

    registerEffect({"Shake", "blur", "shake", {
        {"amount_x", "u_shake_x", 0.0f},
        {"amount_y", "u_shake_y", 0.0f}
    }});

    registerEffect({"Vignette", "blur", "vignette", {
        {"intensity", "u_vignette_int",  0.0f},
        {"softness",  "u_vignette_soft", 0.6f}
    }});

    // === Additional Warp Effects ===

    registerEffect({"Kaleidoscope", "warp", "kaleidoscope", {
        {"segments", "u_kaleidoscope_segments", 0.3f},
        {"rotation", "u_kaleidoscope_rotation", 0.0f}
    }});

    registerEffect({"Fisheye", "warp", "fisheye", {
        {"amount", "u_fisheye_amount", 0.5f}
    }});

    registerEffect({"Swirl", "warp", "swirl", {
        {"amount", "u_swirl_amount", 0.0f},
        {"radius", "u_swirl_radius", 0.5f}
    }});

    // === Additional Color Effects ===

    registerEffect({"Invert", "color", "invert", {
        {"amount", "u_invert_amount", 0.0f}
    }});

    registerEffect({"Posterize", "color", "posterize", {
        {"levels", "u_posterize_levels", 1.0f}
    }});

    registerEffect({"Color Shift", "color", "color_shift", {
        {"red",   "u_color_shift_r", 0.5f},
        {"green", "u_color_shift_g", 0.5f},
        {"blue",  "u_color_shift_b", 0.5f}
    }});

    registerEffect({"Thermal", "color", "thermal", {
        {"amount", "u_thermal_amount", 0.0f}
    }});

    registerEffect({"Contrast", "color", "color_matrix", {
        {"contrast", "u_contrast",      0.5f},
        {"balance",  "u_color_balance", 0.5f}
    }});

    // === Additional Glitch Effects ===

    registerEffect({"Digital Rain", "glitch", "digital_rain", {
        {"intensity", "u_rain_intensity", 0.0f},
        {"speed",     "u_rain_speed",     0.5f}
    }});

    registerEffect({"Noise", "glitch", "noise_overlay", {
        {"amount", "u_noise_amount", 0.0f},
        {"speed",  "u_noise_speed",  0.5f}
    }});

    registerEffect({"Mirror", "glitch", "mirror", {
        {"horizontal", "u_mirror_x", 0.0f},
        {"vertical",   "u_mirror_y", 0.0f}
    }});

    registerEffect({"Pixelate", "glitch", "pixelate", {
        {"size", "u_pixelate_size", 0.0f}
    }});

    // === Additional Blur/Post Effects ===

    registerEffect({"Motion Blur", "blur", "motion_blur", {
        {"amount", "u_motion_blur_amount", 0.0f},
        {"angle",  "u_motion_blur_angle",  0.0f}
    }});

    registerEffect({"Glow", "blur", "glow", {
        {"amount",    "u_glow_amount",    0.0f},
        {"threshold", "u_glow_threshold", 0.5f}
    }});

    registerEffect({"Edge Detect", "blur", "edge_detect", {
        {"amount", "u_edge_amount", 0.0f}
    }});

    // ============================================================
    // 3D / Depth Effects
    // ============================================================

    registerEffect({"Perspective Tilt", "3d", "perspective_tilt", {
        {"tilt_x", "u_tilt_x", 0.5f},
        {"tilt_y", "u_tilt_y", 0.5f}
    }});

    registerEffect({"Cylinder Wrap", "3d", "cylinder_wrap", {
        {"amount", "u_cylinder_amount", 0.0f},
        {"axis",   "u_cylinder_axis",   0.0f}
    }});

    registerEffect({"Sphere Wrap", "3d", "sphere_wrap", {
        {"amount", "u_sphere_amount", 0.0f}
    }});

    registerEffect({"Tunnel", "3d", "tunnel", {
        {"speed",  "u_tunnel_speed",  0.5f},
        {"radius", "u_tunnel_radius", 0.3f}
    }});

    registerEffect({"Page Curl", "3d", "page_curl", {
        {"amount", "u_curl_amount", 0.0f},
        {"radius", "u_curl_radius", 0.5f}
    }});

    registerEffect({"Parallax Layers", "3d", "parallax_layers", {
        {"amount",    "u_parallax_amount",    0.0f},
        {"direction", "u_parallax_direction", 0.0f}
    }});

    // ============================================================
    // Additional Warp Effects
    // ============================================================

    registerEffect({"Polar Coords", "warp", "polar_coords", {
        {"amount", "u_polar_amount", 0.0f}
    }});

    registerEffect({"Twirl", "warp", "twirl", {
        {"amount", "u_twirl_amount", 0.0f},
        {"radius", "u_twirl_radius", 0.5f}
    }});

    registerEffect({"Shear", "warp", "shear", {
        {"x", "u_shear_x", 0.5f},
        {"y", "u_shear_y", 0.5f}
    }});

    registerEffect({"Elastic Bounce", "warp", "elastic_bounce", {
        {"amount", "u_elastic_amount", 0.0f},
        {"freq",   "u_elastic_freq",   0.5f}
    }});

    registerEffect({"Ripple Pond", "warp", "ripple_pond", {
        {"intensity", "u_pond_intensity", 0.0f},
        {"freq",      "u_pond_freq",      0.5f}
    }});

    registerEffect({"Diamond Distort", "warp", "diamond_distort", {
        {"size",   "u_diamond_size",   0.5f},
        {"amount", "u_diamond_amount", 0.0f}
    }});

    registerEffect({"Barrel Distort", "warp", "barrel_distort", {
        {"amount", "u_barrel_amount", 0.5f}
    }});

    registerEffect({"Sine Grid", "warp", "sine_grid", {
        {"freq",   "u_sinegrid_freq",   0.5f},
        {"amount", "u_sinegrid_amount", 0.0f}
    }});

    registerEffect({"Glitch Displace", "warp", "glitch_displace", {
        {"amount", "u_glitchdisp_amount", 0.0f},
        {"speed",  "u_glitchdisp_speed",  0.5f}
    }});

    // ============================================================
    // Additional Color Effects
    // ============================================================

    registerEffect({"Sepia", "color", "sepia", {
        {"amount", "u_sepia_amount", 0.0f}
    }});

    registerEffect({"Cross Process", "color", "cross_process", {
        {"amount", "u_crossprocess_amount", 0.0f}
    }});

    registerEffect({"Split Tone", "color", "split_tone", {
        {"shadow_hue",    "u_splittone_shadow_hue",    0.6f},
        {"highlight_hue", "u_splittone_highlight_hue", 0.1f},
        {"amount",        "u_splittone_amount",        0.0f}
    }});

    registerEffect({"Color Halftone", "color", "color_halftone", {
        {"scale",  "u_halftone_scale",  0.5f},
        {"amount", "u_halftone_amount", 0.0f}
    }});

    registerEffect({"Dither", "color", "ordered_dither", {
        {"levels", "u_dither_levels", 0.5f},
        {"amount", "u_dither_amount", 0.0f}
    }});

    registerEffect({"Heat Map", "color", "heat_map", {
        {"amount", "u_heatmap_amount", 0.0f}
    }});

    registerEffect({"Selective Color", "color", "selective_color", {
        {"hue",   "u_selectcolor_hue",   0.0f},
        {"range", "u_selectcolor_range", 0.2f}
    }});

    registerEffect({"Film Grain", "color", "film_grain", {
        {"amount", "u_grain_amount", 0.0f},
        {"size",   "u_grain_size",   0.5f}
    }});

    registerEffect({"Gamma Levels", "color", "gamma_levels", {
        {"black", "u_levels_black", 0.0f},
        {"white", "u_levels_white", 1.0f},
        {"gamma", "u_levels_gamma", 0.5f}
    }});

    registerEffect({"Solarize", "color", "solarize", {
        {"threshold", "u_solarize_threshold", 0.5f},
        {"amount",    "u_solarize_amount",    0.0f}
    }});

    // ============================================================
    // Pattern / Stylization Effects
    // ============================================================

    registerEffect({"CRT", "pattern", "crt_simulation", {
        {"curvature", "u_crt_curvature", 0.3f},
        {"scanline",  "u_crt_scanline",  0.5f}
    }});

    registerEffect({"VHS", "pattern", "vhs_effect", {
        {"amount",   "u_vhs_amount",   0.0f},
        {"tracking", "u_vhs_tracking", 0.3f}
    }});

    registerEffect({"ASCII Art", "pattern", "ascii_art", {
        {"scale", "u_ascii_scale", 0.5f},
        {"color", "u_ascii_color", 0.5f}
    }});

    registerEffect({"Dot Matrix", "pattern", "dot_matrix", {
        {"scale",  "u_dotmatrix_scale",  0.5f},
        {"amount", "u_dotmatrix_amount", 0.0f}
    }});

    registerEffect({"Crosshatch", "pattern", "crosshatch", {
        {"density", "u_crosshatch_density", 0.5f},
        {"amount",  "u_crosshatch_amount",  0.0f}
    }});

    registerEffect({"Emboss", "pattern", "emboss", {
        {"amount", "u_emboss_amount", 0.0f},
        {"angle",  "u_emboss_angle",  0.0f}
    }});

    registerEffect({"Oil Paint", "pattern", "oil_paint", {
        {"radius", "u_oilpaint_radius", 0.3f}
    }});

    registerEffect({"Pencil Sketch", "pattern", "pencil_sketch", {
        {"amount",  "u_sketch_amount",  0.0f},
        {"density", "u_sketch_density", 0.5f}
    }});

    registerEffect({"Voronoi Glass", "pattern", "voronoi_glass", {
        {"scale", "u_voronoi_scale", 0.5f},
        {"edge",  "u_voronoi_edge",  0.3f}
    }});

    registerEffect({"Cross Stitch", "pattern", "cross_stitch", {
        {"scale",  "u_stitch_scale",  0.5f},
        {"amount", "u_stitch_amount", 0.0f}
    }});

    registerEffect({"Night Vision", "pattern", "night_vision", {
        {"amount", "u_nightvision_amount", 0.0f}
    }});

    // ============================================================
    // Animation Effects
    // ============================================================

    registerEffect({"Strobe", "animation", "strobe", {
        {"rate",      "u_strobe_rate",      0.5f},
        {"intensity", "u_strobe_intensity", 0.0f}
    }});

    registerEffect({"Pulse", "animation", "pulse", {
        {"amount", "u_pulse_amount", 0.0f},
        {"speed",  "u_pulse_speed",  0.5f}
    }});

    registerEffect({"Slit Scan", "animation", "slit_scan", {
        {"amount",    "u_slitscan_amount",    0.0f},
        {"direction", "u_slitscan_direction", 0.0f}
    }});

    // ============================================================
    // Blend / Composite Effects
    // ============================================================

    registerEffect({"Double Exposure", "blend", "double_exposure", {
        {"offset", "u_double_offset", 0.3f},
        {"blend",  "u_double_blend",  0.0f}
    }});

    registerEffect({"Frosted Glass", "blend", "frosted_glass", {
        {"amount", "u_frost_amount", 0.0f},
        {"scale",  "u_frost_scale",  0.5f}
    }});

    registerEffect({"Prism", "blend", "prism_refract", {
        {"amount", "u_prism_amount", 0.0f},
        {"angle",  "u_prism_angle",  0.0f}
    }});

    registerEffect({"Rain on Glass", "blend", "rain_on_glass", {
        {"amount", "u_raindrop_amount", 0.0f},
        {"speed",  "u_raindrop_speed",  0.5f}
    }});

    registerEffect({"Hexagonalize", "blend", "hexagonalize", {
        {"scale", "u_hex_scale", 0.0f}
    }});

    // ============================================================
    // Phase 14: Quick-Win Effects (20 new effects)
    // ============================================================

    registerEffect({"Greyscale", "color", "greyscale", {
        {"method", "u_grey_method", 0.0f},
        {"amount", "u_grey_amount", 0.0f}
    }});

    registerEffect({"Threshold", "color", "threshold", {
        {"level", "u_threshold_level", 0.5f},
        {"amount", "u_threshold_amount", 0.0f}
    }});

    registerEffect({"Exposure", "color", "exposure", {
        {"amount", "u_exposure_amount", 0.5f}
    }});

    registerEffect({"Vibrance", "color", "vibrance", {
        {"amount", "u_vibrance_amount", 0.5f}
    }});

    registerEffect({"Quad Mirror", "warp", "quad_mirror", {
        {"center x", "u_quadmir_cx", 0.5f},
        {"center y", "u_quadmir_cy", 0.5f}
    }});

    registerEffect({"Flip", "warp", "flip", {
        {"horizontal", "u_flip_h", 0.0f},
        {"vertical", "u_flip_v", 0.0f}
    }});

    registerEffect({"Warp Field", "warp", "warp_field", {
        {"amount", "u_warpfield_amount", 0.0f},
        {"frequency", "u_warpfield_freq", 0.5f},
        {"speed", "u_warpfield_speed", 0.3f}
    }});

    registerEffect({"Sharpen", "blur", "sharpen", {
        {"amount", "u_sharpen_amount", 0.0f},
        {"radius", "u_sharpen_radius", 0.3f}
    }});

    registerEffect({"Pixel Explosion", "glitch", "pixel_explosion", {
        {"force", "u_explode_force", 0.0f},
        {"decay", "u_explode_decay", 0.5f},
        {"center x", "u_explode_cx", 0.5f},
        {"center y", "u_explode_cy", 0.5f}
    }});

    registerEffect({"Color Flash", "glitch", "color_flash", {
        {"intensity", "u_flash_intensity", 0.0f},
        {"red", "u_flash_r", 1.0f},
        {"green", "u_flash_g", 1.0f},
        {"blue", "u_flash_b", 1.0f},
        {"decay", "u_flash_decay", 0.5f}
    }});

    registerEffect({"Slide Wrap", "warp", "slide_wrap", {
        {"x", "u_slide_x", 0.5f},
        {"y", "u_slide_y", 0.5f}
    }});

    registerEffect({"Dot Field", "3d", "dot_field", {
        {"size", "u_dotfield_size", 0.3f},
        {"spacing", "u_dotfield_spacing", 0.5f},
        {"depth", "u_dotfield_depth", 0.0f}
    }});

    registerEffect({"Triangulate", "pattern", "triangulate", {
        {"size", "u_tri_size", 0.3f},
        {"amount", "u_tri_amount", 0.0f}
    }});

    registerEffect({"Auto Mask", "color", "auto_mask", {
        {"threshold", "u_automask_threshold", 0.5f},
        {"softness", "u_automask_softness", 0.3f},
        {"invert", "u_automask_invert", 0.0f}
    }});

    registerEffect({"Chroma Key", "color", "chromakey_effect", {
        {"hue", "u_chromakey_hue", 0.33f},
        {"tolerance", "u_chromakey_tolerance", 0.3f},
        {"softness", "u_chromakey_softness", 0.3f},
        {"amount", "u_chromakey_amount", 0.0f}
    }});

    registerEffect({"Tile Grid", "warp", "tile_grid", {
        {"columns", "u_tilegrid_cols", 0.25f},
        {"rows", "u_tilegrid_rows", 0.25f},
        {"offset", "u_tilegrid_offset", 0.0f},
        {"zoom", "u_tilegrid_zoom", 0.5f}
    }});

    registerEffect({"Spot Zoom", "warp", "spot_zoom", {
        {"center x", "u_spotzoom_cx", 0.5f},
        {"center y", "u_spotzoom_cy", 0.5f},
        {"size", "u_spotzoom_size", 0.3f},
        {"zoom", "u_spotzoom_zoom", 0.7f},
        {"shape", "u_spotzoom_shape", 0.0f},
        {"background", "u_spotzoom_bg", 0.3f}
    }});

    registerEffect({"Neon Edge", "pattern", "neon_edge", {
        {"edge", "u_neonedge_edge", 0.6f},
        {"glow", "u_neonedge_glow", 0.5f},
        {"hue", "u_neonedge_hue", 0.5f},
        {"original", "u_neonedge_original", 0.3f}
    }});

    registerEffect({"Cartoon Ink", "pattern", "cartoon_ink", {
        {"edge width", "u_cartoonink_edge", 0.4f},
        {"color steps", "u_cartoonink_steps", 0.4f},
        {"ink strength", "u_cartoonink_ink", 0.7f},
        {"saturation", "u_cartoonink_sat", 0.6f}
    }});

    registerEffect({"Pop Raster", "pattern", "pop_raster", {
        {"palette", "u_popraster_palette", 0.0f},
        {"bands", "u_popraster_bands", 0.4f},
        {"pattern size", "u_popraster_size", 0.3f},
        {"mix", "u_popraster_mix", 0.7f}
    }});

    // ============================================================
    // Phase 15: Medium Effects (14 new effects)
    // ============================================================

    registerEffect({"Palette Remap", "color", "palette_remap", {
        {"palette", "u_palette_index", 0.0f},
        {"cycle", "u_palette_cycle", 0.0f},
        {"amount", "u_palette_amount", 0.0f}
    }});

    registerEffect({"Color Grade", "color", "lut_grade", {
        {"amount", "u_lut_amount", 0.0f}
    }});

    registerEffect({"Bendoscope", "warp", "bendoscope", {
        {"divisions", "u_bendo_divisions", 0.3f},
        {"bend", "u_bendo_bend", 0.5f},
        {"rotation", "u_bendo_rotation", 0.0f}
    }});

    registerEffect({"UV Remap", "warp", "uv_remap", {
        {"amount", "u_uvremap_amount", 0.0f},
        {"scale", "u_uvremap_scale", 0.5f},
        {"speed", "u_uvremap_speed", 0.3f}
    }});

    registerEffect({"Liquid Morph", "warp", "liquid_morph", {
        {"viscosity", "u_goo_viscosity", 0.5f},
        {"amount", "u_goo_amount", 0.0f},
        {"scale", "u_goo_scale", 0.5f}
    }});

    registerEffect({"Edge Blur", "blur", "edge_blur", {
        {"threshold", "u_edgeblur_threshold", 0.5f},
        {"amount", "u_edgeblur_amount", 0.0f}
    }});

    registerEffect({"Brush Strokes", "pattern", "brush_strokes", {
        {"size", "u_brush_size", 0.4f},
        {"angle", "u_brush_angle", 0.0f},
        {"flow", "u_brush_flow", 0.5f},
        {"amount", "u_brush_amount", 0.0f}
    }});

    registerEffect({"Fragment Burst", "glitch", "fragment_burst", {
        {"copies", "u_frag_copies", 0.3f},
        {"spread", "u_frag_spread", 0.3f},
        {"rotation", "u_frag_rotation", 0.2f},
        {"scale", "u_frag_scale", 0.5f}
    }});

    registerEffect({"Signal Destroy", "glitch", "signal_destroy", {
        {"amount", "u_destroy_amount", 0.0f},
        {"speed", "u_destroy_speed", 0.5f},
        {"mode", "u_destroy_mode", 0.0f}
    }});

    registerEffect({"Line Cloner", "composite", "line_cloner", {
        {"copies", "u_lineclone_copies", 0.3f},
        {"offset x", "u_lineclone_ox", 0.8f},
        {"offset y", "u_lineclone_oy", 0.5f},
        {"scale", "u_lineclone_scale", 0.4f},
        {"rotation", "u_lineclone_rotation", 0.5f}
    }});

    registerEffect({"Radial Cloner", "composite", "radial_cloner", {
        {"copies", "u_radclone_copies", 0.3f},
        {"radius", "u_radclone_radius", 0.3f},
        {"rotation", "u_radclone_rotation", 0.0f},
        {"scale", "u_radclone_scale", 0.5f}
    }});

    registerEffect({"Cube Scatter", "composite", "cube_scatter", {
        {"grid x", "u_cubescat_gx", 0.3f},
        {"grid y", "u_cubescat_gy", 0.3f},
        {"explode", "u_cubescat_explode", 0.0f},
        {"rotation", "u_cubescat_rotation", 0.0f}
    }});

    registerEffect({"Zoom Warp", "warp", "infinite_zoom", {
        {"speed", "u_infzoom_speed", 0.3f},
        {"rotation", "u_infzoom_rotation", 0.5f},
        {"center x", "u_infzoom_cx", 0.5f},
        {"center y", "u_infzoom_cy", 0.5f}
    }});

    registerEffect({"Bump Light", "pattern", "bump_light", {
        {"light x", "u_bumplight_lx", 0.5f},
        {"light y", "u_bumplight_ly", 0.3f},
        {"intensity", "u_bumplight_intensity", 0.6f},
        {"height", "u_bumplight_height", 0.5f}
    }});

    // ============================================================
    // Time Effects (temporal = true — use u_prev_frame)
    // ============================================================

    registerEffect({"Echo", "time", "ghost_trails", {
        {"decay",    "u_trail_length", 0.0f},
        {"operator", "u_trail_fade",   0.0f}
    }, true});

    registerEffect({"Posterize Time", "time", "frame_hold", {
        {"frame rate", "u_hold_rate",   0.0f},
        {"amount",     "u_hold_amount", 1.0f}
    }, true});

    registerEffect({"Freeze", "time", "time_freeze", {
        {"amount", "u_freeze_amount", 1.0f}
    }, true});

    registerEffect({"Screen Split", "time", "screen_split", {
        {"columns",         "u_screensplit_cols",  0.0f},
        {"rows",            "u_screensplit_rows",  0.0f},
        {"frames per cell", "u_screensplit_delay", 0.25f},
        {"direction",       "u_screensplit_mode",  0.0f}
    }, false});

    registerEffect({"Frame Stutter", "time", "frame_delay", {
        {"depth",   "u_delay_depth",   0.3f},
        {"stutter", "u_delay_stutter", 0.0f}
    }, false});

    // === Feedback Effect ===

    registerEffect({"Point Zoom", "animation", "feedback", {
        {"amount",     "u_feedback_amount",     0.7f},
        {"zoom",       "u_feedback_zoom",       0.52f},
        {"rotation",   "u_feedback_rotation",   0.52f},
        {"x offset",   "u_feedback_x_offset",   0.5f},
        {"y offset",   "u_feedback_y_offset",   0.5f},
        {"decay",      "u_feedback_decay",       0.7f},
        {"hue shift",  "u_feedback_hue_shift",   0.0f},
        {"saturation", "u_feedback_saturation",  0.5f}
    }});

    registerEffect({"Directional Feedback", "animation", "directional_feedback", {
        {"amount",     "u_dfb_amount",     0.7f},
        {"speed",      "u_dfb_speed",      0.5f},
        {"direction",  "u_dfb_direction",  0.5f},
        {"spread",     "u_dfb_spread",     0.3f},
        {"decay",      "u_dfb_decay",      0.7f},
        {"hue shift",  "u_dfb_hue_shift",  0.0f}
    }});

    // ============================================================
    // Phase 18: Audio-Native Effects (10 effects)
    // These effects use extended audio uniforms — our differentiator.
    // ============================================================

    registerEffect({"Harmonic Displacement", "audio", "harmonic_displace", {
        {"amount",     "u_harmdisplace_amount",  0.5f},
        {"smoothing",  "u_harmdisplace_smooth",  0.5f},
        {"color mode", "u_harmdisplace_color",   0.0f}
    }});

    registerEffect({"Timbral Mosaic", "audio", "timbral_mosaic", {
        {"amount",     "u_timbremosaic_amount",     0.5f},
        {"base size",  "u_timbremosaic_size",       0.5f},
        {"complexity", "u_timbremosaic_complexity", 0.5f}
    }});

    registerEffect({"Structural Morph", "audio", "structural_morph", {
        {"intensity",   "u_structmorph_intensity", 0.5f},
        {"normal style", "u_structmorph_normal",   0.3f},
        {"drop style",  "u_structmorph_drop",      0.5f}
    }});

    registerEffect({"Pitch Chromatic Shift", "color", "pitch_chroma_shift", {
        {"amount",     "u_pitchcolor_amount", 0.5f},
        {"mode",       "u_pitchcolor_mode",   0.0f},
        {"saturation", "u_pitchcolor_sat",    0.5f}
    }});

    registerEffect({"Key Palette", "color", "key_palette", {
        {"amount",     "u_keypalette_amount", 0.5f},
        {"brightness", "u_keypalette_bright", 0.5f},
        {"saturation", "u_keypalette_sat",    0.5f}
    }});

    registerEffect({"Transient Flash", "animation", "transient_flash", {
        {"style",     "u_transflash_style",     0.0f},
        {"intensity", "u_transflash_intensity", 0.5f},
        {"decay",     "u_transflash_decay",     0.5f}
    }});

    registerEffect({"Beat Ripple", "audio", "beat_ripple", {
        {"intensity", "u_beatripple_intensity", 0.5f},
        {"decay",     "u_beatripple_decay",     0.5f},
        {"count",     "u_beatripple_count",     0.3f}
    }});

    registerEffect({"Rhythm Slice", "glitch", "rhythm_slice", {
        {"amount", "u_rhythmslice_amount", 0.5f},
        {"slices", "u_rhythmslice_count",  0.5f},
        {"sync",   "u_rhythmslice_sync",   0.5f}
    }});

    registerEffect({"Density Wave", "warp", "density_wave", {
        {"amount",     "u_densitywave_amount", 0.5f},
        {"direction",  "u_densitywave_dir",    0.0f},
        {"wavelength", "u_densitywave_wl",     0.5f}
    }});

    registerEffect({"Chroma Dissolve", "color", "chroma_dissolve", {
        {"amount",   "u_chromadiss_amount",   0.5f},
        {"softness", "u_chromadiss_softness", 0.5f}
    }});

    // ============================================================
    // Phase 19: Complex Effects (8 effects)
    // ============================================================

    registerEffect({"Luminance Terrain", "3d", "luma_terrain", {
        {"height",   "u_lumaterrain_height",   0.5f},
        {"segments", "u_lumaterrain_segments", 0.5f},
        {"angle",    "u_lumaterrain_angle",    0.3f}
    }});

    registerEffect({"Voxel Matrix", "3d", "voxel_matrix", {
        {"size",     "u_voxel_size",     0.3f},
        {"height",   "u_voxel_height",   0.5f},
        {"rotation", "u_voxel_rotation", 0.0f}
    }});

    registerEffect({"Monitor Wall", "pattern", "monitor_wall", {
        {"columns", "u_monwall_cols",   0.3f},
        {"rows",    "u_monwall_rows",   0.3f},
        {"border",  "u_monwall_border", 0.3f},
        {"glow",    "u_monwall_glow",   0.3f}
    }});

    registerEffect({"Drop Shadow", "blur", "drop_shadow", {
        {"offset x", "u_shadow_ox",      0.6f},
        {"offset y", "u_shadow_oy",      0.4f},
        {"blur",     "u_shadow_blur",    0.3f},
        {"opacity",  "u_shadow_opacity", 0.5f}
    }});

    registerEffect({"Channel Delay", "time", "channel_delay", {
        {"red delay",   "u_delay_r", 0.0f},
        {"green delay", "u_delay_g", 0.0f},
        {"blue delay",  "u_delay_b", 0.0f}
    }, true});

    registerEffect({"Topographic Lines", "pattern", "topo_lines", {
        {"amount",    "u_topo_amount",    0.5f},
        {"levels",    "u_topo_levels",    0.5f},
        {"thickness", "u_topo_thickness", 0.3f},
        {"color mode","u_topo_color",     0.0f}
    }});

    registerEffect({"Data Corruption", "glitch", "data_corrupt", {
        {"amount",       "u_datacorrupt_amount", 0.5f},
        {"block size",   "u_datacorrupt_block",  0.5f},
        {"color damage", "u_datacorrupt_color",  0.5f}
    }});

    registerEffect({"Glitch Sort", "glitch", "glitch_sort", {
        {"amount",    "u_glitchsort_amount",    0.5f},
        {"threshold", "u_glitchsort_threshold", 0.5f},
        {"direction", "u_glitchsort_dir",       0.0f}
    }});
}

std::unique_ptr<Effect> EffectLibrary::createEffect(const juce::String& name) const
{
    const auto* def = getEffectDef(name);
    if (def == nullptr)
        return nullptr;

    auto effect = std::make_unique<Effect>(def->name, def->category, def->shaderName);
    effect->setTemporal(def->temporal);
    for (const auto& p : def->params)
        effect->addParam(p.name, p.uniformName, p.defaultValue);

    return effect;
}

juce::StringArray EffectLibrary::getEffectNames() const
{
    juce::StringArray names;
    for (const auto& def : defs_)
        names.add(def.name);
    return names;
}

juce::StringArray EffectLibrary::getEffectsByCategory(const juce::String& category) const
{
    juce::StringArray names;
    for (const auto& def : defs_)
    {
        if (def.category == category)
            names.add(def.name);
    }
    return names;
}

const EffectLibrary::EffectDef* EffectLibrary::getEffectDef(const juce::String& name) const
{
    for (const auto& def : defs_)
    {
        if (def.name == name)
            return &def;
    }
    return nullptr;
}
