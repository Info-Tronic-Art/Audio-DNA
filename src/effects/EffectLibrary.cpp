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
