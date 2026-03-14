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
}

std::unique_ptr<Effect> EffectLibrary::createEffect(const juce::String& name) const
{
    const auto* def = getEffectDef(name);
    if (def == nullptr)
        return nullptr;

    auto effect = std::make_unique<Effect>(def->name, def->category, def->shaderName);
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
