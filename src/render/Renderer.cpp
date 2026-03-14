#include "Renderer.h"
#include "render/EmbeddedShaders.h"
#include <iostream>
#include <chrono>

using namespace juce::gl;
Renderer::Renderer(FeatureBus& featureBus)
    : featureBus_(featureBus)
{
}

Renderer::~Renderer()
{
    detach();
}

void Renderer::attachTo(juce::Component& component)
{
    glContext_.setOpenGLVersionRequired(juce::OpenGLContext::openGL4_1);
    glContext_.setRenderer(this);
    glContext_.setContinuousRepainting(true);
    glContext_.setComponentPaintingEnabled(false);
    glContext_.attachTo(component);
}

void Renderer::detach()
{
    glContext_.detach();
}

void Renderer::loadImage(const juce::File& imageFile)
{
    std::cerr << "[Renderer] loadImage: " << imageFile.getFullPathName() << std::endl;
    std::lock_guard<std::mutex> lock(pendingImageMutex_);
    pendingImageFile_ = imageFile;
    hasPendingImage_ = true;
}

void Renderer::newOpenGLContextCreated()
{
    std::cerr << "[Renderer] GL context created. Version: "
              << glGetString(GL_VERSION) << std::endl;

    quad_.init();
    initShaders();
    initEffectChain();
    startTime_ = juce::Time::getMillisecondCounterHiRes() / 1000.0;
}

void Renderer::renderOpenGL()
{
    // Handle pending image load (from message thread)
    {
        std::lock_guard<std::mutex> lock(pendingImageMutex_);
        if (hasPendingImage_)
        {
            std::cerr << "[Renderer] Processing pending image..." << std::endl;
            bool ok = texMgr_.loadImage(pendingImageFile_);
            std::cerr << "[Renderer] Image load " << (ok ? "OK" : "FAILED")
                      << ", hasImage=" << texMgr_.hasImage() << std::endl;
            hasPendingImage_ = false;
        }
    }

    // FPS tracking
    double now = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    ++frameCount_;
    if (now - fpsTimer_ >= 1.0)
    {
        currentFps_.store(static_cast<float>(frameCount_) / static_cast<float>(now - fpsTimer_),
                          std::memory_order_relaxed);
        frameCount_ = 0;
        fpsTimer_ = now;
    }

    // Clear to near-black
    juce::OpenGLHelpers::clear(juce::Colour(0xff0a0a14));

    if (!texMgr_.hasImage())
        return; // Nothing to render yet

    // Read latest audio features (lock-free)
    const FeatureSnapshot* snap = featureBus_.acquireRead();
    if (snap == nullptr)
        snap = featureBus_.getLatestRead();

    // Build a default snapshot if none available
    FeatureSnapshot defaultSnap;
    if (snap == nullptr)
        snap = &defaultSnap;

    // Apply audio→effect mappings via MappingEngine
    mappingEngine_.processFrame(*snap, effectChain_);

    // Calculate time
    float time = static_cast<float>(
        juce::Time::getMillisecondCounterHiRes() / 1000.0 - startTime_);

    // Get physical pixel dimensions
    auto* component = glContext_.getTargetComponent();
    float scale = static_cast<float>(glContext_.getRenderingScale());
    float compW = component != nullptr ? static_cast<float>(component->getWidth())  * scale : 1.0f;
    float compH = component != nullptr ? static_cast<float>(component->getHeight()) * scale : 1.0f;

    // Check for locked resolution
    int lockW = lockedWidth_.load(std::memory_order_relaxed);
    int lockH = lockedHeight_.load(std::memory_order_relaxed);
    float renderW = (lockW > 0 && lockH > 0) ? static_cast<float>(lockW) : compW;
    float renderH = (lockW > 0 && lockH > 0) ? static_cast<float>(lockH) : compH;

    // Get the default framebuffer that JUCE's context uses
    GLint defaultFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);

    // Compute letterboxed viewport to maintain correct aspect ratio
    float aspectW = renderW;
    float aspectH = renderH;
    if (lockW <= 0 || lockH <= 0)
    {
        // In auto mode, use the loaded image's aspect ratio
        int imgW = texMgr_.getImageWidth();
        int imgH = texMgr_.getImageHeight();
        if (imgW > 0 && imgH > 0)
        {
            aspectW = static_cast<float>(imgW);
            aspectH = static_cast<float>(imgH);
        }
    }

    float scaleX = compW / aspectW;
    float scaleY = compH / aspectH;
    float fitScale = std::min(scaleX, scaleY);
    float vpW = aspectW * fitScale;
    float vpH = aspectH * fitScale;
    float vpX = (compW - vpW) * 0.5f;
    float vpY = (compH - vpH) * 0.5f;

    // Render the effect chain with letterbox viewport for final output
    auto renderStart = std::chrono::high_resolution_clock::now();

    effectChain_.render(texMgr_.getImageTexture(),
                        shaderMgr_, texMgr_, quad_,
                        time, renderW, renderH,
                        static_cast<GLuint>(defaultFBO),
                        vpX, vpY, vpW, vpH);

    glFinish(); // Ensure GPU work is done before measuring
    auto renderEnd = std::chrono::high_resolution_clock::now();
    double frameMs = std::chrono::duration<double, std::milli>(renderEnd - renderStart).count();

    // EMA smoothing for UI display
    float prevMs = frameTimeMs_.load(std::memory_order_relaxed);
    frameTimeMs_.store(prevMs + 0.1f * (static_cast<float>(frameMs) - prevMs), std::memory_order_relaxed);

    // Adaptive quality: if sustained high frame times, disable heaviest effect
    if (frameMs > static_cast<double>(kFrameTimeBudgetMs))
    {
        if (++highFrameTimeCount_ >= kHighFrameTimeThreshold)
        {
            // Find the last enabled effect and disable it
            for (int i = effectChain_.getNumEffects() - 1; i >= 0; --i)
            {
                auto* fx = effectChain_.getEffect(i);
                if (fx != nullptr && fx->isEnabled())
                {
                    fx->setEnabled(false);
                    std::cerr << "[Renderer] Adaptive quality: disabled '"
                              << fx->getName() << "' (frame time "
                              << static_cast<int>(frameMs * 10) / 10.0 << "ms)" << std::endl;
                    break;
                }
            }
            highFrameTimeCount_ = 0;
        }
    }
    else
    {
        highFrameTimeCount_ = 0;
    }

    // Periodic log
    renderProfileAccum_ += frameMs;
    if (++renderProfileCount_ >= kRenderProfileInterval)
    {
        double avgMs = renderProfileAccum_ / kRenderProfileInterval;
        int numEnabled = 0;
        for (int i = 0; i < effectChain_.getNumEffects(); ++i)
        {
            if (auto* fx = effectChain_.getEffect(i))
                if (fx->isEnabled()) ++numEnabled;
        }
        std::cerr << "[Render Profile] Avg frame: " << static_cast<int>(avgMs * 100) / 100.0
                  << " ms, " << numEnabled << " effects active, "
                  << static_cast<int>(renderW) << "x" << static_cast<int>(renderH) << std::endl;
        renderProfileAccum_ = 0.0;
        renderProfileCount_ = 0;
    }
}

void Renderer::openGLContextClosing()
{
    shaderMgr_.releaseAll();
    texMgr_.release();
    quad_.release();
}

void Renderer::initShaders()
{
    compileAllShaders();
}

void Renderer::compileAllShaders()
{
    auto compile = [&](const juce::String& name, const char* frag) {
        if (shaderMgr_.compileProgram(name, EmbeddedShaders::vertex, frag))
            std::cerr << "[Renderer]   " << name << ": OK" << std::endl;
        else
            std::cerr << "[Renderer]   " << name << ": FAILED" << std::endl;
    };

    // Core
    compile("passthrough",          EmbeddedShaders::passthrough);

    // Warp
    compile("ripple",               EmbeddedShaders::ripple);
    compile("bulge",                EmbeddedShaders::bulge);
    compile("wave",                 EmbeddedShaders::wave);
    compile("liquid",               EmbeddedShaders::liquid);
    compile("kaleidoscope",         EmbeddedShaders::kaleidoscope);
    compile("fisheye",              EmbeddedShaders::fisheye);
    compile("swirl",                EmbeddedShaders::swirl);

    // Color
    compile("hue_shift",            EmbeddedShaders::hueShift);
    compile("saturation",           EmbeddedShaders::saturation);
    compile("brightness",           EmbeddedShaders::brightness);
    compile("duotone",              EmbeddedShaders::duotone);
    compile("chromatic_aberration", EmbeddedShaders::chromaticAberration);
    compile("invert",               EmbeddedShaders::invert);
    compile("posterize",            EmbeddedShaders::posterize);
    compile("color_shift",          EmbeddedShaders::colorShift);
    compile("thermal",              EmbeddedShaders::thermal);
    compile("color_matrix",         EmbeddedShaders::colorMatrix);

    // Glitch
    compile("pixel_scatter",        EmbeddedShaders::pixelScatter);
    compile("rgb_split",            EmbeddedShaders::rgbSplit);
    compile("block_glitch",         EmbeddedShaders::blockGlitch);
    compile("scanlines",            EmbeddedShaders::scanlines);
    compile("digital_rain",         EmbeddedShaders::digitalRain);
    compile("noise_overlay",        EmbeddedShaders::noiseOverlay);
    compile("mirror",               EmbeddedShaders::mirror);
    compile("pixelate",             EmbeddedShaders::pixelate);

    // Blur/Post
    compile("gaussian_blur",        EmbeddedShaders::gaussianBlur);
    compile("zoom_blur",            EmbeddedShaders::zoomBlur);
    compile("shake",                EmbeddedShaders::shake);
    compile("vignette",             EmbeddedShaders::vignette);
    compile("motion_blur",          EmbeddedShaders::motionBlur);
    compile("glow",                 EmbeddedShaders::glow);
    compile("edge_detect",          EmbeddedShaders::edgeDetect);

    // 3D/Depth
    compile("perspective_tilt",     EmbeddedShaders::perspectiveTilt);
    compile("cylinder_wrap",        EmbeddedShaders::cylinderWrap);
    compile("sphere_wrap",          EmbeddedShaders::sphereWrap);
    compile("tunnel",               EmbeddedShaders::tunnel);
    compile("page_curl",            EmbeddedShaders::pageCurl);
    compile("parallax_layers",      EmbeddedShaders::parallaxLayers);

    // Additional Warp
    compile("polar_coords",         EmbeddedShaders::polarCoords);
    compile("twirl",                EmbeddedShaders::twirl);
    compile("shear",                EmbeddedShaders::shear);
    compile("elastic_bounce",       EmbeddedShaders::elasticBounce);
    compile("ripple_pond",          EmbeddedShaders::ripplePond);
    compile("diamond_distort",      EmbeddedShaders::diamondDistort);
    compile("barrel_distort",       EmbeddedShaders::barrelDistort);
    compile("sine_grid",            EmbeddedShaders::sineGrid);
    compile("glitch_displace",      EmbeddedShaders::glitchDisplace);

    // Additional Color
    compile("sepia",                EmbeddedShaders::sepia);
    compile("cross_process",        EmbeddedShaders::crossProcess);
    compile("split_tone",           EmbeddedShaders::splitTone);
    compile("color_halftone",       EmbeddedShaders::colorHalftone);
    compile("ordered_dither",       EmbeddedShaders::orderedDither);
    compile("heat_map",             EmbeddedShaders::heatMap);
    compile("selective_color",      EmbeddedShaders::selectiveColor);
    compile("film_grain",           EmbeddedShaders::filmGrain);
    compile("gamma_levels",         EmbeddedShaders::gammaLevels);
    compile("solarize",             EmbeddedShaders::solarize);

    // Pattern/Stylization
    compile("crt_simulation",       EmbeddedShaders::crtSimulation);
    compile("vhs_effect",           EmbeddedShaders::vhsEffect);
    compile("ascii_art",            EmbeddedShaders::asciiArt);
    compile("dot_matrix",           EmbeddedShaders::dotMatrix);
    compile("crosshatch",           EmbeddedShaders::crosshatch);
    compile("emboss",               EmbeddedShaders::emboss);
    compile("oil_paint",            EmbeddedShaders::oilPaint);
    compile("pencil_sketch",        EmbeddedShaders::pencilSketch);
    compile("voronoi_glass",        EmbeddedShaders::voronoiGlass);
    compile("cross_stitch",         EmbeddedShaders::crossStitch);
    compile("night_vision",         EmbeddedShaders::nightVision);

    // Animation
    compile("strobe",               EmbeddedShaders::strobe);
    compile("pulse",                EmbeddedShaders::pulse);
    compile("slit_scan",            EmbeddedShaders::slitScan);

    // Blend/Composite
    compile("double_exposure",      EmbeddedShaders::doubleExposure);
    compile("frosted_glass",        EmbeddedShaders::frostedGlass);
    compile("prism_refract",        EmbeddedShaders::prismRefract);
    compile("rain_on_glass",        EmbeddedShaders::rainOnGlass);
    compile("hexagonalize",         EmbeddedShaders::hexagonalize);

    std::cerr << "[Renderer] All shaders compiled." << std::endl;
}

void Renderer::initEffectChain()
{
    // Load ALL effects from the EffectLibrary, organized by category
    // Effects are added in category order: warp, color, glitch, blur
    EffectLibrary lib;
    lib.registerDefaults();

    static const juce::String categoryOrder[] = {
        "3d", "warp", "color", "glitch", "pattern", "animation", "blend", "blur"
    };

    for (const auto& cat : categoryOrder)
    {
        auto names = lib.getEffectsByCategory(cat);
        for (const auto& name : names)
        {
            auto effect = lib.createEffect(name);
            if (effect)
            {
                // Start all effects disabled — user enables what they want
                effect->setEnabled(false);
                effectChain_.addEffect(std::move(effect));
            }
        }
    }

    std::cerr << "[Renderer] Loaded " << effectChain_.getNumEffects()
              << " effects from library." << std::endl;

    // Set up a few demo mappings on common effects
    mappingEngine_.clearAll();

    // Find effect indices by name
    auto findEffect = [this](const juce::String& name) -> int {
        for (int i = 0; i < effectChain_.getNumEffects(); ++i)
        {
            auto* fx = effectChain_.getEffect(i);
            if (fx && fx->getName() == name)
                return i;
        }
        return -1;
    };

    int rippleIdx   = findEffect("Ripple");
    int hueIdx      = findEffect("Hue Shift");
    int rgbIdx      = findEffect("RGB Split");
    int vignetteIdx = findEffect("Vignette");

    // Enable the 4 demo effects
    if (rippleIdx >= 0)   effectChain_.getEffect(rippleIdx)->setEnabled(true);
    if (hueIdx >= 0)      effectChain_.getEffect(hueIdx)->setEnabled(true);
    if (rgbIdx >= 0)      effectChain_.getEffect(rgbIdx)->setEnabled(true);
    if (vignetteIdx >= 0) effectChain_.getEffect(vignetteIdx)->setEnabled(true);

    // RMS → Ripple intensity
    if (rippleIdx >= 0)
    {
        Mapping m;
        m.source = MappingSource::RMS;
        m.targetEffectId = static_cast<uint32_t>(rippleIdx);
        m.targetParamIndex = 0;
        m.curve = MappingCurve::Linear;
        m.smoothing = 0.15f;
        mappingEngine_.addMapping(m);
    }

    // SpectralCentroid → Hue Shift
    if (hueIdx >= 0)
    {
        Mapping m;
        m.source = MappingSource::SpectralCentroid;
        m.targetEffectId = static_cast<uint32_t>(hueIdx);
        m.targetParamIndex = 0;
        m.curve = MappingCurve::Linear;
        m.inputMin = 200.0f;
        m.inputMax = 8000.0f;
        m.smoothing = 0.15f;
        mappingEngine_.addMapping(m);
    }

    // OnsetStrength → RGB Split
    if (rgbIdx >= 0)
    {
        Mapping m;
        m.source = MappingSource::OnsetStrength;
        m.targetEffectId = static_cast<uint32_t>(rgbIdx);
        m.targetParamIndex = 0;
        m.curve = MappingCurve::Linear;
        m.smoothing = 0.3f;
        mappingEngine_.addMapping(m);
    }

    // BandBass → Vignette intensity
    if (vignetteIdx >= 0)
    {
        Mapping m;
        m.source = MappingSource::BandBass;
        m.targetEffectId = static_cast<uint32_t>(vignetteIdx);
        m.targetParamIndex = 0;
        m.curve = MappingCurve::Linear;
        m.smoothing = 0.15f;
        mappingEngine_.addMapping(m);
    }
}
