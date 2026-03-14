#include "OutputWindow.h"
#include "render/EmbeddedShaders.h"
#include <iostream>

using namespace juce::gl;

// ============================================================
// OutputRenderer
// ============================================================

OutputRenderer::OutputRenderer(FeatureBus& featureBus,
                               MappingEngine& mappingEngine,
                               EffectChain& effectChain)
    : featureBus_(featureBus),
      mappingEngine_(mappingEngine),
      effectChain_(effectChain)
{
}

void OutputRenderer::attachTo(juce::Component& component)
{
    glContext_.setOpenGLVersionRequired(juce::OpenGLContext::openGL4_1);
    glContext_.setRenderer(this);
    glContext_.setContinuousRepainting(true);
    glContext_.setComponentPaintingEnabled(false);
    glContext_.attachTo(component);
}

void OutputRenderer::detach()
{
    glContext_.detach();
}

void OutputRenderer::loadImage(const juce::File& imageFile)
{
    std::lock_guard<std::mutex> lock(pendingImageMutex_);
    pendingImageFile_ = imageFile;
    hasPendingImage_ = true;
    lastImageFile_ = imageFile;
}

void OutputRenderer::newOpenGLContextCreated()
{
    std::cerr << "[OutputRenderer] GL context created." << std::endl;
    quad_.init();
    initShaders();
    startTime_ = juce::Time::getMillisecondCounterHiRes() / 1000.0;

    // Re-queue the image if we had one loaded before context recreation
    std::lock_guard<std::mutex> lock(pendingImageMutex_);
    if (lastImageFile_.existsAsFile())
    {
        std::cerr << "[OutputRenderer] Re-queuing image after context recreation" << std::endl;
        pendingImageFile_ = lastImageFile_;
        hasPendingImage_ = true;
    }
}

void OutputRenderer::renderOpenGL()
{
    // Handle pending image load
    {
        std::lock_guard<std::mutex> lock(pendingImageMutex_);
        if (hasPendingImage_)
        {
            std::cerr << "[OutputRenderer] Loading image: " << pendingImageFile_.getFullPathName() << std::endl;
            bool ok = texMgr_.loadImage(pendingImageFile_);
            std::cerr << "[OutputRenderer] Image load " << (ok ? "OK" : "FAILED")
                      << ", texID=" << texMgr_.getImageTexture()
                      << ", size=" << texMgr_.getImageWidth() << "x" << texMgr_.getImageHeight() << std::endl;
            hasPendingImage_ = false;
        }
    }

    juce::OpenGLHelpers::clear(juce::Colours::black);

    if (!texMgr_.hasImage())
    {
        static int noImageCount = 0;
        if (++noImageCount % 60 == 1)
            std::cerr << "[OutputRenderer] No image loaded yet (frame " << noImageCount << ")" << std::endl;
        return;
    }

    // Read latest audio features (lock-free)
    const FeatureSnapshot* snap = featureBus_.acquireRead();
    if (snap == nullptr)
        snap = featureBus_.getLatestRead();

    FeatureSnapshot defaultSnap;
    if (snap == nullptr)
        snap = &defaultSnap;

    // Apply mappings (shared MappingEngine writes to shared EffectChain params)
    // Note: the primary Renderer also calls this, but it's idempotent for the
    // same snapshot — both renderers read the same feature values and write the
    // same computed param values.
    mappingEngine_.processFrame(*snap, effectChain_);

    float time = static_cast<float>(
        juce::Time::getMillisecondCounterHiRes() / 1000.0 - startTime_);

    auto* component = glContext_.getTargetComponent();
    float scale = static_cast<float>(glContext_.getRenderingScale());
    float compW = component != nullptr ? static_cast<float>(component->getWidth())  * scale : 1.0f;
    float compH = component != nullptr ? static_cast<float>(component->getHeight()) * scale : 1.0f;

    // Compute letterbox viewport using loaded image aspect ratio
    float vpX = 0.0f, vpY = 0.0f, vpW = compW, vpH = compH;
    int imgW = texMgr_.getImageWidth();
    int imgH = texMgr_.getImageHeight();
    if (imgW > 0 && imgH > 0)
    {
        float scaleX = compW / static_cast<float>(imgW);
        float scaleY = compH / static_cast<float>(imgH);
        float fitScale = std::min(scaleX, scaleY);
        vpW = static_cast<float>(imgW) * fitScale;
        vpH = static_cast<float>(imgH) * fitScale;
        vpX = (compW - vpW) * 0.5f;
        vpY = (compH - vpH) * 0.5f;
    }

    GLint defaultFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);

    effectChain_.render(texMgr_.getImageTexture(),
                        shaderMgr_, texMgr_, quad_,
                        time, compW, compH,
                        static_cast<GLuint>(defaultFBO),
                        vpX, vpY, vpW, vpH);
}

void OutputRenderer::openGLContextClosing()
{
    shaderMgr_.releaseAll();
    texMgr_.release();
    quad_.release();
}

void OutputRenderer::initShaders()
{
    auto compile = [&](const juce::String& name, const char* frag) {
        shaderMgr_.compileProgram(name, EmbeddedShaders::vertex, frag);
    };

    compile("passthrough",          EmbeddedShaders::passthrough);
    compile("ripple",               EmbeddedShaders::ripple);
    compile("bulge",                EmbeddedShaders::bulge);
    compile("wave",                 EmbeddedShaders::wave);
    compile("liquid",               EmbeddedShaders::liquid);
    compile("kaleidoscope",         EmbeddedShaders::kaleidoscope);
    compile("fisheye",              EmbeddedShaders::fisheye);
    compile("swirl",                EmbeddedShaders::swirl);
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
    compile("pixel_scatter",        EmbeddedShaders::pixelScatter);
    compile("rgb_split",            EmbeddedShaders::rgbSplit);
    compile("block_glitch",         EmbeddedShaders::blockGlitch);
    compile("scanlines",            EmbeddedShaders::scanlines);
    compile("digital_rain",         EmbeddedShaders::digitalRain);
    compile("noise_overlay",        EmbeddedShaders::noiseOverlay);
    compile("mirror",               EmbeddedShaders::mirror);
    compile("pixelate",             EmbeddedShaders::pixelate);
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

    std::cerr << "[OutputRenderer] All shaders compiled." << std::endl;
}

// ============================================================
// OutputWindow
// ============================================================

OutputWindow::OutputWindow(FeatureBus& featureBus,
                           MappingEngine& mappingEngine,
                           EffectChain& effectChain)
    : DocumentWindow("Audio-DNA Output",
                     juce::Colours::black,
                     0), // No title bar buttons
      renderer_(featureBus, mappingEngine, effectChain)
{
    setUsingNativeTitleBar(false);
    setTitleBarHeight(0);

    // Add the output component directly as a child (not via content component,
    // which can leave gaps). We manage its bounds in resized().
    addAndMakeVisible(outputComponent_);
    setWantsKeyboardFocus(true);

    renderer_.attachTo(outputComponent_);
}

OutputWindow::~OutputWindow()
{
    renderer_.detach();
}

void OutputWindow::closeButtonPressed()
{
    setAlwaysOnTop(false);
    setVisible(false);
}

void OutputWindow::resized()
{
    DocumentWindow::resized();
    outputComponent_.setBounds(getLocalBounds());
}

void OutputWindow::goFullscreenOnDisplay(const juce::Displays::Display& display)
{
    auto area = display.totalArea;

    // Don't use native macOS fullscreen — it creates a new Space and
    // the GL context transition can fail. Instead, cover the display
    // with a borderless window and set it always-on-top.
    setVisible(true);
    setBounds(area);
    setAlwaysOnTop(true);
    toFront(true);

    // Ensure the output component fills the window
    outputComponent_.setBounds(getLocalBounds());
}

void OutputWindow::loadImage(const juce::File& imageFile)
{
    renderer_.loadImage(imageFile);
}

bool OutputWindow::keyPressed(const juce::KeyPress& key)
{
    if (key.isKeyCode(juce::KeyPress::escapeKey))
    {
        setAlwaysOnTop(false);
        setVisible(false);
        return true;
    }
    return false;
}
