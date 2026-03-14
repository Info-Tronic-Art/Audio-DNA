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
    DBG("Renderer: compiling shaders...");

    auto compile = [&](const juce::String& name, const char* frag) {
        if (shaderMgr_.compileProgram(name, EmbeddedShaders::vertex, frag))
            std::cerr << "[Renderer]   " << name << ": OK" << std::endl;
        else
            std::cerr << "[Renderer]   " << name << ": FAILED" << std::endl;
    };

    compile("passthrough", EmbeddedShaders::passthrough);
    compile("ripple",      EmbeddedShaders::ripple);
    compile("hue_shift",   EmbeddedShaders::hueShift);
    compile("rgb_split",   EmbeddedShaders::rgbSplit);
    compile("vignette",    EmbeddedShaders::vignette);

    DBG("Renderer: shader compilation complete");
}

void Renderer::initEffectChain()
{
    // Create the 4 demo effects in chain order
    auto ripple = std::make_unique<Effect>("Ripple", "warp", "ripple");
    ripple->addParam("intensity", "u_ripple_intensity", 0.0f);
    ripple->addParam("freq",      "u_ripple_freq",      0.5f);
    ripple->addParam("speed",     "u_ripple_speed",     0.5f);
    effectChain_.addEffect(std::move(ripple));

    auto hueShift = std::make_unique<Effect>("Hue Shift", "color", "hue_shift");
    hueShift->addParam("amount", "u_hue_shift", 0.0f);
    effectChain_.addEffect(std::move(hueShift));

    auto rgbSplit = std::make_unique<Effect>("RGB Split", "glitch", "rgb_split");
    rgbSplit->addParam("amount", "u_rgb_split", 0.0f);
    rgbSplit->addParam("angle",  "u_rgb_angle", 0.0f);
    effectChain_.addEffect(std::move(rgbSplit));

    auto vignette = std::make_unique<Effect>("Vignette", "blur", "vignette");
    vignette->addParam("intensity", "u_vignette_int",  0.0f);
    vignette->addParam("softness",  "u_vignette_soft", 0.6f);
    effectChain_.addEffect(std::move(vignette));

    // Set up demo mappings that replicate the old hardcoded UniformBridge behavior:
    //   RMS → Ripple intensity
    //   SpectralCentroid → Hue Shift amount
    //   OnsetStrength → RGB Split amount
    //   BandBass → Vignette intensity

    mappingEngine_.clearAll();

    // RMS → Ripple intensity (effect 0, param 0)
    {
        Mapping m;
        m.source = MappingSource::RMS;
        m.targetEffectId = 0;
        m.targetParamIndex = 0;
        m.curve = MappingCurve::Linear;
        m.inputMin = 0.0f;
        m.inputMax = 1.0f;
        m.outputMin = 0.0f;
        m.outputMax = 1.0f;
        m.smoothing = 0.15f;
        mappingEngine_.addMapping(m);
    }

    // Bass → Ripple freq (effect 0, param 1)
    {
        Mapping m;
        m.source = MappingSource::BandBass;
        m.targetEffectId = 0;
        m.targetParamIndex = 1;
        m.curve = MappingCurve::Linear;
        m.inputMin = 0.0f;
        m.inputMax = 1.0f;
        m.outputMin = 0.0f;
        m.outputMax = 1.0f;
        m.smoothing = 0.15f;
        mappingEngine_.addMapping(m);
    }

    // SpectralCentroid → Hue Shift amount (effect 1, param 0)
    {
        Mapping m;
        m.source = MappingSource::SpectralCentroid;
        m.targetEffectId = 1;
        m.targetParamIndex = 0;
        m.curve = MappingCurve::Linear;
        m.inputMin = 200.0f;
        m.inputMax = 8000.0f;
        m.outputMin = 0.0f;
        m.outputMax = 1.0f;
        m.smoothing = 0.15f;
        mappingEngine_.addMapping(m);
    }

    // OnsetStrength → RGB Split amount (effect 2, param 0)
    {
        Mapping m;
        m.source = MappingSource::OnsetStrength;
        m.targetEffectId = 2;
        m.targetParamIndex = 0;
        m.curve = MappingCurve::Linear;
        m.inputMin = 0.0f;
        m.inputMax = 1.0f;
        m.outputMin = 0.0f;
        m.outputMax = 1.0f;
        m.smoothing = 0.3f;
        mappingEngine_.addMapping(m);
    }

    // BandBass → Vignette intensity (effect 3, param 0)
    {
        Mapping m;
        m.source = MappingSource::BandBass;
        m.targetEffectId = 3;
        m.targetParamIndex = 0;
        m.curve = MappingCurve::Linear;
        m.inputMin = 0.0f;
        m.inputMax = 1.0f;
        m.outputMin = 0.0f;
        m.outputMax = 1.0f;
        m.smoothing = 0.15f;
        mappingEngine_.addMapping(m);
    }
}
