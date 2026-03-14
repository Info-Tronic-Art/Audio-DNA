#include "Renderer.h"
#include <iostream>

using namespace juce::gl;

// Passthrough fragment shader: just samples the texture with no effect.
// Used when no effects are enabled, or as a base "no-op" shader.
static const char* passthroughFragSource = R"(
    #version 410 core

    in vec2 v_texCoord;
    out vec4 fragColor;

    uniform sampler2D u_texture;

    void main()
    {
        fragColor = texture(u_texture, v_texCoord);
    }
)";

// Embedded vertex shader (also saved as passthrough.vert for file-based loading)
static const char* vertexShaderSource = R"(
    #version 410 core

    layout(location = 0) in vec2 a_position;
    layout(location = 1) in vec2 a_texCoord;

    out vec2 v_texCoord;

    void main()
    {
        v_texCoord = a_texCoord;
        gl_Position = vec4(a_position, 0.0, 1.0);
    }
)";

// Embedded copies of the 4 fragment shaders for reliability.
// If shader files exist on disk, ShaderManager loads those instead.

static const char* rippleFragSource = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_time;
    uniform vec2  u_resolution;
    uniform float u_ripple_intensity;
    uniform float u_ripple_freq;
    uniform float u_ripple_speed;
    void main() {
        vec2 uv = v_texCoord;
        vec2 center = vec2(0.5);
        float dist = distance(uv, center);
        float intensity = u_ripple_intensity * 0.05;
        float freq = 5.0 + u_ripple_freq * 25.0;
        float speed = 1.0 + u_ripple_speed * 5.0;
        float wave = sin(dist * freq - u_time * speed) * intensity;
        vec2 dir = normalize(uv - center + vec2(0.0001));
        uv += dir * wave;
        fragColor = texture(u_texture, uv);
    }
)";

static const char* hueShiftFragSource = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_hue_shift;
    vec3 rgb2hsv(vec3 c) {
        vec4 K = vec4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
        vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
        float d = q.x - min(q.w, q.y);
        float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    vec3 hsv2rgb(vec3 c) {
        vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
        vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        vec3 hsv = rgb2hsv(color.rgb);
        hsv.x = fract(hsv.x + u_hue_shift);
        color.rgb = hsv2rgb(hsv);
        fragColor = color;
    }
)";

static const char* rgbSplitFragSource = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_rgb_split;
    uniform float u_rgb_angle;
    void main() {
        float amount = u_rgb_split * 0.03;
        float angle = u_rgb_angle * 6.28318;
        vec2 dir = vec2(cos(angle), sin(angle)) * amount;
        float r = texture(u_texture, v_texCoord + dir).r;
        float g = texture(u_texture, v_texCoord).g;
        float b = texture(u_texture, v_texCoord - dir).b;
        float a = texture(u_texture, v_texCoord).a;
        fragColor = vec4(r, g, b, a);
    }
)";

static const char* vignetteFragSource = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_vignette_int;
    uniform float u_vignette_soft;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        vec2 uv = v_texCoord * 2.0 - 1.0;
        float dist = length(uv) * 0.707;
        float softness = 0.2 + u_vignette_soft * 0.8;
        float vignette = smoothstep(1.0, 1.0 - softness, dist);
        float strength = u_vignette_int;
        color.rgb *= mix(1.0, vignette, strength);
        fragColor = color;
    }
)";

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

    // Get viewport dimensions in physical pixels (Retina-aware)
    auto* component = glContext_.getTargetComponent();
    float scale = static_cast<float>(glContext_.getRenderingScale());
    float width  = component != nullptr ? static_cast<float>(component->getWidth())  * scale : 1.0f;
    float height = component != nullptr ? static_cast<float>(component->getHeight()) * scale : 1.0f;

    // Get the default framebuffer that JUCE's context uses
    GLint defaultFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);

    // Render the effect chain
    effectChain_.render(texMgr_.getImageTexture(),
                        shaderMgr_, texMgr_, quad_,
                        time, width, height,
                        static_cast<GLuint>(defaultFBO));
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
        if (shaderMgr_.compileProgram(name, vertexShaderSource, frag))
            std::cerr << "[Renderer]   " << name << ": OK" << std::endl;
        else
            std::cerr << "[Renderer]   " << name << ": FAILED" << std::endl;
    };

    compile("passthrough", passthroughFragSource);
    compile("ripple",      rippleFragSource);
    compile("hue_shift",   hueShiftFragSource);
    compile("rgb_split",   rgbSplitFragSource);
    compile("vignette",    vignetteFragSource);

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
