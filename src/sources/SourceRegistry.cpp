#include "sources/SourceRegistry.h"

SourceRegistry::SourceRegistry()
{
    registerDefaults();
}

void SourceRegistry::registerSource(const std::string& id, SourceFactory factory)
{
    // Create a temporary instance to get display name and category
    auto temp = factory();
    SourceInfo info;
    info.factory = std::move(factory);
    info.displayName = temp->getDisplayName();
    info.category = temp->getCategory();
    registry_[id] = std::move(info);
}

void SourceRegistry::registerDefaults()
{
    // Perlin Noise
    registerSource("perlin_noise", [] {
        auto s = std::make_unique<ProceduralSource>("perlin_noise", "Perlin Noise", "Noise", "source_perlin_noise");
        s->addParam("Scale", "u_src_scale", 0.5f);
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Octaves", "u_src_octaves", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // Plasma
    registerSource("plasma", [] {
        auto s = std::make_unique<ProceduralSource>("plasma", "Plasma", "Noise", "source_plasma");
        s->addParam("Speed", "u_src_speed", 0.4f);
        s->addParam("Complexity", "u_src_complexity", 0.5f);
        s->addParam("Color Cycle", "u_src_color_cycle", 0.0f);
        s->addParam("Intensity", "u_src_intensity", 0.7f);
        return s;
    });

    // Voronoi
    registerSource("voronoi", [] {
        auto s = std::make_unique<ProceduralSource>("voronoi", "Voronoi", "Noise", "source_voronoi");
        s->addParam("Scale", "u_src_scale", 0.4f);
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Edge Width", "u_src_edge_width", 0.3f);
        s->addParam("Color Mode", "u_src_color_mode", 0.0f);
        return s;
    });

    // Kaleidoscopic Fractal
    registerSource("kaleido_fractal", [] {
        auto s = std::make_unique<ProceduralSource>("kaleido_fractal", "Kaleidoscopic Fractal", "Fractal", "source_kaleido_fractal");
        s->addParam("Iterations", "u_src_iterations", 0.5f);
        s->addParam("Fold Angle", "u_src_fold_angle", 0.4f);
        s->addParam("Zoom", "u_src_zoom", 0.5f);
        s->addParam("Rotation", "u_src_rotation", 0.0f);
        return s;
    });

    // Mandelbrot / Julia
    registerSource("mandelbrot", [] {
        auto s = std::make_unique<ProceduralSource>("mandelbrot", "Mandelbrot / Julia", "Fractal", "source_mandelbrot");
        s->addParam("Zoom", "u_src_zoom", 0.3f);
        s->addParam("Center X", "u_src_center_x", 0.5f);
        s->addParam("Center Y", "u_src_center_y", 0.5f);
        s->addParam("Julia Mix", "u_src_julia_mix", 0.0f);
        s->addParam("Max Iterations", "u_src_max_iter", 0.3f);
        return s;
    });

    // Geometric Tunnel
    registerSource("geometric_tunnel", [] {
        auto s = std::make_unique<ProceduralSource>("geometric_tunnel", "Geometric Tunnel", "Geometric", "source_geometric_tunnel");
        s->addParam("Speed", "u_src_speed", 0.4f);
        s->addParam("Segments", "u_src_segments", 0.3f);
        s->addParam("Twist", "u_src_twist", 0.2f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // Color Gradient
    registerSource("color_gradient", [] {
        auto s = std::make_unique<ProceduralSource>("color_gradient", "Color Gradient", "Geometric", "source_color_gradient");
        s->addParam("Angle", "u_src_angle", 0.25f);
        s->addParam("Speed", "u_src_speed", 0.2f);
        s->addParam("Color 1", "u_src_color1", 0.0f);
        s->addParam("Color 2", "u_src_color2", 0.5f);
        return s;
    });

    // Audio Waveform
    registerSource("audio_waveform", [] {
        auto s = std::make_unique<ProceduralSource>("audio_waveform", "Audio Waveform", "Audio-Visual", "source_audio_waveform");
        s->addParam("Style", "u_src_style", 0.0f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Glow", "u_src_glow", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.5f);
        return s;
    });

    // Reaction-Diffusion (stateful — needs ping-pong FBOs)
    registerSource("reaction_diffusion", [] {
        auto s = std::make_unique<ProceduralSource>("reaction_diffusion", "Reaction-Diffusion", "Nature", "source_reaction_diffusion", true);
        s->addParam("Feed Rate", "u_src_feed", 0.4f);
        s->addParam("Kill Rate", "u_src_kill", 0.4f);
        s->addParam("Diffusion A", "u_src_diffusion_a", 0.5f);
        s->addParam("Diffusion B", "u_src_diffusion_b", 0.5f);
        return s;
    });

    // Cellular Automata (stateful — needs ping-pong FBOs)
    registerSource("cellular_automata", [] {
        auto s = std::make_unique<ProceduralSource>("cellular_automata", "Cellular Automata", "Nature", "source_cellular_automata", true);
        s->addParam("Rule Threshold", "u_src_rule_threshold", 0.5f);
        s->addParam("Birth Low", "u_src_birth_low", 0.25f);
        s->addParam("Birth High", "u_src_birth_high", 0.25f);
        s->addParam("Survival Low", "u_src_survival_low", 0.25f);
        return s;
    });

    // ============================================================
    // Phase 15: Resolume Sources (12 new sources)
    // ============================================================

    // Solid Color
    registerSource("solid_color", [] {
        auto s = std::make_unique<ProceduralSource>("solid_color", "Solid Color", "Utility", "source_solid_color");
        s->addParam("Red", "u_src_red", 1.0f);
        s->addParam("Green", "u_src_green", 1.0f);
        s->addParam("Blue", "u_src_blue", 1.0f);
        return s;
    });

    // Strobe Light
    registerSource("strobe_light", [] {
        auto s = std::make_unique<ProceduralSource>("strobe_light", "Strobe Light", "Utility", "source_strobe_light");
        s->addParam("Frequency", "u_src_frequency", 0.5f);
        s->addParam("Fade", "u_src_fade", 0.3f);
        s->addParam("Color 1 Red", "u_src_c1r", 1.0f);
        s->addParam("Color 1 Green", "u_src_c1g", 1.0f);
        s->addParam("Color 1 Blue", "u_src_c1b", 1.0f);
        s->addParam("Color 2 Red", "u_src_c2r", 0.0f);
        s->addParam("Color 2 Green", "u_src_c2g", 0.0f);
        s->addParam("Color 2 Blue", "u_src_c2b", 0.0f);
        return s;
    });

    // Checkerboard
    registerSource("checkerboard", [] {
        auto s = std::make_unique<ProceduralSource>("checkerboard", "Checkerboard", "Pattern", "source_checkerboard");
        s->addParam("Columns", "u_src_columns", 0.3f);
        s->addParam("Rows", "u_src_rows", 0.3f);
        s->addParam("Color 1 Hue", "u_src_c1_hue", 0.0f);
        s->addParam("Color 2 Hue", "u_src_c2_hue", 0.0f);
        return s;
    });

    // Line Pattern
    registerSource("line_pattern", [] {
        auto s = std::make_unique<ProceduralSource>("line_pattern", "Line Pattern", "Pattern", "source_line_pattern");
        s->addParam("Count", "u_src_count", 0.3f);
        s->addParam("Width", "u_src_width", 0.3f);
        s->addParam("Rotation", "u_src_rotation", 0.0f);
        s->addParam("Speed", "u_src_speed", 0.2f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // Concentric Rings
    registerSource("concentric_rings", [] {
        auto s = std::make_unique<ProceduralSource>("concentric_rings", "Concentric Rings", "Pattern", "source_concentric_rings");
        s->addParam("Count", "u_src_count", 0.3f);
        s->addParam("Spacing", "u_src_spacing", 0.5f);
        s->addParam("Width", "u_src_width", 0.3f);
        s->addParam("Rotation", "u_src_rotation", 0.0f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // Sine Oscillator
    registerSource("sine_oscillator", [] {
        auto s = std::make_unique<ProceduralSource>("sine_oscillator", "Sine Oscillator", "Pattern", "source_sine_oscillator");
        s->addParam("Waves", "u_src_waves", 0.3f);
        s->addParam("Frequency", "u_src_frequency", 0.5f);
        s->addParam("Amplitude", "u_src_amplitude", 0.5f);
        s->addParam("Modulation", "u_src_modulation", 0.0f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // Spiral Pattern
    registerSource("spiral_pattern", [] {
        auto s = std::make_unique<ProceduralSource>("spiral_pattern", "Spiral Pattern", "Pattern", "source_spiral_pattern");
        s->addParam("Arms", "u_src_arms", 0.3f);
        s->addParam("Zoom", "u_src_zoom", 0.5f);
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Distortion", "u_src_distortion", 0.0f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // Metaballs
    registerSource("metaballs", [] {
        auto s = std::make_unique<ProceduralSource>("metaballs", "Metaballs", "Organic", "source_metaballs");
        s->addParam("Count", "u_src_count", 0.3f);
        s->addParam("Size", "u_src_size", 0.4f);
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Blend", "u_src_blend", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // Terrain Lines
    registerSource("terrain_lines", [] {
        auto s = std::make_unique<ProceduralSource>("terrain_lines", "Terrain Lines", "Pattern", "source_terrain_lines");
        s->addParam("Height", "u_src_height", 0.4f);
        s->addParam("Lines", "u_src_lines", 0.4f);
        s->addParam("Jagginess", "u_src_jagginess", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.2f);
        s->addParam("Tilt", "u_src_tilt", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // Shape Generator
    registerSource("shape_generator", [] {
        auto s = std::make_unique<ProceduralSource>("shape_generator", "Shape Generator", "Geometric", "source_shape_generator");
        s->addParam("Shape", "u_src_shape", 0.0f);
        s->addParam("Size", "u_src_size", 0.5f);
        s->addParam("Rotation", "u_src_rotation", 0.0f);
        s->addParam("Outline", "u_src_outline", 0.0f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // Bump Light (source version — procedural bump-lit surface)
    registerSource("bump_light", [] {
        auto s = std::make_unique<ProceduralSource>("bump_light", "Bump Light", "Pattern", "source_bump_light");
        s->addParam("Light X", "u_src_lx", 0.5f);
        s->addParam("Light Y", "u_src_ly", 0.3f);
        s->addParam("Intensity", "u_src_intensity", 0.6f);
        s->addParam("Bumps", "u_src_bumps", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // Infinite Zoom (source version — procedural zooming pattern)
    registerSource("infinite_zoom", [] {
        auto s = std::make_unique<ProceduralSource>("infinite_zoom", "Infinite Zoom", "Geometric", "source_infinite_zoom");
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Layers", "u_src_layers", 0.5f);
        s->addParam("Rotation", "u_src_rotation", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });
}

std::unique_ptr<ProceduralSource> SourceRegistry::createSource(const std::string& id) const
{
    auto it = registry_.find(id);
    if (it == registry_.end())
        return nullptr;
    return it->second.factory();
}

std::vector<std::string> SourceRegistry::getRegisteredIds() const
{
    std::vector<std::string> ids;
    ids.reserve(registry_.size());
    for (const auto& [id, info] : registry_)
        ids.push_back(id);
    return ids;
}

std::string SourceRegistry::getDisplayName(const std::string& id) const
{
    auto it = registry_.find(id);
    return it != registry_.end() ? it->second.displayName : "";
}

std::string SourceRegistry::getCategory(const std::string& id) const
{
    auto it = registry_.find(id);
    return it != registry_.end() ? it->second.category : "";
}

bool SourceRegistry::isRegistered(const std::string& id) const
{
    return registry_.count(id) > 0;
}
