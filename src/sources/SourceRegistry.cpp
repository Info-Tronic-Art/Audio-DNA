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
