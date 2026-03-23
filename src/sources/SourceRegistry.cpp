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
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        s->addParam("Palette", "u_src_palette", 0.6f);
        return s;
    });

    // Mandelbrot / Julia
    registerSource("mandelbrot", [] {
        auto s = std::make_unique<ProceduralSource>("mandelbrot", "Mandelbrot / Julia", "Fractal", "source_mandelbrot");
        s->addParam("Dive Speed", "u_src_dive_speed", 0.0f);
        s->addParam("Location", "u_src_location", 0.0f);
        s->addParam("Zoom", "u_src_zoom", 0.0f);
        s->addParam("Center X", "u_src_center_x", 0.5f);
        s->addParam("Center Y", "u_src_center_y", 0.5f);
        s->addParam("Julia Mix", "u_src_julia_mix", 0.0f);
        s->addParam("Max Iterations", "u_src_max_iter", 0.3f);
        s->addParam("Power", "u_src_power", 0.0f);
        s->addParam("Color Speed", "u_src_color_speed", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        s->addParam("Palette", "u_src_palette", 0.6f);
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
        s->addParam("Feed Rate", "u_src_feed", 0.28f);     // ~0.037 mapped
        s->addParam("Kill Rate", "u_src_kill", 0.32f);     // ~0.064 mapped → spots
        s->addParam("Diffusion A", "u_src_diffusion_a", 0.5f);
        s->addParam("Diffusion B", "u_src_diffusion_b", 0.5f);
        return s;
    });

    // Cellular Automata (stateful — needs ping-pong FBOs)
    registerSource("cellular_automata", [] {
        auto s = std::make_unique<ProceduralSource>("cellular_automata", "Cellular Automata", "Nature", "source_cellular_automata", true);
        s->addParam("Mode", "u_src_mode", 0.0f);
        s->addParam("Birth Low", "u_src_birth_low", 0.25f);
        s->addParam("Birth High", "u_src_birth_high", 0.25f);
        s->addParam("Survival Low", "u_src_survival_low", 0.25f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
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

    // === New sources: Spiral Tunnel, Wireframe 3D, Line Generator ===

    registerSource("spiral_tunnel", [] {
        auto s = std::make_unique<ProceduralSource>("spiral_tunnel", "Spiral Tunnel", "3D", "source_spiral_tunnel");
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Arms", "u_src_arms", 0.3f);
        s->addParam("Depth", "u_src_depth", 0.5f);
        s->addParam("Twist", "u_src_twist", 0.4f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // Wireframe 3D shapes — each shape is its own source preset
    auto registerWireframe = [this](const std::string& id, const std::string& name, float shapeVal) {
        registerSource(id, [name, shapeVal, id] {
            auto s = std::make_unique<ProceduralSource>(id, name, "Wireframe", "source_wireframe_3d");
            s->addParam("Shape", "u_src_shape", shapeVal);
            s->addParam("Density", "u_src_density", 0.3f);
            s->addParam("Rotation X", "u_src_rotation_x", 0.6f);
            s->addParam("Rotation Y", "u_src_rotation_y", 0.6f);
            s->addParam("Rotation Z", "u_src_rotation_z", 0.5f);
            s->addParam("Thickness", "u_src_thickness", 0.3f);
            s->addParam("Perspective", "u_src_perspective", 0.4f);
            s->addParam("Glow", "u_src_glow", 0.3f);
            s->addParam("Color Shift", "u_src_color_shift", 0.0f);
            return s;
        });
    };
    registerWireframe("wire_sphere",      "Wireframe Sphere",      0.072f);
    registerWireframe("wire_torus",       "Wireframe Torus",       0.215f);
    registerWireframe("wire_cube",        "Wireframe Cube",        0.358f);
    registerWireframe("wire_cylinder",    "Wireframe Cylinder",    0.501f);
    registerWireframe("wire_cone",        "Wireframe Cone",        0.644f);
    registerWireframe("wire_icosahedron", "Wireframe Icosahedron", 0.787f);
    registerWireframe("wire_wolf",        "Wireframe Wolf",        0.930f);

    // === Lines category: 10 algorithmic line generators ===

    registerSource("zigzag_lines", [] {
        auto s = std::make_unique<ProceduralSource>("zigzag_lines", "Zigzag Lines", "Lines", "source_zigzag_lines");
        s->addParam("Count", "u_src_count", 0.3f);
        s->addParam("Amplitude", "u_src_amplitude", 0.5f);
        s->addParam("Speed", "u_src_speed", 0.4f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("star_burst", [] {
        auto s = std::make_unique<ProceduralSource>("star_burst", "Star Burst", "Lines", "source_star_burst");
        s->addParam("Rays", "u_src_rays", 0.4f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.6f);
        s->addParam("Taper", "u_src_taper", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("polygon_lines", [] {
        auto s = std::make_unique<ProceduralSource>("polygon_lines", "Polygon Lines", "Lines", "source_polygon_lines");
        s->addParam("Sides", "u_src_sides", 0.3f);
        s->addParam("Size", "u_src_size", 0.5f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Layers", "u_src_layers", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.6f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("waveform_lines", [] {
        auto s = std::make_unique<ProceduralSource>("waveform_lines", "Waveform Lines", "Lines", "source_waveform_lines");
        s->addParam("Waveform", "u_src_waveform", 0.0f);
        s->addParam("Frequency", "u_src_freq", 0.3f);
        s->addParam("Amplitude", "u_src_amplitude", 0.5f);
        s->addParam("Count", "u_src_count", 0.3f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.4f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("lissajous", [] {
        auto s = std::make_unique<ProceduralSource>("lissajous", "Lissajous", "Lines", "source_lissajous");
        s->addParam("Ratio X", "u_src_ratio_x", 0.28f);
        s->addParam("Ratio Y", "u_src_ratio_y", 0.42f);
        s->addParam("Phase", "u_src_phase", 0.25f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("spirograph", [] {
        auto s = std::make_unique<ProceduralSource>("spirograph", "Spirograph", "Lines", "source_spirograph");
        s->addParam("Inner Radius", "u_src_inner", 0.4f);
        s->addParam("Offset", "u_src_offset", 0.5f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("angular_grid", [] {
        auto s = std::make_unique<ProceduralSource>("angular_grid", "Angular Grid", "Lines", "source_angular_grid");
        s->addParam("Angle", "u_src_angle", 0.25f);
        s->addParam("Count", "u_src_count", 0.3f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Symmetry", "u_src_symmetry", 0.2f);
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("fractal_tree", [] {
        auto s = std::make_unique<ProceduralSource>("fractal_tree", "Fractal Tree", "Lines", "source_fractal_tree");
        s->addParam("Branches", "u_src_branches", 0.5f);
        s->addParam("Angle", "u_src_angle", 0.4f);
        s->addParam("Depth", "u_src_depth", 0.4f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("laser_scan", [] {
        auto s = std::make_unique<ProceduralSource>("laser_scan", "Laser Scan", "Lines", "source_laser_scan");
        s->addParam("Beams", "u_src_beams", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.5f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Spread", "u_src_spread", 0.4f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("moire_lines", [] {
        auto s = std::make_unique<ProceduralSource>("moire_lines", "Moire Lines", "Lines", "source_moire_lines");
        s->addParam("Density", "u_src_density", 0.3f);
        s->addParam("Angle Offset", "u_src_angle_offset", 0.1f);
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Layers", "u_src_layers", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // === Fractal sources ===

    registerSource("julia_set", [] {
        auto s = std::make_unique<ProceduralSource>("julia_set", "Julia Set", "Fractal", "source_julia_set");
        s->addParam("Dive Speed", "u_src_dive_speed", 0.0f);
        s->addParam("Location", "u_src_location", 0.0f);
        s->addParam("C Real", "u_src_cx", 0.35f);
        s->addParam("C Imaginary", "u_src_cy", 0.38f);
        s->addParam("Zoom", "u_src_zoom", 0.25f);
        s->addParam("Iterations", "u_src_iterations", 0.3f);
        s->addParam("Color Speed", "u_src_color_speed", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        s->addParam("Palette", "u_src_palette", 0.6f);
        return s;
    });

    registerSource("burning_ship", [] {
        auto s = std::make_unique<ProceduralSource>("burning_ship", "Burning Ship", "Fractal", "source_burning_ship");
        s->addParam("Dive Speed", "u_src_dive_speed", 0.0f);
        s->addParam("Location", "u_src_location", 0.0f);
        s->addParam("Center X", "u_src_center_x", 0.55f);
        s->addParam("Center Y", "u_src_center_y", 0.6f);
        s->addParam("Zoom", "u_src_zoom", 0.2f);
        s->addParam("Iterations", "u_src_iterations", 0.3f);
        s->addParam("Color Speed", "u_src_color_speed", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        s->addParam("Palette", "u_src_palette", 0.6f);
        return s;
    });

    registerSource("newton_fractal", [] {
        auto s = std::make_unique<ProceduralSource>("newton_fractal", "Newton Fractal", "Fractal", "source_newton_fractal");
        s->addParam("Dive Speed", "u_src_dive_speed", 0.0f);
        s->addParam("Power", "u_src_power", 0.2f);
        s->addParam("Zoom", "u_src_zoom", 0.3f);
        s->addParam("Damping", "u_src_damping", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        s->addParam("Palette", "u_src_palette", 0.6f);
        return s;
    });

    registerSource("sierpinski", [] {
        auto s = std::make_unique<ProceduralSource>("sierpinski", "Sierpinski", "Fractal", "source_sierpinski");
        s->addParam("Dive Speed", "u_src_dive_speed", 0.0f);
        s->addParam("Mode", "u_src_mode", 0.0f);
        s->addParam("Zoom", "u_src_zoom", 0.0f);
        s->addParam("Iterations", "u_src_iterations", 0.5f);
        s->addParam("Rotation", "u_src_rotation", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        s->addParam("Palette", "u_src_palette", 0.6f);
        return s;
    });

    registerSource("apollonian", [] {
        auto s = std::make_unique<ProceduralSource>("apollonian", "Apollonian Gasket", "Fractal", "source_apollonian");
        s->addParam("Dive Speed", "u_src_dive_speed", 0.0f);
        s->addParam("Zoom", "u_src_zoom", 0.0f);
        s->addParam("Iterations", "u_src_iterations", 0.4f);
        s->addParam("Rotation", "u_src_rotation", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        s->addParam("Palette", "u_src_palette", 0.6f);
        return s;
    });

    // === 3D Fractal sources (ray marched) ===

    registerSource("mandelbulb", [] {
        auto s = std::make_unique<ProceduralSource>("mandelbulb", "Mandelbulb", "3D", "source_mandelbulb");
        s->addParam("Power", "u_src_power", 0.5f);
        s->addParam("Iterations", "u_src_iterations", 0.4f);
        s->addParam("Angle X", "u_src_rotation_x", 0.55f);
        s->addParam("Angle Y", "u_src_rotation_y", 0.55f);
        s->addParam("Zoom", "u_src_zoom", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.55f);
        s->addParam("Detail", "u_src_detail", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        s->addParam("Cross Section", "u_src_slice", 0.5f);
        s->addParam("Slice Count", "u_src_slice_count", 0.0f);
        s->addParam("Glow", "u_src_glow", 0.0f);
        s->addParam("Trail Distance", "u_src_trail_dist", 0.0f);
        s->addParam("Trail Fade", "u_src_trail_fade", 0.5f);
        s->addParam("Slice Distance", "u_src_slice_dist", 0.3f);
        s->addParam("Feedback", "u_src_feedback", 0.0f);
        s->addParam("Palette", "u_src_palette", 0.6f);
        return s;
    });

    registerSource("menger_sponge", [] {
        auto s = std::make_unique<ProceduralSource>("menger_sponge", "Menger Sponge", "3D", "source_menger_sponge");
        s->addParam("Iterations", "u_src_iterations", 0.5f);
        s->addParam("Angle X", "u_src_rotation_x", 0.55f);
        s->addParam("Angle Y", "u_src_rotation_y", 0.55f);
        s->addParam("Zoom", "u_src_zoom", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.55f);
        s->addParam("Twist", "u_src_twist", 0.0f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        s->addParam("Cross Section", "u_src_slice", 0.5f);
        s->addParam("Slice Count", "u_src_slice_count", 0.0f);
        s->addParam("Trail Distance", "u_src_trail_dist", 0.0f);
        s->addParam("Trail Fade", "u_src_trail_fade", 0.5f);
        s->addParam("Slice Distance", "u_src_slice_dist", 0.3f);
        s->addParam("Feedback", "u_src_feedback", 0.0f);
        s->addParam("Palette", "u_src_palette", 0.6f);
        return s;
    });

    registerSource("kifs", [] {
        auto s = std::make_unique<ProceduralSource>("kifs", "Kaleidoscopic IFS", "3D", "source_kifs");
        s->addParam("Scale", "u_src_scale", 0.4f);
        s->addParam("Iterations", "u_src_iterations", 0.4f);
        s->addParam("Fold Type", "u_src_fold_type", 0.0f);
        s->addParam("Angle X", "u_src_rotation_x", 0.55f);
        s->addParam("Angle Y", "u_src_rotation_y", 0.55f);
        s->addParam("Zoom", "u_src_zoom", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.55f);
        s->addParam("Offset", "u_src_offset", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        s->addParam("Cross Section", "u_src_slice", 0.5f);
        s->addParam("Slice Count", "u_src_slice_count", 0.0f);
        s->addParam("Trail Distance", "u_src_trail_dist", 0.0f);
        s->addParam("Trail Fade", "u_src_trail_fade", 0.5f);
        s->addParam("Slice Distance", "u_src_slice_dist", 0.3f);
        s->addParam("Feedback", "u_src_feedback", 0.0f);
        s->addParam("Palette", "u_src_palette", 0.6f);
        return s;
    });

    // === NEW 3D Fractal sources ===

    registerSource("julia_set_3d", [] {
        auto s = std::make_unique<ProceduralSource>("julia_set_3d", "Julia Set 3D", "3D", "source_julia_set_3d");
        s->addParam("Location", "u_src_location", 0.0f);
        s->addParam("C Real", "u_src_cx", 0.35f);
        s->addParam("C Imaginary", "u_src_cy", 0.6f);
        s->addParam("Angle X", "u_src_rotation_x", 0.55f);
        s->addParam("Angle Y", "u_src_rotation_y", 0.55f);
        s->addParam("Zoom", "u_src_zoom", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.55f);
        s->addParam("Iterations", "u_src_iterations", 0.4f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        s->addParam("Cross Section", "u_src_slice", 0.5f);
        s->addParam("Slice Count", "u_src_slice_count", 0.0f);
        s->addParam("Glow", "u_src_glow", 0.0f);
        s->addParam("Trail Distance", "u_src_trail_dist", 0.0f);
        s->addParam("Trail Fade", "u_src_trail_fade", 0.5f);
        s->addParam("Slice Distance", "u_src_slice_dist", 0.3f);
        s->addParam("Feedback", "u_src_feedback", 0.0f);
        s->addParam("Palette", "u_src_palette", 0.6f);
        return s;
    });

    registerSource("burning_ship_3d", [] {
        auto s = std::make_unique<ProceduralSource>("burning_ship_3d", "Burning Ship 3D", "3D", "source_burning_ship_3d");
        s->addParam("Power", "u_src_power", 0.5f);
        s->addParam("Angle X", "u_src_rotation_x", 0.55f);
        s->addParam("Angle Y", "u_src_rotation_y", 0.55f);
        s->addParam("Zoom", "u_src_zoom", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.55f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        s->addParam("Cross Section", "u_src_slice", 0.5f);
        s->addParam("Slice Count", "u_src_slice_count", 0.0f);
        s->addParam("Glow", "u_src_glow", 0.0f);
        s->addParam("Trail Distance", "u_src_trail_dist", 0.0f);
        s->addParam("Trail Fade", "u_src_trail_fade", 0.5f);
        s->addParam("Slice Distance", "u_src_slice_dist", 0.3f);
        s->addParam("Feedback", "u_src_feedback", 0.0f);
        s->addParam("Palette", "u_src_palette", 0.6f);
        return s;
    });

    registerSource("newton_3d", [] {
        auto s = std::make_unique<ProceduralSource>("newton_3d", "Newton 3D", "3D", "source_newton_3d");
        s->addParam("Power", "u_src_power", 0.2f);
        s->addParam("Angle X", "u_src_rotation_x", 0.55f);
        s->addParam("Angle Y", "u_src_rotation_y", 0.55f);
        s->addParam("Zoom", "u_src_zoom", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.55f);
        s->addParam("Damping", "u_src_damping", 0.5f);
        s->addParam("Height", "u_src_height", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        s->addParam("Trail Distance", "u_src_trail_dist", 0.0f);
        s->addParam("Trail Fade", "u_src_trail_fade", 0.5f);
        s->addParam("Slice Count", "u_src_slice_count", 0.0f);
        s->addParam("Slice Distance", "u_src_slice_dist", 0.3f);
        s->addParam("Feedback", "u_src_feedback", 0.0f);
        s->addParam("Palette", "u_src_palette", 0.6f);
        return s;
    });

    registerSource("sierpinski_tetra", [] {
        auto s = std::make_unique<ProceduralSource>("sierpinski_tetra", "Sierpinski Tetrahedron", "3D", "source_sierpinski_tetra");
        s->addParam("Iterations", "u_src_iterations", 0.5f);
        s->addParam("Angle X", "u_src_rotation_x", 0.55f);
        s->addParam("Angle Y", "u_src_rotation_y", 0.55f);
        s->addParam("Zoom", "u_src_zoom", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.55f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        s->addParam("Cross Section", "u_src_slice", 0.5f);
        s->addParam("Slice Count", "u_src_slice_count", 0.0f);
        s->addParam("Trail Distance", "u_src_trail_dist", 0.0f);
        s->addParam("Trail Fade", "u_src_trail_fade", 0.5f);
        s->addParam("Slice Distance", "u_src_slice_dist", 0.3f);
        s->addParam("Feedback", "u_src_feedback", 0.0f);
        s->addParam("Palette", "u_src_palette", 0.6f);
        return s;
    });

    registerSource("apollonian_3d", [] {
        auto s = std::make_unique<ProceduralSource>("apollonian_3d", "Apollonian 3D", "3D", "source_apollonian_3d");
        s->addParam("Scale", "u_src_scale", 0.3f);
        s->addParam("Iterations", "u_src_iterations", 0.4f);
        s->addParam("Angle X", "u_src_rotation_x", 0.55f);
        s->addParam("Angle Y", "u_src_rotation_y", 0.55f);
        s->addParam("Zoom", "u_src_zoom", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.55f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        s->addParam("Cross Section", "u_src_slice", 0.5f);
        s->addParam("Slice Count", "u_src_slice_count", 0.0f);
        s->addParam("Trail Distance", "u_src_trail_dist", 0.0f);
        s->addParam("Trail Fade", "u_src_trail_fade", 0.5f);
        s->addParam("Slice Distance", "u_src_slice_dist", 0.3f);
        s->addParam("Feedback", "u_src_feedback", 0.0f);
        s->addParam("Palette", "u_src_palette", 0.6f);
        return s;
    });

    // ============================================================
    // Raymarched Torus / Tunnel Sources (7 sources)
    // ============================================================

    // Helper: adds shared torus controls (camera, lens, geometry, deformation, visual)
    auto addTorusControls = [](ProceduralSource* s) {
        // Camera
        s->addParam("Orbit", "u_src_orbit", 0.0f);
        s->addParam("Tilt", "u_src_tilt", 0.49f);
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Zoom", "u_src_zoom", 0.4f);
        // Lens
        s->addParam("Lens Shape", "u_src_lens_shape", 0.0f);
        s->addParam("Lens Rotate", "u_src_lens_rotate", 0.0f);
        s->addParam("Depth Fade", "u_src_depth_fade", 0.0f);
        // Geometry
        s->addParam("Tube Radius", "u_src_tube_radius", 0.32f);
        // Deformation
        s->addParam("Pinch", "u_src_pinch", 0.5f);
        s->addParam("Heart", "u_src_heart", 0.5f);
        // Visual
        s->addParam("Shading", "u_src_shading", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
    };

    registerSource("striped_torus", [&addTorusControls] {
        auto s = std::make_unique<ProceduralSource>("striped_torus", "Striped Torus", "3D", "source_striped_torus");
        s->addParam("Stripe Count", "u_src_stripe_count", 0.3f);
        s->addParam("Twist", "u_src_twist", 0.4f);
        addTorusControls(s.get());
        return s;
    });

    registerSource("spiral_vortex", [&addTorusControls] {
        auto s = std::make_unique<ProceduralSource>("spiral_vortex", "Spiral Vortex", "3D", "source_spiral_vortex");
        s->addParam("Twist", "u_src_twist", 0.3f);
        s->addParam("Stripe Count", "u_src_stripe_count", 0.3f);
        addTorusControls(s.get());
        return s;
    });

    registerSource("checker_torus", [&addTorusControls] {
        auto s = std::make_unique<ProceduralSource>("checker_torus", "Checker Torus", "3D", "source_checker_torus");
        s->addParam("Grid U", "u_src_grid_u", 0.3f);
        s->addParam("Grid V", "u_src_grid_v", 0.3f);
        addTorusControls(s.get());
        return s;
    });

    registerSource("ribbed_vortex", [&addTorusControls] {
        auto s = std::make_unique<ProceduralSource>("ribbed_vortex", "Ribbed Vortex", "3D", "source_ribbed_vortex");
        s->addParam("Ridge Count", "u_src_ridge_count", 0.3f);
        s->addParam("Color Mix", "u_src_color_mix", 0.5f);
        addTorusControls(s.get());
        return s;
    });

    registerSource("wormhole_tunnel", [&addTorusControls] {
        auto s = std::make_unique<ProceduralSource>("wormhole_tunnel", "Wormhole Tunnel", "3D", "source_wormhole_tunnel");
        s->addParam("Warp", "u_src_warp", 0.3f);
        addTorusControls(s.get());
        return s;
    });

    registerSource("twisted_torus", [&addTorusControls] {
        auto s = std::make_unique<ProceduralSource>("twisted_torus", "Twisted Torus", "3D", "source_twisted_torus");
        s->addParam("Twist", "u_src_twist", 0.3f);
        s->addParam("Stripe Count", "u_src_stripe_count", 0.3f);
        addTorusControls(s.get());
        return s;
    });

    registerSource("wormhole", [&addTorusControls] {
        auto s = std::make_unique<ProceduralSource>("wormhole", "Wormhole", "3D", "source_wormhole");
        s->addParam("Warp", "u_src_warp", 0.3f);
        s->addParam("Glow", "u_src_glow", 0.4f);
        addTorusControls(s.get());
        return s;
    });

    registerSource("torus_hole", [] {
        auto s = std::make_unique<ProceduralSource>("torus_hole", "Torus Hole", "3D", "source_torus_hole");
        // Camera
        s->addParam("Orbit", "u_src_orbit", 0.0f);
        s->addParam("Tilt", "u_src_tilt", 0.49f);
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Zoom", "u_src_zoom", 0.4f);
        // Lens
        s->addParam("Lens Shape", "u_src_lens_shape", 0.0f);
        s->addParam("Lens Rotate", "u_src_lens_rotate", 0.0f);
        s->addParam("Depth Fade", "u_src_depth_fade", 0.0f);
        // Pattern
        s->addParam("Stripe Count", "u_src_stripe_count", 0.6f);
        s->addParam("Twist", "u_src_twist", 0.5f);
        s->addParam("Stripe Angle", "u_src_stripe_angle", 0.0f);
        s->addParam("Stripe Scale", "u_src_stripe_scale", 0.33f);
        s->addParam("Stripe Width", "u_src_stripe_width", 0.5f);
        s->addParam("Phi Offset", "u_src_phi_offset", 0.0f);
        s->addParam("Theta Offset", "u_src_theta_offset", 0.0f);
        // Geometry
        s->addParam("Tube Radius", "u_src_tube_radius", 0.32f);
        // Deformation
        s->addParam("Pinch", "u_src_pinch", 0.5f);
        s->addParam("Heart", "u_src_heart", 0.5f);
        // Visual
        s->addParam("Shading", "u_src_shading", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("line_generator", [] {
        auto s = std::make_unique<ProceduralSource>("line_generator", "Line Generator", "Lines", "source_line_generator");
        s->addParam("Pattern", "u_src_pattern", 0.0f);
        s->addParam("Count", "u_src_count", 0.3f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Feedback", "u_src_feedback", 0.5f);
        s->addParam("Feedback Zoom", "u_src_fb_zoom", 0.52f);
        s->addParam("Feedback Rotation", "u_src_fb_rotation", 0.52f);
        s->addParam("Feedback Decay", "u_src_fb_decay", 0.7f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // === Phase 17: Creative Sources (19 new sources) ===

    // P17.1 — Lissajous Weaver (Math)
    registerSource("lissajous_weaver", [] {
        auto s = std::make_unique<ProceduralSource>("lissajous_weaver", "Lissajous Weaver", "Math", "source_lissajous_weaver");
        s->addParam("Frequency X", "u_src_freq_x", 0.3f);
        s->addParam("Frequency Y", "u_src_freq_y", 0.4f);
        s->addParam("Phase", "u_src_phase", 0.25f);
        s->addParam("Decay", "u_src_decay", 0.5f);
        s->addParam("Harmonics", "u_src_harmonics", 0.3f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // P17.2 — Fermat Spiral Garden (Math)
    registerSource("fermat_spiral", [] {
        auto s = std::make_unique<ProceduralSource>("fermat_spiral", "Fermat Spiral Garden", "Math", "source_fermat_spiral");
        s->addParam("Count", "u_src_count", 0.5f);
        s->addParam("Divergence Angle", "u_src_divergence", 0.5f);
        s->addParam("Shape", "u_src_shape", 0.0f);
        s->addParam("Growth", "u_src_growth", 0.3f);
        s->addParam("Pulse", "u_src_pulse", 0.5f);
        s->addParam("Color Spread", "u_src_color_spread", 0.5f);
        return s;
    });

    // P17.3 — Hyperbolic Tiling (Math)
    registerSource("hyperbolic_tiling", [] {
        auto s = std::make_unique<ProceduralSource>("hyperbolic_tiling", "Hyperbolic Tiling", "Math", "source_hyperbolic_tiling");
        s->addParam("Polygon Sides", "u_src_p_sides", 0.4f);
        s->addParam("Vertex Order", "u_src_q_order", 0.2f);
        s->addParam("Rotation", "u_src_rotation", 0.5f);
        s->addParam("Zoom", "u_src_zoom", 0.3f);
        s->addParam("Color Scheme", "u_src_color_scheme", 0.5f);
        s->addParam("Line Width", "u_src_line_width", 0.3f);
        return s;
    });

    // P17.4 — Penrose Pulse (Math)
    registerSource("penrose_pulse", [] {
        auto s = std::make_unique<ProceduralSource>("penrose_pulse", "Penrose Pulse", "Math", "source_penrose_pulse");
        s->addParam("Generation", "u_src_generation", 0.5f);
        s->addParam("Ripple Speed", "u_src_ripple_speed", 0.5f);
        s->addParam("Color Mode", "u_src_color_mode", 0.5f);
        s->addParam("Edge Glow", "u_src_edge_glow", 0.5f);
        s->addParam("Morph", "u_src_morph", 0.0f);
        return s;
    });

    // P17.5 — Moire Interference (Geometric)
    registerSource("moire_interference", [] {
        auto s = std::make_unique<ProceduralSource>("moire_interference", "Moire Interference", "Geometric", "source_moire_interference");
        s->addParam("Pattern", "u_src_pattern", 0.0f);
        s->addParam("Frequency", "u_src_frequency", 0.3f);
        s->addParam("Offset X", "u_src_offset_x", 0.5f);
        s->addParam("Offset Y", "u_src_offset_y", 0.5f);
        s->addParam("Rotation", "u_src_rotation", 0.55f);
        s->addParam("Zoom", "u_src_zoom", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // P17.6 — Crystal Cavern (3D, ray-marched)
    registerSource("crystal_cavern", [] {
        auto s = std::make_unique<ProceduralSource>("crystal_cavern", "Crystal Cavern", "3D", "source_crystal_cavern");
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Crystal Size", "u_src_crystal_size", 0.5f);
        s->addParam("Reflectivity", "u_src_reflectivity", 0.5f);
        s->addParam("Light Color", "u_src_light_color", 0.0f);
        s->addParam("Fog Density", "u_src_fog", 0.3f);
        s->addParam("Complexity", "u_src_complexity", 0.5f);
        return s;
    });

    // P17.7 — Infinite Corridor (3D, ray-marched)
    registerSource("infinite_corridor", [] {
        auto s = std::make_unique<ProceduralSource>("infinite_corridor", "Infinite Corridor", "3D", "source_infinite_corridor");
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Width", "u_src_width", 0.5f);
        s->addParam("Wall Pattern", "u_src_wall_pattern", 0.0f);
        s->addParam("Light Spacing", "u_src_light_spacing", 0.3f);
        s->addParam("Light Intensity", "u_src_light_intensity", 0.5f);
        s->addParam("Color", "u_src_color", 0.0f);
        return s;
    });

    // P17.8 — Orbit Chamber (3D, ray-marched)
    registerSource("orbit_chamber", [] {
        auto s = std::make_unique<ProceduralSource>("orbit_chamber", "Orbit Chamber", "3D", "source_orbit_chamber");
        s->addParam("Object Count", "u_src_obj_count", 0.4f);
        s->addParam("Object Type", "u_src_obj_type", 0.0f);
        s->addParam("Orbit Speed", "u_src_orbit_speed", 0.4f);
        s->addParam("Orbit Radius", "u_src_orbit_radius", 0.5f);
        s->addParam("Material", "u_src_material", 0.5f);
        s->addParam("Light Orbit", "u_src_light_orbit", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // P17.9 — Astral Grid (Geometric)
    registerSource("astral_grid", [] {
        auto s = std::make_unique<ProceduralSource>("astral_grid", "Astral Grid", "Geometric", "source_astral_grid");
        s->addParam("Grid Size", "u_src_grid_size", 0.3f);
        s->addParam("Scroll Speed", "u_src_scroll_speed", 0.4f);
        s->addParam("Tilt", "u_src_tilt", 0.5f);
        s->addParam("Warp", "u_src_warp", 0.0f);
        s->addParam("Glow", "u_src_glow", 0.5f);
        s->addParam("Horizon Color", "u_src_horizon_color", 0.6f);
        return s;
    });

    // P17.10 — Radial Burst (Geometric)
    registerSource("radial_burst", [] {
        auto s = std::make_unique<ProceduralSource>("radial_burst", "Radial Burst", "Geometric", "source_radial_burst");
        s->addParam("Ray Count", "u_src_ray_count", 0.3f);
        s->addParam("Length", "u_src_length", 0.5f);
        s->addParam("Rotation", "u_src_rotation", 0.55f);
        s->addParam("Width", "u_src_width", 0.3f);
        s->addParam("Taper", "u_src_taper", 0.5f);
        s->addParam("Glow", "u_src_glow", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // P17.11 — Hex Grid (Geometric)
    registerSource("hex_grid", [] {
        auto s = std::make_unique<ProceduralSource>("hex_grid", "Hex Grid", "Geometric", "source_hex_grid");
        s->addParam("Cell Size", "u_src_cell_size", 0.3f);
        s->addParam("Pattern", "u_src_pattern", 0.0f);
        s->addParam("Fill", "u_src_fill", 0.5f);
        s->addParam("Edge Width", "u_src_edge_width", 0.3f);
        s->addParam("Rotation", "u_src_rotation", 0.0f);
        s->addParam("Color Mode", "u_src_color_mode", 0.5f);
        return s;
    });

    // P17.12 — Sacred Geometry (Geometric)
    registerSource("sacred_geometry", [] {
        auto s = std::make_unique<ProceduralSource>("sacred_geometry", "Sacred Geometry", "Geometric", "source_sacred_geometry");
        s->addParam("Pattern", "u_src_pattern", 0.0f);
        s->addParam("Rotation", "u_src_rotation", 0.5f);
        s->addParam("Breathe", "u_src_breathe", 0.3f);
        s->addParam("Line Width", "u_src_line_width", 0.3f);
        s->addParam("Glow", "u_src_glow", 0.3f);
        s->addParam("Reveal", "u_src_reveal", 1.0f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // P17.13 — Fire Wall (Nature)
    registerSource("fire_wall", [] {
        auto s = std::make_unique<ProceduralSource>("fire_wall", "Fire Wall", "Nature", "source_fire_wall");
        s->addParam("Height", "u_src_height", 0.6f);
        s->addParam("Turbulence", "u_src_turbulence", 0.5f);
        s->addParam("Speed", "u_src_speed", 0.4f);
        s->addParam("Temperature", "u_src_temperature", 0.7f);
        s->addParam("Density", "u_src_density", 0.6f);
        s->addParam("Wind", "u_src_wind", 0.0f);
        return s;
    });

    // P17.14 — Water Caustics (Nature)
    registerSource("water_caustics", [] {
        auto s = std::make_unique<ProceduralSource>("water_caustics", "Water Caustics", "Nature", "source_water_caustics");
        s->addParam("Complexity", "u_src_complexity", 0.5f);
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Brightness", "u_src_brightness", 0.5f);
        s->addParam("Color", "u_src_color", 0.0f);
        s->addParam("Distortion", "u_src_distortion", 0.3f);
        s->addParam("Scale", "u_src_scale", 0.3f);
        return s;
    });

    // P17.15 — Electric Arc (Nature)
    registerSource("electric_arc", [] {
        auto s = std::make_unique<ProceduralSource>("electric_arc", "Electric Arc", "Nature", "source_electric_arc");
        s->addParam("Arc Count", "u_src_arc_count", 0.2f);
        s->addParam("Chaos", "u_src_chaos", 0.5f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Branches", "u_src_branches", 0.3f);
        s->addParam("Glow", "u_src_glow", 0.5f);
        s->addParam("Color", "u_src_color", 0.5f);
        return s;
    });

    // P17.16 — Laser Scanner (Lighting)
    registerSource("laser_scanner", [] {
        auto s = std::make_unique<ProceduralSource>("laser_scanner", "Laser Scanner", "Lighting", "source_laser_scanner");
        s->addParam("Pattern", "u_src_pattern", 0.0f);
        s->addParam("Beam Count", "u_src_beam_count", 0.3f);
        s->addParam("Color", "u_src_color", 0.3f);
        s->addParam("Speed", "u_src_speed", 0.5f);
        s->addParam("Spread", "u_src_spread", 0.5f);
        s->addParam("Flicker", "u_src_flicker", 0.2f);
        return s;
    });

    // P17.17 — Scroll Plane (3D)
    registerSource("scroll_plane", [] {
        auto s = std::make_unique<ProceduralSource>("scroll_plane", "Scroll Plane", "3D", "source_scroll_plane");
        s->addParam("Speed X", "u_src_speedx", 0.6f);
        s->addParam("Speed Y", "u_src_speedy", 0.5f);
        s->addParam("Scale", "u_src_scale", 0.3f);
        s->addParam("Warp", "u_src_warp", 0.0f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // P17.18 — Rotating Cube Map (3D)
    registerSource("rotating_cube_map", [] {
        auto s = std::make_unique<ProceduralSource>("rotating_cube_map", "Rotating Cube Map", "3D", "source_rotating_cube_map");
        s->addParam("Rotation X", "u_src_rotx", 0.6f);
        s->addParam("Rotation Y", "u_src_roty", 0.55f);
        s->addParam("Scale", "u_src_scale", 0.3f);
        s->addParam("Light", "u_src_light", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // P17.19 — Dual Plane Drift (3D)
    registerSource("dual_plane_drift", [] {
        auto s = std::make_unique<ProceduralSource>("dual_plane_drift", "Dual Plane Drift", "3D", "source_dual_plane_drift");
        s->addParam("Speed", "u_src_speed", 0.4f);
        s->addParam("Rotation", "u_src_rotation", 0.0f);
        s->addParam("Distance", "u_src_distance", 0.5f);
        s->addParam("Scale", "u_src_scale", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    // ============================================================
    // Phase 18: Audio-Native Sources (8 sources + Text Wall)
    // ============================================================

    registerSource("spectrum_landscape", [] {
        auto s = std::make_unique<ProceduralSource>("spectrum_landscape", "Spectrum Landscape", "Audio-Visual", "source_spectrum_landscape");
        s->addParam("Height Scale", "u_src_height", 0.5f);
        s->addParam("Camera Angle", "u_src_camera", 0.3f);
        s->addParam("Color Mode", "u_src_color_mode", 0.0f);
        s->addParam("Glow", "u_src_glow", 0.5f);
        s->addParam("Smoothing", "u_src_smoothing", 0.5f);
        return s;
    });

    registerSource("chromatic_ring", [] {
        auto s = std::make_unique<ProceduralSource>("chromatic_ring", "Chromatic Ring", "Audio-Visual", "source_chromatic_ring");
        s->addParam("Ring Width", "u_src_ring_width", 0.5f);
        s->addParam("Glow", "u_src_glow", 0.5f);
        s->addParam("Rotation", "u_src_rotation", 0.0f);
        s->addParam("Ripple", "u_src_ripple", 0.5f);
        return s;
    });

    registerSource("band_tower", [] {
        auto s = std::make_unique<ProceduralSource>("band_tower", "Band Tower", "Audio-Visual", "source_band_tower");
        s->addParam("Shape", "u_src_shape", 0.0f);
        s->addParam("Spacing", "u_src_spacing", 0.3f);
        s->addParam("Reflection", "u_src_reflection", 0.3f);
        s->addParam("Color Mode", "u_src_color_mode", 0.0f);
        s->addParam("Smoothing", "u_src_smoothing", 0.5f);
        s->addParam("3D Rotation", "u_src_rotation_3d", 0.0f);
        return s;
    });

    registerSource("timbral_nebula", [] {
        auto s = std::make_unique<ProceduralSource>("timbral_nebula", "Timbral Nebula", "Audio-Visual", "source_timbral_nebula");
        s->addParam("Particle Count", "u_src_particles", 0.5f);
        s->addParam("Spread", "u_src_spread", 0.5f);
        s->addParam("Trail Length", "u_src_trail", 0.3f);
        s->addParam("Color Source", "u_src_color_source", 0.0f);
        s->addParam("Glow", "u_src_glow", 0.5f);
        s->addParam("Sensitivity", "u_src_sensitivity", 0.5f);
        return s;
    });

    registerSource("structural_landscape", [] {
        auto s = std::make_unique<ProceduralSource>("structural_landscape", "Structural Landscape", "Audio-Visual", "source_structural_landscape");
        s->addParam("Terrain Scale", "u_src_scale", 0.5f);
        s->addParam("History Length", "u_src_history", 0.5f);
        s->addParam("Drama", "u_src_drama", 0.5f);
        s->addParam("Color Palette", "u_src_palette", 0.0f);
        s->addParam("Fog", "u_src_fog", 0.3f);
        s->addParam("Camera Height", "u_src_camera", 0.5f);
        return s;
    });

    registerSource("cymatics", [] {
        auto s = std::make_unique<ProceduralSource>("cymatics", "Cymatics", "Audio-Visual", "source_cymatics");
        s->addParam("Resonance", "u_src_resonance", 0.5f);
        s->addParam("Damping", "u_src_damping", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("spectral_waterfall", [] {
        auto s = std::make_unique<ProceduralSource>("spectral_waterfall", "Spectral Waterfall", "Audio-Visual", "source_spectral_waterfall");
        s->addParam("Scroll Speed", "u_src_scroll", 0.5f);
        s->addParam("Color Mode", "u_src_color_mode", 0.0f);
        s->addParam("Log Scale", "u_src_log_scale", 0.5f);
        return s;
    });

    registerSource("spectral_ring", [] {
        auto s = std::make_unique<ProceduralSource>("spectral_ring", "Spectral Ring", "Audio-Visual", "source_spectral_ring");
        s->addParam("Radius", "u_src_radius", 0.5f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Glow", "u_src_glow", 0.5f);
        s->addParam("Rotation", "u_src_rotation", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("text_wall", [] {
        auto s = std::make_unique<ProceduralSource>("text_wall", "Scrolling Text Wall", "Text", "source_text_wall");
        s->addParam("Speed", "u_src_speed", 0.4f);
        s->addParam("Density", "u_src_density", 0.6f);
        s->addParam("Size", "u_src_size", 0.4f);
        s->addParam("Color Shift", "u_src_color_shift", 0.3f);
        return s;
    });

    // ============================================================
    // Phase 19: Remaining Sources (12 new sources)
    // ============================================================

    registerSource("superformula", [] {
        auto s = std::make_unique<ProceduralSource>("superformula", "Superformula", "Math", "source_superformula");
        s->addParam("Symmetry", "u_src_m", 0.3f);
        s->addParam("Roundness", "u_src_n1", 0.5f);
        s->addParam("Concavity", "u_src_n2", 0.5f);
        s->addParam("Blobbiness", "u_src_n3", 0.5f);
        s->addParam("Size", "u_src_size", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("truchet_labyrinth", [] {
        auto s = std::make_unique<ProceduralSource>("truchet_labyrinth", "Truchet Labyrinth", "Math", "source_truchet");
        s->addParam("Density", "u_src_density", 0.5f);
        s->addParam("Style", "u_src_style", 0.0f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Glow", "u_src_glow", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("rose_curves", [] {
        auto s = std::make_unique<ProceduralSource>("rose_curves", "Rose Curves", "Math", "source_rose");
        s->addParam("Petals", "u_src_k", 0.5f);
        s->addParam("Thickness", "u_src_thickness", 0.3f);
        s->addParam("Layers", "u_src_layers", 0.5f);
        s->addParam("Glow", "u_src_glow", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("fibonacci_spiral", [] {
        auto s = std::make_unique<ProceduralSource>("fibonacci_spiral", "Fibonacci Spiral", "Math", "source_fibonacci");
        s->addParam("Elements", "u_src_elements", 0.5f);
        s->addParam("Size", "u_src_size", 0.5f);
        s->addParam("Spread", "u_src_spread", 0.5f);
        s->addParam("Glow", "u_src_glow", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("lightning_storm", [] {
        auto s = std::make_unique<ProceduralSource>("lightning_storm", "Lightning Storm", "Particle", "source_lightning");
        s->addParam("Intensity", "u_src_intensity", 0.5f);
        s->addParam("Branches", "u_src_branches", 0.5f);
        s->addParam("Glow", "u_src_glow", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("fire", [] {
        auto s = std::make_unique<ProceduralSource>("fire", "Fire", "Nature", "source_fire");
        s->addParam("Height", "u_src_height", 0.5f);
        s->addParam("Turbulence", "u_src_turbulence", 0.5f);
        s->addParam("Speed", "u_src_speed", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("starfield", [] {
        auto s = std::make_unique<ProceduralSource>("starfield", "Starfield", "Particle", "source_starfield");
        s->addParam("Speed", "u_src_speed", 0.5f);
        s->addParam("Density", "u_src_density", 0.5f);
        s->addParam("Streak", "u_src_streak", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("particle_nebula", [] {
        auto s = std::make_unique<ProceduralSource>("particle_nebula", "Particle Nebula", "Particle", "source_nebula");
        s->addParam("Density", "u_src_density", 0.5f);
        s->addParam("Scale", "u_src_scale", 0.5f);
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("radar_sweep", [] {
        auto s = std::make_unique<ProceduralSource>("radar_sweep", "Radar Sweep", "Geometric", "source_radar");
        s->addParam("Speed", "u_src_speed", 0.5f);
        s->addParam("Decay", "u_src_decay", 0.5f);
        s->addParam("Grid", "u_src_grid", 0.3f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("glitch_grid", [] {
        auto s = std::make_unique<ProceduralSource>("glitch_grid", "Glitch Grid", "Pattern", "source_glitch_grid");
        s->addParam("Grid Size", "u_src_grid", 0.4f);
        s->addParam("Chaos", "u_src_chaos", 0.5f);
        s->addParam("Flicker", "u_src_flicker", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("dna_helix", [] {
        auto s = std::make_unique<ProceduralSource>("dna_helix", "DNA Helix", "3D", "source_dna");
        s->addParam("Speed", "u_src_speed", 0.3f);
        s->addParam("Zoom", "u_src_zoom", 0.5f);
        s->addParam("Glow", "u_src_glow", 0.5f);
        s->addParam("Color Shift", "u_src_color_shift", 0.0f);
        return s;
    });

    registerSource("dot_matrix_wave", [] {
        auto s = std::make_unique<ProceduralSource>("dot_matrix_wave", "Dot Matrix Wave", "Geometric", "source_dot_matrix");
        s->addParam("Density", "u_src_density", 0.5f);
        s->addParam("Speed", "u_src_speed", 0.5f);
        s->addParam("Damping", "u_src_damping", 0.5f);
        s->addParam("Dot Size", "u_src_size", 0.3f);
        s->addParam("Color", "u_src_color", 0.0f);
        s->addParam("Sources", "u_src_sources", 0.5f);
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
