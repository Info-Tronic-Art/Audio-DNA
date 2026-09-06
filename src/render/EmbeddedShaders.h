#pragma once

// Embedded GLSL shader source strings shared between Renderer and OutputRenderer.
// If shader files exist on disk, ShaderManager loads those instead.

namespace EmbeddedShaders
{

inline const char* vertex = R"(
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

inline const char* passthrough = R"(
    #version 410 core

    in vec2 v_texCoord;
    out vec4 fragColor;

    uniform sampler2D u_texture;

    void main()
    {
        fragColor = texture(u_texture, v_texCoord);
    }
)";

// Opacity blend: output with adjustable alpha for layer opacity
inline const char* opacityBlend = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_opacity;
    void main()
    {
        vec4 col = texture(u_texture, v_texCoord);
        fragColor = vec4(col.rgb, col.a * u_opacity);
    }
)";

// S167-L4b fix: clip opacity blend -- scales RGB (dims toward black), leaves
// alpha untouched. See applyClipOpacity()'s comment in CompositorEngine.cpp
// for why this has to be RGB and not alpha: blendLayerOntoAccumulator's
// Multiply/Screen/Darken/Lighten modes never reference GL_SRC_ALPHA at all,
// so an alpha-only reduction (opacityBlend above) is a complete no-op under
// those modes, and a layer left at the Opaque type with layer.opacity at its
// 1.0 default disables blending entirely (a straight overwrite that also
// never reads alpha). Scaling RGB directly survives every blend mode and
// every layer type, the same way masterOpacity/layer.opacity's own
// dim-toward-black techniques do.
inline const char* clipOpacityBlend = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_opacity;
    void main()
    {
        vec4 col = texture(u_texture, v_texCoord);
        fragColor = vec4(col.rgb * u_opacity, col.a);
    }
)";

// Dry/wet compositing shader: blends effected (u_texture) with pre-effect (u_original)
inline const char* effectDryWet = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;    // effected result
    uniform sampler2D u_original;   // pre-effect original
    uniform float u_drywet;
    void main() {
        vec4 eff = texture(u_texture, v_texCoord);
        vec4 orig = texture(u_original, v_texCoord);
        fragColor = mix(orig, eff, u_drywet);
    }
)";

// P25: Composition-level transform shader
// Applies position, scale, rotation around an anchor point to the entire output.
inline const char* compTransform = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform vec2  u_comp_position;  // Normalized offset (-1 to 1)
    uniform float u_comp_scale;     // Scale factor (1.0 = 100%)
    uniform float u_comp_rotation;  // Rotation in radians
    uniform vec2  u_comp_anchor;    // Anchor point (0,0 = center)

    void main() {
        // Transform UV coordinates
        vec2 uv = v_texCoord - 0.5;  // Center at origin

        // Apply anchor offset
        uv -= u_comp_anchor * 0.5;

        // Apply rotation
        float c = cos(u_comp_rotation);
        float s = sin(u_comp_rotation);
        uv = mat2(c, -s, s, c) * uv;

        // Apply scale (inverse: smaller scale = zoom in)
        uv /= max(u_comp_scale, 0.001);

        // Undo anchor offset
        uv += u_comp_anchor * 0.5;

        // Apply position offset
        uv -= u_comp_position * 0.5;

        // Back to [0,1]
        uv += 0.5;

        // Sample with black outside bounds
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        else
            fragColor = texture(u_texture, uv);
    }
)";

// P25: Cross-deck transition shader
// Blends two textures with configurable blend mode during deck transitions.
inline const char* deckTransition = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_textureA;   // Outgoing deck
    uniform sampler2D u_textureB;   // Incoming deck
    uniform float u_progress;       // 0 = full A, 1 = full B
    uniform int u_blendMode;        // 0=Alpha, 1=Add, 2=Multiply

    void main() {
        vec4 a = texture(u_textureA, v_texCoord);
        vec4 b = texture(u_textureB, v_texCoord);
        float t = clamp(u_progress, 0.0, 1.0);

        vec4 result;
        if (u_blendMode == 1) {
            // Additive: lerp but add the contributions
            result = a * (1.0 - t) + b * t;
            result.rgb = min(result.rgb + a.rgb * b.rgb * t * (1.0 - t) * 4.0, vec3(1.0));
            result.a = 1.0;
        } else if (u_blendMode == 2) {
            // Multiply: lerp with multiply blending at crossover
            vec4 mul = vec4(a.rgb * b.rgb, 1.0);
            result = mix(mix(a, mul, t), mix(mul, b, t), t);
        } else {
            // Alpha (default): simple crossfade
            result = mix(a, b, t);
        }
        fragColor = result;
    }
)";

// === Shared GLSL Utility Functions ===
// These are prepended to shaders that need them during compilation.

inline const char* glslNoiseFunctions = R"(
float hash11(float p) { return fract(sin(p) * 43758.5453); }
float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
vec2 hash22(vec2 p) { return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453); }
vec3 hash23(vec2 p) { vec3 q=vec3(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3)),dot(p,vec2(419.2,371.9))); return fract(sin(q)*43758.5453); }

float snoise2D(vec2 v) {
    const vec4 C = vec4(0.211324865405187, 0.366025403784439, -0.577350269189626, 0.024390243902439);
    vec2 i = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);
    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod(i, 289.0);
    vec3 p = mod(((i.y + vec3(0.0, i1.y, 1.0)) * 34.0 + 1.0) * (i.y + vec3(0.0, i1.y, 1.0)), 289.0);
    p = mod(((p + i.x + vec3(0.0, i1.x, 1.0)) * 34.0 + 1.0) * (p + i.x + vec3(0.0, i1.x, 1.0)), 289.0);
    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m; m = m*m;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * (a0*a0 + h*h);
    vec3 g;
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

float fbm2D(vec2 p) {
    float f = 0.0; float w = 0.5;
    for (int i = 0; i < 5; i++) { f += w * snoise2D(p); p *= 2.0; w *= 0.5; }
    return f;
}
)";

inline const char* glslUtilFunctions = R"(
vec2 rotateUV(vec2 uv, float angle) {
    float c = cos(angle), s = sin(angle);
    return mat2(c, -s, s, c) * uv;
}

vec3 hueRotate(vec3 col, float angle) {
    float c = cos(angle * 6.28318), s = sin(angle * 6.28318);
    mat3 m = mat3(0.299+0.701*c+0.168*s, 0.587-0.587*c+0.330*s, 0.114-0.114*c-0.497*s,
                  0.299-0.299*c-0.328*s, 0.587+0.413*c+0.035*s, 0.114-0.114*c+0.292*s,
                  0.299-0.299*c+1.250*s, 0.587-0.587*c-1.050*s, 0.114+0.886*c-0.203*s);
    return m * col;
}

float remap(float val, float inMin, float inMax, float outMin, float outMax) {
    return outMin + (outMax - outMin) * clamp((val - inMin) / (inMax - inMin), 0.0, 1.0);
}
)";

inline const char* glslSDFFunctions = R"(
float sdCircle(vec2 p, float r) { return length(p) - r; }
float sdBox(vec2 p, vec2 b) { vec2 d = abs(p) - b; return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0); }
float sdHexagon(vec2 p, float r) { vec2 q = abs(p); return max(q.x * 0.866025 + q.y * 0.5, q.y) - r; }
float sdRoundedBox(vec2 p, vec2 b, float r) { vec2 q = abs(p) - b + r; return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r; }
float sdSegment(vec2 p, vec2 a, vec2 b) { vec2 pa = p-a, ba = b-a; float h = clamp(dot(pa,ba)/dot(ba,ba), 0.0, 1.0); return length(pa - ba*h); }
float sdStar(vec2 p, float r, int n, float m) {
    float an = 3.141593 / float(n);
    float en = 3.141593 / m;
    vec2 acs = vec2(cos(an), sin(an));
    vec2 ecs = vec2(cos(en), sin(en));
    float bn = mod(atan(p.x, p.y), 2.0*an) - an;
    p = length(p) * vec2(cos(bn), abs(sin(bn)));
    p -= r * acs;
    p += ecs * clamp(-dot(p, ecs), 0.0, r * acs.y / ecs.y);
    return length(p) * sign(p.x);
}
)";

inline const char* ripple = R"(
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

inline const char* hueShift = R"(
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

inline const char* rgbSplit = R"(
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

inline const char* vignette = R"(
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
        float vig = smoothstep(1.0, 1.0 - softness, dist);
        float strength = u_vignette_int;
        color.rgb *= mix(1.0, vig, strength);
        fragColor = color;
    }
)";

// ============================================================================
// WARP EFFECTS
// ============================================================================

inline const char* bulge = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_bulge_amount;
    uniform float u_bulge_center_x;
    uniform float u_bulge_center_y;
    void main() {
        vec2 center = vec2(u_bulge_center_x, u_bulge_center_y);
        vec2 uv = v_texCoord;
        vec2 delta = uv - center;
        float dist = length(delta);
        float radius = 0.5;
        float amount = u_bulge_amount * 2.0 - 1.0; // remap [0,1] to [-1,1]
        if (dist < radius) {
            float pct = dist / radius;
            float theta = pct;
            if (amount > 0.0) {
                theta = pow(pct, 1.0 + amount * 2.0);
            } else {
                theta = pow(pct, 1.0 / (1.0 - amount * 2.0));
            }
            uv = center + normalize(delta + vec2(0.0001)) * theta * radius;
        }
        fragColor = texture(u_texture, uv);
    }
)";

inline const char* wave = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_wave_amp;
    uniform float u_wave_freq;
    uniform float u_wave_direction;
    uniform float u_time;
    void main() {
        vec2 uv = v_texCoord;
        float amp = u_wave_amp * 0.05;
        float freq = 2.0 + u_wave_freq * 20.0;
        float angle = u_wave_direction * 3.14159265;
        vec2 dir = vec2(cos(angle), sin(angle));
        vec2 perp = vec2(-dir.y, dir.x);
        float phase = dot(uv, dir) * freq - u_time * 3.0;
        uv += perp * sin(phase) * amp;
        fragColor = texture(u_texture, uv);
    }
)";

inline const char* liquid = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_liquid_visc;
    uniform float u_liquid_turb;
    uniform float u_time;

    // Simplex-inspired hash-based noise
    vec2 hash(vec2 p) {
        p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
        return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
    }
    float noise(vec2 p) {
        vec2 i = floor(p);
        vec2 f = fract(p);
        vec2 u = f * f * (3.0 - 2.0 * f);
        return mix(mix(dot(hash(i + vec2(0.0,0.0)), f - vec2(0.0,0.0)),
                       dot(hash(i + vec2(1.0,0.0)), f - vec2(1.0,0.0)), u.x),
                   mix(dot(hash(i + vec2(0.0,1.0)), f - vec2(0.0,1.0)),
                       dot(hash(i + vec2(1.0,1.0)), f - vec2(1.0,1.0)), u.x), u.y);
    }
    float fbm(vec2 p) {
        float val = 0.0;
        float amp = 0.5;
        for (int i = 0; i < 5; i++) {
            val += amp * noise(p);
            p *= 2.0;
            amp *= 0.5;
        }
        return val;
    }
    void main() {
        vec2 uv = v_texCoord;
        float speed = 0.3 + (1.0 - u_liquid_visc) * 2.0;
        float strength = u_liquid_turb * 0.08;
        float t = u_time * speed;
        vec2 offset;
        offset.x = fbm(uv * 3.0 + vec2(t * 0.7, t * 0.3));
        offset.y = fbm(uv * 3.0 + vec2(t * -0.4, t * 0.6) + vec2(5.2, 1.3));
        uv += offset * strength;
        fragColor = texture(u_texture, uv);
    }
)";

inline const char* kaleidoscope = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_kaleidoscope_segments;
    uniform float u_kaleidoscope_rotation;
    uniform float u_time;
    void main() {
        vec2 uv = v_texCoord - 0.5;
        float segments = 2.0 + u_kaleidoscope_segments * 14.0; // 2 to 16 segments
        float rotation = u_kaleidoscope_rotation * 6.28318 + u_time * 0.5;
        float angle = atan(uv.y, uv.x) + rotation;
        float r = length(uv);
        float segAngle = 6.28318 / segments;
        angle = mod(angle, segAngle);
        // Mirror alternating segments
        if (mod(floor((atan(uv.y, uv.x) + rotation) / segAngle), 2.0) >= 1.0) {
            angle = segAngle - angle;
        }
        vec2 newUv = vec2(cos(angle), sin(angle)) * r + 0.5;
        fragColor = texture(u_texture, newUv);
    }
)";

inline const char* fisheye = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_fisheye_amount;
    void main() {
        vec2 uv = v_texCoord * 2.0 - 1.0; // [-1, 1]
        float amount = u_fisheye_amount * 2.0 - 1.0; // [-1, 1]: negative=pincushion, positive=barrel
        float r = length(uv);
        float theta = atan(uv.y, uv.x);
        if (amount > 0.0) {
            // Barrel distortion
            float power = 1.0 + amount * 1.5;
            r = pow(r, power);
        } else {
            // Pincushion distortion
            float power = 1.0 / (1.0 - amount * 1.5);
            r = pow(r, power);
        }
        vec2 distorted = vec2(cos(theta), sin(theta)) * r;
        vec2 finalUv = distorted * 0.5 + 0.5;
        if (finalUv.x < 0.0 || finalUv.x > 1.0 || finalUv.y < 0.0 || finalUv.y > 1.0)
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        else
            fragColor = texture(u_texture, finalUv);
    }
)";

inline const char* swirl = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_swirl_amount;
    uniform float u_swirl_radius;
    uniform float u_time;
    void main() {
        vec2 center = vec2(0.5);
        vec2 uv = v_texCoord;
        vec2 delta = uv - center;
        float dist = length(delta);
        float radius = 0.1 + u_swirl_radius * 0.9;
        float angle = (u_swirl_amount * 2.0 - 1.0) * 10.0; // [-10, 10] radians max
        if (dist < radius) {
            float pct = (radius - dist) / radius;
            float swirlAngle = pct * pct * angle;
            float s = sin(swirlAngle);
            float c = cos(swirlAngle);
            delta = vec2(c * delta.x - s * delta.y, s * delta.x + c * delta.y);
            uv = center + delta;
        }
        fragColor = texture(u_texture, uv);
    }
)";

// ============================================================================
// COLOR EFFECTS
// ============================================================================

inline const char* saturation = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_saturation;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float sat = u_saturation * 3.0; // 0=grayscale, 1.5=normal, 3.0=oversaturated
        float luma = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        color.rgb = mix(vec3(luma), color.rgb, sat);
        fragColor = color;
    }
)";

inline const char* brightness = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_brightness;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float b = u_brightness * 2.0 - 1.0; // [-1, 1]: negative=darken, positive=brighten
        color.rgb += b;
        color.rgb = clamp(color.rgb, 0.0, 1.0);
        fragColor = color;
    }
)";

inline const char* duotone = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_duotone_a_r;
    uniform float u_duotone_a_g;
    uniform float u_duotone_a_b;
    uniform float u_duotone_b_r;
    uniform float u_duotone_b_g;
    uniform float u_duotone_b_b;
    uniform float u_duotone_mix;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float luma = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        vec3 colorA = vec3(u_duotone_a_r, u_duotone_a_g, u_duotone_a_b);
        vec3 colorB = vec3(u_duotone_b_r, u_duotone_b_g, u_duotone_b_b);
        vec3 duotoned = mix(colorA, colorB, luma);
        color.rgb = mix(color.rgb, duotoned, u_duotone_mix);
        fragColor = color;
    }
)";

inline const char* chromaticAberration = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_chroma_amount;
    uniform float u_chroma_angle;
    void main() {
        float amount = u_chroma_amount * 0.04;
        float angle = u_chroma_angle * 6.28318;
        vec2 dir = vec2(cos(angle), sin(angle)) * amount;
        // Radial weighting: stronger aberration at edges
        vec2 fromCenter = v_texCoord - 0.5;
        float edgeFactor = length(fromCenter) * 2.0;
        vec2 offset = dir * edgeFactor;
        float r = texture(u_texture, v_texCoord + offset).r;
        float g = texture(u_texture, v_texCoord).g;
        float b = texture(u_texture, v_texCoord - offset).b;
        float a = texture(u_texture, v_texCoord).a;
        fragColor = vec4(r, g, b, a);
    }
)";

inline const char* invert = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_invert_amount;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        vec3 inverted = 1.0 - color.rgb;
        color.rgb = mix(color.rgb, inverted, u_invert_amount);
        fragColor = color;
    }
)";

inline const char* posterize = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_posterize_levels;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float levels = 2.0 + u_posterize_levels * 30.0; // 2 to 32 levels
        color.rgb = floor(color.rgb * levels + 0.5) / levels;
        fragColor = color;
    }
)";

inline const char* colorShift = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_color_shift_r;
    uniform float u_color_shift_g;
    uniform float u_color_shift_b;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        // Each channel shifts [-1, 1]
        color.r += u_color_shift_r * 2.0 - 1.0;
        color.g += u_color_shift_g * 2.0 - 1.0;
        color.b += u_color_shift_b * 2.0 - 1.0;
        color.rgb = clamp(color.rgb, 0.0, 1.0);
        fragColor = color;
    }
)";

inline const char* thermal = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_thermal_amount;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float luma = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        // Thermal palette: black -> blue -> magenta -> red -> yellow -> white
        vec3 thermal;
        if (luma < 0.2) {
            thermal = mix(vec3(0.0, 0.0, 0.1), vec3(0.1, 0.0, 0.6), luma / 0.2);
        } else if (luma < 0.4) {
            thermal = mix(vec3(0.1, 0.0, 0.6), vec3(0.7, 0.0, 0.5), (luma - 0.2) / 0.2);
        } else if (luma < 0.6) {
            thermal = mix(vec3(0.7, 0.0, 0.5), vec3(1.0, 0.2, 0.0), (luma - 0.4) / 0.2);
        } else if (luma < 0.8) {
            thermal = mix(vec3(1.0, 0.2, 0.0), vec3(1.0, 0.9, 0.0), (luma - 0.6) / 0.2);
        } else {
            thermal = mix(vec3(1.0, 0.9, 0.0), vec3(1.0, 1.0, 1.0), (luma - 0.8) / 0.2);
        }
        color.rgb = mix(color.rgb, thermal, u_thermal_amount);
        fragColor = color;
    }
)";

inline const char* colorMatrix = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_contrast;
    uniform float u_color_balance;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        // Contrast: remap [0,1] to [-1,1] range for adjustment
        float contrast = u_contrast * 4.0 - 1.0; // -1 to 3
        color.rgb = (color.rgb - 0.5) * max(contrast + 1.0, 0.0) + 0.5;
        // Color balance: shift warm/cool
        float balance = u_color_balance * 2.0 - 1.0; // [-1,1]
        color.r += balance * 0.15;
        color.b -= balance * 0.15;
        color.g += abs(balance) * 0.03; // slight green compensation
        color.rgb = clamp(color.rgb, 0.0, 1.0);
        fragColor = color;
    }
)";

// ============================================================================
// GLITCH EFFECTS
// ============================================================================

inline const char* pixelScatter = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_scatter_amount;
    uniform float u_scatter_seed;
    uniform float u_time;

    float hash21(vec2 p) {
        p = fract(p * vec2(234.34, 435.345));
        p += dot(p, p + 34.23);
        return fract(p.x * p.y);
    }
    void main() {
        vec2 uv = v_texCoord;
        float amount = u_scatter_amount * 0.1;
        float seed = u_scatter_seed * 100.0 + floor(u_time * 8.0);
        vec2 blockUv = floor(uv * 64.0) / 64.0;
        float r1 = hash21(blockUv + seed);
        float r2 = hash21(blockUv + seed + 17.0);
        // Only scatter some pixels (probability based on amount)
        if (hash21(blockUv + seed + 31.0) < u_scatter_amount) {
            uv += (vec2(r1, r2) * 2.0 - 1.0) * amount;
        }
        fragColor = texture(u_texture, uv);
    }
)";

inline const char* blockGlitch = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_block_glitch_int;
    uniform float u_block_glitch_size;
    uniform float u_time;

    float hash(float n) { return fract(sin(n) * 43758.5453); }
    float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

    void main() {
        vec2 uv = v_texCoord;
        float blockSize = 0.02 + u_block_glitch_size * 0.15;
        float t = floor(u_time * 6.0); // Update 6 times per second
        vec2 block = floor(uv / blockSize);
        float rnd = hash21(block + t);
        // Only glitch some blocks
        if (rnd < u_block_glitch_int * 0.5) {
            float shift = (hash(rnd * 13.0 + t) * 2.0 - 1.0) * u_block_glitch_int * 0.2;
            uv.x += shift;
            // Occasionally do color channel separation in glitched blocks
            if (rnd < u_block_glitch_int * 0.15) {
                float r = texture(u_texture, uv + vec2(shift * 0.5, 0.0)).r;
                float g = texture(u_texture, uv).g;
                float b = texture(u_texture, uv - vec2(shift * 0.5, 0.0)).b;
                fragColor = vec4(r, g, b, 1.0);
                return;
            }
        }
        fragColor = texture(u_texture, uv);
    }
)";

inline const char* scanlines = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_scanline_int;
    uniform float u_scanline_freq;
    uniform float u_time;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float freq = 100.0 + u_scanline_freq * 800.0; // 100 to 900 lines
        float scroll = u_time * 30.0;
        float scanline = sin((v_texCoord.y * freq + scroll) * 3.14159) * 0.5 + 0.5;
        scanline = pow(scanline, 0.5); // Sharpen the lines
        float darkening = mix(1.0, scanline, u_scanline_int * 0.6);
        // Add slight horizontal jitter on strong lines
        float jitter = sin(v_texCoord.y * freq * 2.0 + u_time * 5.0) * u_scanline_int * 0.003;
        vec4 jittered = texture(u_texture, v_texCoord + vec2(jitter, 0.0));
        color.rgb = mix(color.rgb, jittered.rgb, u_scanline_int * 0.3);
        color.rgb *= darkening;
        fragColor = color;
    }
)";

inline const char* digitalRain = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_rain_intensity;
    uniform float u_rain_speed;
    uniform float u_time;

    float hash21(vec2 p) {
        p = fract(p * vec2(234.34, 435.345));
        p += dot(p, p + 34.23);
        return fract(p.x * p.y);
    }
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float cellSize = 0.025;
        vec2 cell = floor(v_texCoord / cellSize);
        float speed = 1.0 + u_rain_speed * 5.0;
        // Each column has a different speed and phase
        float colSpeed = 0.5 + hash21(vec2(cell.x, 0.0)) * 2.0;
        float colPhase = hash21(vec2(cell.x, 1.0)) * 100.0;
        float drop = fract(-u_time * speed * colSpeed * 0.2 + colPhase + cell.y * 0.05);
        // Create a trail effect
        float trail = pow(drop, 3.0);
        // Character-like pattern using hash
        float charPattern = step(0.3, hash21(cell + floor(u_time * speed * 2.0)));
        float brightness = trail * charPattern;
        // Green-tinted rain
        vec3 rain = vec3(0.1, 1.0, 0.3) * brightness;
        color.rgb = mix(color.rgb, color.rgb + rain, u_rain_intensity);
        fragColor = color;
    }
)";

inline const char* noiseOverlay = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_noise_amount;
    uniform float u_noise_speed;
    uniform float u_time;

    float hash21(vec2 p) {
        vec3 p3 = fract(vec3(p.xyx) * 0.1031);
        p3 += dot(p3, p3.yzx + 33.33);
        return fract((p3.x + p3.y) * p3.z);
    }
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float speed = 1.0 + u_noise_speed * 30.0;
        float seed = floor(u_time * speed);
        // Fine film grain
        float grain = hash21(v_texCoord * 1000.0 + seed) * 2.0 - 1.0;
        // Add some larger noise chunks for digital feel
        vec2 coarse = floor(v_texCoord * 200.0);
        float digital = hash21(coarse + seed * 7.0) * 2.0 - 1.0;
        float noise = mix(grain, digital, 0.3);
        color.rgb += noise * u_noise_amount * 0.5;
        color.rgb = clamp(color.rgb, 0.0, 1.0);
        fragColor = color;
    }
)";

inline const char* mirror = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_mirror_x;
    uniform float u_mirror_y;
    void main() {
        vec2 uv = v_texCoord;
        // Mirror X: when amount > 0.5, mirror left half to right
        if (u_mirror_x > 0.5) {
            uv.x = uv.x < 0.5 ? uv.x : 1.0 - uv.x;
        }
        // Mirror Y: when amount > 0.5, mirror top half to bottom
        if (u_mirror_y > 0.5) {
            uv.y = uv.y < 0.5 ? uv.y : 1.0 - uv.y;
        }
        fragColor = texture(u_texture, uv);
    }
)";

inline const char* pixelate = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_pixelate_size;
    uniform vec2 u_resolution;
    void main() {
        // Map [0,1] to pixel sizes: 1px (no effect) to ~64px blocks
        float size = 1.0 + u_pixelate_size * 63.0;
        vec2 texSize = u_resolution;
        vec2 cellCount = texSize / size;
        vec2 uv = floor(v_texCoord * cellCount + 0.5) / cellCount;
        fragColor = texture(u_texture, uv);
    }
)";

// ============================================================================
// BLUR / POST EFFECTS
// ============================================================================

inline const char* gaussianBlur = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_blur_radius;
    uniform vec2 u_resolution;
    void main() {
        // 9-tap Gaussian approximation, applied in both directions
        float radius = u_blur_radius * 10.0; // 0 to 10 pixels
        vec2 texel = 1.0 / u_resolution;
        // Gaussian weights for sigma ~= radius/3 (precomputed for 9-tap)
        float weights[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);
        vec4 result = texture(u_texture, v_texCoord) * weights[0];
        // Horizontal pass
        for (int i = 1; i < 5; i++) {
            vec2 offset = vec2(float(i) * radius * texel.x, 0.0);
            result += texture(u_texture, v_texCoord + offset) * weights[i];
            result += texture(u_texture, v_texCoord - offset) * weights[i];
        }
        // Vertical pass on the accumulated result
        // For a single-pass approximation, we sample diagonally too
        vec4 result2 = texture(u_texture, v_texCoord) * weights[0];
        for (int i = 1; i < 5; i++) {
            vec2 offset = vec2(0.0, float(i) * radius * texel.y);
            result2 += texture(u_texture, v_texCoord + offset) * weights[i];
            result2 += texture(u_texture, v_texCoord - offset) * weights[i];
        }
        // Average both passes
        fragColor = (result + result2) * 0.5;
    }
)";

inline const char* zoomBlur = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_zoom_blur;
    uniform float u_zoom_center_x;
    uniform float u_zoom_center_y;
    void main() {
        vec2 center = vec2(u_zoom_center_x, u_zoom_center_y);
        vec2 dir = v_texCoord - center;
        float amount = u_zoom_blur * 0.1;
        const int samples = 16;
        vec4 color = vec4(0.0);
        float totalWeight = 0.0;
        for (int i = 0; i < samples; i++) {
            float t = float(i) / float(samples - 1);
            float weight = 1.0 - t * 0.5; // Weight closer samples more
            vec2 sampleUv = v_texCoord - dir * t * amount;
            color += texture(u_texture, sampleUv) * weight;
            totalWeight += weight;
        }
        fragColor = color / totalWeight;
    }
)";

inline const char* shake = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_shake_x;
    uniform float u_shake_y;
    void main() {
        vec2 offset = vec2(
            (u_shake_x * 2.0 - 1.0) * 0.05,
            (u_shake_y * 2.0 - 1.0) * 0.05
        );
        fragColor = texture(u_texture, v_texCoord + offset);
    }
)";

inline const char* motionBlur = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_motion_blur_amount;
    uniform float u_motion_blur_angle;
    uniform vec2 u_resolution;
    void main() {
        float angle = u_motion_blur_angle * 6.28318;
        float amount = u_motion_blur_amount * 20.0; // up to 20 pixels
        vec2 dir = vec2(cos(angle), sin(angle)) / u_resolution * amount;
        const int samples = 16;
        vec4 color = vec4(0.0);
        for (int i = 0; i < samples; i++) {
            float t = (float(i) / float(samples - 1)) - 0.5; // [-0.5, 0.5]
            color += texture(u_texture, v_texCoord + dir * t);
        }
        fragColor = color / float(samples);
    }
)";

inline const char* glow = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_glow_amount;
    uniform float u_glow_threshold;
    uniform vec2 u_resolution;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        vec2 texel = 1.0 / u_resolution;
        float threshold = u_glow_threshold;
        // Extract bright areas and blur them
        vec4 bright = vec4(0.0);
        float totalWeight = 0.0;
        // 13-tap blur kernel for glow
        for (int x = -3; x <= 3; x++) {
            for (int y = -3; y <= 3; y++) {
                float radius = 3.0 + u_glow_amount * 12.0;
                vec2 offset = vec2(float(x), float(y)) * texel * radius;
                vec4 s = texture(u_texture, v_texCoord + offset);
                float luma = dot(s.rgb, vec3(0.2126, 0.7152, 0.0722));
                // Only include bright pixels
                float brightPass = max(0.0, luma - threshold) / max(1.0 - threshold, 0.001);
                float weight = exp(-float(x*x + y*y) / 8.0);
                bright += s * brightPass * weight;
                totalWeight += weight;
            }
        }
        bright /= totalWeight;
        // Add glow to original
        color.rgb += bright.rgb * u_glow_amount * 2.0;
        fragColor = color;
    }
)";

inline const char* edgeDetect = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_edge_amount;
    uniform vec2 u_resolution;
    void main() {
        vec2 texel = 1.0 / u_resolution;
        vec4 color = texture(u_texture, v_texCoord);
        // Sobel operator
        float tl = dot(texture(u_texture, v_texCoord + vec2(-texel.x,  texel.y)).rgb, vec3(0.2126, 0.7152, 0.0722));
        float t  = dot(texture(u_texture, v_texCoord + vec2( 0.0,      texel.y)).rgb, vec3(0.2126, 0.7152, 0.0722));
        float tr = dot(texture(u_texture, v_texCoord + vec2( texel.x,  texel.y)).rgb, vec3(0.2126, 0.7152, 0.0722));
        float l  = dot(texture(u_texture, v_texCoord + vec2(-texel.x,  0.0    )).rgb, vec3(0.2126, 0.7152, 0.0722));
        float r  = dot(texture(u_texture, v_texCoord + vec2( texel.x,  0.0    )).rgb, vec3(0.2126, 0.7152, 0.0722));
        float bl = dot(texture(u_texture, v_texCoord + vec2(-texel.x, -texel.y)).rgb, vec3(0.2126, 0.7152, 0.0722));
        float b  = dot(texture(u_texture, v_texCoord + vec2( 0.0,     -texel.y)).rgb, vec3(0.2126, 0.7152, 0.0722));
        float br = dot(texture(u_texture, v_texCoord + vec2( texel.x, -texel.y)).rgb, vec3(0.2126, 0.7152, 0.0722));
        // Sobel kernels
        float gx = -tl - 2.0*l - bl + tr + 2.0*r + br;
        float gy = -tl - 2.0*t - tr + bl + 2.0*b + br;
        float edge = sqrt(gx * gx + gy * gy);
        edge = clamp(edge * 3.0, 0.0, 1.0); // Amplify edges
        // Mix: 0 = original, 1 = edges only (white on black)
        vec3 edgeColor = vec3(edge);
        color.rgb = mix(color.rgb, edgeColor, u_edge_amount);
        fragColor = color;
    }
)";

// ============================================================================
// 3D / DEPTH EFFECTS
// ============================================================================

inline const char* perspectiveTilt = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_tilt_x;
    uniform float u_tilt_y;
    uniform vec2 u_resolution;
    void main() {
        vec2 uv = v_texCoord;
        float tiltX = (u_tilt_x * 2.0 - 1.0) * 1.5;
        float tiltY = (u_tilt_y * 2.0 - 1.0) * 1.5;
        float z = 1.0 + (uv.y - 0.5) * tiltX + (uv.x - 0.5) * tiltY;
        vec2 newUV = 0.5 + (uv - 0.5) / max(z, 0.01);
        if (newUV.x < 0.0 || newUV.x > 1.0 || newUV.y < 0.0 || newUV.y > 1.0)
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        else
            fragColor = texture(u_texture, newUV);
    }
)";

inline const char* cylinderWrap = R"(
    #version 410 core
    #define PI 3.14159265359
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_cylinder_amount;
    uniform float u_cylinder_axis;
    void main() {
        vec2 uv = v_texCoord;
        float amount = u_cylinder_amount;
        float axisBlend = u_cylinder_axis;
        // Horizontal cylinder (axis = 0) vs vertical (axis = 1)
        vec2 centered = uv - 0.5;
        // Horizontal wrap
        float angleH = centered.x * PI * amount;
        float xH = sin(angleH) * 0.5 / max(amount * PI * 0.5, 0.001);
        float zH = cos(angleH);
        float newXH = xH + 0.5;
        float darkenH = max(zH, 0.0);
        // Vertical wrap
        float angleV = centered.y * PI * amount;
        float yV = sin(angleV) * 0.5 / max(amount * PI * 0.5, 0.001);
        float zV = cos(angleV);
        float newYV = yV + 0.5;
        float darkenV = max(zV, 0.0);
        vec2 newUV = vec2(
            mix(newXH, uv.x, axisBlend),
            mix(uv.y, newYV, axisBlend)
        );
        float darken = mix(darkenH, darkenV, axisBlend);
        if (newUV.x < 0.0 || newUV.x > 1.0 || newUV.y < 0.0 || newUV.y > 1.0)
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        else {
            vec4 color = texture(u_texture, newUV);
            color.rgb *= mix(1.0, darken, amount);
            fragColor = color;
        }
    }
)";

inline const char* sphereWrap = R"(
    #version 410 core
    #define PI 3.14159265359
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_sphere_amount;
    void main() {
        vec2 uv = v_texCoord * 2.0 - 1.0;
        float dist = length(uv);
        float amount = u_sphere_amount;
        if (dist > 1.0) {
            fragColor = mix(texture(u_texture, v_texCoord), vec4(0.0, 0.0, 0.0, 1.0), amount);
            return;
        }
        // Sphere normal
        float z = sqrt(1.0 - dist * dist);
        // Refract/map UV through sphere
        vec2 sphereUV = uv / (z + 1.0);
        vec2 newUV = mix(v_texCoord, sphereUV * 0.5 + 0.5, amount);
        vec4 color = texture(u_texture, newUV);
        // Darken edges based on z-normal (Lambertian-like)
        float shading = mix(1.0, 0.3 + 0.7 * z, amount);
        // Specular highlight
        float spec = pow(max(dot(vec3(uv, z), normalize(vec3(0.3, 0.3, 1.0))), 0.0), 32.0);
        color.rgb *= shading;
        color.rgb += spec * amount * 0.3;
        fragColor = color;
    }
)";

inline const char* tunnel = R"(
    #version 410 core
    #define TAU 6.28318530718
    #define PI 3.14159265359
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_tunnel_speed;
    uniform float u_tunnel_radius;
    uniform float u_time;
    void main() {
        vec2 uv = v_texCoord * 2.0 - 1.0;
        float dist = length(uv);
        float angle = atan(uv.y, uv.x);
        float speed = u_tunnel_speed * 2.0;
        float radius = 0.05 + u_tunnel_radius * 0.45;
        // Prevent division by zero near center
        float tunnelDist = max(dist, radius);
        vec2 tunnelUV = vec2(
            angle / TAU + 0.5,
            radius / tunnelDist + u_time * speed
        );
        vec4 color = texture(u_texture, fract(tunnelUV));
        // Darken center (depth fog)
        float fog = smoothstep(0.0, 0.5, dist);
        color.rgb *= fog;
        fragColor = color;
    }
)";

inline const char* pageCurl = R"(
    #version 410 core
    #define PI 3.14159265359
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_curl_amount;
    uniform float u_curl_radius;
    void main() {
        vec2 uv = v_texCoord;
        float amount = u_curl_amount;
        float radius = 0.02 + u_curl_radius * 0.15;
        // Curl line moves from right to left as amount increases
        float curlX = 1.0 - amount;
        if (uv.x > curlX) {
            // Distance into the curl region
            float d = uv.x - curlX;
            // Angle around the cylinder
            float theta = d / radius;
            if (theta < PI) {
                // On the curled cylinder surface
                float cylX = curlX - radius * (1.0 - cos(theta));
                float cylY = uv.y;
                // Shade based on angle
                float shade = 0.5 + 0.5 * cos(theta);
                if (cylX >= 0.0 && cylX <= 1.0) {
                    vec4 color = texture(u_texture, vec2(cylX, cylY));
                    color.rgb *= shade;
                    fragColor = color;
                } else {
                    fragColor = vec4(0.0, 0.0, 0.0, 1.0);
                }
            } else {
                // Behind the curl - show back of page (darkened)
                float backX = curlX - radius * 2.0 + (d - PI * radius);
                if (backX >= 0.0 && backX <= 1.0) {
                    vec4 color = texture(u_texture, vec2(backX, uv.y));
                    color.rgb *= 0.3;
                    fragColor = color;
                } else {
                    fragColor = vec4(0.0, 0.0, 0.0, 1.0);
                }
            }
        } else {
            // Shadow near curl edge
            float shadow = smoothstep(curlX - 0.1, curlX, uv.x);
            vec4 color = texture(u_texture, uv);
            color.rgb *= 1.0 - shadow * 0.3 * amount;
            fragColor = color;
        }
    }
)";

inline const char* parallaxLayers = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_parallax_amount;
    uniform float u_parallax_direction;
    void main() {
        vec2 uv = v_texCoord;
        float amount = u_parallax_amount * 0.05;
        float angle = u_parallax_direction * 6.28318;
        vec2 dir = vec2(cos(angle), sin(angle));
        // Sample brightness as depth
        float depth = dot(texture(u_texture, uv).rgb, vec3(0.2126, 0.7152, 0.0722));
        // Offset UV by depth * direction
        vec2 offset = dir * depth * amount;
        fragColor = texture(u_texture, uv + offset);
    }
)";

// ============================================================================
// ADVANCED WARP EFFECTS
// ============================================================================

inline const char* polarCoords = R"(
    #version 410 core
    #define TAU 6.28318530718
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_polar_amount;
    void main() {
        vec2 uv = v_texCoord;
        vec2 centered = uv - 0.5;
        float angle = atan(centered.y, centered.x);
        float radius = length(centered);
        vec2 polarUV = vec2(angle / TAU + 0.5, radius * 2.0);
        vec2 newUV = mix(uv, polarUV, u_polar_amount);
        fragColor = texture(u_texture, fract(newUV));
    }
)";

inline const char* twirl = R"(
    #version 410 core
    #define TAU 6.28318530718
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_twirl_amount;
    uniform float u_twirl_radius;
    void main() {
        vec2 center = vec2(0.5);
        vec2 uv = v_texCoord;
        vec2 delta = uv - center;
        float dist = length(delta);
        float radius = 0.1 + u_twirl_radius * 0.9;
        float amount = (u_twirl_amount * 2.0 - 1.0) * TAU * 2.0;
        if (dist < radius) {
            float pct = 1.0 - dist / radius;
            float angle = pct * pct * amount;
            float s = sin(angle);
            float c = cos(angle);
            delta = vec2(c * delta.x - s * delta.y, s * delta.x + c * delta.y);
            uv = center + delta;
        }
        fragColor = texture(u_texture, uv);
    }
)";

inline const char* shear = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_shear_x;
    uniform float u_shear_y;
    void main() {
        vec2 uv = v_texCoord;
        float sx = (u_shear_x * 2.0 - 1.0) * 1.0;
        float sy = (u_shear_y * 2.0 - 1.0) * 1.0;
        uv.x += (uv.y - 0.5) * sx;
        uv.y += (uv.x - 0.5) * sy;
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        else
            fragColor = texture(u_texture, uv);
    }
)";

inline const char* elasticBounce = R"(
    #version 410 core
    #define PI 3.14159265359
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_elastic_amount;
    uniform float u_elastic_freq;
    uniform float u_time;
    void main() {
        vec2 uv = v_texCoord;
        float amount = u_elastic_amount * 0.06;
        float freq = 2.0 + u_elastic_freq * 18.0;
        float decay = exp(-abs(uv.y - 0.5) * 4.0);
        float offsetX = amount * sin(uv.y * freq * PI + u_time * 5.0) * decay;
        float offsetY = amount * sin(uv.x * freq * PI + u_time * 5.0 + PI * 0.5) * decay;
        uv.x += offsetX;
        uv.y += offsetY;
        fragColor = texture(u_texture, uv);
    }
)";

inline const char* ripplePond = R"(
    #version 410 core
    #define PI 3.14159265359
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_pond_intensity;
    uniform float u_pond_freq;
    uniform float u_time;
    void main() {
        vec2 uv = v_texCoord;
        float intensity = u_pond_intensity * 0.03;
        float freq = 10.0 + u_pond_freq * 40.0;
        // 4 ripple centers
        vec2 centers[4] = vec2[](
            vec2(0.3, 0.3),
            vec2(0.7, 0.7),
            vec2(0.2, 0.7),
            vec2(0.8, 0.3)
        );
        vec2 totalOffset = vec2(0.0);
        for (int i = 0; i < 4; i++) {
            vec2 delta = uv - centers[i];
            float dist = length(delta);
            float wave = sin(dist * freq - u_time * 4.0) / (1.0 + dist * 10.0);
            vec2 dir = normalize(delta + vec2(0.0001));
            totalOffset += dir * wave;
        }
        uv += totalOffset * intensity;
        fragColor = texture(u_texture, uv);
    }
)";

inline const char* diamondDistort = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_diamond_size;
    uniform float u_diamond_amount;
    void main() {
        vec2 uv = v_texCoord;
        float size = 0.02 + u_diamond_size * 0.15;
        float amount = u_diamond_amount * 0.03;
        // Diamond grid cell
        vec2 cell = floor(uv / size);
        vec2 cellUV = fract(uv / size) - 0.5;
        // Manhattan distance from cell center
        float manhattan = abs(cellUV.x) + abs(cellUV.y);
        // Offset toward/away from center based on Manhattan distance
        vec2 gradient = sign(cellUV) * amount;
        vec2 offset = gradient * (1.0 - manhattan);
        fragColor = texture(u_texture, uv + offset);
    }
)";

inline const char* barrelDistort = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_barrel_amount;
    void main() {
        vec2 uv = v_texCoord;
        vec2 offset = uv - 0.5;
        float r2 = dot(offset, offset);
        float amount = (u_barrel_amount * 2.0 - 1.0) * 2.0; // [-2, 2]
        vec2 newUV = 0.5 + offset * (1.0 + amount * r2);
        if (newUV.x < 0.0 || newUV.x > 1.0 || newUV.y < 0.0 || newUV.y > 1.0)
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        else
            fragColor = texture(u_texture, newUV);
    }
)";

inline const char* sineGrid = R"(
    #version 410 core
    #define PI 3.14159265359
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_sinegrid_freq;
    uniform float u_sinegrid_amount;
    uniform float u_time;
    void main() {
        vec2 uv = v_texCoord;
        float freq = 3.0 + u_sinegrid_freq * 20.0;
        float amount = u_sinegrid_amount * 0.04;
        float t = u_time * 2.0;
        uv.x += sin(uv.y * freq * PI + t) * amount;
        uv.y += sin(uv.x * freq * PI + t * 1.3) * amount;
        fragColor = texture(u_texture, uv);
    }
)";

inline const char* glitchDisplace = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_glitchdisp_amount;
    uniform float u_glitchdisp_speed;
    uniform float u_time;

    float hash(float n) { return fract(sin(n) * 43758.5453); }

    void main() {
        vec2 uv = v_texCoord;
        float speed = 2.0 + u_glitchdisp_speed * 20.0;
        float t = floor(u_time * speed);
        float amount = u_glitchdisp_amount * 0.08;
        // Row-based displacement
        float row = floor(uv.y * 80.0);
        float rowHash = hash(row + t * 7.0);
        // Only displace some rows
        if (rowHash > 0.7) {
            float shift = (hash(row + t * 13.0) * 2.0 - 1.0) * amount * (rowHash - 0.7) * 3.33;
            // Separate RGB channels
            float r = texture(u_texture, vec2(uv.x + shift * 1.2, uv.y)).r;
            float g = texture(u_texture, vec2(uv.x + shift, uv.y)).g;
            float b = texture(u_texture, vec2(uv.x + shift * 0.8, uv.y)).b;
            fragColor = vec4(r, g, b, 1.0);
        } else {
            fragColor = texture(u_texture, uv);
        }
    }
)";

// ============================================================================
// ADVANCED COLOR EFFECTS
// ============================================================================

inline const char* sepia = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_sepia_amount;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float luma = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        vec3 sepiaColor = vec3(luma) * vec3(1.2, 1.0, 0.8);
        sepiaColor = clamp(sepiaColor, 0.0, 1.0);
        color.rgb = mix(color.rgb, sepiaColor, u_sepia_amount);
        fragColor = color;
    }
)";

inline const char* crossProcess = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_crossprocess_amount;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        // Per-channel S-curves to simulate cross-processing
        // Red: boost shadows, crush highlights
        float r = color.r;
        r = r * r * (3.0 - 2.0 * r); // smoothstep
        r = mix(r, pow(r, 0.6), 0.5); // lift shadows
        // Green: strong S-curve with boost
        float g = color.g;
        g = g * g * (3.0 - 2.0 * g);
        g *= 1.1;
        // Blue: crush, shift toward cyan
        float b = color.b;
        b = pow(b, 1.5);
        b = mix(b, b * 0.7 + 0.15, 0.5);
        vec3 processed = clamp(vec3(r, g, b), 0.0, 1.0);
        color.rgb = mix(color.rgb, processed, u_crossprocess_amount);
        fragColor = color;
    }
)";

inline const char* splitTone = R"(
    #version 410 core
    #define PI 3.14159265359
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_splittone_shadow_hue;
    uniform float u_splittone_highlight_hue;
    uniform float u_splittone_amount;

    vec3 hueToRGB(float h) {
        float r = abs(h * 6.0 - 3.0) - 1.0;
        float g = 2.0 - abs(h * 6.0 - 2.0);
        float b = 2.0 - abs(h * 6.0 - 4.0);
        return clamp(vec3(r, g, b), 0.0, 1.0);
    }

    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float luma = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        vec3 shadowTint = hueToRGB(u_splittone_shadow_hue);
        vec3 highlightTint = hueToRGB(u_splittone_highlight_hue);
        // Shadows get shadow tint, highlights get highlight tint
        float shadowMask = 1.0 - smoothstep(0.0, 0.5, luma);
        float highlightMask = smoothstep(0.5, 1.0, luma);
        vec3 tinted = color.rgb;
        tinted = mix(tinted, tinted * shadowTint, shadowMask * 0.5);
        tinted = mix(tinted, tinted * highlightTint + highlightTint * 0.1, highlightMask * 0.5);
        color.rgb = mix(color.rgb, tinted, u_splittone_amount);
        fragColor = color;
    }
)";

inline const char* colorHalftone = R"(
    #version 410 core
    #define PI 3.14159265359
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_halftone_scale;
    uniform float u_halftone_amount;
    uniform vec2 u_resolution;

    float halftoneLayer(vec2 uv, float angle, float density, float value) {
        float s = sin(angle);
        float c = cos(angle);
        vec2 rotUV = vec2(c * uv.x - s * uv.y, s * uv.x + c * uv.y);
        vec2 cell = fract(rotUV * density) - 0.5;
        float dot_size = sqrt(1.0 - value) * 0.5;
        return smoothstep(dot_size + 0.01, dot_size - 0.01, length(cell));
    }

    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        vec2 uv = v_texCoord * u_resolution / max(u_resolution.x, u_resolution.y);
        float density = 20.0 + (1.0 - u_halftone_scale) * 80.0;
        // Convert to CMY
        float c_val = 1.0 - color.r;
        float m_val = 1.0 - color.g;
        float y_val = 1.0 - color.b;
        float k_val = min(min(c_val, m_val), y_val);
        // Halftone each channel at different angles
        float cDot = halftoneLayer(uv, 15.0 * PI / 180.0, density, c_val);
        float mDot = halftoneLayer(uv, 75.0 * PI / 180.0, density, m_val);
        float yDot = halftoneLayer(uv, 0.0, density, y_val);
        float kDot = halftoneLayer(uv, 45.0 * PI / 180.0, density, k_val);
        vec3 halftoned = vec3(1.0) - vec3(cDot, mDot, yDot) - vec3(kDot);
        halftoned = clamp(halftoned, 0.0, 1.0);
        color.rgb = mix(color.rgb, halftoned, u_halftone_amount);
        fragColor = color;
    }
)";

inline const char* orderedDither = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_dither_levels;
    uniform float u_dither_amount;
    uniform vec2 u_resolution;

    // 8x8 Bayer matrix (normalized to [0,1])
    float bayer8(vec2 pos) {
        ivec2 p = ivec2(mod(pos, 8.0));
        // Encode 8x8 Bayer pattern procedurally
        int x = p.x;
        int y = p.y;
        int value = 0;
        // Bit-interleave for Bayer pattern
        int xc = x ^ y;
        int yc = y;
        value = (xc & 1) | ((yc & 1) << 1) | ((xc & 2) << 1) | ((yc & 2) << 2) | ((xc & 4) << 2) | ((yc & 4) << 3);
        return float(value) / 64.0;
    }

    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float levels = 2.0 + u_dither_levels * 14.0; // 2 to 16 levels
        vec2 pixelPos = v_texCoord * u_resolution;
        float threshold = bayer8(pixelPos) - 0.5;
        vec3 dithered = floor(color.rgb * levels + threshold + 0.5) / levels;
        dithered = clamp(dithered, 0.0, 1.0);
        color.rgb = mix(color.rgb, dithered, u_dither_amount);
        fragColor = color;
    }
)";

inline const char* heatMap = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_heatmap_amount;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float luma = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        // Scientific heat map: blue -> cyan -> green -> yellow -> red
        vec3 heat;
        if (luma < 0.25) {
            heat = mix(vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 1.0), luma / 0.25);
        } else if (luma < 0.5) {
            heat = mix(vec3(0.0, 1.0, 1.0), vec3(0.0, 1.0, 0.0), (luma - 0.25) / 0.25);
        } else if (luma < 0.75) {
            heat = mix(vec3(0.0, 1.0, 0.0), vec3(1.0, 1.0, 0.0), (luma - 0.5) / 0.25);
        } else {
            heat = mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), (luma - 0.75) / 0.25);
        }
        color.rgb = mix(color.rgb, heat, u_heatmap_amount);
        fragColor = color;
    }
)";

inline const char* selectiveColor = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_selectcolor_hue;
    uniform float u_selectcolor_range;

    vec3 rgb2hsv(vec3 c) {
        vec4 K = vec4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
        vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
        float d = q.x - min(q.w, q.y);
        float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }

    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        vec3 hsv = rgb2hsv(color.rgb);
        float targetHue = u_selectcolor_hue;
        float range = 0.02 + u_selectcolor_range * 0.2;
        // Hue distance (wrapping)
        float hueDist = abs(hsv.x - targetHue);
        hueDist = min(hueDist, 1.0 - hueDist);
        // Mask: 1 = selected hue, 0 = everything else
        float mask = 1.0 - smoothstep(0.0, range, hueDist);
        // Desaturate non-selected areas
        float luma = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        vec3 gray = vec3(luma);
        color.rgb = mix(gray, color.rgb, mask);
        fragColor = color;
    }
)";

inline const char* filmGrain = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_grain_amount;
    uniform float u_grain_size;
    uniform float u_time;

    float hash21(vec2 p) {
        vec3 p3 = fract(vec3(p.xyx) * 0.1031);
        p3 += dot(p3, p3.yzx + 33.33);
        return fract((p3.x + p3.y) * p3.z);
    }

    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float grainScale = 1.0 + (1.0 - u_grain_size) * 500.0;
        float seed = floor(u_time * 24.0); // 24fps grain update
        vec2 grainUV = floor(v_texCoord * grainScale) / grainScale;
        float grain = hash21(grainUV + seed) * 2.0 - 1.0;
        // Reduce grain in bright areas (photographic grain behavior)
        float luma = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        float grainMask = 1.0 - luma * 0.5;
        color.rgb += grain * u_grain_amount * 0.3 * grainMask;
        color.rgb = clamp(color.rgb, 0.0, 1.0);
        fragColor = color;
    }
)";

inline const char* gammaLevels = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_levels_black;
    uniform float u_levels_white;
    uniform float u_levels_gamma;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float black = u_levels_black * 0.5; // black point [0, 0.5]
        float white = 0.5 + u_levels_white * 0.5; // white point [0.5, 1.0]
        float gamma = 0.2 + u_levels_gamma * 4.0; // gamma [0.2, 4.2]
        // Remap levels
        color.rgb = (color.rgb - black) / max(white - black, 0.001);
        color.rgb = clamp(color.rgb, 0.0, 1.0);
        // Apply gamma
        color.rgb = pow(color.rgb, vec3(1.0 / gamma));
        fragColor = color;
    }
)";

inline const char* solarize = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_solarize_threshold;
    uniform float u_solarize_amount;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        // Sabattier effect: invert portions above threshold
        float threshold = u_solarize_threshold;
        vec3 solarized;
        solarized.r = color.r > threshold ? 1.0 - color.r : color.r;
        solarized.g = color.g > threshold ? 1.0 - color.g : color.g;
        solarized.b = color.b > threshold ? 1.0 - color.b : color.b;
        // Alternative smooth version
        vec3 smoothSolar = abs(color.rgb * 2.0 - 1.0);
        solarized = mix(solarized, smoothSolar, 0.3);
        color.rgb = mix(color.rgb, solarized, u_solarize_amount);
        fragColor = color;
    }
)";

// ============================================================================
// PATTERN / OVERLAY EFFECTS
// ============================================================================

inline const char* crtSimulation = R"(
    #version 410 core
    #define PI 3.14159265359
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_crt_curvature;
    uniform float u_crt_scanline;
    uniform vec2 u_resolution;
    uniform float u_time;
    void main() {
        vec2 uv = v_texCoord;
        // Barrel distortion for CRT curvature
        vec2 centered = uv * 2.0 - 1.0;
        float r2 = dot(centered, centered);
        float curvature = u_crt_curvature * 0.3;
        centered *= 1.0 + curvature * r2;
        uv = centered * 0.5 + 0.5;
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }
        vec4 color = texture(u_texture, uv);
        // Phosphor RGB subpixels
        vec2 pixel = uv * u_resolution;
        int subpixel = int(mod(pixel.x, 3.0));
        vec3 phosphor = vec3(
            subpixel == 0 ? 1.0 : 0.4,
            subpixel == 1 ? 1.0 : 0.4,
            subpixel == 2 ? 1.0 : 0.4
        );
        color.rgb *= mix(vec3(1.0), phosphor, u_crt_scanline * 0.6);
        // Scanlines
        float scanline = sin(pixel.y * PI) * 0.5 + 0.5;
        scanline = pow(scanline, 0.3);
        color.rgb *= mix(1.0, scanline, u_crt_scanline * 0.4);
        // Slight color bleeding
        float bleedR = texture(u_texture, uv + vec2(1.0 / u_resolution.x, 0.0)).r;
        color.r = mix(color.r, bleedR, u_crt_scanline * 0.1);
        // Corner vignette
        float vig = 1.0 - r2 * curvature * 0.5;
        color.rgb *= vig;
        fragColor = color;
    }
)";

inline const char* vhsEffect = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_vhs_amount;
    uniform float u_vhs_tracking;
    uniform float u_time;
    uniform vec2 u_resolution;

    float hash(float n) { return fract(sin(n) * 43758.5453); }

    void main() {
        vec2 uv = v_texCoord;
        float amount = u_vhs_amount;
        // Horizontal jitter
        float jitter = (hash(floor(uv.y * 200.0) + floor(u_time * 15.0)) * 2.0 - 1.0) * amount * 0.01;
        uv.x += jitter;
        // Tracking line
        float trackPos = fract(u_time * 0.1 * (0.5 + u_vhs_tracking));
        float trackDist = abs(uv.y - trackPos);
        float trackLine = smoothstep(0.03, 0.0, trackDist) * u_vhs_tracking;
        uv.x += trackLine * 0.05;
        uv.y += trackLine * 0.01;
        // Color bleed (shift chroma channels)
        float chromaShift = amount * 0.005;
        float r = texture(u_texture, vec2(uv.x + chromaShift, uv.y)).r;
        float g = texture(u_texture, uv).g;
        float b = texture(u_texture, vec2(uv.x - chromaShift, uv.y)).b;
        vec3 color = vec3(r, g, b);
        // Color quantization (VHS color depth)
        float quantize = 64.0 - amount * 40.0;
        color = floor(color * quantize + 0.5) / quantize;
        // Static noise
        float noise = hash(uv.y * 1000.0 + u_time * 100.0) * amount * 0.1;
        color += noise;
        // Slight desaturation
        float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
        color = mix(color, vec3(luma), amount * 0.2);
        fragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
    }
)";

inline const char* asciiArt = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_ascii_scale;
    uniform float u_ascii_color;
    uniform vec2 u_resolution;

    // 5x5 character bitmaps encoded as 25-bit integers
    // Characters: ' ', '.', ':', '-', '=', '+', '*', '#', '@'
    float getChar(int idx, vec2 pos) {
        ivec2 p = ivec2(pos * 5.0);
        if (p.x < 0 || p.x >= 5 || p.y < 0 || p.y >= 5) return 0.0;
        int bit = p.y * 5 + p.x;
        // Simple character patterns
        int patterns[9] = int[](
            0,           // space
            0x0000020,   // .
            0x0020020,   // :
            0x0003800,   // -
            0x003E3E0,   // =
            0x0023880,   // +
            0x0154A80,   // *
            0x1F7DFBE,   // #
            0x1F7FFBE    // @
        );
        int pattern = patterns[idx];
        return float((pattern >> bit) & 1);
    }

    void main() {
        float cellSize = 4.0 + u_ascii_scale * 12.0; // 4 to 16 pixels
        vec2 cellCount = u_resolution / cellSize;
        vec2 cell = floor(v_texCoord * cellCount);
        vec2 cellUV = fract(v_texCoord * cellCount);
        // Sample at cell center
        vec2 sampleUV = (cell + 0.5) / cellCount;
        vec4 color = texture(u_texture, sampleUV);
        float luma = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        // Map brightness to character index (0-8)
        int charIdx = int(luma * 8.99);
        charIdx = clamp(charIdx, 0, 8);
        float charPixel = getChar(charIdx, cellUV);
        // Color mode: 0 = white on black, 1 = original colors
        vec3 charColor = mix(vec3(charPixel), color.rgb * charPixel, u_ascii_color);
        vec3 bgColor = mix(vec3(0.0), color.rgb * 0.2, u_ascii_color);
        vec3 result = mix(bgColor, charColor, charPixel);
        fragColor = vec4(result, 1.0);
    }
)";

inline const char* dotMatrix = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_dotmatrix_scale;
    uniform float u_dotmatrix_amount;
    void main() {
        float cellSize = 0.005 + u_dotmatrix_scale * 0.03;
        vec2 cell = floor(v_texCoord / cellSize);
        vec2 cellCenter = (cell + 0.5) * cellSize;
        vec2 cellUV = (v_texCoord - cell * cellSize) / cellSize;
        // Sample at cell center
        vec4 color = texture(u_texture, cellCenter);
        float luma = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        // Dot radius based on brightness
        float radius = luma * 0.5;
        float dist = length(cellUV - 0.5);
        float dot = smoothstep(radius + 0.02, radius - 0.02, dist);
        vec3 dotColor = vec3(dot);
        vec3 result = mix(texture(u_texture, v_texCoord).rgb, dotColor, u_dotmatrix_amount);
        fragColor = vec4(result, 1.0);
    }
)";

inline const char* crosshatch = R"(
    #version 410 core
    #define PI 3.14159265359
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_crosshatch_density;
    uniform float u_crosshatch_amount;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float luma = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        float density = 30.0 + u_crosshatch_density * 100.0;
        // Multiple hatching directions at different darkness levels
        float hatch = 1.0;
        // First pass: 45 degrees (darkest areas)
        if (luma < 0.8) {
            float line1 = abs(sin((v_texCoord.x + v_texCoord.y) * density * PI));
            hatch *= smoothstep(0.1, 0.3, line1 + luma * 0.5);
        }
        // Second pass: -45 degrees (darker areas)
        if (luma < 0.6) {
            float line2 = abs(sin((v_texCoord.x - v_texCoord.y) * density * PI));
            hatch *= smoothstep(0.1, 0.3, line2 + luma * 0.7);
        }
        // Third pass: horizontal (very dark)
        if (luma < 0.4) {
            float line3 = abs(sin(v_texCoord.y * density * PI));
            hatch *= smoothstep(0.1, 0.3, line3 + luma * 0.9);
        }
        // Fourth pass: vertical (extremely dark)
        if (luma < 0.2) {
            float line4 = abs(sin(v_texCoord.x * density * PI));
            hatch *= smoothstep(0.1, 0.3, line4 + luma * 1.1);
        }
        vec3 hatched = vec3(hatch);
        color.rgb = mix(color.rgb, hatched, u_crosshatch_amount);
        fragColor = color;
    }
)";

inline const char* emboss = R"(
    #version 410 core
    #define PI 3.14159265359
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_emboss_amount;
    uniform float u_emboss_angle;
    uniform vec2 u_resolution;
    void main() {
        vec2 texel = 1.0 / u_resolution;
        float angle = u_emboss_angle * 2.0 * PI;
        vec2 dir = vec2(cos(angle), sin(angle)) * texel;
        // Directional emboss kernel
        vec4 color = texture(u_texture, v_texCoord);
        float s1 = dot(texture(u_texture, v_texCoord + dir).rgb, vec3(0.2126, 0.7152, 0.0722));
        float s2 = dot(texture(u_texture, v_texCoord - dir).rgb, vec3(0.2126, 0.7152, 0.0722));
        float emboss = s1 - s2 + 0.5;
        vec3 embossed = vec3(emboss);
        color.rgb = mix(color.rgb, embossed, u_emboss_amount);
        fragColor = color;
    }
)";

inline const char* oilPaint = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_oilpaint_radius;
    uniform vec2 u_resolution;
    void main() {
        vec2 texel = 1.0 / u_resolution;
        int radius = 2 + int(u_oilpaint_radius * 2.0); // 2-4
        // Kuwahara filter: 4 quadrants, pick lowest variance
        vec3 mean[4] = vec3[](vec3(0.0), vec3(0.0), vec3(0.0), vec3(0.0));
        vec3 sqMean[4] = vec3[](vec3(0.0), vec3(0.0), vec3(0.0), vec3(0.0));
        int count = 0;
        for (int j = -radius; j <= 0; j++) {
            for (int i = -radius; i <= 0; i++) {
                vec3 s = texture(u_texture, v_texCoord + vec2(i, j) * texel).rgb;
                mean[0] += s; sqMean[0] += s * s;
            }
        }
        for (int j = -radius; j <= 0; j++) {
            for (int i = 0; i <= radius; i++) {
                vec3 s = texture(u_texture, v_texCoord + vec2(i, j) * texel).rgb;
                mean[1] += s; sqMean[1] += s * s;
            }
        }
        for (int j = 0; j <= radius; j++) {
            for (int i = -radius; i <= 0; i++) {
                vec3 s = texture(u_texture, v_texCoord + vec2(i, j) * texel).rgb;
                mean[2] += s; sqMean[2] += s * s;
            }
        }
        for (int j = 0; j <= radius; j++) {
            for (int i = 0; i <= radius; i++) {
                vec3 s = texture(u_texture, v_texCoord + vec2(i, j) * texel).rgb;
                mean[3] += s; sqMean[3] += s * s;
            }
        }
        float n = float((radius + 1) * (radius + 1));
        float minVar = 1e10;
        vec3 result = vec3(0.0);
        for (int q = 0; q < 4; q++) {
            mean[q] /= n;
            sqMean[q] /= n;
            vec3 variance = sqMean[q] - mean[q] * mean[q];
            float totalVar = variance.r + variance.g + variance.b;
            if (totalVar < minVar) {
                minVar = totalVar;
                result = mean[q];
            }
        }
        fragColor = vec4(result, 1.0);
    }
)";

inline const char* pencilSketch = R"(
    #version 410 core
    #define PI 3.14159265359
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_sketch_amount;
    uniform float u_sketch_density;
    uniform vec2 u_resolution;
    void main() {
        vec2 texel = 1.0 / u_resolution;
        vec4 color = texture(u_texture, v_texCoord);
        // Sobel edge detection
        float tl = dot(texture(u_texture, v_texCoord + vec2(-texel.x,  texel.y)).rgb, vec3(0.299, 0.587, 0.114));
        float t  = dot(texture(u_texture, v_texCoord + vec2( 0.0,      texel.y)).rgb, vec3(0.299, 0.587, 0.114));
        float tr = dot(texture(u_texture, v_texCoord + vec2( texel.x,  texel.y)).rgb, vec3(0.299, 0.587, 0.114));
        float l  = dot(texture(u_texture, v_texCoord + vec2(-texel.x,  0.0    )).rgb, vec3(0.299, 0.587, 0.114));
        float r  = dot(texture(u_texture, v_texCoord + vec2( texel.x,  0.0    )).rgb, vec3(0.299, 0.587, 0.114));
        float bl = dot(texture(u_texture, v_texCoord + vec2(-texel.x, -texel.y)).rgb, vec3(0.299, 0.587, 0.114));
        float b  = dot(texture(u_texture, v_texCoord + vec2( 0.0,     -texel.y)).rgb, vec3(0.299, 0.587, 0.114));
        float br = dot(texture(u_texture, v_texCoord + vec2( texel.x, -texel.y)).rgb, vec3(0.299, 0.587, 0.114));
        float gx = -tl - 2.0*l - bl + tr + 2.0*r + br;
        float gy = -tl - 2.0*t - tr + bl + 2.0*b + br;
        float edge = 1.0 - clamp(sqrt(gx * gx + gy * gy) * 3.0, 0.0, 1.0);
        // Luminance-based hatching
        float luma = dot(color.rgb, vec3(0.299, 0.587, 0.114));
        float density = 40.0 + u_sketch_density * 120.0;
        float hatch1 = abs(sin((v_texCoord.x + v_texCoord.y) * density * PI));
        float hatch2 = abs(sin((v_texCoord.x - v_texCoord.y) * density * PI));
        float hatching = 1.0;
        if (luma < 0.6) hatching *= smoothstep(0.1, 0.4, hatch1 + luma);
        if (luma < 0.3) hatching *= smoothstep(0.1, 0.4, hatch2 + luma * 2.0);
        // Combine edges and hatching on paper-white background
        float sketch = min(edge, hatching);
        vec3 sketched = vec3(0.95) * sketch;
        color.rgb = mix(color.rgb, sketched, u_sketch_amount);
        fragColor = color;
    }
)";

inline const char* voronoiGlass = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_voronoi_scale;
    uniform float u_voronoi_edge;
    uniform float u_time;

    vec2 hash2(vec2 p) {
        p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
        return fract(sin(p) * 43758.5453);
    }

    void main() {
        float scale = 5.0 + u_voronoi_scale * 30.0;
        vec2 uv = v_texCoord * scale;
        vec2 cell = floor(uv);
        vec2 frac = fract(uv);
        float minDist = 10.0;
        float secondDist = 10.0;
        vec2 nearestPoint = vec2(0.0);
        // 3x3 neighbor search
        for (int y = -1; y <= 1; y++) {
            for (int x = -1; x <= 1; x++) {
                vec2 neighbor = vec2(float(x), float(y));
                vec2 point = hash2(cell + neighbor);
                // Animate slightly
                point = 0.5 + 0.5 * sin(u_time * 0.5 + point * 6.28318);
                vec2 diff = neighbor + point - frac;
                float dist = length(diff);
                if (dist < minDist) {
                    secondDist = minDist;
                    minDist = dist;
                    nearestPoint = (cell + neighbor + point) / scale;
                } else if (dist < secondDist) {
                    secondDist = dist;
                }
            }
        }
        // Sample texture at cell center
        vec4 color = texture(u_texture, nearestPoint);
        // Dark edges between cells
        float edgeWidth = u_voronoi_edge * 0.3;
        float edgeDist = secondDist - minDist;
        float edge = smoothstep(0.0, edgeWidth, edgeDist);
        color.rgb *= mix(0.2, 1.0, edge);
        fragColor = color;
    }
)";

inline const char* crossStitch = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_stitch_scale;
    uniform float u_stitch_amount;
    void main() {
        float cellSize = 0.008 + u_stitch_scale * 0.04;
        vec2 cell = floor(v_texCoord / cellSize);
        vec2 cellCenter = (cell + 0.5) * cellSize;
        vec2 cellUV = (v_texCoord - cell * cellSize) / cellSize;
        // Sample color at cell center
        vec4 sampleColor = texture(u_texture, cellCenter);
        // X-stitch pattern: two diagonal lines forming an X
        float d1 = abs(cellUV.x - cellUV.y); // forward diagonal
        float d2 = abs(cellUV.x - (1.0 - cellUV.y)); // backward diagonal
        float lineWidth = 0.15;
        float stitch = min(
            smoothstep(lineWidth, lineWidth - 0.05, d1),
            1.0
        );
        stitch = max(stitch, smoothstep(lineWidth, lineWidth - 0.05, d2));
        // Add thread texture with slight brightness variation
        float thread = 0.85 + 0.15 * sin(cellUV.x * 20.0) * sin(cellUV.y * 20.0);
        vec3 stitchColor = sampleColor.rgb * thread;
        // Background (fabric)
        vec3 fabric = vec3(0.9, 0.88, 0.83);
        vec3 result = mix(fabric, stitchColor, stitch);
        vec3 original = texture(u_texture, v_texCoord).rgb;
        fragColor = vec4(mix(original, result, u_stitch_amount), 1.0);
    }
)";

inline const char* nightVision = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_nightvision_amount;
    uniform float u_time;

    float hash21(vec2 p) {
        vec3 p3 = fract(vec3(p.xyx) * 0.1031);
        p3 += dot(p3, p3.yzx + 33.33);
        return fract((p3.x + p3.y) * p3.z);
    }

    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float amount = u_nightvision_amount;
        // Convert to luminance with gamma boost
        float luma = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        luma = pow(luma, 0.6); // boost darks
        // Green phosphor tint
        vec3 nvColor = vec3(0.1, 1.0, 0.2) * luma;
        // Scan noise
        float noise = hash21(v_texCoord * 500.0 + floor(u_time * 30.0)) * 0.15;
        nvColor += noise;
        // Horizontal scan line
        float scanline = sin(v_texCoord.y * 800.0 + u_time * 5.0) * 0.03;
        nvColor += scanline;
        // Circular vignette (NVG tube)
        vec2 centered = v_texCoord * 2.0 - 1.0;
        float vignette = 1.0 - smoothstep(0.5, 1.0, length(centered));
        nvColor *= vignette;
        color.rgb = mix(color.rgb, nvColor, amount);
        fragColor = color;
    }
)";

// ============================================================================
// ANIMATION EFFECTS
// ============================================================================

inline const char* strobe = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_strobe_rate;
    uniform float u_strobe_intensity;
    uniform float u_time;
    void main() {
        vec4 color = texture(u_texture, v_texCoord);
        float rate = 1.0 + u_strobe_rate * 15.0; // 1 to 16 Hz
        float duty = 0.1 + u_strobe_intensity * 0.8; // duty cycle 10%-90%
        float phase = fract(u_time * rate);
        // Square wave: on when phase < duty, off otherwise
        float on = step(phase, duty);
        color.rgb *= on;
        fragColor = color;
    }
)";

inline const char* pulse = R"(
    #version 410 core
    #define TAU 6.28318530718
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_pulse_amount;
    uniform float u_pulse_speed;
    uniform float u_time;
    void main() {
        float speed = 0.5 + u_pulse_speed * 5.0;
        float scale = 1.0 + sin(u_time * speed * TAU) * u_pulse_amount * 0.1;
        vec2 uv = 0.5 + (v_texCoord - 0.5) / scale;
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        else
            fragColor = texture(u_texture, uv);
    }
)";

inline const char* slitScan = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_slitscan_amount;
    uniform float u_slitscan_direction;
    uniform float u_time;
    void main() {
        vec2 uv = v_texCoord;
        float amount = u_slitscan_amount * 0.15;
        // Time offset varies by position
        float dirBlend = u_slitscan_direction;
        // Horizontal slit: offset varies with y
        float offsetH = sin(uv.y * 6.28318 + u_time * 2.0) * amount;
        // Vertical slit: offset varies with x
        float offsetV = sin(uv.x * 6.28318 + u_time * 2.0) * amount;
        uv.x += mix(offsetH, 0.0, dirBlend);
        uv.y += mix(0.0, offsetV, dirBlend);
        fragColor = texture(u_texture, uv);
    }
)";

// ============================================================================
// BLEND / COMPOSITE EFFECTS
// ============================================================================

inline const char* doubleExposure = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_double_offset;
    uniform float u_double_blend;
    void main() {
        vec4 color1 = texture(u_texture, v_texCoord);
        // Second exposure: offset and possibly flipped
        float offset = (u_double_offset * 2.0 - 1.0) * 0.3;
        vec2 uv2 = vec2(v_texCoord.x + offset, 1.0 - v_texCoord.y); // flip + offset
        vec4 color2 = texture(u_texture, uv2);
        // Screen blend: 1 - (1-a)*(1-b)
        vec3 screened = 1.0 - (1.0 - color1.rgb) * (1.0 - color2.rgb);
        vec3 result = mix(color1.rgb, screened, u_double_blend);
        fragColor = vec4(result, color1.a);
    }
)";

inline const char* frostedGlass = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_frost_amount;
    uniform float u_frost_scale;
    uniform float u_time;

    float hash21(vec2 p) {
        vec3 p3 = fract(vec3(p.xyx) * 0.1031);
        p3 += dot(p3, p3.yzx + 33.33);
        return fract((p3.x + p3.y) * p3.z);
    }

    void main() {
        vec2 uv = v_texCoord;
        float amount = u_frost_amount * 0.02;
        float scale = 100.0 + u_frost_scale * 400.0;
        float seed = floor(u_time * 12.0); // Update at 12fps for subtle shimmer
        // 4-tap average with random offsets for frost
        vec4 color = vec4(0.0);
        for (int i = 0; i < 4; i++) {
            vec2 noiseCoord = floor(uv * scale + float(i) * 17.0) / scale;
            float rx = hash21(noiseCoord + seed + float(i) * 7.0) * 2.0 - 1.0;
            float ry = hash21(noiseCoord + seed + float(i) * 13.0 + 100.0) * 2.0 - 1.0;
            color += texture(u_texture, uv + vec2(rx, ry) * amount);
        }
        fragColor = color * 0.25;
    }
)";

inline const char* prismRefract = R"(
    #version 410 core
    #define PI 3.14159265359
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_prism_amount;
    uniform float u_prism_angle;
    void main() {
        float amount = u_prism_amount * 0.04;
        float angle = u_prism_angle * 2.0 * PI;
        vec2 dir = vec2(cos(angle), sin(angle));
        // 6-channel rainbow sampling at graduated offsets
        // Red -> Orange -> Yellow -> Green -> Cyan -> Blue
        vec3 color = vec3(0.0);
        // Weighted spectral sampling
        float offsets[6] = float[](-1.0, -0.6, -0.2, 0.2, 0.6, 1.0);
        vec3 weights[6] = vec3[](
            vec3(1.0, 0.0, 0.0),   // Red
            vec3(0.5, 0.5, 0.0),   // Orange/Yellow
            vec3(0.0, 1.0, 0.0),   // Green
            vec3(0.0, 0.5, 0.5),   // Cyan
            vec3(0.0, 0.0, 1.0),   // Blue
            vec3(0.3, 0.0, 0.7)    // Violet
        );
        vec3 totalWeight = vec3(0.0);
        for (int i = 0; i < 6; i++) {
            vec2 offset = dir * offsets[i] * amount;
            vec3 sample_color = texture(u_texture, v_texCoord + offset).rgb;
            color += sample_color * weights[i];
            totalWeight += weights[i];
        }
        color /= totalWeight;
        fragColor = vec4(color, 1.0);
    }
)";

inline const char* rainOnGlass = R"(
    #version 410 core
    #define PI 3.14159265359
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_raindrop_amount;
    uniform float u_raindrop_speed;
    uniform float u_time;

    vec2 hash2(vec2 p) {
        p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
        return fract(sin(p) * 43758.5453);
    }

    void main() {
        vec2 uv = v_texCoord;
        float speed = 0.5 + u_raindrop_speed * 2.0;
        float amount = u_raindrop_amount;
        vec2 totalRefract = vec2(0.0);
        // Multiple scales of raindrops
        for (int layer = 0; layer < 2; layer++) {
            float scale = 8.0 + float(layer) * 8.0;
            vec2 st = uv * scale;
            // Scroll drops downward
            st.y += u_time * speed * (1.0 + float(layer) * 0.5);
            vec2 cell = floor(st);
            vec2 frac = fract(st);
            for (int y = -1; y <= 1; y++) {
                for (int x = -1; x <= 1; x++) {
                    vec2 neighbor = vec2(float(x), float(y));
                    vec2 point = hash2(cell + neighbor);
                    // Drop falls within cell
                    float dropPhase = fract(u_time * speed * 0.3 + point.y * 10.0);
                    point.y = mod(point.y + dropPhase, 1.0);
                    vec2 diff = neighbor + point - frac;
                    float dist = length(diff);
                    float dropSize = 0.15 + point.x * 0.1;
                    if (dist < dropSize) {
                        // Refraction inside drop
                        vec2 refract_dir = diff / dropSize;
                        float strength = (1.0 - dist / dropSize);
                        totalRefract += refract_dir * strength * 0.02 * amount;
                    }
                }
            }
        }
        // Blur slightly for wet glass effect
        vec4 color = texture(u_texture, uv + totalRefract);
        // Slight desaturation for wet look
        float luma = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        color.rgb = mix(color.rgb, mix(vec3(luma), color.rgb, 0.85), amount * 0.3);
        fragColor = color;
    }
)";

inline const char* hexagonalize = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_hex_scale;
    void main() {
        float scale = 10.0 + u_hex_scale * 60.0;
        vec2 uv = v_texCoord * scale;
        // Hex grid constants
        float sqrt3 = 1.7320508;
        // Two candidate hex centers
        vec2 a = vec2(
            floor(uv.x),
            floor(uv.y / sqrt3) * sqrt3
        );
        // Offset row
        vec2 b = vec2(
            floor(uv.x + 0.5) + 0.5,
            (floor(uv.y / sqrt3) + 0.5) * sqrt3
        );
        // Nearest center in offset grid
        vec2 c = vec2(
            floor(uv.x - 0.5) + 0.5,
            (floor((uv.y) / sqrt3) + 0.5) * sqrt3
        );
        // Find closest hex center among candidates
        float da = length(uv - a);
        float db = length(uv - b);
        // Proper hex tiling: check two rows of offset hex centers
        vec2 hexA = a;
        vec2 hexB = a + vec2(0.5, sqrt3 * 0.5);
        vec2 hexC = a + vec2(-0.5, sqrt3 * 0.5);
        vec2 hexD = a + vec2(0.0, sqrt3);
        float dA = length(uv - hexA);
        float dB = length(uv - hexB);
        float dC = length(uv - hexC);
        float dD = length(uv - hexD);
        vec2 nearest = hexA;
        float minD = dA;
        if (dB < minD) { minD = dB; nearest = hexB; }
        if (dC < minD) { minD = dC; nearest = hexC; }
        if (dD < minD) { minD = dD; nearest = hexD; }
        // Sample at hex center
        vec2 sampleUV = nearest / scale;
        fragColor = texture(u_texture, sampleUV);
    }
)";

// =============================================================
// TRANSPARENCY / KEYING SHADERS (used by keyboard launcher)
// =============================================================

// Alpha transparency — pass through the image's alpha channel
inline const char* transparencyAlpha = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_opacity;
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        fragColor = vec4(col.rgb, col.a * u_opacity);
    }
)";

// Luma key — dark pixels become transparent
inline const char* transparencyLumaKey = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_opacity;
    uniform float u_luma_threshold;
    uniform float u_luma_softness;
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float luma = dot(col.rgb, vec3(0.299, 0.587, 0.114));
        float alpha = smoothstep(u_luma_threshold, u_luma_threshold + u_luma_softness, luma);
        fragColor = vec4(col.rgb, alpha * u_opacity);
    }
)";

// Chroma key — specified color becomes transparent
inline const char* transparencyChromaKey = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_opacity;
    uniform vec3 u_chroma_key_color;
    uniform float u_chroma_tolerance;
    uniform float u_chroma_softness;
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float dist = distance(col.rgb, u_chroma_key_color);
        float alpha = smoothstep(u_chroma_tolerance, u_chroma_tolerance + u_chroma_softness, dist);
        fragColor = vec4(col.rgb, alpha * u_opacity);
    }
)";

// Light transparency — bright pixels are opaque, dark are transparent
inline const char* transparencyLight = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_opacity;
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float alpha = max(col.r, max(col.g, col.b));
        fragColor = vec4(col.rgb, alpha * u_opacity);
    }
)";

// Alpha blend compositing — blends source onto destination using source alpha
inline const char* compositeBlend = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;      // source (key layer with alpha)
    uniform sampler2D u_destination;  // what's underneath
    void main() {
        vec4 src = texture(u_texture, v_texCoord);
        vec4 dst = texture(u_destination, v_texCoord);
        // Standard alpha blending: src over dst
        float outAlpha = src.a + dst.a * (1.0 - src.a);
        vec3 outRgb = (src.rgb * src.a + dst.rgb * dst.a * (1.0 - src.a));
        if (outAlpha > 0.001) outRgb /= outAlpha;
        fragColor = vec4(outRgb, outAlpha);
    }
)";

// ============================================================
// === PROCEDURAL SOURCE SHADERS (Phase 10) ===
// ============================================================
// These generate content from scratch (no u_texture input needed).
// Common uniforms: u_time, u_resolution, u_rms, u_bass, u_mid, u_high,
//                  u_beatPhase, u_spectralCentroid, u_onsetStrength

inline const char* sourcePerlinNoise = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_beatPhase;
    uniform float u_src_scale;
    uniform float u_src_speed;
    uniform float u_src_octaves;
    uniform float u_src_color_shift;

    // Simplex-style noise
    vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
    vec4 mod289(vec4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
    vec4 permute(vec4 x) { return mod289(((x * 34.0) + 1.0) * x); }
    vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }

    float snoise(vec3 v) {
        const vec2 C = vec2(1.0/6.0, 1.0/3.0);
        const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);
        vec3 i = floor(v + dot(v, C.yyy));
        vec3 x0 = v - i + dot(i, C.xxx);
        vec3 g = step(x0.yzx, x0.xyz);
        vec3 l = 1.0 - g;
        vec3 i1 = min(g.xyz, l.zxy);
        vec3 i2 = max(g.xyz, l.zxy);
        vec3 x1 = x0 - i1 + C.xxx;
        vec3 x2 = x0 - i2 + C.yyy;
        vec3 x3 = x0 - D.yyy;
        i = mod289(i);
        vec4 p = permute(permute(permute(
            i.z + vec4(0.0, i1.z, i2.z, 1.0))
            + i.y + vec4(0.0, i1.y, i2.y, 1.0))
            + i.x + vec4(0.0, i1.x, i2.x, 1.0));
        float n_ = 0.142857142857;
        vec3 ns = n_ * D.wyz - D.xzx;
        vec4 j = p - 49.0 * floor(p * ns.z * ns.z);
        vec4 x_ = floor(j * ns.z);
        vec4 y_ = floor(j - 7.0 * x_);
        vec4 x2_ = x_ * ns.x + ns.yyyy;
        vec4 y2_ = y_ * ns.x + ns.yyyy;
        vec4 h = 1.0 - abs(x2_) - abs(y2_);
        vec4 b0 = vec4(x2_.xy, y2_.xy);
        vec4 b1 = vec4(x2_.zw, y2_.zw);
        vec4 s0 = floor(b0) * 2.0 + 1.0;
        vec4 s1 = floor(b1) * 2.0 + 1.0;
        vec4 sh = -step(h, vec4(0.0));
        vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
        vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;
        vec3 p0 = vec3(a0.xy, h.x);
        vec3 p1 = vec3(a0.zw, h.y);
        vec3 p2 = vec3(a1.xy, h.z);
        vec3 p3 = vec3(a1.zw, h.w);
        vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2,p2), dot(p3,p3)));
        p0 *= norm.x; p1 *= norm.y; p2 *= norm.z; p3 *= norm.w;
        vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
        m = m * m;
        return 42.0 * dot(m*m, vec4(dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3)));
    }

    float fbm(vec3 p, int octaves) {
        float value = 0.0;
        float amplitude = 0.5;
        float frequency = 1.0;
        for (int i = 0; i < 8; i++) {
            if (i >= octaves) break;
            value += amplitude * snoise(p * frequency);
            frequency *= 2.0;
            amplitude *= 0.5;
        }
        return value;
    }

    void main() {
        vec2 uv = v_texCoord;
        float scale = 2.0 + u_src_scale * 8.0;
        float speed = 0.1 + u_src_speed * 0.5;
        int octaves = 1 + int(u_src_octaves * 7.0);
        float t = u_time * speed;

        float n = fbm(vec3(uv * scale, t), octaves);
        n = n * 0.5 + 0.5; // remap to [0,1]

        // Audio reactivity: bass pumps scale, RMS brightens
        n *= 0.8 + u_rms * 0.4;

        // Color based on noise value + color shift
        float hue = n + u_src_color_shift + u_bass * 0.1;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= n;

        fragColor = vec4(col, 1.0);
    }
)";

inline const char* sourcePlasma = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_beatPhase;
    uniform float u_src_speed;
    uniform float u_src_complexity;
    uniform float u_src_color_cycle;
    uniform float u_src_intensity;

    void main() {
        vec2 uv = v_texCoord * 2.0 - 1.0;
        float speed = 0.5 + u_src_speed * 2.0;
        float t = u_time * speed;
        float complexity = 2.0 + u_src_complexity * 6.0;

        float v1 = sin(uv.x * complexity + t);
        float v2 = sin(complexity * (uv.x * sin(t * 0.5) + uv.y * cos(t * 0.3)) + t);
        float cx = uv.x + 0.5 * sin(t * 0.3);
        float cy = uv.y + 0.5 * cos(t * 0.4);
        float v3 = sin(sqrt(100.0 * (cx*cx + cy*cy) + 1.0) + t);
        float v4 = sin(uv.y * complexity * 0.7 - t * 0.7);

        float v = (v1 + v2 + v3 + v4) * 0.25;
        v *= 0.8 + u_rms * 0.4;

        float cycle = u_src_color_cycle * 3.14159;
        float intensity = 0.5 + u_src_intensity * 0.5;
        vec3 col;
        col.r = intensity * (sin(v * 3.14159 + cycle) * 0.5 + 0.5);
        col.g = intensity * (sin(v * 3.14159 + cycle + 2.094) * 0.5 + 0.5);
        col.b = intensity * (sin(v * 3.14159 + cycle + 4.189) * 0.5 + 0.5);

        fragColor = vec4(col, 1.0);
    }
)";

inline const char* sourceVoronoi = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_beatPhase;
    uniform float u_src_scale;
    uniform float u_src_speed;
    uniform float u_src_edge_width;
    uniform float u_src_color_mode;

    vec2 hash2(vec2 p) {
        p = vec2(dot(p, vec2(127.1, 311.7)),
                 dot(p, vec2(269.5, 183.3)));
        return fract(sin(p) * 43758.5453);
    }

    void main() {
        float scale = 3.0 + u_src_scale * 12.0;
        float speed = 0.2 + u_src_speed * 0.8;
        vec2 uv = v_texCoord * scale;
        vec2 ip = floor(uv);
        vec2 fp = fract(uv);

        float minDist = 10.0;
        float secondDist = 10.0;
        vec2 nearestCell = vec2(0.0);

        for (int y = -1; y <= 1; y++) {
            for (int x = -1; x <= 1; x++) {
                vec2 neighbor = vec2(float(x), float(y));
                vec2 point = hash2(ip + neighbor);
                point = 0.5 + 0.5 * sin(u_time * speed + 6.2831 * point);
                float d = length(neighbor + point - fp);
                if (d < minDist) {
                    secondDist = minDist;
                    minDist = d;
                    nearestCell = ip + neighbor;
                } else if (d < secondDist) {
                    secondDist = d;
                }
            }
        }

        float edge = secondDist - minDist;
        float edgeWidth = 0.02 + u_src_edge_width * 0.15;
        float edgeLine = 1.0 - smoothstep(0.0, edgeWidth, edge);

        // Cell color from hash
        vec3 cellColor = 0.5 + 0.5 * cos(6.28318 * (hash2(nearestCell).x + vec3(0.0, 0.33, 0.67) + u_src_color_mode));
        cellColor *= (0.7 + u_rms * 0.5);

        // Mix cell color with edge highlight
        vec3 col = mix(cellColor * 0.6, vec3(1.0), edgeLine);

        fragColor = vec4(col, 1.0);
    }
)";

inline const char* sourceKaleidoFractal = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_beatPhase;
    uniform float u_src_iterations;
    uniform float u_src_fold_angle;
    uniform float u_src_zoom;
    uniform float u_src_rotation;
    uniform float u_src_color_shift;
    uniform float u_src_palette;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        float zoom = 0.5 + u_src_zoom * 2.0;
        uv *= zoom;

        int iters = 4 + int(u_src_iterations * 12.0);
        float angle = 0.5 + u_src_fold_angle * 2.5;
        float rot = u_src_rotation * 6.28318 + u_time * 0.1;

        float cs = cos(rot), sn = sin(rot);
        uv = vec2(uv.x * cs - uv.y * sn, uv.x * sn + uv.y * cs);

        float d = 1e10;
        for (int i = 0; i < 16; i++) {
            if (i >= iters) break;
            uv = abs(uv) - angle;
            float a = 0.7853 + float(i) * 0.1 + u_time * 0.02;
            float c = cos(a), s = sin(a);
            uv = vec2(uv.x * c - uv.y * s, uv.x * s + uv.y * c);
            d = min(d, length(uv));
        }

        // Much brighter coloring — softer falloff, higher base brightness
        float t = d * 4.0 + u_src_color_shift * 6.28 + u_time * 0.3;
        int palIdx = int(u_src_palette * 7.0);
        vec3 col = fracPalette(t, palIdx);
        // Soft glow falloff instead of harsh exp
        float glow = 1.0 / (1.0 + d * d * 4.0);
        col *= glow * (0.9 + u_rms * 0.4);

        fragColor = vec4(col, 1.0);
    }
)";

inline const char* sourceMandelbrot = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_beatPhase;
    uniform float u_src_zoom;
    uniform float u_src_center_x;
    uniform float u_src_center_y;
    uniform float u_src_julia_mix;
    uniform float u_src_max_iter;
    uniform float u_src_power;
    uniform float u_src_color_speed;
    uniform float u_src_color_shift;
    uniform float u_src_dive_speed;
    uniform float u_src_location;
    uniform float u_src_palette;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        // === CENTER: Location presets override Center X/Y ===
        // 10 interesting Mandelbrot locations
        vec2 locations[10];
        locations[0] = vec2(-0.7436439, 0.1318259);   // Seahorse valley
        locations[1] = vec2(-0.1011, 0.9563);          // Spiral arm
        locations[2] = vec2(-1.7497, 0.0);             // Mini-brot antenna
        locations[3] = vec2(0.2501, 0.0);              // Elephant valley
        locations[4] = vec2(-0.162, 1.0405);           // Double spiral
        locations[5] = vec2(-1.25066, 0.02012);        // Mini-brot near antenna
        locations[6] = vec2(-0.745428, 0.113009);      // Deep seahorse spiral
        locations[7] = vec2(-0.0452407, 0.9868162);    // Galaxy spiral
        locations[8] = vec2(0.3580, 0.6435);           // Mini Julia island
        locations[9] = vec2(-1.985540, 0.0);           // Far mini-brot

        vec2 center;
        float locIdx = u_src_location * 9.0;
        if (locIdx > 0.5) {
            // Location preset selected — smoothly interpolate between them
            int li = int(locIdx);
            float frac = locIdx - float(li);
            int li2 = min(li + 1, 9);
            frac = frac * frac * (3.0 - 2.0 * frac);
            center = mix(locations[min(li, 9)], locations[min(li2, 9)], frac);
        } else {
            // Manual center: X [0,1] -> [-2.5, 1.5], Y [0,1] -> [-1.5, 1.5]
            // At default 0.5/0.5 = (-0.5, 0.0) = center of the Mandelbrot set
            center = vec2(u_src_center_x * 4.0 - 2.5,
                          u_src_center_y * 3.0 - 1.5);
        }

        // === ZOOM: continuous, no wrap. Dive speed = auto-zoom clamped at float precision limit ===
        float diveRate = u_src_dive_speed * u_src_dive_speed * 2.0;
        float zoomExp = u_src_zoom * 7.0 + u_time * diveRate * 0.12;
        zoomExp = min(zoomExp, 7.0); // float precision limit
        float zoom = exp(-zoomExp);

        uv = uv * zoom + center;

        // Auto-increase iterations with zoom depth
        int baseIter = 32 + int(u_src_max_iter * 224.0);
        int zoomIter = int(zoomExp * 30.0);
        int maxIter = min(baseIter + zoomIter, 400);
        // Power: 2-4 range
        float power = 2.0 + u_src_power * 2.0;

        vec2 c, z;
        float juliaMix = u_src_julia_mix;
        vec2 juliaC = vec2(-0.7 + 0.3 * sin(u_time * 0.1), 0.27 + 0.15 * cos(u_time * 0.13));
        if (juliaMix > 0.5) { z = uv; c = juliaC; }
        else { z = vec2(0.0); c = uv; }

        int iter = 0;
        for (int i = 0; i < 400; i++) {
            if (i >= maxIter) break;
            if (dot(z, z) > 4.0) break;
            if (power < 2.5) {
                z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
            } else {
                float r = length(z);
                float theta = atan(z.y, z.x);
                float rn = pow(r, power);
                z = rn * vec2(cos(power * theta), sin(power * theta)) + c;
            }
            iter = i;
        }

        // Smooth iteration count — clamp to avoid negatives at high power
        float si = float(iter);
        if (iter < maxIter - 1) {
            si = max(float(iter) - log2(log2(dot(z,z))) + 4.0, 0.0);
        }

        vec3 col = vec3(0.0);
        if (iter < maxIter - 1) {
            float t = si * (0.01 + u_src_color_speed * 0.08) + u_src_color_shift * 6.28 + u_time * 0.02;
            int palIdx = int(u_src_palette * 7.0);
            col = fracPalette(t, palIdx);
        }

        col *= (0.7 + u_rms * 0.5);
        fragColor = vec4(col, 1.0);
    }
)";

inline const char* sourceGeometricTunnel = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_beatPhase;
    uniform float u_src_speed;
    uniform float u_src_segments;
    uniform float u_src_twist;
    uniform float u_src_color_shift;

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        float speed = 0.5 + u_src_speed * 2.0;
        float t = u_time * speed;

        // Polar coordinates
        float angle = atan(uv.y, uv.x);
        float radius = length(uv);

        // Tunnel transform
        float depth = 1.0 / (radius + 0.001);
        float twist = u_src_twist * 3.0;

        float texU = angle / 3.14159 + twist * depth * 0.1;
        float texV = depth + t;

        // Grid pattern
        float segments = 4.0 + u_src_segments * 12.0;
        float grid = abs(fract(texU * segments) - 0.5) * 2.0;
        grid *= abs(fract(texV * 2.0) - 0.5) * 2.0;

        // Audio reactivity
        float pulse = 0.7 + u_bass * 0.5;
        grid *= pulse;

        // Color
        float hue = texU * 0.5 + texV * 0.1 + u_src_color_shift;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= grid;

        // Fade at edges and center
        float vignette = smoothstep(0.0, 0.3, radius) * smoothstep(2.0, 0.5, radius);
        col *= vignette;

        fragColor = vec4(col, 1.0);
    }
)";

inline const char* sourceColorGradient = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_beatPhase;
    uniform float u_spectralCentroid;
    uniform float u_src_angle;
    uniform float u_src_speed;
    uniform float u_src_color1;
    uniform float u_src_color2;

    void main() {
        vec2 uv = v_texCoord;

        // Gradient direction from angle
        float angle = u_src_angle * 6.28318;
        float speed = u_src_speed * 0.5;
        float t = dot(uv - 0.5, vec2(cos(angle), sin(angle))) + 0.5;
        t += u_time * speed;
        t = fract(t);

        // Two-color gradient with smooth transitions
        vec3 col1 = 0.5 + 0.5 * cos(6.28318 * (u_src_color1 + vec3(0.0, 0.33, 0.67)));
        vec3 col2 = 0.5 + 0.5 * cos(6.28318 * (u_src_color2 + vec3(0.0, 0.33, 0.67)));

        vec3 col = mix(col1, col2, smoothstep(0.0, 1.0, t));

        // Audio: brightness pump
        col *= 0.7 + u_rms * 0.5;

        fragColor = vec4(col, 1.0);
    }
)";

inline const char* sourceAudioWaveform = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_mid;
    uniform float u_high;
    uniform float u_beatPhase;
    uniform float u_spectralCentroid;
    uniform float u_onsetStrength;
    uniform float u_src_style;
    uniform float u_src_thickness;
    uniform float u_src_glow;
    uniform float u_src_color_shift;

    void main() {
        vec2 uv = v_texCoord;
        vec2 p = uv * 2.0 - 1.0;

        float thickness = 0.005 + u_src_thickness * 0.03;
        float glowAmount = 0.5 + u_src_glow * 2.0;

        // Generate waveform from audio features
        float wave = 0.0;
        float x = uv.x;

        // Multi-band waveform: bass as slow wave, mid as faster, high as fastest
        wave += u_bass * 0.4 * sin(x * 3.14159 * 2.0 + u_time * 1.5);
        wave += u_mid * 0.3 * sin(x * 3.14159 * 6.0 + u_time * 3.0);
        wave += u_high * 0.2 * sin(x * 3.14159 * 14.0 + u_time * 5.0);
        wave += u_onsetStrength * 0.3 * sin(x * 3.14159 * 20.0 + u_time * 8.0);

        // Distance from waveform line
        float dist = abs(p.y - wave);

        // Style: 0 = line, 0.5 = filled, 1.0 = mirrored bars
        float style = u_src_style;
        float intensity;

        if (style < 0.33) {
            // Line mode
            intensity = smoothstep(thickness * 2.0, thickness * 0.5, dist);
            intensity += exp(-dist * dist * (200.0 / (glowAmount * glowAmount))) * 0.5; // glow
        } else if (style < 0.66) {
            // Filled mode
            float fillDist = (wave > 0.0) ? max(0.0, wave - p.y) : max(0.0, p.y - wave);
            if ((wave > 0.0 && p.y > 0.0 && p.y < wave) || (wave < 0.0 && p.y < 0.0 && p.y > wave))
                intensity = 0.8;
            else
                intensity = smoothstep(thickness * 2.0, thickness * 0.5, dist) * 0.5;
        } else {
            // Mirrored bars
            float barWidth = 1.0 / 32.0;
            float barX = floor(x / barWidth) * barWidth + barWidth * 0.5;
            float barVal = u_bass * 0.4 * abs(sin(barX * 6.28 + u_time)) +
                           u_mid * 0.3 * abs(sin(barX * 12.56 + u_time * 2.0)) +
                           u_rms * 0.3;
            float barDist = abs(x - barX) / barWidth;
            float inBar = step(barDist, 0.4) * step(abs(p.y), barVal);
            intensity = inBar * 0.9;
        }

        // Color
        float hue = u_src_color_shift + u_spectralCentroid * 0.0001;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= intensity;

        fragColor = vec4(col, 1.0);
    }
)";

inline const char* sourceReactionDiffusion = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;  // previous state (ping-pong)
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_beatPhase;
    uniform float u_onsetStrength;
    uniform float u_src_feed;
    uniform float u_src_kill;
    uniform float u_src_diffusion_a;
    uniform float u_src_diffusion_b;

    void main() {
        vec2 pixel = 1.0 / u_resolution;
        vec2 uv = v_texCoord;

        // Sample current state (R = chemical A, G = chemical B)
        vec4 state = texture(u_texture, uv);
        float A = state.r;
        float B = state.g;

        // Laplacian (3x3 kernel)
        float lapA = 0.0, lapB = 0.0;
        lapA += texture(u_texture, uv + vec2(-pixel.x, 0.0)).r * 0.2;
        lapA += texture(u_texture, uv + vec2( pixel.x, 0.0)).r * 0.2;
        lapA += texture(u_texture, uv + vec2(0.0, -pixel.y)).r * 0.2;
        lapA += texture(u_texture, uv + vec2(0.0,  pixel.y)).r * 0.2;
        lapA += texture(u_texture, uv + vec2(-pixel.x, -pixel.y)).r * 0.05;
        lapA += texture(u_texture, uv + vec2( pixel.x, -pixel.y)).r * 0.05;
        lapA += texture(u_texture, uv + vec2(-pixel.x,  pixel.y)).r * 0.05;
        lapA += texture(u_texture, uv + vec2( pixel.x,  pixel.y)).r * 0.05;
        lapA -= A;

        lapB += texture(u_texture, uv + vec2(-pixel.x, 0.0)).g * 0.2;
        lapB += texture(u_texture, uv + vec2( pixel.x, 0.0)).g * 0.2;
        lapB += texture(u_texture, uv + vec2(0.0, -pixel.y)).g * 0.2;
        lapB += texture(u_texture, uv + vec2(0.0,  pixel.y)).g * 0.2;
        lapB += texture(u_texture, uv + vec2(-pixel.x, -pixel.y)).g * 0.05;
        lapB += texture(u_texture, uv + vec2( pixel.x, -pixel.y)).g * 0.05;
        lapB += texture(u_texture, uv + vec2(-pixel.x,  pixel.y)).g * 0.05;
        lapB += texture(u_texture, uv + vec2( pixel.x,  pixel.y)).g * 0.05;
        lapB -= B;

        // Gray-Scott parameters (audio-reactive)
        float feed = 0.02 + u_src_feed * 0.06 + u_bass * 0.01;
        float kill = 0.05 + u_src_kill * 0.02 + u_rms * 0.005;
        float dA = 0.8 + u_src_diffusion_a * 0.4;
        float dB = 0.3 + u_src_diffusion_b * 0.2;

        float ABB = A * B * B;
        float newA = A + (dA * lapA - ABB + feed * (1.0 - A));
        float newB = B + (dB * lapB + ABB - (kill + feed) * B);

        newA = clamp(newA, 0.0, 1.0);
        newB = clamp(newB, 0.0, 1.0);

        // Seed on onset
        if (u_onsetStrength > 0.5) {
            float seedDist = length(uv - vec2(0.5 + 0.3 * sin(u_time), 0.5 + 0.3 * cos(u_time * 0.7)));
            if (seedDist < 0.05) {
                newA = 0.5;
                newB = 0.25;
            }
        }

        // Initialize if nearly empty
        if (state.a < 0.1) {
            newA = 1.0;
            newB = 0.0;
            float d = length(uv - 0.5);
            if (d < 0.1) { newA = 0.5; newB = 0.25; }
        }

        // Visual output: rich cosine palette based on chemical concentrations
        float v = newB * 3.0; // B chemical drives the visual
        float t = v + u_time * 0.01;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (t * 0.5 + vec3(0.0, 0.33, 0.67)));
        col *= v + 0.1;
        // Add glow to active regions
        col += vec3(0.2, 0.05, 0.1) * smoothstep(0.1, 0.3, newB);

        fragColor = vec4(col, 1.0);
    }
)";

inline const char* sourceCellularAutomata = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;  // previous state (ping-pong)
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_beatPhase;
    uniform float u_onsetStrength;
    uniform float u_src_mode;          // 0=Life, 0.33=Brian's Brain, 0.67=HighLife
    uniform float u_src_birth_low;
    uniform float u_src_birth_high;
    uniform float u_src_survival_low;
    uniform float u_src_color_shift;

    void main() {
        vec2 pixel = 1.0 / u_resolution;
        vec2 uv = v_texCoord;

        vec4 state = texture(u_texture, uv);
        // R = cell state (0=dead, 0.5=dying[Brain's Brain], 1=alive)
        // G = trail/age
        float cellState = state.r;
        int mode = int(u_src_mode * 2.99); // 0=Life, 1=Brain's Brain, 2=HighLife

        // Count alive neighbors (Moore neighborhood)
        float aliveNeighbors = 0.0;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                vec2 neighbor = uv + vec2(float(dx), float(dy)) * pixel;
                float ns = texture(u_texture, neighbor).r;
                aliveNeighbors += step(0.75, ns); // only fully alive cells count
            }
        }

        float newState = 0.0;

        if (mode == 1) {
            // Brian's Brain: 3 states (off→on→dying→off)
            if (cellState > 0.75) {
                newState = 0.5; // alive → dying
            } else if (cellState > 0.25) {
                newState = 0.0; // dying → dead
            } else {
                // Dead → alive if exactly 2 alive neighbors
                newState = (aliveNeighbors > 1.5 && aliveNeighbors < 2.5) ? 1.0 : 0.0;
            }
        } else {
            // Game of Life or HighLife
            float alive = step(0.75, cellState);
            float birthLow = 2.5 + u_src_birth_low * 2.0;
            float birthHigh = 3.5 + u_src_birth_high * 2.0;
            float survLow = 1.5 + u_src_survival_low * 2.0;

            if (mode == 2) {
                // HighLife: B36/S23 (Life + birth at 6)
                if (alive > 0.5) {
                    newState = (aliveNeighbors > 1.5 && aliveNeighbors < 3.5) ? 1.0 : 0.0;
                } else {
                    newState = ((aliveNeighbors > 2.5 && aliveNeighbors < 3.5) ||
                                (aliveNeighbors > 5.5 && aliveNeighbors < 6.5)) ? 1.0 : 0.0;
                }
            } else {
                // Standard Life with configurable rules
                if (alive > 0.5) {
                    newState = step(survLow, aliveNeighbors) * step(aliveNeighbors, birthHigh);
                } else {
                    newState = step(birthLow, aliveNeighbors) * step(aliveNeighbors, birthHigh);
                }
            }
        }

        // Seed on strong onsets
        if (u_onsetStrength > 0.6) {
            float seedDist = length(uv - vec2(0.5 + 0.4 * sin(u_time * 1.3), 0.5 + 0.4 * cos(u_time)));
            if (seedDist < 0.08) {
                float h = fract(sin(dot(uv * u_resolution + u_time * 100.0, vec2(12.9898, 78.233))) * 43758.5453);
                newState = step(0.4, h);
            }
        }

        // Initialize if nearly empty
        if (state.a < 0.1) {
            float h = fract(sin(dot(uv * u_resolution, vec2(12.9898, 78.233))) * 43758.5453);
            newState = step(0.6, h);
        }

        // Trail effect with color palette
        float trail = state.g * 0.96;
        if (newState > 0.75) trail = 1.0;
        else if (newState > 0.25) trail = max(trail, 0.5); // dying cells glow

        // Color: cosine palette with trail
        float hue = u_src_color_shift + trail * 0.3;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= trail;
        // Alive cells are bright white/colored, dying cells are dimmer
        if (newState > 0.75) col = mix(col, vec3(1.0), 0.5);

        fragColor = vec4(newState, trail, col.b, 1.0);
    }
)";

// Layer transform shader: translate, scale, rotate around anchor point
inline const char* layer_transform = R"(
    #version 410 core

    in vec2 v_texCoord;
    out vec4 fragColor;

    uniform sampler2D u_texture;
    uniform vec2 u_translate;   // Normalized translation (-1 to 1)
    uniform vec2 u_anchor;      // Anchor point (0-1, default 0.5,0.5)
    uniform float u_scale;      // Scale factor (1.0 = no change)
    uniform float u_rotation;   // Rotation in radians

    void main()
    {
        vec2 uv = v_texCoord;

        // Translate to anchor-relative space
        uv -= u_anchor;

        // Apply rotation
        float c = cos(-u_rotation);
        float s = sin(-u_rotation);
        uv = vec2(uv.x * c - uv.y * s, uv.x * s + uv.y * c);

        // Apply scale (inverse for UV mapping)
        uv /= max(u_scale, 0.001);

        // Apply translation (inverse)
        uv -= u_translate;

        // Translate back from anchor space
        uv += u_anchor;

        // Clamp to edges — out-of-bounds is transparent
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
            fragColor = vec4(0.0);
        else
            fragColor = texture(u_texture, uv);
    }
)";

// Mask layer shader: converts to greyscale luminance for alpha masking
inline const char* mask_luminance = R"(
    #version 410 core

    in vec2 v_texCoord;
    out vec4 fragColor;

    uniform sampler2D u_texture;      // Mask source
    uniform sampler2D u_accumulator;  // Current composited result

    void main()
    {
        vec4 maskColor = texture(u_texture, v_texCoord);
        float luma = dot(maskColor.rgb, vec3(0.2126, 0.7152, 0.0722));
        vec4 accum = texture(u_accumulator, v_texCoord);
        fragColor = vec4(accum.rgb, accum.a * luma);
    }
)";

// ============================================================
// Phase 14: Quick-Win Effects (20 new effects)
// ============================================================

// P14.1: Greyscale
inline const char* greyscale = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_grey_method;
    uniform float u_grey_amount;
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float luma = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));
        float avg = (col.r + col.g + col.b) / 3.0;
        float desat = (max(max(col.r, col.g), col.b) + min(min(col.r, col.g), col.b)) / 2.0;
        float grey = mix(mix(luma, avg, clamp((u_grey_method - 0.33) * 3.0, 0.0, 1.0)),
                         desat, clamp((u_grey_method - 0.66) * 3.0, 0.0, 1.0));
        fragColor = vec4(mix(col.rgb, vec3(grey), u_grey_amount), col.a);
    }
)";

// P14.2: Threshold
inline const char* threshold = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_threshold_level;
    uniform float u_threshold_amount;
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float luma = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));
        float bw = step(u_threshold_level, luma);
        fragColor = vec4(mix(col.rgb, vec3(bw), u_threshold_amount), col.a);
    }
)";

// P14.3: Exposure
inline const char* exposure = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_exposure_amount;
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float ev = (u_exposure_amount - 0.5) * 6.0;
        fragColor = vec4(col.rgb * pow(2.0, ev), col.a);
    }
)";

// P14.4: Vibrance
inline const char* vibrance = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_vibrance_amount;
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float maxC = max(max(col.r, col.g), col.b);
        float minC = min(min(col.r, col.g), col.b);
        float sat = maxC - minC;
        float strength = (u_vibrance_amount - 0.5) * 2.0;
        float adjust = strength * (1.0 - sat) * 0.5;
        float luma = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));
        fragColor = vec4(mix(vec3(luma), col.rgb, 1.0 + adjust), col.a);
    }
)";

// P14.5: Quad Mirror
inline const char* quadMirror = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_quadmir_cx;
    uniform float u_quadmir_cy;
    void main() {
        vec2 uv = v_texCoord;
        uv.x = (uv.x < u_quadmir_cx) ? uv.x : 2.0 * u_quadmir_cx - uv.x;
        uv.y = (uv.y < u_quadmir_cy) ? uv.y : 2.0 * u_quadmir_cy - uv.y;
        fragColor = texture(u_texture, uv);
    }
)";

// P14.6: Flip
inline const char* flip = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_flip_h;
    uniform float u_flip_v;
    void main() {
        vec2 uv = v_texCoord;
        if (u_flip_h > 0.5) uv.x = 1.0 - uv.x;
        if (u_flip_v > 0.5) uv.y = 1.0 - uv.y;
        fragColor = texture(u_texture, uv);
    }
)";

// P14.7: Warp Field
inline const char* warpField = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_time;
    uniform float u_warpfield_amount;
    uniform float u_warpfield_freq;
    uniform float u_warpfield_speed;
    void main() {
        vec2 uv = v_texCoord;
        float amt = u_warpfield_amount;
        float freq = u_warpfield_freq;
        float spd = u_warpfield_speed;
        for (int i = 0; i < 4; i++) {
            vec2 attractor = vec2(
                0.5 + 0.3 * sin(u_time * spd + float(i) * 1.5),
                0.5 + 0.3 * cos(u_time * spd * 0.7 + float(i) * 2.1)
            );
            vec2 diff = uv - attractor;
            float dist = length(diff);
            uv += diff / (dist * dist + 0.1) * amt * 0.01 * freq;
        }
        fragColor = texture(u_texture, uv);
    }
)";

// P14.8: Sharpen (unsharp mask)
inline const char* sharpen = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform vec2 u_resolution;
    uniform float u_sharpen_amount;
    uniform float u_sharpen_radius;
    void main() {
        vec2 px = u_sharpen_radius * 3.0 / u_resolution;
        vec4 col = texture(u_texture, v_texCoord);
        // 3x3 box blur for unsharp mask
        vec4 blurred = vec4(0.0);
        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                blurred += texture(u_texture, v_texCoord + vec2(float(x), float(y)) * px);
            }
        }
        blurred /= 9.0;
        vec4 sharpened = col + (col - blurred) * u_sharpen_amount * 3.0;
        fragColor = clamp(sharpened, 0.0, 1.0);
    }
)";

// P14.9: Pixel Explosion
inline const char* pixelExplosion = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_explode_force;
    uniform float u_explode_decay;
    uniform float u_explode_cx;
    uniform float u_explode_cy;
    void main() {
        vec2 center = vec2(u_explode_cx, u_explode_cy);
        vec2 dir = v_texCoord - center;
        float dist = length(dir);
        float luma = dot(texture(u_texture, v_texCoord).rgb, vec3(0.299, 0.587, 0.114));
        vec2 offset = normalize(dir + 0.0001) * u_explode_force * luma * 0.2 * exp(-dist * u_explode_decay * 5.0);
        vec2 uv = v_texCoord - offset;
        fragColor = texture(u_texture, uv);
    }
)";

// P14.10: Color Flash
inline const char* colorFlash = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_time;
    uniform float u_flash_intensity;
    uniform float u_flash_r;
    uniform float u_flash_g;
    uniform float u_flash_b;
    uniform float u_flash_decay;
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        vec3 flashColor = vec3(u_flash_r, u_flash_g, u_flash_b);
        float flash = u_flash_intensity * pow(1.0 - fract(u_time * (1.0 + u_flash_decay * 10.0)), 3.0);
        fragColor = vec4(mix(col.rgb, flashColor, flash), col.a);
    }
)";

// P14.11: Slide Wrap
inline const char* slideWrap = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_slide_x;
    uniform float u_slide_y;
    void main() {
        vec2 uv = fract(v_texCoord + vec2(u_slide_x - 0.5, u_slide_y - 0.5));
        fragColor = texture(u_texture, uv);
    }
)";

// P14.12: Dot Field
inline const char* dotField = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_dotfield_size;
    uniform float u_dotfield_spacing;
    uniform float u_dotfield_depth;
    void main() {
        float gridSize = (u_dotfield_spacing * 0.04) + 0.01;
        vec2 gridPos = floor(v_texCoord / gridSize) * gridSize + gridSize * 0.5;
        vec4 gridColor = texture(u_texture, gridPos);
        float luma = dot(gridColor.rgb, vec3(0.299, 0.587, 0.114));
        float dist = distance(v_texCoord, gridPos);
        float dotRadius = luma * u_dotfield_size * gridSize * 0.6;
        float d = smoothstep(dotRadius, dotRadius - 0.001, dist);
        fragColor = vec4(gridColor.rgb * d, d);
    }
)";

// P14.13: Triangulate
inline const char* triangulate = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_tri_size;
    uniform float u_tri_amount;
    void main() {
        float s = (u_tri_size * 0.08) + 0.01;
        vec2 pos = v_texCoord / s;
        float row = floor(pos.y);
        float col = floor(pos.x - mod(row, 2.0) * 0.5);
        vec2 center = vec2(col + mod(row, 2.0) * 0.5 + 0.5, row + 0.5) * s;
        vec4 triColor = texture(u_texture, center);
        fragColor = mix(texture(u_texture, v_texCoord), triColor, u_tri_amount);
    }
)";

// P14.14: Auto Mask
inline const char* autoMask = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_automask_threshold;
    uniform float u_automask_softness;
    uniform float u_automask_invert;
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float luma = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));
        float mask = smoothstep(u_automask_threshold - u_automask_softness * 0.5,
                                u_automask_threshold + u_automask_softness * 0.5, luma);
        if (u_automask_invert > 0.5) mask = 1.0 - mask;
        fragColor = vec4(col.rgb, col.a * mask);
    }
)";

// P14.15: Chroma Key (effect version)
inline const char* chromakeyEffect = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_chromakey_hue;
    uniform float u_chromakey_tolerance;
    uniform float u_chromakey_softness;
    uniform float u_chromakey_amount;
    vec3 rgb2hsv(vec3 c) {
        vec4 K = vec4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
        vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
        float d = q.x - min(q.w, q.y);
        float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        vec3 hsv = rgb2hsv(col.rgb);
        float hueDist = min(abs(hsv.x - u_chromakey_hue), 1.0 - abs(hsv.x - u_chromakey_hue));
        float mask = smoothstep(u_chromakey_tolerance - u_chromakey_softness * 0.5,
                                u_chromakey_tolerance + u_chromakey_softness * 0.5, hueDist);
        float alpha = mix(1.0, mask, u_chromakey_amount);
        fragColor = vec4(col.rgb, col.a * alpha);
    }
)";

// P14.16: Tile Grid
inline const char* tileGrid = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_tilegrid_cols;
    uniform float u_tilegrid_rows;
    uniform float u_tilegrid_offset;
    uniform float u_tilegrid_zoom;
    void main() {
        float cols = mix(1.0, 16.0, u_tilegrid_cols);
        float rows = mix(1.0, 16.0, u_tilegrid_rows);
        float rowIdx = floor(v_texCoord.y * rows);
        float xOffset = mod(rowIdx, 2.0) * u_tilegrid_offset * 0.5;
        vec2 tileUV = fract(vec2(v_texCoord.x * cols + xOffset, v_texCoord.y * rows));
        tileUV = (tileUV - 0.5) / max(mix(0.1, 2.0, u_tilegrid_zoom), 0.001) + 0.5;
        fragColor = texture(u_texture, clamp(tileUV, 0.0, 1.0));
    }
)";

// P14.17: Spot Zoom
inline const char* spotZoom = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_spotzoom_cx;
    uniform float u_spotzoom_cy;
    uniform float u_spotzoom_size;
    uniform float u_spotzoom_zoom;
    uniform float u_spotzoom_shape;
    uniform float u_spotzoom_bg;
    void main() {
        vec2 center = vec2(u_spotzoom_cx, u_spotzoom_cy);
        float size = mix(0.05, 0.8, u_spotzoom_size);
        float zoom = mix(1.0, 10.0, u_spotzoom_zoom);
        vec2 diff = v_texCoord - center;
        float dist;
        if (u_spotzoom_shape < 0.5) {
            dist = length(diff);
        } else {
            dist = max(abs(diff.x), abs(diff.y));
        }
        float inRegion = smoothstep(size, size - 0.01, dist);
        vec2 zoomedUV = center + (v_texCoord - center) / zoom;
        vec4 zoomedColor = texture(u_texture, zoomedUV);
        vec4 bgColor = texture(u_texture, v_texCoord) * u_spotzoom_bg;
        fragColor = mix(bgColor, zoomedColor, inRegion);
    }
)";

// P14.18: Neon Edge
inline const char* neonEdge = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform vec2 u_resolution;
    uniform float u_neonedge_edge;
    uniform float u_neonedge_glow;
    uniform float u_neonedge_hue;
    uniform float u_neonedge_original;

    vec3 hsv2rgb(vec3 c) {
        vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
        vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    float getLuma(vec2 uv) {
        return dot(texture(u_texture, uv).rgb, vec3(0.2126, 0.7152, 0.0722));
    }

    float sobelMag(vec2 uv, vec2 px) {
        float tl = getLuma(uv + vec2(-px.x, px.y));
        float t  = getLuma(uv + vec2(0.0, px.y));
        float tr = getLuma(uv + vec2(px.x, px.y));
        float l  = getLuma(uv + vec2(-px.x, 0.0));
        float r  = getLuma(uv + vec2(px.x, 0.0));
        float bl = getLuma(uv + vec2(-px.x, -px.y));
        float b  = getLuma(uv + vec2(0.0, -px.y));
        float br = getLuma(uv + vec2(px.x, -px.y));
        float gx = -tl - 2.0*l - bl + tr + 2.0*r + br;
        float gy = -tl - 2.0*t - tr + bl + 2.0*b + br;
        return sqrt(gx*gx + gy*gy);
    }

    void main() {
        vec2 px = 1.0 / u_resolution;
        float edge = sobelMag(v_texCoord, px) * u_neonedge_edge * 2.0;
        vec3 neonColor = hsv2rgb(vec3(u_neonedge_hue, 1.0, 1.0));
        // Glow: sample multiple offsets
        float glow = 0.0;
        float glowR = u_neonedge_glow * 3.0;
        for (int i = -2; i <= 2; i++) {
            for (int j = -2; j <= 2; j++) {
                glow += sobelMag(v_texCoord + vec2(float(i), float(j)) * px * glowR, px);
            }
        }
        glow /= 25.0;
        vec3 neonGlow = neonColor * (edge + glow * 0.5);
        vec3 original = texture(u_texture, v_texCoord).rgb;
        fragColor = vec4(mix(neonGlow, original + neonGlow, u_neonedge_original), 1.0);
    }
)";

// P14.19: Cartoon Ink
inline const char* cartoonInk = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform vec2 u_resolution;
    uniform float u_cartoonink_edge;
    uniform float u_cartoonink_steps;
    uniform float u_cartoonink_ink;
    uniform float u_cartoonink_sat;

    float getLuma(vec2 uv) {
        return dot(texture(u_texture, uv).rgb, vec3(0.2126, 0.7152, 0.0722));
    }

    void main() {
        vec2 px = 1.0 / u_resolution;
        vec4 base = texture(u_texture, v_texCoord);
        // Color quantization
        float steps = mix(2.0, 12.0, u_cartoonink_steps);
        vec3 quantized = floor(base.rgb * steps + 0.5) / steps;
        // Saturation boost
        float luma = dot(quantized, vec3(0.2126, 0.7152, 0.0722));
        float satMul = mix(0.0, 2.0, u_cartoonink_sat);
        quantized = mix(vec3(luma), quantized, satMul);
        // Sobel edge detection
        float edgeWidth = u_cartoonink_edge + 0.1;
        vec2 ep = px * edgeWidth;
        float tl = getLuma(v_texCoord + vec2(-ep.x, ep.y));
        float t  = getLuma(v_texCoord + vec2(0.0, ep.y));
        float tr = getLuma(v_texCoord + vec2(ep.x, ep.y));
        float l  = getLuma(v_texCoord + vec2(-ep.x, 0.0));
        float r  = getLuma(v_texCoord + vec2(ep.x, 0.0));
        float bl = getLuma(v_texCoord + vec2(-ep.x, -ep.y));
        float b  = getLuma(v_texCoord + vec2(0.0, -ep.y));
        float br = getLuma(v_texCoord + vec2(ep.x, -ep.y));
        float gx = -tl - 2.0*l - bl + tr + 2.0*r + br;
        float gy = -tl - 2.0*t - tr + bl + 2.0*b + br;
        float edge = sqrt(gx*gx + gy*gy);
        float ink = smoothstep(0.1, 0.3, edge) * u_cartoonink_ink;
        fragColor = vec4(quantized * (1.0 - ink), base.a);
    }
)";

// P14.20: Pop Raster
inline const char* popRaster = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_popraster_palette;
    uniform float u_popraster_bands;
    uniform float u_popraster_size;
    uniform float u_popraster_mix;
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float lum = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));
        float numBands = mix(2.0, 8.0, u_popraster_bands);
        int band = int(lum * numBands);
        float bandNorm = float(band) / numBands;
        // Palette selection (4 palettes)
        int paletteIdx = int(u_popraster_palette * 3.99);
        vec3 bandColor;
        if (paletteIdx == 0) { // Warhol
            bandColor = mix(vec3(0.8, 0.0, 0.4), vec3(1.0, 0.9, 0.0), bandNorm);
        } else if (paletteIdx == 1) { // Lichtenstein
            bandColor = mix(vec3(0.0, 0.2, 0.8), vec3(1.0, 0.0, 0.0), bandNorm);
        } else if (paletteIdx == 2) { // CMYK
            bandColor = mix(vec3(0.0, 0.8, 0.8), vec3(0.9, 0.0, 0.6), bandNorm);
        } else { // Neon
            bandColor = mix(vec3(0.0, 1.0, 0.5), vec3(1.0, 0.0, 1.0), bandNorm);
        }
        // Halftone dot pattern overlay
        float patternSize = mix(4.0, 64.0, u_popraster_size);
        float dotPattern = smoothstep(0.4, 0.6,
            length(fract(v_texCoord * patternSize) - 0.5) * 2.0 - (1.0 - lum));
        vec3 result = mix(bandColor, bandColor * dotPattern, 0.3);
        fragColor = vec4(mix(col.rgb, result, u_popraster_mix), col.a);
    }
)";

// ============================================================
// Phase 14: Transition Shaders (15 clip-to-clip transitions)
// ============================================================

// P14.21: Dissolve (simple crossfade)
inline const char* transitionDissolve = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;      // new clip
    uniform sampler2D u_prevTexture;  // previous clip
    uniform float u_crossfadeProgress;
    void main() {
        vec4 prev = texture(u_prevTexture, v_texCoord);
        vec4 next = texture(u_texture, v_texCoord);
        fragColor = mix(prev, next, u_crossfadeProgress);
    }
)";

// P14.22: Wipe Left
inline const char* transitionWipeLeft = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform sampler2D u_prevTexture;
    uniform float u_crossfadeProgress;
    void main() {
        vec4 prev = texture(u_prevTexture, v_texCoord);
        vec4 next = texture(u_texture, v_texCoord);
        float edge = smoothstep(u_crossfadeProgress - 0.02, u_crossfadeProgress + 0.02, v_texCoord.x);
        fragColor = mix(next, prev, edge);
    }
)";

// P14.23: Wipe Right
inline const char* transitionWipeRight = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform sampler2D u_prevTexture;
    uniform float u_crossfadeProgress;
    void main() {
        vec4 prev = texture(u_prevTexture, v_texCoord);
        vec4 next = texture(u_texture, v_texCoord);
        float edge = smoothstep(u_crossfadeProgress - 0.02, u_crossfadeProgress + 0.02, 1.0 - v_texCoord.x);
        fragColor = mix(next, prev, edge);
    }
)";

// P14.24: Wipe Up
inline const char* transitionWipeUp = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform sampler2D u_prevTexture;
    uniform float u_crossfadeProgress;
    void main() {
        vec4 prev = texture(u_prevTexture, v_texCoord);
        vec4 next = texture(u_texture, v_texCoord);
        float edge = smoothstep(u_crossfadeProgress - 0.02, u_crossfadeProgress + 0.02, 1.0 - v_texCoord.y);
        fragColor = mix(next, prev, edge);
    }
)";

// P14.25: Wipe Down
inline const char* transitionWipeDown = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform sampler2D u_prevTexture;
    uniform float u_crossfadeProgress;
    void main() {
        vec4 prev = texture(u_prevTexture, v_texCoord);
        vec4 next = texture(u_texture, v_texCoord);
        float edge = smoothstep(u_crossfadeProgress - 0.02, u_crossfadeProgress + 0.02, v_texCoord.y);
        fragColor = mix(next, prev, edge);
    }
)";

// P14.26: Push Left
inline const char* transitionPushLeft = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform sampler2D u_prevTexture;
    uniform float u_crossfadeProgress;
    void main() {
        float p = u_crossfadeProgress;
        vec2 prevUV = v_texCoord + vec2(p, 0.0);
        vec2 nextUV = v_texCoord + vec2(p - 1.0, 0.0);
        vec4 prev = texture(u_prevTexture, prevUV);
        vec4 next = texture(u_texture, nextUV);
        if (prevUV.x > 1.0) prev = vec4(0.0);
        if (nextUV.x < 0.0) next = vec4(0.0);
        fragColor = (v_texCoord.x < 1.0 - p) ? prev : next;
    }
)";

// P14.27: Push Right
inline const char* transitionPushRight = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform sampler2D u_prevTexture;
    uniform float u_crossfadeProgress;
    void main() {
        float p = u_crossfadeProgress;
        vec2 prevUV = v_texCoord + vec2(-p, 0.0);
        vec2 nextUV = v_texCoord + vec2(1.0 - p, 0.0);
        vec4 prev = texture(u_prevTexture, prevUV);
        vec4 next = texture(u_texture, nextUV);
        if (prevUV.x < 0.0) prev = vec4(0.0);
        if (nextUV.x > 1.0) next = vec4(0.0);
        fragColor = (v_texCoord.x > p) ? prev : next;
    }
)";

// P14.28: Push Up
inline const char* transitionPushUp = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform sampler2D u_prevTexture;
    uniform float u_crossfadeProgress;
    void main() {
        float p = u_crossfadeProgress;
        vec2 prevUV = v_texCoord + vec2(0.0, -p);
        vec2 nextUV = v_texCoord + vec2(0.0, 1.0 - p);
        vec4 prev = texture(u_prevTexture, prevUV);
        vec4 next = texture(u_texture, nextUV);
        if (prevUV.y < 0.0) prev = vec4(0.0);
        if (nextUV.y > 1.0) next = vec4(0.0);
        fragColor = (v_texCoord.y > p) ? prev : next;
    }
)";

// P14.29: Push Down
inline const char* transitionPushDown = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform sampler2D u_prevTexture;
    uniform float u_crossfadeProgress;
    void main() {
        float p = u_crossfadeProgress;
        vec2 prevUV = v_texCoord + vec2(0.0, p);
        vec2 nextUV = v_texCoord + vec2(0.0, p - 1.0);
        vec4 prev = texture(u_prevTexture, prevUV);
        vec4 next = texture(u_texture, nextUV);
        if (prevUV.y > 1.0) prev = vec4(0.0);
        if (nextUV.y < 0.0) next = vec4(0.0);
        fragColor = (v_texCoord.y < 1.0 - p) ? prev : next;
    }
)";

// P14.30: Zoom In
inline const char* transitionZoomIn = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform sampler2D u_prevTexture;
    uniform float u_crossfadeProgress;
    void main() {
        float p = u_crossfadeProgress;
        vec4 prev = texture(u_prevTexture, v_texCoord);
        // Next zooms in from center
        float scale = max(p, 0.001);
        vec2 nextUV = (v_texCoord - 0.5) / scale + 0.5;
        vec4 next = texture(u_texture, nextUV);
        float nextAlpha = (nextUV.x >= 0.0 && nextUV.x <= 1.0 && nextUV.y >= 0.0 && nextUV.y <= 1.0) ? p : 0.0;
        fragColor = mix(prev, next, nextAlpha);
    }
)";

// P14.31: Zoom Out
inline const char* transitionZoomOut = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform sampler2D u_prevTexture;
    uniform float u_crossfadeProgress;
    void main() {
        float p = u_crossfadeProgress;
        vec4 next = texture(u_texture, v_texCoord);
        // Prev zooms out from center
        float scale = max(1.0 - p, 0.001);
        vec2 prevUV = (v_texCoord - 0.5) / scale + 0.5;
        vec4 prev = texture(u_prevTexture, prevUV);
        float prevAlpha = (prevUV.x >= 0.0 && prevUV.x <= 1.0 && prevUV.y >= 0.0 && prevUV.y <= 1.0) ? (1.0 - p) : 0.0;
        fragColor = mix(next, prev, prevAlpha);
    }
)";

// P14.32: Iris Circle
inline const char* transitionIris = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform sampler2D u_prevTexture;
    uniform float u_crossfadeProgress;
    void main() {
        float p = u_crossfadeProgress;
        vec4 prev = texture(u_prevTexture, v_texCoord);
        vec4 next = texture(u_texture, v_texCoord);
        float dist = length(v_texCoord - 0.5);
        float radius = p * 0.75;
        float mask = smoothstep(radius, radius - 0.02, dist);
        fragColor = mix(prev, next, mask);
    }
)";

// P14.33: Flip Horizontal
inline const char* transitionFlipH = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform sampler2D u_prevTexture;
    uniform float u_crossfadeProgress;
    void main() {
        float p = u_crossfadeProgress;
        if (p < 0.5) {
            // First half: prev squishes horizontally
            float scale = 1.0 - p * 2.0;
            vec2 uv = vec2((v_texCoord.x - 0.5) / max(scale, 0.01) + 0.5, v_texCoord.y);
            fragColor = (uv.x >= 0.0 && uv.x <= 1.0) ? texture(u_prevTexture, uv) : vec4(0.0);
        } else {
            // Second half: next expands
            float scale = (p - 0.5) * 2.0;
            vec2 uv = vec2((v_texCoord.x - 0.5) / max(scale, 0.01) + 0.5, v_texCoord.y);
            fragColor = (uv.x >= 0.0 && uv.x <= 1.0) ? texture(u_texture, uv) : vec4(0.0);
        }
    }
)";

// P14.34: Cut (instant)
inline const char* transitionCut = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform sampler2D u_prevTexture;
    uniform float u_crossfadeProgress;
    void main() {
        fragColor = (u_crossfadeProgress < 0.5) ?
            texture(u_prevTexture, v_texCoord) :
            texture(u_texture, v_texCoord);
    }
)";

// P14.35: Fade to Black
inline const char* transitionFadeBlack = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform sampler2D u_prevTexture;
    uniform float u_crossfadeProgress;
    void main() {
        float p = u_crossfadeProgress;
        if (p < 0.5) {
            vec4 prev = texture(u_prevTexture, v_texCoord);
            fragColor = prev * (1.0 - p * 2.0);
        } else {
            vec4 next = texture(u_texture, v_texCoord);
            fragColor = next * ((p - 0.5) * 2.0);
        }
    }
)";

// ============================================================
// Phase 15: Medium Effects (12 new effects)
// ============================================================

inline const char* paletteRemap = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_palette_index;
    uniform float u_palette_cycle;
    uniform float u_palette_amount;

    vec3 iqPalette(float t, vec3 a, vec3 b, vec3 c, vec3 d) {
        return a + b * cos(6.28318 * (c * t + d));
    }

    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float luma = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));
        float t = fract(luma + u_palette_cycle);

        // 8 palettes selected by u_palette_index
        int idx = int(u_palette_index * 7.99);
        vec3 a, b, c, d;
        if (idx == 0)      { a=vec3(0.5,0.5,0.5); b=vec3(0.5,0.5,0.5); c=vec3(1.0,1.0,1.0); d=vec3(0.00,0.33,0.67); } // neon
        else if (idx == 1) { a=vec3(0.5,0.5,0.5); b=vec3(0.5,0.5,0.5); c=vec3(1.0,0.7,0.4); d=vec3(0.00,0.15,0.20); } // sunset
        else if (idx == 2) { a=vec3(0.5,0.5,0.5); b=vec3(0.5,0.5,0.5); c=vec3(1.0,1.0,1.0); d=vec3(0.30,0.20,0.20); } // ocean
        else if (idx == 3) { a=vec3(0.5,0.5,0.5); b=vec3(0.5,0.5,0.5); c=vec3(2.0,1.0,0.0); d=vec3(0.50,0.20,0.25); } // fire
        else if (idx == 4) { a=vec3(0.8,0.8,0.8); b=vec3(0.2,0.2,0.2); c=vec3(1.0,1.0,1.0); d=vec3(0.00,0.10,0.20); } // pastel
        else if (idx == 5) { a=vec3(0.0,0.3,0.0); b=vec3(0.0,0.5,0.0); c=vec3(0.0,1.0,0.0); d=vec3(0.00,0.00,0.00); } // matrix
        else if (idx == 6) { a=vec3(0.5,0.5,0.5); b=vec3(0.5,0.5,0.5); c=vec3(1.0,1.0,0.5); d=vec3(0.80,0.90,0.30); } // thermal
        else               { a=vec3(0.5,0.5,0.8); b=vec3(0.5,0.5,0.3); c=vec3(1.0,1.0,1.0); d=vec3(0.20,0.30,0.50); } // ice

        vec3 mapped = iqPalette(t, a, b, c, d);
        fragColor = vec4(mix(col.rgb, mapped, u_palette_amount), col.a);
    }
)";

inline const char* lutGrade = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_lut_amount;

    // Built-in cinematic color grade (no external LUT file needed)
    // Applies a warm/cool cross-process style grade
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        // Lift shadows warm, push highlights cool
        vec3 shadows = vec3(0.1, 0.05, 0.0);
        vec3 highlights = vec3(-0.05, 0.0, 0.1);
        float luma = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));
        vec3 graded = col.rgb + shadows * (1.0 - luma) + highlights * luma;
        // Add slight S-curve contrast
        graded = graded * graded * (3.0 - 2.0 * graded);
        fragColor = vec4(mix(col.rgb, graded, u_lut_amount), col.a);
    }
)";

inline const char* bendoscope = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_time;
    uniform float u_bendo_divisions;
    uniform float u_bendo_bend;
    uniform float u_bendo_rotation;
    void main() {
        vec2 p = v_texCoord - 0.5;
        float angle = atan(p.y, p.x);
        float radius = length(p);
        float segs = floor(u_bendo_divisions * 10.0) + 2.0;
        float segAngle = 6.28318 / segs;
        angle = angle + u_bendo_rotation * 6.28318;
        angle = mod(angle + 3.14159, segAngle) - segAngle * 0.5;
        if (angle < 0.0) angle = -angle; // reflect
        // Apply bend curvature
        radius += sin(angle * 3.0) * u_bendo_bend * 0.2;
        vec2 uv = vec2(cos(angle), sin(angle)) * radius + 0.5;
        fragColor = texture(u_texture, uv);
    }
)";

inline const char* uvRemap = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_time;
    uniform float u_uvremap_amount;
    uniform float u_uvremap_scale;
    uniform float u_uvremap_speed;

    // Inline simplex-style noise for UV displacement
    float uvHash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
    float uvNoise(vec2 p) {
        vec2 i = floor(p); vec2 f = fract(p);
        f = f * f * (3.0 - 2.0 * f);
        return mix(mix(uvHash(i), uvHash(i + vec2(1.0, 0.0)), f.x),
                   mix(uvHash(i + vec2(0.0, 1.0)), uvHash(i + vec2(1.0, 1.0)), f.x), f.y);
    }
    float uvFbm(vec2 p) {
        float v = 0.0; float a = 0.5;
        for (int i = 0; i < 4; i++) { v += a * uvNoise(p); p *= 2.0; a *= 0.5; }
        return v;
    }
    void main() {
        float scale = 1.0 + u_uvremap_scale * 8.0;
        float speed = u_uvremap_speed * 0.5;
        vec2 n = vec2(
            uvFbm(v_texCoord * scale + vec2(u_time * speed, 0.0)),
            uvFbm(v_texCoord * scale + vec2(0.0, u_time * speed) + 100.0)
        );
        vec2 uv = v_texCoord + (n - 0.5) * u_uvremap_amount * 0.3;
        fragColor = texture(u_texture, uv);
    }
)";

inline const char* liquidMorph = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_time;
    uniform float u_goo_viscosity;
    uniform float u_goo_amount;
    uniform float u_goo_scale;

    float gooHash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
    float gooNoise(vec2 p) {
        vec2 i = floor(p); vec2 f = fract(p);
        f = f * f * (3.0 - 2.0 * f);
        return mix(mix(gooHash(i), gooHash(i + vec2(1.0, 0.0)), f.x),
                   mix(gooHash(i + vec2(0.0, 1.0)), gooHash(i + vec2(1.0, 1.0)), f.x), f.y);
    }
    float gooFbm(vec2 p) {
        float v = 0.0; float a = 0.5;
        for (int i = 0; i < 5; i++) { v += a * gooNoise(p); p *= 2.0; a *= 0.5; }
        return v;
    }
    void main() {
        float scale = 1.0 + u_goo_scale * 5.0;
        float t = u_time * (0.1 + u_goo_viscosity * 0.6);
        vec2 flow = vec2(
            gooFbm(v_texCoord * scale + vec2(t * 0.7, t * 0.3)),
            gooFbm(v_texCoord * scale + vec2(t * 0.3 + 50.0, t * 0.7 + 20.0))
        );
        vec2 uv = v_texCoord + (flow - 0.5) * u_goo_amount * 0.2;
        fragColor = texture(u_texture, uv);
    }
)";

inline const char* edgeBlur = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform vec2 u_resolution;
    uniform float u_edgeblur_threshold;
    uniform float u_edgeblur_amount;
    void main() {
        vec2 px = 1.0 / u_resolution;
        vec4 col = texture(u_texture, v_texCoord);
        // Sobel edge detection
        float tl = dot(texture(u_texture, v_texCoord + vec2(-px.x, -px.y)).rgb, vec3(0.333));
        float t  = dot(texture(u_texture, v_texCoord + vec2(0.0,   -px.y)).rgb, vec3(0.333));
        float tr = dot(texture(u_texture, v_texCoord + vec2( px.x, -px.y)).rgb, vec3(0.333));
        float l  = dot(texture(u_texture, v_texCoord + vec2(-px.x,  0.0)).rgb,  vec3(0.333));
        float r  = dot(texture(u_texture, v_texCoord + vec2( px.x,  0.0)).rgb,  vec3(0.333));
        float bl = dot(texture(u_texture, v_texCoord + vec2(-px.x,  px.y)).rgb, vec3(0.333));
        float b  = dot(texture(u_texture, v_texCoord + vec2(0.0,    px.y)).rgb, vec3(0.333));
        float br = dot(texture(u_texture, v_texCoord + vec2( px.x,  px.y)).rgb, vec3(0.333));
        float gx = -tl - 2.0*l - bl + tr + 2.0*r + br;
        float gy = -tl - 2.0*t - tr + bl + 2.0*b + br;
        float edge = sqrt(gx*gx + gy*gy);
        float blurStrength = smoothstep(u_edgeblur_threshold * 0.5, 1.0, edge) * u_edgeblur_amount;
        // 5-tap box blur at edges
        vec4 blurred = vec4(0.0);
        float blurRadius = blurStrength * 5.0;
        for (int x = -2; x <= 2; x++) {
            for (int y = -2; y <= 2; y++) {
                blurred += texture(u_texture, v_texCoord + vec2(float(x), float(y)) * px * blurRadius);
            }
        }
        blurred /= 25.0;
        fragColor = mix(col, blurred, blurStrength);
    }
)";

inline const char* brushStrokes = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform vec2 u_resolution;
    uniform float u_brush_size;
    uniform float u_brush_angle;
    uniform float u_brush_flow;
    uniform float u_brush_amount;
    void main() {
        vec2 px = 1.0 / u_resolution;
        vec4 col = texture(u_texture, v_texCoord);
        // Compute local gradient direction
        float lx = dot(texture(u_texture, v_texCoord + vec2(-px.x, 0.0)).rgb, vec3(0.333));
        float rx = dot(texture(u_texture, v_texCoord + vec2( px.x, 0.0)).rgb, vec3(0.333));
        float ty = dot(texture(u_texture, v_texCoord + vec2(0.0, -px.y)).rgb, vec3(0.333));
        float by = dot(texture(u_texture, v_texCoord + vec2(0.0,  px.y)).rgb, vec3(0.333));
        float angle = atan(by - ty, rx - lx) + u_brush_angle * 6.28318;
        angle += (u_brush_flow - 0.5) * 3.14159;
        vec2 dir = vec2(cos(angle), sin(angle));
        // Sample along stroke direction
        vec4 stroke = vec4(0.0);
        float size = 1.0 + u_brush_size * 6.0;
        for (int i = -3; i <= 3; i++) {
            vec2 offset = dir * float(i) * px * size;
            stroke += texture(u_texture, v_texCoord + offset);
        }
        stroke /= 7.0;
        fragColor = mix(col, stroke, u_brush_amount);
    }
)";

inline const char* fragmentBurst = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_time;
    uniform float u_frag_copies;
    uniform float u_frag_spread;
    uniform float u_frag_rotation;
    uniform float u_frag_scale;
    void main() {
        int n = int(u_frag_copies * 6.0) + 2;
        vec4 result = vec4(0.0);
        for (int i = 0; i < 8; i++) {
            if (i >= n) break;
            float fi = float(i);
            float a = fi / float(n) * 6.28318;
            vec2 offset = vec2(cos(a), sin(a)) * u_frag_spread * 0.3;
            float rot = u_frag_rotation * fi * 0.5;
            float s = 1.0 / (1.0 + fi * (1.0 - u_frag_scale) * 0.3);
            vec2 uv = v_texCoord - 0.5 - offset;
            float c = cos(rot), sn = sin(rot);
            uv = mat2(c, -sn, sn, c) * uv;
            uv = uv / s + 0.5;
            result += texture(u_texture, uv) / float(n);
        }
        fragColor = result;
    }
)";

inline const char* signalDestroy = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_time;
    uniform float u_destroy_amount;
    uniform float u_destroy_speed;
    uniform float u_destroy_mode;

    float dHash(float p) { return fract(sin(p) * 43758.5453); }
    float dHash2(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float amt = u_destroy_amount;
        if (amt < 0.001) { fragColor = col; return; }

        float t = floor(u_time * (1.0 + u_destroy_speed * 30.0));
        vec2 uv = v_texCoord;

        // Block-based corruption
        vec2 block = floor(uv * (10.0 + amt * 20.0));
        float n = dHash2(block + t);

        // UV scatter
        uv += (vec2(dHash(n * 17.0), dHash(n * 31.0)) - 0.5) * amt * 0.2;

        // Channel separation
        float r = texture(u_texture, uv + vec2(amt * 0.04 * n, 0.0)).r;
        float g = texture(u_texture, uv).g;
        float b = texture(u_texture, uv - vec2(amt * 0.04 * n, 0.0)).b;

        // Block corruption based on mode
        if (u_destroy_mode > 0.33 && n > 1.0 - amt * 0.4) {
            r = dHash(n + 1.0); g = dHash(n + 2.0); b = dHash(n + 3.0);
        }
        // Scanline glitch
        if (u_destroy_mode > 0.66) {
            float scanline = dHash(floor(v_texCoord.y * 200.0) + t);
            if (scanline > 1.0 - amt * 0.3) {
                uv.x += (scanline - 0.5) * amt * 0.15;
                r = texture(u_texture, uv).r;
                g = texture(u_texture, uv + vec2(0.01, 0.0)).g;
                b = texture(u_texture, uv - vec2(0.01, 0.0)).b;
            }
        }
        fragColor = vec4(r, g, b, col.a);
    }
)";

inline const char* lineCloner = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_lineclone_copies;
    uniform float u_lineclone_ox;
    uniform float u_lineclone_oy;
    uniform float u_lineclone_scale;
    uniform float u_lineclone_rotation;
    void main() {
        int n = int(u_lineclone_copies * 8.0) + 2;
        vec4 result = vec4(0.0);
        float fn = float(n);
        for (int i = 0; i < 10; i++) {
            if (i >= n) break;
            float fi = float(i) - (fn - 1.0) * 0.5;
            // ox/oy: 0 = spread left/up, 0.5 = no spread, 1 = spread right/down
            // Spacing scales with copy count so copies don't overlap
            float spacingX = (u_lineclone_ox - 0.5) * 2.0;
            float spacingY = (u_lineclone_oy - 0.5) * 2.0;
            vec2 offset = vec2(spacingX * fi * 0.12, spacingY * fi * 0.12);
            float rot = (u_lineclone_rotation - 0.5) * fi * 0.5;
            float s = 0.3 + u_lineclone_scale * 1.4;
            vec2 uv = v_texCoord - 0.5 - offset;
            float c = cos(rot), sn = sin(rot);
            uv = mat2(c, -sn, sn, c) * uv;
            uv = uv / s + 0.5;
            vec4 samp = texture(u_texture, uv);
            // Lighten blend: take max per channel (works with opaque textures)
            result = max(result, samp);
        }
        fragColor = result;
    }
)";

inline const char* radialCloner = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_radclone_copies;
    uniform float u_radclone_radius;
    uniform float u_radclone_rotation;
    uniform float u_radclone_scale;
    void main() {
        int n = int(u_radclone_copies * 10.0) + 2;
        vec4 result = vec4(0.0);
        for (int i = 0; i < 12; i++) {
            if (i >= n) break;
            float angle = float(i) / float(n) * 6.28318 + u_radclone_rotation * 6.28318;
            vec2 center = vec2(0.5) + vec2(cos(angle), sin(angle)) * u_radclone_radius * 0.4;
            float s = max(0.1, u_radclone_scale * 2.0);
            vec2 uv = (v_texCoord - center) / s + 0.5;
            vec4 samp = texture(u_texture, uv);
            // Lighten blend: take max per channel
            result = max(result, samp);
        }
        fragColor = result;
    }
)";

inline const char* cubeScatter = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_time;
    uniform float u_cubescat_gx;
    uniform float u_cubescat_gy;
    uniform float u_cubescat_explode;
    uniform float u_cubescat_rotation;

    float csHash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

    void main() {
        float gx = floor(u_cubescat_gx * 6.0) + 2.0;
        float gy = floor(u_cubescat_gy * 6.0) + 2.0;
        vec2 gridSize = vec2(gx, gy);
        vec2 cell = floor(v_texCoord * gridSize);
        vec2 cellUV = fract(v_texCoord * gridSize);

        float rnd = csHash(cell);
        float rnd2 = csHash(cell + 0.5);

        // Per-tile offset for explosion
        vec2 offset = (vec2(csHash(cell + 0.1), csHash(cell + 0.2)) - 0.5) * u_cubescat_explode * 0.5;

        // Per-tile rotation
        float rot = (rnd - 0.5) * u_cubescat_rotation * 3.14159;
        vec2 uv = cellUV - 0.5;
        float c = cos(rot), s = sin(rot);
        uv = mat2(c, -s, s, c) * uv;
        uv += 0.5;

        // Map back to full image UV
        vec2 originalUV = (cell + uv + offset) / gridSize;
        fragColor = texture(u_texture, originalUV);
    }
)";

inline const char* infiniteZoom = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_time;
    uniform float u_infzoom_speed;
    uniform float u_infzoom_rotation;
    uniform float u_infzoom_cx;
    uniform float u_infzoom_cy;
    void main() {
        vec2 center = vec2(u_infzoom_cx, u_infzoom_cy);
        vec2 uv = v_texCoord - center;
        // Progressive zoom per frame
        float zoomAmt = u_infzoom_speed * 2.0;
        float scale = 1.0 - zoomAmt * 0.01;
        uv *= scale;
        // Rotation per frame
        float rotAmt = (u_infzoom_rotation - 0.5) * 4.0;
        float angle = rotAmt * 0.01;
        float c = cos(angle), s = sin(angle);
        uv = mat2(c, -s, s, c) * uv;
        uv += center;
        fragColor = texture(u_texture, uv);
    }
)";

inline const char* bumpLight = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform vec2 u_resolution;
    uniform float u_bumplight_lx;
    uniform float u_bumplight_ly;
    uniform float u_bumplight_intensity;
    uniform float u_bumplight_height;
    void main() {
        vec2 px = 1.0 / u_resolution;
        vec4 base = texture(u_texture, v_texCoord);
        // Sobel for normal map from luminance
        float tl = dot(texture(u_texture, v_texCoord + vec2(-px.x, -px.y)).rgb, vec3(0.333));
        float tr = dot(texture(u_texture, v_texCoord + vec2( px.x, -px.y)).rgb, vec3(0.333));
        float bl = dot(texture(u_texture, v_texCoord + vec2(-px.x,  px.y)).rgb, vec3(0.333));
        float br = dot(texture(u_texture, v_texCoord + vec2( px.x,  px.y)).rgb, vec3(0.333));
        float dX = (tr + br) - (tl + bl);
        float dY = (bl + br) - (tl + tr);
        float height = u_bumplight_height * 2.0;
        vec3 normal = normalize(vec3(dX * height, dY * height, 1.0));
        // Light direction from light position
        vec3 lightDir = normalize(vec3(u_bumplight_lx - v_texCoord.x, u_bumplight_ly - v_texCoord.y, 0.3));
        float intensity = u_bumplight_intensity * 2.0;
        float diffuse = max(dot(normal, lightDir), 0.0) * intensity;
        fragColor = vec4(base.rgb * (0.3 + diffuse), base.a);
    }
)";

// ============================================================
// Phase 15: Resolume Sources (12 new sources)
// ============================================================

inline const char* sourceSolidColor = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_src_red;
    uniform float u_src_green;
    uniform float u_src_blue;
    void main() {
        fragColor = vec4(u_src_red, u_src_green, u_src_blue, 1.0);
    }
)";

inline const char* sourceStrobeLight = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform float u_beatPhase;
    uniform float u_src_frequency;
    uniform float u_src_fade;
    uniform float u_src_c1r;
    uniform float u_src_c1g;
    uniform float u_src_c1b;
    uniform float u_src_c2r;
    uniform float u_src_c2g;
    uniform float u_src_c2b;
    void main() {
        float freq = 1.0 + u_src_frequency * 29.0;
        float phase = fract(u_time * freq);
        float flash = 1.0 - smoothstep(0.0, max(0.01, u_src_fade), phase);
        vec3 c1 = vec3(u_src_c1r, u_src_c1g, u_src_c1b);
        vec3 c2 = vec3(u_src_c2r, u_src_c2g, u_src_c2b);
        fragColor = vec4(mix(c2, c1, flash), 1.0);
    }
)";

inline const char* sourceCheckerboard = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform float u_src_columns;
    uniform float u_src_rows;
    uniform float u_src_c1_hue;
    uniform float u_src_c2_hue;

    vec3 hsv2rgb(float h, float s, float v) {
        vec3 c = clamp(abs(mod(h * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
        return v * mix(vec3(1.0), c, s);
    }
    void main() {
        float cols = floor(u_src_columns * 14.0) + 2.0;
        float rows = floor(u_src_rows * 14.0) + 2.0;
        float check = mod(floor(v_texCoord.x * cols) + floor(v_texCoord.y * rows), 2.0);
        vec3 c1 = hsv2rgb(u_src_c1_hue, 0.8, 1.0);
        vec3 c2 = (u_src_c2_hue > 0.01) ? hsv2rgb(u_src_c2_hue, 0.8, 1.0) : vec3(0.0);
        fragColor = vec4(mix(c2, c1, check), 1.0);
    }
)";

inline const char* sourceLinePattern = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform float u_src_count;
    uniform float u_src_width;
    uniform float u_src_rotation;
    uniform float u_src_speed;
    uniform float u_src_color_shift;

    vec3 hsv2rgb_lp(float h) {
        return 0.5 + 0.5 * cos(6.28318 * (h + vec3(0.0, 0.33, 0.67)));
    }
    void main() {
        // Rotate UV
        vec2 uv = v_texCoord - 0.5;
        float angle = u_src_rotation * 3.14159;
        float c = cos(angle), s = sin(angle);
        uv = mat2(c, -s, s, c) * uv;
        uv += 0.5;
        uv.y += u_time * u_src_speed * 0.5;
        float lines = floor(u_src_count * 36.0) + 4.0;
        float lineVal = abs(fract(uv.y * lines) - 0.5);
        float width = u_src_width * 0.5;
        float mask = smoothstep(width, width - 0.02, lineVal);
        vec3 col = hsv2rgb_lp(u_src_color_shift);
        fragColor = vec4(col * mask, 1.0);
    }
)";

inline const char* sourceConcentricRings = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_src_count;
    uniform float u_src_spacing;
    uniform float u_src_width;
    uniform float u_src_rotation;
    uniform float u_src_color_shift;
    void main() {
        float dist = distance(v_texCoord, vec2(0.5));
        float rings = floor(u_src_count * 17.0) + 3.0;
        float spacing = 0.5 + u_src_spacing * 2.0;
        float ring = abs(fract(dist * rings * spacing + u_time * u_src_rotation * 0.5) - 0.5);
        float width = u_src_width * 0.5;
        float mask = smoothstep(width, width - 0.02, ring);
        // Audio reactive: RMS pulses brightness
        mask *= 0.8 + u_rms * 0.4;
        float hue = u_src_color_shift + dist * 0.3;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * mask, 1.0);
    }
)";

inline const char* sourceSineOscillator = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_src_waves;
    uniform float u_src_frequency;
    uniform float u_src_amplitude;
    uniform float u_src_modulation;
    uniform float u_src_thickness;
    uniform float u_src_color_shift;
    void main() {
        float n = floor(u_src_waves * 9.0) + 1.0;
        float mask = 0.0;
        for (float i = 0.0; i < 10.0; i++) {
            if (i >= n) break;
            float phase = v_texCoord.x * (u_src_frequency * 10.0 + 1.0) + u_time * 2.0 + i * 0.5;
            float modSig = sin(phase * u_src_modulation * 5.0) * 0.3;
            float wave = sin(phase + modSig) * u_src_amplitude * 0.3;
            float y = 0.5 + wave + (i - n * 0.5) * 0.12;
            float dist = abs(v_texCoord.y - y);
            float thick = max(0.002, u_src_thickness * 0.04);
            mask += smoothstep(thick, 0.0, dist);
        }
        mask = clamp(mask, 0.0, 1.0);
        mask *= 0.8 + u_rms * 0.4;
        float hue = u_src_color_shift + v_texCoord.x * 0.2;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * mask, 1.0);
    }
)";

inline const char* sourceSpiralPattern = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_src_arms;
    uniform float u_src_zoom;
    uniform float u_src_speed;
    uniform float u_src_distortion;
    uniform float u_src_color_shift;
    void main() {
        vec2 p = v_texCoord - 0.5;
        float angle = atan(p.y, p.x);
        float radius = length(p);
        float n = floor(u_src_arms * 7.0) + 1.0;
        float spiral = fract(angle / 6.28318 * n + log(radius + 0.001) * u_src_zoom * 5.0 + u_time * u_src_speed);
        spiral += sin(radius * 20.0) * u_src_distortion * 0.2;
        float mask = smoothstep(0.3, 0.5, spiral) * smoothstep(0.7, 0.5, spiral);
        mask *= 0.8 + u_rms * 0.4;
        float hue = u_src_color_shift + radius * 0.5 + u_time * 0.1;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * mask, 1.0);
    }
)";

inline const char* sourceMetaballs = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_src_count;
    uniform float u_src_size;
    uniform float u_src_speed;
    uniform float u_src_blend;
    uniform float u_src_color_shift;
    void main() {
        int n = int(u_src_count * 7.0) + 3;
        float field = 0.0;
        float colorMix = 0.0;
        for (int i = 0; i < 10; i++) {
            if (i >= n) break;
            float fi = float(i);
            vec2 pos = vec2(
                0.5 + 0.3 * sin(u_time * u_src_speed + fi * 1.7),
                0.5 + 0.3 * cos(u_time * u_src_speed * 0.8 + fi * 2.3)
            );
            float dist = distance(v_texCoord, pos);
            float blobSize = u_src_size * 0.1;
            field += blobSize / (dist * dist + 0.001);
            colorMix += fi / float(n) * blobSize / (dist * dist + 0.001);
        }
        float threshold = u_src_blend * 5.0 + 1.0;
        float mask = smoothstep(threshold, threshold + 0.5, field);
        colorMix = colorMix / max(field, 0.001);
        float hue = u_src_color_shift + colorMix * 0.3;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= mask;
        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)";

inline const char* sourceTerrainLines = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_src_height;
    uniform float u_src_lines;
    uniform float u_src_jagginess;
    uniform float u_src_speed;
    uniform float u_src_tilt;
    uniform float u_src_color_shift;

    float terrainHash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
    float terrainNoise(vec2 p) {
        vec2 i = floor(p); vec2 f = fract(p);
        f = f * f * (3.0 - 2.0 * f);
        return mix(mix(terrainHash(i), terrainHash(i + vec2(1.0, 0.0)), f.x),
                   mix(terrainHash(i + vec2(0.0, 1.0)), terrainHash(i + vec2(1.0, 1.0)), f.x), f.y);
    }
    void main() {
        float numLines = floor(u_src_lines * 35.0) + 5.0;
        float mask = 0.0;
        for (float i = 0.0; i < 40.0; i++) {
            if (i >= numLines) break;
            float baseY = i / numLines;
            float perspective = 1.0 + (baseY - 0.5) * u_src_tilt;
            float noiseVal = terrainNoise(vec2(v_texCoord.x * (u_src_jagginess * 10.0 + 1.0), i * 0.3 + u_time * u_src_speed));
            float lineY = baseY + (noiseVal - 0.5) * u_src_height * 0.15 * perspective;
            float dist = abs(v_texCoord.y - lineY);
            mask += smoothstep(0.003, 0.0, dist);
        }
        mask = clamp(mask, 0.0, 1.0);
        mask *= 0.8 + u_rms * 0.4;
        float hue = u_src_color_shift + v_texCoord.y * 0.3;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * mask, 1.0);
    }
)";

inline const char* sourceShapeGenerator = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_src_shape;
    uniform float u_src_size;
    uniform float u_src_rotation;
    uniform float u_src_outline;
    uniform float u_src_color_shift;

    float sdCircle_sg(vec2 p, float r) { return length(p) - r; }
    float sdBox_sg(vec2 p, vec2 b) { vec2 d = abs(p) - b; return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0); }
    float sdTriangle_sg(vec2 p, float r) {
        p.x = abs(p.x);
        float d = max(dot(p, vec2(0.866, 0.5)) - r * 0.5, -p.y - r * 0.5);
        return d;
    }
    float sdHexagon_sg(vec2 p, float r) { vec2 q = abs(p); return max(q.x * 0.866025 + q.y * 0.5, q.y) - r; }
    float sdStar_sg(vec2 p, float r) {
        float a = atan(p.y, p.x) + 1.5708;
        float n = 5.0;
        float seg = 6.28318 / n;
        a = abs(mod(a, seg) - seg * 0.5);
        return length(p) * cos(a) - r * 0.5;
    }
    float sdRing_sg(vec2 p, float r) { return abs(length(p) - r) - r * 0.15; }
    float sdCross_sg(vec2 p, float r) {
        vec2 d = abs(p);
        return min(max(d.x - r * 0.15, d.y - r), max(d.x - r, d.y - r * 0.15));
    }

    void main() {
        vec2 p = v_texCoord - 0.5;
        float rot = u_src_rotation * 6.28318 + u_time * 0.3;
        float c = cos(rot), s = sin(rot);
        p = mat2(c, -s, s, c) * p;
        float size = 0.1 + u_src_size * 0.4;

        float d;
        float shape = u_src_shape;
        if      (shape < 0.143) d = sdCircle_sg(p, size);
        else if (shape < 0.286) d = sdBox_sg(p, vec2(size));
        else if (shape < 0.429) d = sdTriangle_sg(p, size);
        else if (shape < 0.572) d = sdHexagon_sg(p, size);
        else if (shape < 0.715) d = sdStar_sg(p, size);
        else if (shape < 0.858) d = sdRing_sg(p, size);
        else                    d = sdCross_sg(p, size);

        float fill = (u_src_outline > 0.5) ? abs(d) - 0.015 : d;
        float mask = smoothstep(0.01, -0.01, fill);
        mask *= 0.8 + u_rms * 0.4;
        float hue = u_src_color_shift + u_time * 0.05;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * mask, 1.0);
    }
)";

inline const char* sourceBumpLight = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_src_lx;
    uniform float u_src_ly;
    uniform float u_src_intensity;
    uniform float u_src_bumps;
    uniform float u_src_color_shift;

    float bumpHash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
    float bumpNoise(vec2 p) {
        vec2 i = floor(p); vec2 f = fract(p);
        f = f * f * (3.0 - 2.0 * f);
        return mix(mix(bumpHash(i), bumpHash(i + vec2(1.0, 0.0)), f.x),
                   mix(bumpHash(i + vec2(0.0, 1.0)), bumpHash(i + vec2(1.0, 1.0)), f.x), f.y);
    }
    float bumpFbm(vec2 p) {
        float v = 0.0; float a = 0.5;
        for (int i = 0; i < 4; i++) { v += a * bumpNoise(p); p *= 2.0; a *= 0.5; }
        return v;
    }
    void main() {
        float scale = 3.0 + u_src_bumps * 12.0;
        float heightMap = bumpFbm(v_texCoord * scale + u_time * 0.2);
        // Normal from height field
        float eps = 0.005;
        float hL = bumpFbm((v_texCoord + vec2(-eps, 0.0)) * scale + u_time * 0.2);
        float hR = bumpFbm((v_texCoord + vec2( eps, 0.0)) * scale + u_time * 0.2);
        float hD = bumpFbm((v_texCoord + vec2(0.0, -eps)) * scale + u_time * 0.2);
        float hU = bumpFbm((v_texCoord + vec2(0.0,  eps)) * scale + u_time * 0.2);
        vec3 normal = normalize(vec3(hL - hR, hD - hU, 0.3));
        vec3 lightDir = normalize(vec3(u_src_lx - v_texCoord.x, u_src_ly - v_texCoord.y, 0.4));
        float diffuse = max(dot(normal, lightDir), 0.0) * u_src_intensity * 2.0;
        diffuse *= 0.8 + u_rms * 0.4;
        float hue = u_src_color_shift + heightMap * 0.3;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * (0.2 + diffuse), 1.0);
    }
)";

inline const char* sourceInfiniteZoom = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_src_speed;
    uniform float u_src_layers;
    uniform float u_src_rotation;
    uniform float u_src_color_shift;

    float izHash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

    void main() {
        vec2 p = v_texCoord - 0.5;
        float speed = 0.2 + u_src_speed * 1.0;
        float numLayers = floor(u_src_layers * 7.0) + 3.0;
        float rotSpeed = (u_src_rotation - 0.5) * 2.0;
        float mask = 0.0;
        for (float i = 0.0; i < 10.0; i++) {
            if (i >= numLayers) break;
            float t = fract(u_time * speed * 0.1 + i / numLayers);
            float scale = mix(0.1, 3.0, t);
            vec2 uv = p / scale;
            float angle = rotSpeed * t * 3.14159;
            float c = cos(angle), s = sin(angle);
            uv = mat2(c, -s, s, c) * uv;
            uv += 0.5;
            // Generate a pattern per layer
            float pattern = izHash(floor(uv * (4.0 + i * 2.0)));
            float alpha = smoothstep(1.0, 0.5, t) * smoothstep(0.0, 0.2, t);
            mask += pattern * alpha * 0.5;
        }
        mask = clamp(mask, 0.0, 1.0);
        mask *= 0.8 + u_rms * 0.4;
        float hue = u_src_color_shift + u_time * 0.05;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * mask, 1.0);
    }
)";

// ============================================================
// FEEDBACK EFFECT — persistent frame-to-frame feedback with transform
// ============================================================
inline const char* feedback = R"(
    #version 410 core

    in vec2 v_texCoord;
    out vec4 fragColor;

    uniform sampler2D u_texture;      // Current frame
    uniform sampler2D u_feedbackTex;  // Previous frame (persistent)
    uniform float u_time;
    uniform vec2 u_resolution;

    uniform float u_feedback_amount;   // 0=none, 1=full feedback
    uniform float u_feedback_zoom;     // 0.5=zoom in, 0.5=center (no zoom)
    uniform float u_feedback_rotation; // 0.5=no rotation
    uniform float u_feedback_x_offset; // 0.5=no offset
    uniform float u_feedback_y_offset; // 0.5=no offset
    uniform float u_feedback_decay;    // 0=fast decay, 1=slow decay (long trails)
    uniform float u_feedback_hue_shift;// 0=none, hue rotation per frame
    uniform float u_feedback_saturation;// 0.5=unchanged

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

    void main()
    {
        float amount = u_feedback_amount;
        float zoom = 1.0 + (u_feedback_zoom - 0.5) * 0.1;  // 0.95 to 1.05
        float rot = (u_feedback_rotation - 0.5) * 0.1;       // ±0.05 radians/frame
        float ox = (u_feedback_x_offset - 0.5) * 0.02;       // ±0.01 per frame
        float oy = (u_feedback_y_offset - 0.5) * 0.02;
        float decay = 0.8 + u_feedback_decay * 0.19;          // 0.8 to 0.99

        // Transform UV for feedback read
        vec2 uv = v_texCoord - 0.5;
        float c = cos(rot), s = sin(rot);
        uv = mat2(c, -s, s, c) * uv;  // rotate
        uv /= zoom;                    // zoom
        uv += 0.5 + vec2(ox, oy);      // offset

        vec4 current = texture(u_texture, v_texCoord);
        vec4 prev = texture(u_feedbackTex, uv);

        // Apply decay to previous frame
        prev.rgb *= decay;

        // Hue shift on feedback
        float hueShift = u_feedback_hue_shift * 0.02;
        if (hueShift > 0.001) {
            vec3 hsv = rgb2hsv(prev.rgb);
            hsv.x = fract(hsv.x + hueShift);
            // Saturation adjust
            hsv.y *= 0.5 + u_feedback_saturation;
            prev.rgb = hsv2rgb(hsv);
        }

        // Blend: current over feedback
        fragColor = mix(prev, current, 1.0 - amount);
        fragColor.a = 1.0;
    }
)";

// ============================================================
// DIRECTIONAL FEEDBACK — feeds back along edge/motion direction
// ============================================================
inline const char* directionalFeedback = R"(
    #version 410 core

    in vec2 v_texCoord;
    out vec4 fragColor;

    uniform sampler2D u_texture;
    uniform sampler2D u_feedbackTex;
    uniform float u_time;
    uniform vec2 u_resolution;

    uniform float u_dfb_amount;    // feedback strength
    uniform float u_dfb_speed;     // displacement speed
    uniform float u_dfb_direction; // 0=outward, 0.5=along edges, 1=inward
    uniform float u_dfb_spread;    // displacement distance
    uniform float u_dfb_decay;     // trail persistence
    uniform float u_dfb_hue_shift; // color shift per frame

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

    void main()
    {
        vec2 px = 1.0 / u_resolution;
        float amount = u_dfb_amount;
        float speed = u_dfb_speed * 0.02;
        float spread = u_dfb_spread * 0.03;
        float decay = 0.8 + u_dfb_decay * 0.19;
        float dirMode = u_dfb_direction;

        // Compute gradient (edge direction) from current frame
        vec4 left  = texture(u_texture, v_texCoord + vec2(-px.x, 0.0));
        vec4 right = texture(u_texture, v_texCoord + vec2( px.x, 0.0));
        vec4 up    = texture(u_texture, v_texCoord + vec2(0.0,  px.y));
        vec4 down  = texture(u_texture, v_texCoord + vec2(0.0, -px.y));

        // Sobel-like gradient
        float gx = dot(right.rgb - left.rgb, vec3(0.333));
        float gy = dot(up.rgb - down.rgb, vec3(0.333));
        vec2 grad = vec2(gx, gy);
        float gradLen = length(grad);

        // Direction modes:
        // 0.0 = outward from center (point zoom style but following brightness)
        // 0.5 = along edges (perpendicular to gradient = tangent)
        // 1.0 = inward toward center
        vec2 outward = normalize(v_texCoord - 0.5 + 0.001);
        vec2 tangent = vec2(-grad.y, grad.x); // perpendicular to gradient
        vec2 inward = -outward;

        vec2 dir;
        if (dirMode < 0.33)
            dir = mix(outward, tangent, dirMode * 3.0);
        else if (dirMode < 0.67)
            dir = mix(tangent, inward, (dirMode - 0.33) * 3.0);
        else
            dir = inward;

        // Scale displacement by edge strength
        float edgeScale = mix(1.0, gradLen * 5.0, 0.5);
        vec2 offset = dir * spread * edgeScale;

        // Read feedback with directional displacement
        vec2 fbUV = v_texCoord - offset;
        vec4 prev = texture(u_feedbackTex, fbUV) * decay;

        // Hue shift
        float hueShift = u_dfb_hue_shift * 0.02;
        if (hueShift > 0.001) {
            vec3 hsv = rgb2hsv(prev.rgb);
            hsv.x = fract(hsv.x + hueShift);
            prev.rgb = hsv2rgb(hsv);
        }

        vec4 current = texture(u_texture, v_texCoord);
        fragColor = mix(prev, current, 1.0 - amount);
        fragColor.a = 1.0;
    }
)";

// ============================================================
// SPIRAL TUNNEL SOURCE — hypnotic 3D spiral/tunnel
// ============================================================
inline const char* sourceSpiralTunnel = R"(
    #version 410 core

    in vec2 v_texCoord;
    out vec4 fragColor;

    uniform float u_time;
    uniform vec2 u_resolution;

    uniform float u_src_speed;       // rotation/travel speed
    uniform float u_src_arms;        // number of spiral arms
    uniform float u_src_depth;       // tunnel depth (perspective)
    uniform float u_src_twist;       // spiral twist amount
    uniform float u_src_color_shift; // tint color

    uniform float u_rms;
    uniform float u_beatPhase;

    void main()
    {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        float speed = 0.3 + u_src_speed * 2.0;
        int arms = int(u_src_arms * 12.0) + 2;
        float tubeR = 0.3 + u_src_depth * 0.7; // tube radius
        float twist = 1.0 + u_src_twist * 10.0;

        // Camera travels along the inside of a torus tube
        // We transform the view ray into torus-local coordinates
        float camAngle = u_time * speed * 0.3;
        float roll = u_time * speed * 0.2;

        // For each pixel, cast ray from camera center
        float cr = cos(roll), sr = sin(roll);
        vec2 ruv = vec2(uv.x * cr - uv.y * sr, uv.x * sr + uv.y * cr);

        // Map screen position to angle around tube cross-section
        // and distance along the tube
        float r = length(ruv);
        float screenAngle = atan(ruv.y, ruv.x);

        // Create the tunnel effect: 1/r maps radial distance to depth
        float depth = tubeR / (r + 0.001);

        // Torus surface coordinates
        // majorAngle = how far along the ring we're looking
        float majorAngle = camAngle + depth * 0.5;
        // minorAngle = where on the tube cross-section
        float minorAngle = screenAngle;

        // Spiral stripe pattern wrapping around the torus
        float stripe = sin((majorAngle * float(arms) + minorAngle * twist));
        float pattern = smoothstep(-0.05, 0.05, stripe);

        // Depth shading (darker further away)
        float shade = 1.0 / (1.0 + depth * 0.08);

        // Tube edge darkening (center is brighter = looking deeper)
        float edgeFade = smoothstep(0.0, 0.15, r) * (1.0 - smoothstep(1.2, 2.0, r));

        // Color
        vec3 col1 = vec3(1.0);
        vec3 col2 = vec3(0.0);

        float hue = u_src_color_shift;
        if (hue > 0.01) {
            col1 = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        }

        vec3 col = mix(col2, col1, pattern) * shade * edgeFade;
        col *= 0.8 + u_rms * 0.4;

        fragColor = vec4(col, 1.0);
    }
)";

// ============================================================
// WIREFRAME 3D SOURCE — parametric 3D wireframe meshes
// ============================================================
inline const char* sourceWireframe3D = R"(
    #version 410 core

    in vec2 v_texCoord;
    out vec4 fragColor;

    uniform float u_time;
    uniform vec2 u_resolution;

    uniform float u_src_shape;       // 0=sphere, ~0.14=torus, ~0.29=cube, ~0.43=cylinder, ~0.57=cone, ~0.71=icosahedron, ~0.86=wolf
    uniform float u_src_density;     // mesh density (grid lines)
    uniform float u_src_rotation_x;  // rotation speed X
    uniform float u_src_rotation_y;  // rotation speed Y
    uniform float u_src_rotation_z;  // rotation speed Z
    uniform float u_src_thickness;   // line thickness
    uniform float u_src_perspective; // camera distance/FOV
    uniform float u_src_glow;        // line glow amount
    uniform float u_src_color_shift; // color

    // Audio
    uniform float u_rms;
    uniform float u_beatPhase;

    mat3 rotateX(float a) {
        float c = cos(a), s = sin(a);
        return mat3(1,0,0, 0,c,-s, 0,s,c);
    }
    mat3 rotateY(float a) {
        float c = cos(a), s = sin(a);
        return mat3(c,0,s, 0,1,0, -s,0,c);
    }
    mat3 rotateZ(float a) {
        float c = cos(a), s = sin(a);
        return mat3(c,-s,0, s,c,0, 0,0,1);
    }

    // Project 3D point to 2D screen
    vec2 project(vec3 p, float camDist) {
        float perspective = camDist / (camDist + p.z);
        return p.xy * perspective;
    }

    // Distance from point to line segment in 2D
    float lineDist(vec2 p, vec2 a, vec2 b) {
        vec2 pa = p - a, ba = b - a;
        float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
        return length(pa - ba * h);
    }

    // Helper: draw triangle edges and return min distance
    float triEdges(vec2 uv, vec3 a, vec3 b, vec3 c, float camDist, float prev) {
        prev = min(prev, lineDist(uv, project(a, camDist), project(b, camDist)));
        prev = min(prev, lineDist(uv, project(b, camDist), project(c, camDist)));
        prev = min(prev, lineDist(uv, project(c, camDist), project(a, camDist)));
        return prev;
    }

    void main()
    {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        float t = u_time;
        float rxSpeed = (u_src_rotation_x - 0.5) * 4.0;
        float rySpeed = (u_src_rotation_y - 0.5) * 4.0;
        float rzSpeed = (u_src_rotation_z - 0.5) * 4.0;
        mat3 rot = rotateX(t * rxSpeed) * rotateY(t * rySpeed) * rotateZ(t * rzSpeed);

        float camDist = 2.0 + u_src_perspective * 6.0;
        int gridN = int(u_src_density * 20.0) + 4;
        float thick = 0.002 + u_src_thickness * 0.015;
        float glowSize = thick * (1.0 + u_src_glow * 8.0);

        int shape = int(u_src_shape * 6.99);

        float minDist = 100.0;

        // === SPHERE (shape 0) ===
        if (shape == 0) {
            float r = 0.6;
            // Latitude lines
            for (int i = 0; i <= gridN; i++) {
                float phi = 3.14159 * float(i) / float(gridN);
                for (int j = 0; j < gridN * 2; j++) {
                    float t1 = 6.28318 * float(j) / float(gridN * 2);
                    float t2 = 6.28318 * float(j + 1) / float(gridN * 2);
                    vec3 a = rot * vec3(r*sin(phi)*cos(t1), r*cos(phi), r*sin(phi)*sin(t1));
                    vec3 b = rot * vec3(r*sin(phi)*cos(t2), r*cos(phi), r*sin(phi)*sin(t2));
                    float d = lineDist(uv, project(a, camDist), project(b, camDist));
                    minDist = min(minDist, d);
                }
            }
            // Longitude lines
            for (int j = 0; j < gridN; j++) {
                float theta = 6.28318 * float(j) / float(gridN);
                for (int i = 0; i < gridN; i++) {
                    float p1 = 3.14159 * float(i) / float(gridN);
                    float p2 = 3.14159 * float(i + 1) / float(gridN);
                    vec3 a = rot * vec3(r*sin(p1)*cos(theta), r*cos(p1), r*sin(p1)*sin(theta));
                    vec3 b = rot * vec3(r*sin(p2)*cos(theta), r*cos(p2), r*sin(p2)*sin(theta));
                    float d = lineDist(uv, project(a, camDist), project(b, camDist));
                    minDist = min(minDist, d);
                }
            }
        }
        // === TORUS (shape 1) ===
        else if (shape == 1) {
            float R = 0.5, rr = 0.2;
            for (int i = 0; i < gridN; i++) {
                float u1 = 6.28318 * float(i) / float(gridN);
                float u2 = 6.28318 * float(i + 1) / float(gridN);
                for (int j = 0; j < gridN; j++) {
                    float v1 = 6.28318 * float(j) / float(gridN);
                    float v2 = 6.28318 * float(j + 1) / float(gridN);
                    // Ring segments
                    vec3 a = rot * vec3((R+rr*cos(v1))*cos(u1), rr*sin(v1), (R+rr*cos(v1))*sin(u1));
                    vec3 b = rot * vec3((R+rr*cos(v1))*cos(u2), rr*sin(v1), (R+rr*cos(v1))*sin(u2));
                    minDist = min(minDist, lineDist(uv, project(a, camDist), project(b, camDist)));
                    // Tube segments
                    vec3 c = rot * vec3((R+rr*cos(v2))*cos(u1), rr*sin(v2), (R+rr*cos(v2))*sin(u1));
                    minDist = min(minDist, lineDist(uv, project(a, camDist), project(c, camDist)));
                }
            }
        }
        // === CUBE (shape 2) ===
        else if (shape == 2) {
            float s = 0.5;
            // 8 corners, 12 edges drawn explicitly
            vec3 c0=vec3(-s,-s,-s), c1=vec3(s,-s,-s), c2=vec3(s,s,-s), c3=vec3(-s,s,-s);
            vec3 c4=vec3(-s,-s,s),  c5=vec3(s,-s,s),  c6=vec3(s,s,s),  c7=vec3(-s,s,s);
            // Bottom face
            minDist = min(minDist, lineDist(uv, project(rot*c0,camDist), project(rot*c1,camDist)));
            minDist = min(minDist, lineDist(uv, project(rot*c1,camDist), project(rot*c2,camDist)));
            minDist = min(minDist, lineDist(uv, project(rot*c2,camDist), project(rot*c3,camDist)));
            minDist = min(minDist, lineDist(uv, project(rot*c3,camDist), project(rot*c0,camDist)));
            // Top face
            minDist = min(minDist, lineDist(uv, project(rot*c4,camDist), project(rot*c5,camDist)));
            minDist = min(minDist, lineDist(uv, project(rot*c5,camDist), project(rot*c6,camDist)));
            minDist = min(minDist, lineDist(uv, project(rot*c6,camDist), project(rot*c7,camDist)));
            minDist = min(minDist, lineDist(uv, project(rot*c7,camDist), project(rot*c4,camDist)));
            // Verticals
            minDist = min(minDist, lineDist(uv, project(rot*c0,camDist), project(rot*c4,camDist)));
            minDist = min(minDist, lineDist(uv, project(rot*c1,camDist), project(rot*c5,camDist)));
            minDist = min(minDist, lineDist(uv, project(rot*c2,camDist), project(rot*c6,camDist)));
            minDist = min(minDist, lineDist(uv, project(rot*c3,camDist), project(rot*c7,camDist)));
            // Grid lines on faces
            int gN = max(2, gridN / 3);
            for (int i = 1; i < gN; i++) {
                float f = -s + 2.0*s*float(i)/float(gN);
                vec3 a1 = rot*vec3(-s,f,-s), b1 = rot*vec3(s,f,-s);
                vec3 a2 = rot*vec3(-s,f,s),  b2 = rot*vec3(s,f,s);
                minDist = min(minDist, lineDist(uv, project(a1,camDist), project(b1,camDist)));
                minDist = min(minDist, lineDist(uv, project(a2,camDist), project(b2,camDist)));
                vec3 a3 = rot*vec3(f,-s,-s), b3 = rot*vec3(f,s,-s);
                vec3 a4 = rot*vec3(f,-s,s),  b4 = rot*vec3(f,s,s);
                minDist = min(minDist, lineDist(uv, project(a3,camDist), project(b3,camDist)));
                minDist = min(minDist, lineDist(uv, project(a4,camDist), project(b4,camDist)));
            }
        }
        // === CYLINDER (shape 3) ===
        else if (shape == 3) {
            float r = 0.4, h = 0.7;
            for (int i = 0; i < gridN; i++) {
                float t1 = 6.28318 * float(i) / float(gridN);
                float t2 = 6.28318 * float(i + 1) / float(gridN);
                // Top circle
                vec3 a = rot * vec3(r*cos(t1), h, r*sin(t1));
                vec3 b = rot * vec3(r*cos(t2), h, r*sin(t2));
                minDist = min(minDist, lineDist(uv, project(a,camDist), project(b,camDist)));
                // Bottom circle
                vec3 c = rot * vec3(r*cos(t1), -h, r*sin(t1));
                vec3 dd = rot * vec3(r*cos(t2), -h, r*sin(t2));
                minDist = min(minDist, lineDist(uv, project(c,camDist), project(dd,camDist)));
                // Vertical line
                minDist = min(minDist, lineDist(uv, project(a,camDist), project(c,camDist)));
            }
            // Horizontal rings
            for (int ring = 1; ring < gridN/2; ring++) {
                float y = -h + 2.0*h*float(ring)/float(gridN/2);
                for (int i = 0; i < gridN; i++) {
                    float t1 = 6.28318 * float(i) / float(gridN);
                    float t2 = 6.28318 * float(i + 1) / float(gridN);
                    vec3 a = rot * vec3(r*cos(t1), y, r*sin(t1));
                    vec3 b = rot * vec3(r*cos(t2), y, r*sin(t2));
                    minDist = min(minDist, lineDist(uv, project(a,camDist), project(b,camDist)));
                }
            }
        }
        // === CONE (shape 4) ===
        else if (shape == 4) {
            float r = 0.5, h = 0.8;
            vec3 tip = rot * vec3(0.0, h, 0.0);
            for (int i = 0; i < gridN; i++) {
                float t1 = 6.28318 * float(i) / float(gridN);
                float t2 = 6.28318 * float(i + 1) / float(gridN);
                // Base circle
                vec3 a = rot * vec3(r*cos(t1), -h*0.5, r*sin(t1));
                vec3 b = rot * vec3(r*cos(t2), -h*0.5, r*sin(t2));
                minDist = min(minDist, lineDist(uv, project(a,camDist), project(b,camDist)));
                // Slant line to tip
                minDist = min(minDist, lineDist(uv, project(a,camDist), project(tip,camDist)));
            }
        }
        // === ICOSAHEDRON (shape 5) ===
        else if (shape == 5) {
            float p = (1.0 + sqrt(5.0)) / 2.0;
            float sc = 0.35;
            // 12 vertices
            vec3 v0=sc*vec3(-1,p,0),  v1=sc*vec3(1,p,0),  v2=sc*vec3(-1,-p,0), v3=sc*vec3(1,-p,0);
            vec3 v4=sc*vec3(0,-1,p),  v5=sc*vec3(0,1,p),  v6=sc*vec3(0,-1,-p), v7=sc*vec3(0,1,-p);
            vec3 v8=sc*vec3(p,0,-1),  v9=sc*vec3(p,0,1),  v10=sc*vec3(-p,0,-1),v11=sc*vec3(-p,0,1);
            minDist=triEdges(uv,rot*v0,rot*v11,rot*v5,camDist,minDist);
            minDist=triEdges(uv,rot*v0,rot*v5,rot*v1,camDist,minDist);
            minDist=triEdges(uv,rot*v0,rot*v1,rot*v7,camDist,minDist);
            minDist=triEdges(uv,rot*v0,rot*v7,rot*v10,camDist,minDist);
            minDist=triEdges(uv,rot*v0,rot*v10,rot*v11,camDist,minDist);
            minDist=triEdges(uv,rot*v1,rot*v5,rot*v9,camDist,minDist);
            minDist=triEdges(uv,rot*v5,rot*v11,rot*v4,camDist,minDist);
            minDist=triEdges(uv,rot*v11,rot*v10,rot*v2,camDist,minDist);
            minDist=triEdges(uv,rot*v10,rot*v7,rot*v6,camDist,minDist);
            minDist=triEdges(uv,rot*v7,rot*v1,rot*v8,camDist,minDist);
            minDist=triEdges(uv,rot*v3,rot*v9,rot*v4,camDist,minDist);
            minDist=triEdges(uv,rot*v3,rot*v4,rot*v2,camDist,minDist);
            minDist=triEdges(uv,rot*v3,rot*v2,rot*v6,camDist,minDist);
            minDist=triEdges(uv,rot*v3,rot*v6,rot*v8,camDist,minDist);
            minDist=triEdges(uv,rot*v3,rot*v8,rot*v9,camDist,minDist);
            minDist=triEdges(uv,rot*v4,rot*v9,rot*v5,camDist,minDist);
            minDist=triEdges(uv,rot*v2,rot*v4,rot*v11,camDist,minDist);
            minDist=triEdges(uv,rot*v6,rot*v2,rot*v10,camDist,minDist);
            minDist=triEdges(uv,rot*v8,rot*v6,rot*v7,camDist,minDist);
            minDist=triEdges(uv,rot*v9,rot*v8,rot*v1,camDist,minDist);
        }
        // === WOLF (shape 6) — low-poly wolf head ===
        else {
            // Simplified low-poly wolf head: 24 vertices, 38 triangles
            float ws = 0.55;
            vec3 w[24];
            // Muzzle tip
            w[0] = ws*vec3(0.0, -0.1, 0.9);
            // Nose bridge
            w[1] = ws*vec3(-0.12, 0.05, 0.7);
            w[2] = ws*vec3(0.12, 0.05, 0.7);
            // Upper muzzle
            w[3] = ws*vec3(-0.15, 0.12, 0.5);
            w[4] = ws*vec3(0.15, 0.12, 0.5);
            // Lower jaw
            w[5] = ws*vec3(-0.1, -0.2, 0.6);
            w[6] = ws*vec3(0.1, -0.2, 0.6);
            // Cheeks
            w[7] = ws*vec3(-0.35, 0.0, 0.3);
            w[8] = ws*vec3(0.35, 0.0, 0.3);
            // Eyes
            w[9] = ws*vec3(-0.2, 0.2, 0.45);
            w[10] = ws*vec3(0.2, 0.2, 0.45);
            // Brow
            w[11] = ws*vec3(-0.25, 0.35, 0.35);
            w[12] = ws*vec3(0.25, 0.35, 0.35);
            // Forehead
            w[13] = ws*vec3(0.0, 0.45, 0.3);
            // Ear tips
            w[14] = ws*vec3(-0.35, 0.7, 0.1);
            w[15] = ws*vec3(0.35, 0.7, 0.1);
            // Ear bases
            w[16] = ws*vec3(-0.3, 0.4, 0.2);
            w[17] = ws*vec3(0.3, 0.4, 0.2);
            // Ear inner
            w[18] = ws*vec3(-0.2, 0.5, 0.25);
            w[19] = ws*vec3(0.2, 0.5, 0.25);
            // Skull back
            w[20] = ws*vec3(-0.3, 0.2, -0.1);
            w[21] = ws*vec3(0.3, 0.2, -0.1);
            // Neck
            w[22] = ws*vec3(-0.2, -0.15, 0.0);
            w[23] = ws*vec3(0.2, -0.15, 0.0);

            // Muzzle
            minDist=triEdges(uv,rot*w[0],rot*w[1],rot*w[2],camDist,minDist);
            minDist=triEdges(uv,rot*w[0],rot*w[5],rot*w[1],camDist,minDist);
            minDist=triEdges(uv,rot*w[0],rot*w[6],rot*w[2],camDist,minDist);
            minDist=triEdges(uv,rot*w[0],rot*w[5],rot*w[6],camDist,minDist);
            minDist=triEdges(uv,rot*w[1],rot*w[3],rot*w[9],camDist,minDist);
            minDist=triEdges(uv,rot*w[2],rot*w[4],rot*w[10],camDist,minDist);
            minDist=triEdges(uv,rot*w[1],rot*w[5],rot*w[7],camDist,minDist);
            minDist=triEdges(uv,rot*w[2],rot*w[6],rot*w[8],camDist,minDist);
            // Face
            minDist=triEdges(uv,rot*w[3],rot*w[9],rot*w[11],camDist,minDist);
            minDist=triEdges(uv,rot*w[4],rot*w[10],rot*w[12],camDist,minDist);
            minDist=triEdges(uv,rot*w[9],rot*w[11],rot*w[13],camDist,minDist);
            minDist=triEdges(uv,rot*w[10],rot*w[12],rot*w[13],camDist,minDist);
            minDist=triEdges(uv,rot*w[11],rot*w[12],rot*w[13],camDist,minDist);
            minDist=triEdges(uv,rot*w[3],rot*w[7],rot*w[9],camDist,minDist);
            minDist=triEdges(uv,rot*w[4],rot*w[8],rot*w[10],camDist,minDist);
            minDist=triEdges(uv,rot*w[7],rot*w[9],rot*w[11],camDist,minDist);
            minDist=triEdges(uv,rot*w[8],rot*w[10],rot*w[12],camDist,minDist);
            // Ears
            minDist=triEdges(uv,rot*w[11],rot*w[16],rot*w[14],camDist,minDist);
            minDist=triEdges(uv,rot*w[16],rot*w[14],rot*w[18],camDist,minDist);
            minDist=triEdges(uv,rot*w[11],rot*w[18],rot*w[13],camDist,minDist);
            minDist=triEdges(uv,rot*w[12],rot*w[17],rot*w[15],camDist,minDist);
            minDist=triEdges(uv,rot*w[17],rot*w[15],rot*w[19],camDist,minDist);
            minDist=triEdges(uv,rot*w[12],rot*w[19],rot*w[13],camDist,minDist);
            // Skull
            minDist=triEdges(uv,rot*w[11],rot*w[16],rot*w[20],camDist,minDist);
            minDist=triEdges(uv,rot*w[12],rot*w[17],rot*w[21],camDist,minDist);
            minDist=triEdges(uv,rot*w[16],rot*w[20],rot*w[22],camDist,minDist);
            minDist=triEdges(uv,rot*w[17],rot*w[21],rot*w[23],camDist,minDist);
            minDist=triEdges(uv,rot*w[7],rot*w[20],rot*w[22],camDist,minDist);
            minDist=triEdges(uv,rot*w[8],rot*w[21],rot*w[23],camDist,minDist);
            minDist=triEdges(uv,rot*w[5],rot*w[7],rot*w[22],camDist,minDist);
            minDist=triEdges(uv,rot*w[6],rot*w[8],rot*w[23],camDist,minDist);
            minDist=triEdges(uv,rot*w[5],rot*w[22],rot*w[23],camDist,minDist);
            minDist=triEdges(uv,rot*w[5],rot*w[6],rot*w[23],camDist,minDist);
            // Jaw bottom
            minDist=triEdges(uv,rot*w[7],rot*w[8],rot*w[20],camDist,minDist);
            minDist=triEdges(uv,rot*w[20],rot*w[21],rot*w[8],camDist,minDist);
        }

        // Anti-aliased line rendering with glow
        float line = smoothstep(thick, 0.0, minDist);
        float glow = smoothstep(glowSize, 0.0, minDist) * 0.4;
        float mask = line + glow;

        // Color
        float hue = u_src_color_shift;
        vec3 col;
        if (hue < 0.01)
            col = vec3(1.0); // white wireframe
        else
            col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));

        // Audio reactivity
        col *= 0.8 + u_rms * 0.4;

        fragColor = vec4(col * mask, 1.0);
    }
)";

// ============================================================
// LINE GENERATOR SOURCE — pattern generator with built-in feedback
// ============================================================
inline const char* sourceLineGenerator = R"(
    #version 410 core

    in vec2 v_texCoord;
    out vec4 fragColor;

    uniform sampler2D u_feedbackTex;  // Previous frame for feedback
    uniform float u_time;
    uniform vec2 u_resolution;

    uniform float u_src_pattern;     // 0=horiz, 0.17=vert, 0.33=diag, 0.5=grid, 0.67=radial, 0.83=random
    uniform float u_src_count;       // number of lines
    uniform float u_src_thickness;   // line width
    uniform float u_src_speed;       // animation speed
    uniform float u_src_feedback;    // feedback amount (0=none, 1=full)
    uniform float u_src_fb_zoom;     // feedback zoom (0.5=none)
    uniform float u_src_fb_rotation; // feedback rotation (0.5=none)
    uniform float u_src_fb_decay;    // feedback decay speed
    uniform float u_src_color_shift; // color

    // Audio
    uniform float u_rms;
    uniform float u_beatPhase;

    float hash(vec2 p) {
        return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
    }

    void main()
    {
        vec2 uv = v_texCoord;
        int pattern = int(u_src_pattern * 5.99);
        int count = int(u_src_count * 30.0) + 2;
        float thick = 0.002 + u_src_thickness * 0.04;
        float speed = u_src_speed * 3.0;
        float t = u_time * speed;

        float mask = 0.0;

        if (pattern == 0) {
            // Horizontal lines
            for (int i = 0; i < count; i++) {
                float y = float(i + 1) / float(count + 1);
                y += sin(t + float(i) * 1.7) * 0.05;
                float d = abs(uv.y - y);
                mask += smoothstep(thick, 0.0, d);
            }
        }
        else if (pattern == 1) {
            // Vertical lines
            for (int i = 0; i < count; i++) {
                float x = float(i + 1) / float(count + 1);
                x += sin(t + float(i) * 2.3) * 0.05;
                float d = abs(uv.x - x);
                mask += smoothstep(thick, 0.0, d);
            }
        }
        else if (pattern == 2) {
            // Diagonal lines
            for (int i = 0; i < count; i++) {
                float offset = float(i) / float(count) + t * 0.1;
                float d = abs(fract(uv.x + uv.y + offset) - 0.5);
                mask += smoothstep(thick * 1.5, 0.0, d);
            }
        }
        else if (pattern == 3) {
            // Grid
            int halfCount = max(2, count / 2);
            for (int i = 0; i < halfCount; i++) {
                float y = float(i + 1) / float(halfCount + 1);
                float x = float(i + 1) / float(halfCount + 1);
                y += sin(t * 0.5 + float(i)) * 0.02;
                x += cos(t * 0.5 + float(i)) * 0.02;
                mask += smoothstep(thick, 0.0, abs(uv.y - y));
                mask += smoothstep(thick, 0.0, abs(uv.x - x));
            }
        }
        else if (pattern == 4) {
            // Radial lines from center
            vec2 p = uv - 0.5;
            float angle = atan(p.y, p.x);
            for (int i = 0; i < count; i++) {
                float targetAngle = 6.28318 * float(i) / float(count) + t * 0.2;
                float d = abs(sin((angle - targetAngle) * 0.5));
                mask += smoothstep(thick * 2.0, 0.0, d) * smoothstep(0.0, 0.1, length(p));
            }
        }
        else {
            // Random / scattered
            for (int i = 0; i < min(count, 20); i++) {
                float fi = float(i);
                float x1 = hash(vec2(fi, 0.0));
                float y1 = hash(vec2(fi, 1.0));
                float x2 = hash(vec2(fi, 2.0));
                float y2 = hash(vec2(fi, 3.0));
                // Animate endpoints
                x1 += sin(t * 0.5 + fi * 1.3) * 0.15;
                y1 += cos(t * 0.7 + fi * 0.9) * 0.15;
                x2 += sin(t * 0.6 + fi * 2.1) * 0.15;
                y2 += cos(t * 0.4 + fi * 1.7) * 0.15;
                // Line distance
                vec2 a = vec2(x1, y1), b = vec2(x2, y2);
                vec2 pa = uv - a, ba = b - a;
                float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
                float d = length(pa - ba * h);
                mask += smoothstep(thick, 0.0, d);
            }
        }

        mask = clamp(mask, 0.0, 1.0);

        // Color
        float hue = u_src_color_shift;
        vec3 col;
        if (hue < 0.01)
            col = vec3(1.0);
        else
            col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));

        vec3 lineColor = col * mask;

        // === Built-in feedback ===
        float fbAmount = u_src_feedback;
        if (fbAmount > 0.01) {
            float fbZoom = 1.0 + (u_src_fb_zoom - 0.5) * 0.08;
            float fbRot = (u_src_fb_rotation - 0.5) * 0.08;
            float fbDecay = 0.8 + u_src_fb_decay * 0.19;

            vec2 fbUV = uv - 0.5;
            float c = cos(fbRot), s = sin(fbRot);
            fbUV = mat2(c, -s, s, c) * fbUV;
            fbUV /= fbZoom;
            fbUV += 0.5;

            vec3 prev = texture(u_feedbackTex, fbUV).rgb * fbDecay;
            lineColor = max(lineColor, prev * fbAmount);
        }

        // Audio reactivity
        lineColor *= 0.8 + u_rms * 0.4;

        fragColor = vec4(lineColor, 1.0);
    }
)";

// ============================================================
// LINE SOURCES — 10 algorithmic line generators
// ============================================================

// 1. Zigzag Lines — angular zigzag patterns
inline const char* sourceZigzagLines = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_count;
    uniform float u_src_amplitude;
    uniform float u_src_speed;
    uniform float u_src_thickness;
    uniform float u_src_color_shift;
    uniform float u_rms;
    void main() {
        int n = int(u_src_count * 14.0) + 2;
        float thick = 0.002 + u_src_thickness * 0.03;
        float amp = u_src_amplitude * 0.3;
        float t = u_time * (0.5 + u_src_speed * 2.0);
        float mask = 0.0;
        for (int i = 0; i < n; i++) {
            float y = float(i + 1) / float(n + 1);
            float seg = floor(v_texCoord.x * 8.0 + t);
            float frac = fract(v_texCoord.x * 8.0 + t);
            float dir = mod(seg, 2.0) * 2.0 - 1.0;
            float zigY = y + dir * frac * amp - dir * 0.5 * amp;
            mask += smoothstep(thick, 0.0, abs(v_texCoord.y - zigY));
        }
        mask = clamp(mask, 0.0, 1.0);
        float hue = u_src_color_shift;
        vec3 col = hue < 0.01 ? vec3(1.0) : 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * mask * (0.8 + u_rms * 0.4), 1.0);
    }
)";

// 2. Star Burst — lines radiating from center with rotation
inline const char* sourceStarBurst = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_rays;
    uniform float u_src_thickness;
    uniform float u_src_speed;
    uniform float u_src_taper;
    uniform float u_src_color_shift;
    uniform float u_rms;
    void main() {
        vec2 p = v_texCoord - 0.5;
        float aspect = u_resolution.x / u_resolution.y;
        p.x *= aspect;
        int nRays = int(u_src_rays * 20.0) + 3;
        float thick = 0.005 + u_src_thickness * 0.04;
        float angle = atan(p.y, p.x) + u_time * (u_src_speed - 0.5) * 2.0;
        float r = length(p);
        float segAngle = 6.28318 / float(nRays);
        float a = mod(angle + 3.14159, segAngle) - segAngle * 0.5;
        float d = abs(sin(a)) * r;
        float taper = 1.0 - r * u_src_taper * 2.0;
        float mask = smoothstep(thick * taper, 0.0, d) * step(0.0, taper);
        float hue = u_src_color_shift;
        vec3 col = hue < 0.01 ? vec3(1.0) : 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * mask * (0.8 + u_rms * 0.4), 1.0);
    }
)";

// 3. Polygon Lines — rotating polygon outlines
inline const char* sourcePolygonLines = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_sides;
    uniform float u_src_size;
    uniform float u_src_thickness;
    uniform float u_src_layers;
    uniform float u_src_speed;
    uniform float u_src_color_shift;
    uniform float u_rms;
    float sdPolygon(vec2 p, int n, float r) {
        float an = 6.28318 / float(n);
        float a = atan(p.y, p.x) + an * 0.5;
        a = mod(a, an) - an * 0.5;
        return length(p) * cos(a) - r * cos(an * 0.5);
    }
    void main() {
        vec2 p = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        p.x *= aspect;
        int sides = int(u_src_sides * 7.0) + 3;
        float thick = 0.005 + u_src_thickness * 0.03;
        int nLayers = int(u_src_layers * 6.0) + 1;
        float rot = u_time * (u_src_speed - 0.5) * 3.0;
        float mask = 0.0;
        for (int i = 0; i < nLayers; i++) {
            float sz = (0.2 + u_src_size * 0.5) * (1.0 - float(i) * 0.15);
            float r = rot + float(i) * 0.3;
            float c = cos(r), s = sin(r);
            vec2 rp = vec2(p.x * c - p.y * s, p.x * s + p.y * c);
            float d = abs(sdPolygon(rp, sides, sz));
            mask += smoothstep(thick, 0.0, d);
        }
        mask = clamp(mask, 0.0, 1.0);
        float hue = u_src_color_shift;
        vec3 col = hue < 0.01 ? vec3(1.0) : 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * mask * (0.8 + u_rms * 0.4), 1.0);
    }
)";

// 4. Waveform Lines — multiple sine/saw/square wave patterns
inline const char* sourceWaveformLines = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_waveform;
    uniform float u_src_freq;
    uniform float u_src_amplitude;
    uniform float u_src_count;
    uniform float u_src_thickness;
    uniform float u_src_speed;
    uniform float u_src_color_shift;
    uniform float u_rms;
    void main() {
        int n = int(u_src_count * 8.0) + 1;
        float thick = 0.002 + u_src_thickness * 0.03;
        float freq = 1.0 + u_src_freq * 10.0;
        float amp = 0.05 + u_src_amplitude * 0.3;
        float t = u_time * (0.5 + u_src_speed * 3.0);
        int waveType = int(u_src_waveform * 3.99);
        float mask = 0.0;
        for (int i = 0; i < n; i++) {
            float y = float(i + 1) / float(n + 1);
            float phase = v_texCoord.x * freq + t + float(i) * 1.5;
            float wave = 0.0;
            if (waveType == 0) wave = sin(phase);
            else if (waveType == 1) wave = fract(phase / 6.28318) * 2.0 - 1.0;
            else if (waveType == 2) wave = step(0.0, sin(phase)) * 2.0 - 1.0;
            else wave = sin(phase) + sin(phase * 2.0) * 0.5 + sin(phase * 3.0) * 0.33;
            float wy = y + wave * amp;
            mask += smoothstep(thick, 0.0, abs(v_texCoord.y - wy));
        }
        mask = clamp(mask, 0.0, 1.0);
        float hue = u_src_color_shift;
        vec3 col = hue < 0.01 ? vec3(1.0) : 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * mask * (0.8 + u_rms * 0.4), 1.0);
    }
)";

// 5. Lissajous Lines — parametric curves
inline const char* sourceLissajous = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_ratio_x;
    uniform float u_src_ratio_y;
    uniform float u_src_phase;
    uniform float u_src_thickness;
    uniform float u_src_speed;
    uniform float u_src_color_shift;
    uniform float u_rms;
    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        float rx = float(int(u_src_ratio_x * 7.0) + 1);
        float ry = float(int(u_src_ratio_y * 7.0) + 1);
        float ph = u_src_phase * 6.28318 + u_time * u_src_speed * 0.5;
        float thick = 0.005 + u_src_thickness * 0.03;
        float minDist = 100.0;
        for (int i = 0; i < 200; i++) {
            float t = float(i) / 200.0 * 6.28318;
            vec2 p = vec2(sin(rx * t + ph), sin(ry * t)) * 0.7;
            minDist = min(minDist, length(uv - p));
        }
        float mask = smoothstep(thick, 0.0, minDist);
        float hue = u_src_color_shift;
        vec3 col = hue < 0.01 ? vec3(1.0) : 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * mask * (0.8 + u_rms * 0.4), 1.0);
    }
)";

// 6. Spirograph Lines — epitrochoid/hypotrochoid curves
inline const char* sourceSpirograph = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_inner;
    uniform float u_src_offset;
    uniform float u_src_thickness;
    uniform float u_src_speed;
    uniform float u_src_color_shift;
    uniform float u_rms;
    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        float R = 0.6;
        float r = 0.1 + u_src_inner * 0.4;
        float d = 0.1 + u_src_offset * 0.5;
        float thick = 0.004 + u_src_thickness * 0.02;
        float phase = u_time * u_src_speed * 0.3;
        float minDist = 100.0;
        for (int i = 0; i < 300; i++) {
            float t = float(i) / 300.0 * 6.28318 * 10.0 + phase;
            float x = (R - r) * cos(t) + d * cos((R - r) / r * t);
            float y = (R - r) * sin(t) - d * sin((R - r) / r * t);
            minDist = min(minDist, length(uv - vec2(x, y)));
        }
        float mask = smoothstep(thick, 0.0, minDist);
        float hue = u_src_color_shift;
        vec3 col = hue < 0.01 ? vec3(1.0) : 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * mask * (0.8 + u_rms * 0.4), 1.0);
    }
)";

// 7. Angular Grid — angled intersecting lines forming geometric patterns
inline const char* sourceAngularGrid = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_angle;
    uniform float u_src_count;
    uniform float u_src_thickness;
    uniform float u_src_symmetry;
    uniform float u_src_speed;
    uniform float u_src_color_shift;
    uniform float u_rms;
    void main() {
        vec2 uv = v_texCoord;
        int n = int(u_src_count * 14.0) + 3;
        float thick = 0.002 + u_src_thickness * 0.02;
        float baseAngle = u_src_angle * 3.14159;
        int nDirs = int(u_src_symmetry * 5.0) + 2;
        float t = u_time * u_src_speed * 0.3;
        float mask = 0.0;
        for (int d = 0; d < nDirs; d++) {
            float angle = baseAngle + float(d) * 3.14159 / float(nDirs) + t;
            float c = cos(angle), s = sin(angle);
            float proj = uv.x * c + uv.y * s;
            float lineVal = abs(fract(proj * float(n)) - 0.5);
            mask += smoothstep(thick * float(n), 0.0, lineVal);
        }
        mask = clamp(mask, 0.0, 1.0);
        float hue = u_src_color_shift;
        vec3 col = hue < 0.01 ? vec3(1.0) : 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * mask * (0.8 + u_rms * 0.4), 1.0);
    }
)";

// 8. Fractal Lines — recursive branching tree pattern
inline const char* sourceFractalTree = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_branches;
    uniform float u_src_angle;
    uniform float u_src_depth;
    uniform float u_src_thickness;
    uniform float u_src_speed;
    uniform float u_src_color_shift;
    uniform float u_rms;
    float lineSeg(vec2 p, vec2 a, vec2 b) {
        vec2 pa = p - a, ba = b - a;
        float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
        return length(pa - ba * h);
    }
    void main() {
        vec2 uv = v_texCoord;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x = (uv.x - 0.5) * aspect + 0.5;
        float thick = 0.003 + u_src_thickness * 0.02;
        float brAngle = 0.2 + u_src_angle * 1.2;
        int nBranch = int(u_src_branches * 2.0) + 2;
        int maxDepth = int(u_src_depth * 5.0) + 2;
        float t = u_time * u_src_speed * 0.5;
        float sway = sin(t) * 0.15;
        float minDist = 100.0;
        // Trunk
        vec2 base = vec2(0.5, 0.05);
        vec2 top = vec2(0.5, 0.35);
        minDist = min(minDist, lineSeg(uv, base, top));
        // Level 1 branches
        float len = 0.2;
        for (int b = 0; b < nBranch; b++) {
            float angle = brAngle * (float(b) - float(nBranch-1)*0.5) + sway;
            vec2 end1 = top + vec2(sin(angle), cos(angle)) * len;
            minDist = min(minDist, lineSeg(uv, top, end1));
            // Level 2
            if (maxDepth >= 2) {
                float len2 = len * 0.65;
                for (int b2 = 0; b2 < nBranch; b2++) {
                    float a2 = angle + brAngle * 0.7 * (float(b2) - float(nBranch-1)*0.5) + sway * 0.5;
                    vec2 end2 = end1 + vec2(sin(a2), cos(a2)) * len2;
                    minDist = min(minDist, lineSeg(uv, end1, end2));
                    // Level 3
                    if (maxDepth >= 3) {
                        float len3 = len2 * 0.55;
                        for (int b3 = 0; b3 < min(nBranch, 3); b3++) {
                            float a3 = a2 + brAngle * 0.5 * (float(b3) - 1.0) + sway * 0.3;
                            vec2 end3 = end2 + vec2(sin(a3), cos(a3)) * len3;
                            minDist = min(minDist, lineSeg(uv, end2, end3));
                        }
                    }
                }
            }
        }
        float mask = smoothstep(thick, 0.0, minDist);
        float hue = u_src_color_shift;
        vec3 col = hue < 0.01 ? vec3(1.0) : 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * mask * (0.8 + u_rms * 0.4), 1.0);
    }
)";

// 9. Laser Scan — sweeping beam lines
inline const char* sourceLaserScan = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_beams;
    uniform float u_src_speed;
    uniform float u_src_thickness;
    uniform float u_src_spread;
    uniform float u_src_color_shift;
    uniform float u_rms;
    void main() {
        vec2 p = v_texCoord - 0.5;
        float aspect = u_resolution.x / u_resolution.y;
        p.x *= aspect;
        int n = int(u_src_beams * 8.0) + 1;
        float thick = 0.003 + u_src_thickness * 0.025;
        float t = u_time * (0.5 + u_src_speed * 3.0);
        float spread = u_src_spread * 0.5;
        float mask = 0.0;
        for (int i = 0; i < n; i++) {
            float fi = float(i);
            float angle = t + fi * 6.28318 / float(n);
            vec2 dir = vec2(cos(angle), sin(angle));
            float scanY = sin(t * 2.0 + fi * 1.7) * spread;
            // Fan beam from bottom center
            vec2 origin = vec2(0.0, -0.4 + scanY);
            vec2 toP = p - origin;
            float proj = dot(toP, dir);
            float perp = abs(toP.x * dir.y - toP.y * dir.x);
            if (proj > 0.0) {
                float fade = 1.0 / (1.0 + proj * 2.0);
                mask += smoothstep(thick, 0.0, perp) * fade;
            }
        }
        mask = clamp(mask, 0.0, 1.0);
        float hue = u_src_color_shift;
        vec3 col = hue < 0.01 ? vec3(1.0) : 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * mask * (0.8 + u_rms * 0.4), 1.0);
    }
)";

// 10. Moiré Lines — overlapping line grids creating interference patterns
inline const char* sourceMoireLines = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_density;
    uniform float u_src_angle_offset;
    uniform float u_src_speed;
    uniform float u_src_layers;
    uniform float u_src_color_shift;
    uniform float u_rms;
    void main() {
        vec2 uv = v_texCoord;
        int nLayers = int(u_src_layers * 4.0) + 2;
        float freq = 10.0 + u_src_density * 60.0;
        float t = u_time * u_src_speed * 0.5;
        float angleOff = u_src_angle_offset * 1.0;
        float val = 1.0;
        for (int i = 0; i < nLayers; i++) {
            float angle = float(i) * 3.14159 / float(nLayers) + t * (0.1 + float(i) * 0.05) + angleOff;
            float c = cos(angle), s = sin(angle);
            float proj = uv.x * c + uv.y * s;
            float stripe = sin(proj * freq * 6.28318) * 0.5 + 0.5;
            val *= stripe;
        }
        float mask = val;
        float hue = u_src_color_shift;
        vec3 col = hue < 0.01 ? vec3(1.0) : 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * mask * (0.8 + u_rms * 0.4), 1.0);
    }
)";

// ============================================================
// FRACTAL SOURCES — Tier 1 (fragment shader native)
// ============================================================
// UX OVERHAUL: All 2D fractals now have:
//   - Location presets (u_src_location cycles through interesting coordinates)
//   - Color palettes (u_src_palette selects from 8 cosine palettes)
//   - Iteration auto-scaling with manual zoom depth
//   - Gentler parameter sensitivity (quadratic curves)
//   - Dive speed locks center to interesting target
// ============================================================

// Julia Set — animate c for morphing fractals, with preset c-values and palettes
inline const char* sourceJuliaSet = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_cx;
    uniform float u_src_cy;
    uniform float u_src_zoom;
    uniform float u_src_iterations;
    uniform float u_src_color_speed;
    uniform float u_src_color_shift;
    uniform float u_src_dive_speed;
    uniform float u_src_location;
    uniform float u_src_palette;
    uniform float u_rms;
    uniform float u_beatPhase;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        // === ZOOM: continuous, no wrap. Dive speed = auto-zoom + c-morph ===
        float diveRate = u_src_dive_speed * u_src_dive_speed * 2.0;
        float zoomExp = u_src_zoom * 6.0 + u_time * diveRate * 0.12;
        zoomExp = min(zoomExp, 6.0); // float precision limit
        float zoom = exp(zoomExp);
        uv /= zoom;

        // 10 beautiful Julia c-values near the Mandelbrot boundary
        vec2 presets[10];
        presets[0] = vec2(-0.7, 0.27015);      // Classic spiral
        presets[1] = vec2(-0.4, 0.6);           // Dragon
        presets[2] = vec2(0.355, 0.355);        // Symmetric dendrite
        presets[3] = vec2(-0.54, 0.54);         // Snowflake
        presets[4] = vec2(-0.123, 0.745);       // Douady rabbit
        presets[5] = vec2(0.0, 0.8);            // Dendrite
        presets[6] = vec2(-0.75, 0.0);          // San Marco / Basilica
        presets[7] = vec2(-0.77, 0.22);         // Galaxy spiral
        presets[8] = vec2(0.285, 0.01);         // Spiral nebula
        presets[9] = vec2(-0.8, 0.156);         // Organic tendrils

        // === C-VALUE: Location presets override C Real/C Imaginary ===
        // Dive speed morphs through presets (Julia "dive" = c-value animation)
        float cx, cy;
        float diveRate = u_src_dive_speed * u_src_dive_speed * 2.0;
        float locIdx = u_src_location * 9.0;

        if (diveRate > 0.001) {
            // Dive: smoothly morph c through all presets
            float phase = u_time * diveRate * 0.3;
            float idx = mod(phase, 10.0);
            int i0 = int(idx);
            int i1 = int(mod(idx + 1.0, 10.0));
            float frac = fract(idx);
            frac = frac * frac * (3.0 - 2.0 * frac);
            cx = mix(presets[i0].x, presets[i1].x, frac);
            cy = mix(presets[i0].y, presets[i1].y, frac);
        } else if (locIdx > 0.5) {
            // Location preset selected
            int li = int(locIdx);
            float frac = locIdx - float(li);
            int li2 = min(li + 1, 9);
            frac = frac * frac * (3.0 - 2.0 * frac);
            cx = mix(presets[min(li, 9)].x, presets[min(li2, 9)].x, frac);
            cy = mix(presets[min(li, 9)].y, presets[min(li2, 9)].y, frac);
        } else {
            // Manual c-value: cx [0,1] -> [-1, 0.5], cy [0,1] -> [-1, 1]
            cx = u_src_cx * 1.5 - 1.0;
            cy = u_src_cy * 2.0 - 1.0;
            // Subtle audio modulation
            cx += sin(u_time * 0.2) * 0.02 * u_rms;
            cy += cos(u_time * 0.15) * 0.02 * u_rms;
        }

        vec2 z = uv;
        vec2 c = vec2(cx, cy);

        // Auto-scale iterations with zoom depth
        int baseIter = int(u_src_iterations * 200.0) + 20;
        int zoomIter = int(zoomExp * 15.0);
        int maxIter = min(baseIter + zoomIter, 400);

        int i;
        for (i = 0; i < 400; i++) {
            if (i >= maxIter) break;
            z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
            if (dot(z,z) > 4.0) break;
        }
        if (i >= maxIter) { fragColor = vec4(0,0,0,1); return; }
        float si = max(float(i) - log2(log2(dot(z,z))) + 4.0, 0.0);
        float t = si * (0.02 + u_src_color_speed * 0.1) + u_src_color_shift * 6.28;
        int palIdx = int(u_src_palette * 7.0);
        vec3 col = fracPalette(t, palIdx);
        col *= 0.85 + u_rms * 0.3;
        fragColor = vec4(col, 1.0);
    }
)";

// Burning Ship — aggressive flame-like fractal, with location presets and palettes
inline const char* sourceBurningShip = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_center_x;
    uniform float u_src_center_y;
    uniform float u_src_zoom;
    uniform float u_src_iterations;
    uniform float u_src_color_speed;
    uniform float u_src_color_shift;
    uniform float u_src_dive_speed;
    uniform float u_src_location;
    uniform float u_src_palette;
    uniform float u_rms;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        // === CENTER: Location presets override Center X/Y ===
        vec2 locations[6];
        locations[0] = vec2(-1.762, -0.028);    // Smokestack
        locations[1] = vec2(-1.755, 0.02);      // Ship hull detail
        locations[2] = vec2(-0.4, -0.6);        // Main body center
        locations[3] = vec2(-1.94, -0.0005);    // Far antenna mini-ship
        locations[4] = vec2(-1.781, -0.0);      // Mast spiral
        locations[5] = vec2(-0.15, -1.03);      // South continent

        vec2 center;
        float locIdx = u_src_location * 5.0;
        if (locIdx > 0.5) {
            int li = min(int(locIdx), 5);
            center = locations[li];
        } else {
            // Manual center: X [0,1] -> [-2.0, 0.5], Y [0,1] -> [-1.5, 0.5]
            // At default 0.5/0.5 -> (-0.75, -0.5) = center of main ship body
            center = vec2(u_src_center_x * 2.5 - 2.0,
                          u_src_center_y * 2.0 - 1.5);
        }

        // === ZOOM: continuous, no wrap. Dive speed = auto-zoom clamped at float precision limit ===
        float diveRate = u_src_dive_speed * u_src_dive_speed * 2.0;
        float zoomExp = u_src_zoom * 7.0 + u_time * diveRate * 0.12;
        zoomExp = min(zoomExp, 7.0);
        float zoom = exp(zoomExp);

        uv = uv / zoom + center;

        vec2 z = vec2(0.0);
        vec2 c = uv;
        int baseIter = int(u_src_iterations * 200.0) + 20;
        int zoomIter = int(zoomExp * 25.0);
        int maxIter = min(baseIter + zoomIter, 400);
        int i;
        for (i = 0; i < 400; i++) {
            if (i >= maxIter) break;
            z = abs(z);
            z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
            if (dot(z,z) > 4.0) break;
        }
        if (i >= maxIter) { fragColor = vec4(0,0,0,1); return; }
        float si = max(float(i) - log2(log2(dot(z,z))) + 4.0, 0.0);
        float t = si * (0.02 + u_src_color_speed * 0.1) + u_src_color_shift * 6.28;
        int palIdx = int(u_src_palette * 7.0);
        vec3 col = fracPalette(t, palIdx);
        col *= 0.85 + u_rms * 0.3;
        fragColor = vec4(col, 1.0);
    }
)";

// Newton Fractal — psychedelic basins of attraction, with palettes and presets
inline const char* sourceNewtonFractal = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_power;
    uniform float u_src_zoom;
    uniform float u_src_damping;
    uniform float u_src_color_shift;
    uniform float u_src_dive_speed;
    uniform float u_src_palette;
    uniform float u_rms;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    vec2 cmul(vec2 a, vec2 b) { return vec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x); }
    vec2 cdiv(vec2 a, vec2 b) { return cmul(a, vec2(b.x,-b.y)) / dot(b,b); }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        // === ZOOM: continuous, no wrap. Dive speed = auto-zoom clamped ===
        float diveRate = u_src_dive_speed * u_src_dive_speed * 2.0;
        float zoomExp = u_src_zoom * 5.0 + u_time * diveRate * 0.12;
        zoomExp = min(zoomExp, 5.0);
        float zoom = exp(zoomExp);

        // Center: always target boundary between first two roots (0.5, 0.866)
        // The Newton fractal is most interesting at root boundaries
        vec2 nCenter = vec2(0.5, 0.866) * (diveRate > 0.001 ? 1.0 : 0.0);
        uv = uv / zoom + nCenter;
        // Power: 3 to 8 with quadratic sensitivity
        float rawPow = u_src_power;
        int n = int(rawPow * rawPow * 5.0) + 3;
        float damp = 0.5 + u_src_damping * 1.0;
        vec2 z = uv;
        int maxIter = 50 + int(zoomExp * 6.0);
        maxIter = min(maxIter, 120);
        int i;
        float minDist = 1e10;
        int closestRoot = 0;
        for (i = 0; i < 120; i++) {
            if (i >= maxIter) break;
            float r = length(z);
            if (r < 0.0001) break;
            float theta = atan(z.y, z.x);
            vec2 zn = pow(r, float(n)) * vec2(cos(float(n)*theta), sin(float(n)*theta));
            vec2 zn1 = pow(r, float(n-1)) * vec2(cos(float(n-1)*theta), sin(float(n-1)*theta));
            vec2 fz = zn - vec2(1.0, 0.0);
            vec2 fpz = float(n) * zn1;
            z -= damp * cdiv(fz, fpz);
            for (int k = 0; k < 8; k++) {
                if (k >= n) break;
                float ra = 6.28318 * float(k) / float(n);
                vec2 root = vec2(cos(ra), sin(ra));
                float d = length(z - root);
                if (d < minDist) { minDist = d; closestRoot = k; }
            }
            if (minDist < 0.001) break;
        }
        float hue = float(closestRoot) / float(n) + u_src_color_shift;
        float brightness = 1.0 - float(i) / float(maxIter);
        int palIdx = int(u_src_palette * 7.0);
        vec3 col = fracPalette(hue, palIdx);
        col *= brightness * (0.8 + u_rms * 0.4);
        fragColor = vec4(col, 1.0);
    }
)";

// Sierpinski — triangle and carpet modes, with palettes and zoom auto-iter
inline const char* sourceSierpinski = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_mode;
    uniform float u_src_zoom;
    uniform float u_src_iterations;
    uniform float u_src_rotation;
    uniform float u_src_color_shift;
    uniform float u_src_dive_speed;
    uniform float u_src_palette;
    uniform float u_rms;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        // Zoom: continuous with dive speed, no wrap
        float diveRate = u_src_dive_speed * u_src_dive_speed * 2.0;
        float zoomExp = u_src_zoom * 5.0 + u_time * diveRate * 0.12;
        zoomExp = min(zoomExp, 5.0);
        float zoomPow = exp(zoomExp * 0.5);
        float rot = u_time * (u_src_rotation - 0.5) * 1.0;
        float c = cos(rot), s = sin(rot);
        uv = vec2(uv.x*c - uv.y*s, uv.x*s + uv.y*c);
        uv /= zoomPow;
        // Auto-scale iterations with zoom
        int baseIter = int(u_src_iterations * 12.0) + 3;
        int zoomIter = int(zoomExp * 1.5);
        int maxIter = min(baseIter + zoomIter, 20);
        bool isCarpet = u_src_mode > 0.5;
        float val = 1.0;
        float iterFrac = 0.0;
        if (isCarpet) {
            // Sierpinski Carpet: remove center cell at each scale
            vec2 p = fract(uv * 0.5 + 0.5);
            for (int i = 0; i < 20; i++) {
                if (i >= maxIter) break;
                p *= 3.0;
                vec2 cell = floor(p);
                if (cell.x == 1.0 && cell.y == 1.0) { val = 0.0; iterFrac = float(i)/float(maxIter); break; }
                p = fract(p);
            }
        } else {
            // Sierpinski Triangle: modular arithmetic approach
            // A point is in the gasket if at every scale, it's NOT in the upper-right quadrant
            vec2 p = fract(uv * 0.5 + 0.5);
            for (int i = 0; i < 20; i++) {
                if (i >= maxIter) break;
                p *= 2.0;
                vec2 cell = floor(p);
                // If both coordinates are >= 1 (upper-right), it's a hole
                if (cell.x >= 1.0 && cell.y >= 1.0) { val = 0.0; iterFrac = float(i)/float(maxIter); break; }
                p = fract(p);
            }
        }
        int palIdx = int(u_src_palette * 7.0);
        float hue = u_src_color_shift + iterFrac;
        vec3 col;
        if (u_src_palette < 0.01 && u_src_color_shift < 0.01) col = vec3(val);
        else col = val * fracPalette(hue, palIdx);
        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)";

// Apollonian Gasket — circle packing fractal, with palettes
inline const char* sourceApollonian = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_zoom;
    uniform float u_src_iterations;
    uniform float u_src_rotation;
    uniform float u_src_color_shift;
    uniform float u_src_dive_speed;
    uniform float u_src_palette;
    uniform float u_rms;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        // Zoom: continuous with dive speed, no wrap
        float diveRate = u_src_dive_speed * u_src_dive_speed * 2.0;
        float zoomExp = u_src_zoom * 5.0 + u_time * diveRate * 0.12;
        zoomExp = min(zoomExp, 5.0);
        float zoom = exp(zoomExp * 0.5);
        float rot = u_time * (u_src_rotation - 0.5) * 0.5;
        float ca = cos(rot), sa = sin(rot);
        uv = vec2(uv.x*ca - uv.y*sa, uv.x*sa + uv.y*ca);
        uv /= zoom;
        vec2 p = uv;
        float minDist = 1e10;
        float iterColor = 0.0;
        int baseIter = int(u_src_iterations * 20.0) + 8;
        int zoomIter = int(zoomExp * 2.0);
        int maxIter = min(baseIter + zoomIter, 40);
        for (int i = 0; i < 40; i++) {
            if (i >= maxIter) break;
            p = abs(p);
            if (p.x < p.y) p = p.yx;
            p -= vec2(1.0, 1.0);
            if (p.y < -0.5 * p.x) p = vec2(p.x - p.y, p.y + p.x) * 0.7071;
            float d = length(p);
            if (d < minDist) { minDist = d; iterColor = float(i); }
            p *= 2.0;
            p -= vec2(1.5, 0.5);
        }
        // Brighter, more contrasty coloring
        float glow = 1.0 / (1.0 + minDist * minDist * 8.0);
        float hue = u_src_color_shift + iterColor * 0.1 + log(minDist + 0.01) * 0.3;
        int palIdx = int(u_src_palette * 7.0);
        vec3 col = fracPalette(hue, palIdx);
        col *= glow * (0.8 + u_rms * 0.4);
        fragColor = vec4(col, 1.0);
    }
)";

// ============================================================
// 3D FRACTAL SOURCES — Ray Marched
// All 3D fractals now have: palette, cross-section slice, glow,
// orbit trap coloring, lighting, camera orbit, dive speed
// ============================================================

// Mandelbulb — 3D Mandelbrot analog, with power morphing, cross-section, palettes
inline const char* sourceMandelbulb = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_power;
    uniform float u_src_iterations;
    uniform float u_src_rotation_x;
    uniform float u_src_rotation_y;
    uniform float u_src_detail;
    uniform float u_src_color_shift;
    uniform float u_src_slice;
    uniform float u_src_glow;
    uniform float u_src_palette;
    uniform float u_src_zoom;
    uniform float u_src_speed;
    uniform float u_src_trail_dist;
    uniform float u_src_trail_fade;
    uniform float u_src_slice_count;
    uniform float u_src_slice_dist;
    uniform float u_src_feedback;
    uniform float u_rms;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    mat3 rotY(float a) { float c=cos(a),s=sin(a); return mat3(c,0,s, 0,1,0, -s,0,c); }
    mat3 rotX(float a) { float c=cos(a),s=sin(a); return mat3(1,0,0, 0,c,-s, 0,s,c); }

    float mandelbulbDE(vec3 pos, float power, out float trap) {
        vec3 z = pos;
        float dr = 1.0, r = 0.0;
        trap = 1e10;
        for (int i = 0; i < 12; i++) {
            r = length(z);
            if (r > 2.0) break;
            trap = min(trap, length(z));
            float theta = acos(clamp(z.z / r, -1.0, 1.0));
            float phi = atan(z.y, z.x);
            dr = pow(r, power - 1.0) * power * dr + 1.0;
            float zr = pow(r, power);
            theta *= power; phi *= power;
            z = zr * vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
            z += pos;
        }
        return 0.5 * log(r) * r / dr;
    }

    vec3 calcNormal(vec3 p, float pw) {
        vec2 e = vec2(0.001, 0.0);
        float dummy;
        return normalize(vec3(
            mandelbulbDE(p+e.xyy,pw,dummy) - mandelbulbDE(p-e.xyy,pw,dummy),
            mandelbulbDE(p+e.yxy,pw,dummy) - mandelbulbDE(p-e.yxy,pw,dummy),
            mandelbulbDE(p+e.yyx,pw,dummy) - mandelbulbDE(p-e.yyx,pw,dummy)));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        float rawPow = u_src_power;
        float power = 2.0 + rawPow * rawPow * 14.0;

        // Auto-rotation: 0.5 = stopped, <0.5 = reverse, >0.5 = forward
        float autoSpeed = (u_src_speed - 0.5) * 2.0;
        float angleX = u_src_rotation_x * 6.28318 + u_time * autoSpeed;
        float angleY = u_src_rotation_y * 6.28318 + u_time * autoSpeed * 0.7;
        mat3 rot = rotY(angleY) * rotX(angleX);

        // Camera distance: zoom [0,1] -> [5.0, 0.3]
        float camDist = mix(5.0, 0.3, u_src_zoom);
        vec3 ro = rot * vec3(0, 0, camDist);
        vec3 target = vec3(0);
        vec3 fwd = normalize(target - ro);
        vec3 right = normalize(cross(fwd, vec3(0,1,0)));
        vec3 up = cross(right, fwd);
        vec3 rd = normalize(fwd + uv.x * right + uv.y * up);

        // Cross-section: single or multi-slice
        float sliceZ = (u_src_slice - 0.5) * 3.0;
        bool useSlice = abs(u_src_slice - 0.5) > 0.01;
        int sliceCount = int(u_src_slice_count * 4.0) + 1;
        float sliceSpacing = 0.1 + u_src_slice_dist * 0.8;

        float glowAmt = u_src_glow * u_src_glow * 4.0;
        int numTrails = int(u_src_trail_dist * 5.0);
        float trailDecay = 0.3 + u_src_trail_fade * 0.5;

        float t = 0.0;
        float glow = 0.0;
        bool hit = false;
        float trapVal = 0.0;
        for (int i = 0; i < 80; i++) {
            vec3 p = ro + rd * t;
            float trap;
            float d = mandelbulbDE(p, power, trap);
            if (useSlice) {
                if (sliceCount <= 1) {
                    d = max(d, abs(p.z - sliceZ) - 0.02);
                } else {
                    float md = 1e10;
                    for (int s = 0; s < 5; s++) {
                        if (s >= sliceCount) break;
                        float z = sliceZ + float(s - sliceCount/2) * sliceSpacing;
                        md = min(md, abs(p.z - z) - 0.02);
                    }
                    d = max(d, md);
                }
            }
            float glowBase = 1.0 / (1.0 + d * d * 500.0);
            float trailGlow = 0.0;
            if (d < 0.1 && numTrails > 0) {
                for (int tr = 0; tr < 5; tr++) {
                    if (tr >= numTrails) break;
                    float offset = float(tr + 1) * 0.02;
                    vec3 tp = p + rd * offset;
                    float tdmy;
                    float td = mandelbulbDE(tp, power, tdmy);
                    float tg = 1.0 / (1.0 + td * td * 500.0);
                    trailGlow += tg * pow(trailDecay, float(tr + 1));
                }
            }
            glow += glowBase + trailGlow * 0.3;
            // Feedback: modulate glow with periodic rings for echo/feedback look
            if (u_src_feedback > 0.01 && d < 0.5) {
                float fbRings = sin(d * u_src_feedback * 100.0) * 0.5 + 0.5;
                glow += fbRings * u_src_feedback * glowBase * 3.0;
            }
            if (d < 0.001) { hit = true; trapVal = trap; break; }
            t += d;
            if (t > 10.0) break;
        }
        vec3 col = vec3(0.0);
        if (hit) {
            vec3 p = ro + rd * t;
            vec3 n = calcNormal(p, power);
            vec3 light = normalize(vec3(1, 2, 3));
            float diff = max(dot(n, light), 0.0) * 0.7 + 0.3;
            float hue = u_src_color_shift + trapVal * 2.0;
            int palIdx = int(u_src_palette * 7.0);
            col = diff * fracPalette(hue, palIdx);
        }
        col += vec3(0.1, 0.15, 0.3) * glow * 0.015 * (1.0 + glowAmt);

        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)";

// Menger Sponge — 3D Sierpinski cube, with palettes and cross-section
inline const char* sourceMengerSponge = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_iterations;
    uniform float u_src_rotation_x;
    uniform float u_src_rotation_y;
    uniform float u_src_twist;
    uniform float u_src_color_shift;
    uniform float u_src_slice;
    uniform float u_src_palette;
    uniform float u_src_zoom;
    uniform float u_src_speed;
    uniform float u_src_trail_dist;
    uniform float u_src_trail_fade;
    uniform float u_src_slice_count;
    uniform float u_src_slice_dist;
    uniform float u_src_feedback;
    uniform float u_rms;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    mat3 rotY(float a) { float c=cos(a),s=sin(a); return mat3(c,0,s, 0,1,0, -s,0,c); }
    mat3 rotX(float a) { float c=cos(a),s=sin(a); return mat3(1,0,0, 0,c,-s, 0,s,c); }

    float sdBox(vec3 p, vec3 b) {
        vec3 q = abs(p) - b;
        return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
    }

    float mengerDE(vec3 p, int iters, float twistAmt) {
        float d = sdBox(p, vec3(1.0));
        float s = 1.0;
        for (int i = 0; i < 8; i++) {
            if (i >= iters) break;
            if (twistAmt > 0.01) {
                float ta = twistAmt * float(i) * 0.3;
                float tc = cos(ta), ts = sin(ta);
                p.xz = vec2(p.x*tc - p.z*ts, p.x*ts + p.z*tc);
            }
            vec3 a = mod(p * s, 2.0) - 1.0;
            s *= 3.0;
            vec3 r = abs(1.0 - 3.0 * abs(a));
            float da = max(r.x, r.y);
            float db = max(r.y, r.z);
            float dc = max(r.z, r.x);
            float c = (min(da, min(db, dc)) - 1.0) / s;
            d = max(d, c);
        }
        return d;
    }

    vec3 calcNormal(vec3 p, int it, float tw) {
        vec2 e = vec2(0.001, 0.0);
        return normalize(vec3(
            mengerDE(p+e.xyy,it,tw) - mengerDE(p-e.xyy,it,tw),
            mengerDE(p+e.yxy,it,tw) - mengerDE(p-e.yxy,it,tw),
            mengerDE(p+e.yyx,it,tw) - mengerDE(p-e.yyx,it,tw)));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        int iters = int(u_src_iterations * 5.0) + 2;
        float twistAmt = u_src_twist * 2.0;
        float autoSpeed = (u_src_speed - 0.5) * 2.0;
        float rx = u_src_rotation_x * 6.28318 + u_time * autoSpeed;
        float ry = u_src_rotation_y * 6.28318 + u_time * autoSpeed * 0.7;
        mat3 rot = rotY(ry) * rotX(rx);
        float camDist = mix(5.0, 0.3, u_src_zoom);
        vec3 ro = rot * vec3(0, 0, camDist);
        vec3 fwd = normalize(-ro);
        vec3 right = normalize(cross(fwd, vec3(0,1,0)));
        vec3 up = cross(right, fwd);
        vec3 rd = normalize(fwd + uv.x * right + uv.y * up);

        float sliceZ = (u_src_slice - 0.5) * 3.0;
        bool useSlice = abs(u_src_slice - 0.5) > 0.01;
        int sliceCount = int(u_src_slice_count * 4.0) + 1;
        float sliceSpacing = 0.1 + u_src_slice_dist * 0.8;

        int numTrails = int(u_src_trail_dist * 5.0);
        float trailDecay = 0.3 + u_src_trail_fade * 0.5;

        float t = 0.0;
        float glow = 0.0;
        bool hit = false;
        for (int i = 0; i < 80; i++) {
            vec3 p = ro + rd * t;
            float d = mengerDE(p, iters, twistAmt);
            if (useSlice) {
                if (sliceCount <= 1) {
                    d = max(d, abs(p.z - sliceZ) - 0.02);
                } else {
                    float md = 1e10;
                    for (int s = 0; s < 5; s++) {
                        if (s >= sliceCount) break;
                        float z = sliceZ + float(s - sliceCount/2) * sliceSpacing;
                        md = min(md, abs(p.z - z) - 0.02);
                    }
                    d = max(d, md);
                }
            }
            float glowBase = 1.0 / (1.0 + d * d * 200.0);
            float trailGlow = 0.0;
            if (d < 0.1 && numTrails > 0) {
                for (int tr = 0; tr < 5; tr++) {
                    if (tr >= numTrails) break;
                    float offset = float(tr + 1) * 0.02;
                    vec3 tp = p + rd * offset;
                    float td = mengerDE(tp, iters, twistAmt);
                    float tg = 1.0 / (1.0 + td * td * 200.0);
                    trailGlow += tg * pow(trailDecay, float(tr + 1));
                }
            }
            glow += glowBase + trailGlow * 0.3;
            if (u_src_feedback > 0.01 && d < 0.5) {
                float fbRings = sin(d * u_src_feedback * 100.0) * 0.5 + 0.5;
                glow += fbRings * u_src_feedback * glowBase * 3.0;
            }
            if (d < 0.001) { hit = true; break; }
            t += d;
            if (t > 10.0) break;
        }
        vec3 col = vec3(0.0);
        if (hit) {
            vec3 p = ro + rd * t;
            vec3 n = calcNormal(p, iters, twistAmt);
            vec3 light = normalize(vec3(1, 2, 3));
            float diff = max(dot(n, light), 0.0) * 0.7 + 0.3;
            float hue = u_src_color_shift + abs(n.x) * 0.2 + abs(n.y) * 0.3;
            int palIdx = int(u_src_palette * 7.0);
            col = diff * fracPalette(hue, palIdx);
        }
        col += vec3(0.15, 0.1, 0.25) * glow * 0.01;

        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)";

// KIFS — Kaleidoscopic IFS with palettes and cross-section
inline const char* sourceKIFS = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_scale;
    uniform float u_src_iterations;
    uniform float u_src_fold_type;
    uniform float u_src_rotation_x;
    uniform float u_src_rotation_y;
    uniform float u_src_offset;
    uniform float u_src_color_shift;
    uniform float u_src_slice;
    uniform float u_src_palette;
    uniform float u_src_zoom;
    uniform float u_src_speed;
    uniform float u_src_trail_dist;
    uniform float u_src_trail_fade;
    uniform float u_src_slice_count;
    uniform float u_src_slice_dist;
    uniform float u_src_feedback;
    uniform float u_rms;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    mat3 rotY(float a) { float c=cos(a),s=sin(a); return mat3(c,0,s, 0,1,0, -s,0,c); }
    mat3 rotX(float a) { float c=cos(a),s=sin(a); return mat3(1,0,0, 0,c,-s, 0,s,c); }

    float kifsDE(vec3 p, float sc, int iters, float foldType, float off) {
        vec3 offset = vec3(1.0) * off * 2.0;
        float iterRot = u_time * 0.1;
        for (int i = 0; i < 15; i++) {
            if (i >= iters) break;
            p = abs(p);
            if (foldType < 0.33) {
                if (p.x - p.y < 0.0) p.xy = p.yx;
                if (p.x - p.z < 0.0) p.xz = p.zx;
                if (p.y - p.z < 0.0) p.yz = p.zy;
            } else if (foldType < 0.67) {
                if (p.x - p.y < 0.0) p.xy = p.yx;
                if (p.x - p.z < 0.0) p.xz = p.zx;
                float a = iterRot + float(i) * 0.5;
                float rc = cos(a), rs = sin(a);
                p.yz = vec2(p.y*rc - p.z*rs, p.y*rs + p.z*rc);
            } else {
                if (p.x + p.y < 0.0) { float t = -p.y; p.y = -p.x; p.x = t; }
                if (p.x + p.z < 0.0) { float t = -p.z; p.z = -p.x; p.x = t; }
                if (p.y + p.z < 0.0) { float t = -p.z; p.z = -p.y; p.y = t; }
            }
            p = sc * p - offset * (sc - 1.0);
        }
        return length(p) * pow(sc, -float(iters));
    }

    vec3 calcNormal(vec3 p, float sc, int it, float ft, float off) {
        vec2 e = vec2(0.001, 0.0);
        return normalize(vec3(
            kifsDE(p+e.xyy,sc,it,ft,off) - kifsDE(p-e.xyy,sc,it,ft,off),
            kifsDE(p+e.yxy,sc,it,ft,off) - kifsDE(p-e.yxy,sc,it,ft,off),
            kifsDE(p+e.yyx,sc,it,ft,off) - kifsDE(p-e.yyx,sc,it,ft,off)));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        float sc = 1.5 + u_src_scale * 2.0;
        int iters = int(u_src_iterations * 10.0) + 3;
        float foldType = u_src_fold_type;
        float off = u_src_offset;
        float autoSpeed = (u_src_speed - 0.5) * 2.0;
        float rx = u_src_rotation_x * 6.28318 + u_time * autoSpeed;
        float ry = u_src_rotation_y * 6.28318 + u_time * autoSpeed * 0.7;
        mat3 rot = rotY(ry) * rotX(rx);
        float camDist = mix(5.0, 0.3, u_src_zoom);
        vec3 ro = rot * vec3(0, 0, camDist);
        vec3 fwd = normalize(-ro);
        vec3 right = normalize(cross(fwd, vec3(0,1,0)));
        vec3 up = cross(right, fwd);
        vec3 rd = normalize(fwd + uv.x * right + uv.y * up);

        float sliceZ = (u_src_slice - 0.5) * 4.0;
        bool useSlice = abs(u_src_slice - 0.5) > 0.01;
        int sliceCount = int(u_src_slice_count * 4.0) + 1;
        float sliceSpacing = 0.1 + u_src_slice_dist * 0.8;

        int numTrails = int(u_src_trail_dist * 5.0);
        float trailDecay = 0.3 + u_src_trail_fade * 0.5;

        float t = 0.0;
        float glow = 0.0;
        bool hit = false;
        for (int i = 0; i < 80; i++) {
            vec3 p = ro + rd * t;
            float d = kifsDE(p, sc, iters, foldType, off);
            if (useSlice) {
                if (sliceCount <= 1) {
                    d = max(d, abs(p.z - sliceZ) - 0.02);
                } else {
                    float md = 1e10;
                    for (int s = 0; s < 5; s++) {
                        if (s >= sliceCount) break;
                        float z = sliceZ + float(s - sliceCount/2) * sliceSpacing;
                        md = min(md, abs(p.z - z) - 0.02);
                    }
                    d = max(d, md);
                }
            }
            float glowBase = 1.0 / (1.0 + d * d * 300.0);
            float trailGlow = 0.0;
            if (d < 0.1 && numTrails > 0) {
                for (int tr = 0; tr < 5; tr++) {
                    if (tr >= numTrails) break;
                    float offset = float(tr + 1) * 0.02;
                    vec3 tp = p + rd * offset;
                    float td = kifsDE(tp, sc, iters, foldType, off);
                    float tg = 1.0 / (1.0 + td * td * 300.0);
                    trailGlow += tg * pow(trailDecay, float(tr + 1));
                }
            }
            glow += glowBase + trailGlow * 0.3;
            if (u_src_feedback > 0.01 && d < 0.5) {
                float fbRings = sin(d * u_src_feedback * 100.0) * 0.5 + 0.5;
                glow += fbRings * u_src_feedback * glowBase * 3.0;
            }
            if (d < 0.001) { hit = true; break; }
            t += d;
            if (t > 10.0) break;
        }
        vec3 col = vec3(0.0);
        if (hit) {
            vec3 p = ro + rd * t;
            vec3 n = calcNormal(p, sc, iters, foldType, off);
            vec3 light = normalize(vec3(1, 2, 3));
            float diff = max(dot(n, light), 0.0) * 0.7 + 0.3;
            float hue = u_src_color_shift + dot(abs(n), vec3(0.3, 0.5, 0.2));
            int palIdx = int(u_src_palette * 7.0);
            col = diff * fracPalette(hue, palIdx);
        }
        col += vec3(0.1, 0.2, 0.3) * glow * 0.012;

        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)";

// ============================================================
// NEW 3D FRACTAL SOURCES
// ============================================================

// Julia Set 3D — Quaternion Julia set, ray marched
inline const char* sourceJuliaSet3D = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_cx;
    uniform float u_src_cy;
    uniform float u_src_rotation_x;
    uniform float u_src_rotation_y;
    uniform float u_src_iterations;
    uniform float u_src_color_shift;
    uniform float u_src_slice;
    uniform float u_src_glow;
    uniform float u_src_palette;
    uniform float u_src_location;
    uniform float u_src_zoom;
    uniform float u_src_speed;
    uniform float u_src_trail_dist;
    uniform float u_src_trail_fade;
    uniform float u_src_slice_count;
    uniform float u_src_slice_dist;
    uniform float u_src_feedback;
    uniform float u_rms;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    mat3 rotY(float a) { float c=cos(a),s=sin(a); return mat3(c,0,s, 0,1,0, -s,0,c); }
    mat3 rotX(float a) { float c=cos(a),s=sin(a); return mat3(1,0,0, 0,c,-s, 0,s,c); }

    // Quaternion multiply: q = (x, y, z, w) = xi + yj + zk + w
    vec4 qmul(vec4 a, vec4 b) {
        return vec4(
            a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
            a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
            a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w,
            a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z);
    }

    float julia3DDE(vec3 pos, vec4 c, out float trap) {
        vec4 z = vec4(pos, 0.0);
        float dz = 1.0;
        trap = 1e10;
        for (int i = 0; i < 10; i++) {
            dz = 2.0 * length(z) * dz;
            z = qmul(z, z) + c;
            trap = min(trap, length(z.xyz));
            if (dot(z, z) > 4.0) break;
        }
        float r = length(z);
        return 0.5 * r * log(r) / max(dz, 0.0001);
    }

    vec3 calcNormal(vec3 p, vec4 c) {
        vec2 e = vec2(0.001, 0.0);
        float dummy;
        return normalize(vec3(
            julia3DDE(p+e.xyy,c,dummy) - julia3DDE(p-e.xyy,c,dummy),
            julia3DDE(p+e.yxy,c,dummy) - julia3DDE(p-e.yxy,c,dummy),
            julia3DDE(p+e.yyx,c,dummy) - julia3DDE(p-e.yyx,c,dummy)));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        // Preset quaternion c-values
        vec4 presets[6];
        presets[0] = vec4(-0.2, 0.6, 0.2, 0.0);
        presets[1] = vec4(-0.213, -0.0410, -0.563, -0.560);
        presets[2] = vec4(-0.1, 0.6, 0.0, 0.0);
        presets[3] = vec4(-0.291, -0.399, 0.339, 0.437);
        presets[4] = vec4(0.185, 0.478, 0.125, -0.392);
        presets[5] = vec4(-0.4, 0.6, 0.2, -0.1);

        vec4 c;
        float locIdx = u_src_location * 5.0;
        if (locIdx > 0.1) {
            int li = int(locIdx);
            c = presets[min(li, 5)];
        } else {
            float rawCx = (u_src_cx - 0.5);
            float rawCy = (u_src_cy - 0.5);
            c = vec4(sign(rawCx)*rawCx*rawCx*4.0, sign(rawCy)*rawCy*rawCy*4.0, 0.0, 0.0);
        }

        float autoSpeed = (u_src_speed - 0.5) * 2.0;
        float rx = u_src_rotation_x * 6.28318 + u_time * autoSpeed;
        float ry = u_src_rotation_y * 6.28318 + u_time * autoSpeed * 0.7;
        mat3 rot = rotY(ry) * rotX(rx);
        float camDist = mix(5.0, 0.3, u_src_zoom);
        vec3 ro = rot * vec3(0, 0, camDist);
        vec3 fwd = normalize(-ro);
        vec3 right = normalize(cross(fwd, vec3(0,1,0)));
        vec3 up = cross(right, fwd);
        vec3 rd = normalize(fwd + uv.x * right + uv.y * up);

        float sliceZ = (u_src_slice - 0.5) * 3.0;
        bool useSlice = abs(u_src_slice - 0.5) > 0.01;
        int sliceCount = int(u_src_slice_count * 4.0) + 1;
        float sliceSpacing = 0.1 + u_src_slice_dist * 0.8;
        float glowAmt = u_src_glow * u_src_glow * 4.0;
        int numTrails = int(u_src_trail_dist * 5.0);
        float trailDecay = 0.3 + u_src_trail_fade * 0.5;

        float t = 0.0, glow = 0.0, trapVal = 0.0;
        bool hit = false;
        for (int i = 0; i < 80; i++) {
            vec3 p = ro + rd * t;
            float trap;
            float d = julia3DDE(p, c, trap);
            if (useSlice) {
                if (sliceCount <= 1) {
                    d = max(d, abs(p.z - sliceZ) - 0.02);
                } else {
                    float md = 1e10;
                    for (int s = 0; s < 5; s++) {
                        if (s >= sliceCount) break;
                        float z = sliceZ + float(s - sliceCount/2) * sliceSpacing;
                        md = min(md, abs(p.z - z) - 0.02);
                    }
                    d = max(d, md);
                }
            }
            float glowBase = 1.0 / (1.0 + d * d * 500.0);
            float trailGlow = 0.0;
            if (d < 0.1 && numTrails > 0) {
                for (int tr = 0; tr < 5; tr++) {
                    if (tr >= numTrails) break;
                    float offset = float(tr + 1) * 0.02;
                    vec3 tp = p + rd * offset;
                    float tdmy;
                    float td = julia3DDE(tp, c, tdmy);
                    float tg = 1.0 / (1.0 + td * td * 500.0);
                    trailGlow += tg * pow(trailDecay, float(tr + 1));
                }
            }
            glow += glowBase + trailGlow * 0.3;
            // Feedback: modulate glow with periodic rings for echo/feedback look
            if (u_src_feedback > 0.01 && d < 0.5) {
                float fbRings = sin(d * u_src_feedback * 100.0) * 0.5 + 0.5;
                glow += fbRings * u_src_feedback * glowBase * 3.0;
            }
            if (d < 0.001) { hit = true; trapVal = trap; break; }
            t += d;
            if (t > 10.0) break;
        }
        vec3 col = vec3(0.0);
        if (hit) {
            vec3 p = ro + rd * t;
            vec3 n = calcNormal(p, c);
            vec3 light = normalize(vec3(1, 2, 3));
            float diff = max(dot(n, light), 0.0) * 0.7 + 0.3;
            float hue = u_src_color_shift + trapVal * 2.0;
            int palIdx = int(u_src_palette * 7.0);
            col = diff * fracPalette(hue, palIdx);
        }
        col += vec3(0.15, 0.1, 0.3) * glow * 0.015 * (1.0 + glowAmt);

        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)";

// Burning Ship 3D — triplex algebra with abs(), ray marched
inline const char* sourceBurningShip3D = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_power;
    uniform float u_src_rotation_x;
    uniform float u_src_rotation_y;
    uniform float u_src_color_shift;
    uniform float u_src_slice;
    uniform float u_src_glow;
    uniform float u_src_palette;
    uniform float u_src_zoom;
    uniform float u_src_speed;
    uniform float u_src_trail_dist;
    uniform float u_src_trail_fade;
    uniform float u_src_slice_count;
    uniform float u_src_slice_dist;
    uniform float u_src_feedback;
    uniform float u_rms;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    mat3 rotY(float a) { float c=cos(a),s=sin(a); return mat3(c,0,s, 0,1,0, -s,0,c); }
    mat3 rotX(float a) { float c=cos(a),s=sin(a); return mat3(1,0,0, 0,c,-s, 0,s,c); }

    float burningShip3DDE(vec3 pos, float power, out float trap) {
        vec3 z = pos;
        float dr = 1.0, r = 0.0;
        trap = 1e10;
        for (int i = 0; i < 10; i++) {
            z = abs(z); // The burning ship twist in 3D
            r = length(z);
            if (r > 2.0) break;
            trap = min(trap, r);
            float theta = acos(clamp(z.z / r, -1.0, 1.0));
            float phi = atan(z.y, z.x);
            dr = pow(r, power - 1.0) * power * dr + 1.0;
            float zr = pow(r, power);
            theta *= power; phi *= power;
            z = zr * vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
            z += pos;
        }
        return 0.5 * log(r) * r / max(dr, 0.0001);
    }

    vec3 calcNormal(vec3 p, float pw) {
        vec2 e = vec2(0.001, 0.0);
        float dummy;
        return normalize(vec3(
            burningShip3DDE(p+e.xyy,pw,dummy) - burningShip3DDE(p-e.xyy,pw,dummy),
            burningShip3DDE(p+e.yxy,pw,dummy) - burningShip3DDE(p-e.yxy,pw,dummy),
            burningShip3DDE(p+e.yyx,pw,dummy) - burningShip3DDE(p-e.yyx,pw,dummy)));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        float power = 2.0 + u_src_power * u_src_power * 14.0;
        float autoSpeed = (u_src_speed - 0.5) * 2.0;
        float rx = u_src_rotation_x * 6.28318 + u_time * autoSpeed;
        float ry = u_src_rotation_y * 6.28318 + u_time * autoSpeed * 0.7;
        mat3 rot = rotY(ry) * rotX(rx);
        float camDist = mix(5.0, 0.3, u_src_zoom);
        vec3 ro = rot * vec3(0, 0, camDist);
        vec3 fwd = normalize(-ro);
        vec3 right = normalize(cross(fwd, vec3(0,1,0)));
        vec3 up = cross(right, fwd);
        vec3 rd = normalize(fwd + uv.x * right + uv.y * up);

        float sliceZ = (u_src_slice - 0.5) * 3.0;
        bool useSlice = abs(u_src_slice - 0.5) > 0.01;
        int sliceCount = int(u_src_slice_count * 4.0) + 1;
        float sliceSpacing = 0.1 + u_src_slice_dist * 0.8;
        float glowAmt = u_src_glow * u_src_glow * 4.0;
        int numTrails = int(u_src_trail_dist * 5.0);
        float trailDecay = 0.3 + u_src_trail_fade * 0.5;

        float t = 0.0, glow = 0.0, trapVal = 0.0;
        bool hit = false;
        for (int i = 0; i < 80; i++) {
            vec3 p = ro + rd * t;
            float trap;
            float d = burningShip3DDE(p, power, trap);
            if (useSlice) {
                if (sliceCount <= 1) {
                    d = max(d, abs(p.z - sliceZ) - 0.02);
                } else {
                    float md = 1e10;
                    for (int s = 0; s < 5; s++) {
                        if (s >= sliceCount) break;
                        float z = sliceZ + float(s - sliceCount/2) * sliceSpacing;
                        md = min(md, abs(p.z - z) - 0.02);
                    }
                    d = max(d, md);
                }
            }
            float glowBase = 1.0 / (1.0 + d * d * 500.0);
            float trailGlow = 0.0;
            if (d < 0.1 && numTrails > 0) {
                for (int tr = 0; tr < 5; tr++) {
                    if (tr >= numTrails) break;
                    float offset = float(tr + 1) * 0.02;
                    vec3 tp = p + rd * offset;
                    float tdmy;
                    float td = burningShip3DDE(tp, power, tdmy);
                    float tg = 1.0 / (1.0 + td * td * 500.0);
                    trailGlow += tg * pow(trailDecay, float(tr + 1));
                }
            }
            glow += glowBase + trailGlow * 0.3;
            // Feedback: modulate glow with periodic rings for echo/feedback look
            if (u_src_feedback > 0.01 && d < 0.5) {
                float fbRings = sin(d * u_src_feedback * 100.0) * 0.5 + 0.5;
                glow += fbRings * u_src_feedback * glowBase * 3.0;
            }
            if (d < 0.001) { hit = true; trapVal = trap; break; }
            t += d;
            if (t > 10.0) break;
        }
        vec3 col = vec3(0.0);
        if (hit) {
            vec3 p = ro + rd * t;
            vec3 n = calcNormal(p, power);
            vec3 light = normalize(vec3(1, 2, 3));
            float diff = max(dot(n, light), 0.0) * 0.7 + 0.3;
            float hue = u_src_color_shift + trapVal * 2.0;
            int palIdx = int(u_src_palette * 7.0);
            col = diff * fracPalette(hue, palIdx);
        }
        col += vec3(0.2, 0.1, 0.05) * glow * 0.015 * (1.0 + glowAmt);

        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)";

// Newton 3D — height field from 2D Newton iteration, ray marched
inline const char* sourceNewton3D = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_power;
    uniform float u_src_rotation_x;
    uniform float u_src_rotation_y;
    uniform float u_src_damping;
    uniform float u_src_color_shift;
    uniform float u_src_height;
    uniform float u_src_palette;
    uniform float u_src_zoom;
    uniform float u_src_speed;
    uniform float u_src_trail_dist;
    uniform float u_src_trail_fade;
    uniform float u_src_slice_count;
    uniform float u_src_slice_dist;
    uniform float u_src_feedback;
    uniform float u_rms;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    mat3 rotY(float a) { float c=cos(a),s=sin(a); return mat3(c,0,s, 0,1,0, -s,0,c); }
    mat3 rotX(float a) { float c=cos(a),s=sin(a); return mat3(1,0,0, 0,c,-s, 0,s,c); }

    vec2 cmul(vec2 a, vec2 b) { return vec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x); }
    vec2 cdiv(vec2 a, vec2 b) { return cmul(a, vec2(b.x,-b.y)) / dot(b,b); }

    // Newton iteration count as height + root coloring
    void newtonIterate(vec2 uv, int n, float damp, out float height, out int rootIdx) {
        vec2 z = uv;
        float minDist = 1e10;
        rootIdx = 0;
        int i;
        for (i = 0; i < 30; i++) {
            float r = length(z);
            if (r < 0.0001) break;
            float theta = atan(z.y, z.x);
            vec2 zn = pow(r, float(n)) * vec2(cos(float(n)*theta), sin(float(n)*theta));
            vec2 zn1 = pow(r, float(n-1)) * vec2(cos(float(n-1)*theta), sin(float(n-1)*theta));
            vec2 fz = zn - vec2(1.0, 0.0);
            vec2 fpz = float(n) * zn1;
            z -= damp * cdiv(fz, fpz);
            for (int k = 0; k < 8; k++) {
                if (k >= n) break;
                float ra = 6.28318 * float(k) / float(n);
                vec2 root = vec2(cos(ra), sin(ra));
                float d = length(z - root);
                if (d < minDist) { minDist = d; rootIdx = k; }
            }
            if (minDist < 0.001) break;
        }
        height = 1.0 - float(i) / 30.0;
    }

    float newtonDE(vec3 p, int n, float damp, float heightScale) {
        float h; int root;
        newtonIterate(p.xz, n, damp, h, root);
        return p.y - h * heightScale;
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        int n = int(u_src_power * u_src_power * 5.0) + 3;
        float damp = 0.5 + u_src_damping * 1.0;
        float heightScale = 0.2 + u_src_height * 1.5;
        float autoSpeed = (u_src_speed - 0.5) * 2.0;
        float rx = u_src_rotation_x * 6.28318 + u_time * autoSpeed + 0.3;
        float ry = u_src_rotation_y * 6.28318 + u_time * autoSpeed * 0.7;
        mat3 rot = rotY(ry) * rotX(rx);
        float camDist = mix(5.0, 0.3, u_src_zoom);
        vec3 ro = rot * vec3(0, 1.5, camDist);
        vec3 target = vec3(0, 0.2, 0);
        vec3 fwd = normalize(target - ro);
        vec3 right = normalize(cross(fwd, vec3(0,1,0)));
        vec3 up = cross(right, fwd);
        vec3 rd = normalize(fwd + uv.x * right + uv.y * up);

        int numTrails = int(u_src_trail_dist * 5.0);
        float trailDecay = 0.3 + u_src_trail_fade * 0.5;

        float t = 0.0;
        float glow = 0.0;
        bool hit = false;
        vec3 hitP = vec3(0);
        for (int i = 0; i < 80; i++) {
            vec3 p = ro + rd * t;
            float d = newtonDE(p, n, damp, heightScale);
            float glowBase = 1.0 / (1.0 + d * d * 100.0);
            float trailGlow = 0.0;
            if (abs(d) < 0.1 && numTrails > 0) {
                for (int tr = 0; tr < 5; tr++) {
                    if (tr >= numTrails) break;
                    float offset = float(tr + 1) * 0.02;
                    vec3 tp = p + rd * offset;
                    float td = newtonDE(tp, n, damp, heightScale);
                    float tg = 1.0 / (1.0 + td * td * 100.0);
                    trailGlow += tg * pow(trailDecay, float(tr + 1));
                }
            }
            glow += glowBase + trailGlow * 0.3;
            if (u_src_feedback > 0.01 && abs(d) < 0.5) {
                float fbRings = sin(abs(d) * u_src_feedback * 100.0) * 0.5 + 0.5;
                glow += fbRings * u_src_feedback * glowBase * 3.0;
            }
            if (abs(d) < 0.005) { hit = true; hitP = p; break; }
            t += max(d * 0.5, 0.005); // Conservative step
            if (t > 10.0) break;
        }
        vec3 col = vec3(0.0);
        if (hit) {
            // Normal via finite differences
            vec2 e = vec2(0.01, 0.0);
            vec3 n3 = normalize(vec3(
                newtonDE(hitP+e.xyy,n,damp,heightScale) - newtonDE(hitP-e.xyy,n,damp,heightScale),
                newtonDE(hitP+e.yxy,n,damp,heightScale) - newtonDE(hitP-e.yxy,n,damp,heightScale),
                newtonDE(hitP+e.yyx,n,damp,heightScale) - newtonDE(hitP-e.yyx,n,damp,heightScale)));
            vec3 light = normalize(vec3(1, 2, 3));
            float diff = max(dot(n3, light), 0.0) * 0.7 + 0.3;
            float h; int root;
            newtonIterate(hitP.xz, n, damp, h, root);
            float hue = float(root) / float(n) + u_src_color_shift;
            int palIdx = int(u_src_palette * 7.0);
            col = diff * fracPalette(hue, palIdx);
        }
        col += vec3(0.1, 0.15, 0.2) * glow * 0.008;

        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)";

// Sierpinski Tetrahedron — 3D IFS fractal via folding
inline const char* sourceSierpinskiTetra = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_iterations;
    uniform float u_src_rotation_x;
    uniform float u_src_rotation_y;
    uniform float u_src_color_shift;
    uniform float u_src_slice;
    uniform float u_src_palette;
    uniform float u_src_zoom;
    uniform float u_src_speed;
    uniform float u_src_trail_dist;
    uniform float u_src_trail_fade;
    uniform float u_src_slice_count;
    uniform float u_src_slice_dist;
    uniform float u_src_feedback;
    uniform float u_rms;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    mat3 rotY(float a) { float c=cos(a),s=sin(a); return mat3(c,0,s, 0,1,0, -s,0,c); }
    mat3 rotX(float a) { float c=cos(a),s=sin(a); return mat3(1,0,0, 0,c,-s, 0,s,c); }

    float sierpinskiTetraDE(vec3 z, int iters) {
        float Scale = 2.0;
        vec3 Offset = vec3(1.0, 1.0, 1.0);
        for (int n = 0; n < 15; n++) {
            if (n >= iters) break;
            // Tetrahedral folds
            if (z.x + z.y < 0.0) z.xy = -z.yx;
            if (z.x + z.z < 0.0) z.xz = -z.zx;
            if (z.y + z.z < 0.0) z.zy = -z.yz;
            z = z * Scale - Offset * (Scale - 1.0);
        }
        return length(z) * pow(Scale, -float(iters));
    }

    vec3 calcNormal(vec3 p, int it) {
        vec2 e = vec2(0.001, 0.0);
        return normalize(vec3(
            sierpinskiTetraDE(p+e.xyy,it) - sierpinskiTetraDE(p-e.xyy,it),
            sierpinskiTetraDE(p+e.yxy,it) - sierpinskiTetraDE(p-e.yxy,it),
            sierpinskiTetraDE(p+e.yyx,it) - sierpinskiTetraDE(p-e.yyx,it)));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        int iters = int(u_src_iterations * 12.0) + 3;
        float autoSpeed = (u_src_speed - 0.5) * 2.0;
        float rx = u_src_rotation_x * 6.28318 + u_time * autoSpeed;
        float ry = u_src_rotation_y * 6.28318 + u_time * autoSpeed * 0.7;
        mat3 rot = rotY(ry) * rotX(rx);
        float camDist = mix(5.0, 0.3, u_src_zoom);
        vec3 ro = rot * vec3(0, 0, camDist);
        vec3 fwd = normalize(-ro);
        vec3 right = normalize(cross(fwd, vec3(0,1,0)));
        vec3 up = cross(right, fwd);
        vec3 rd = normalize(fwd + uv.x * right + uv.y * up);

        float sliceZ = (u_src_slice - 0.5) * 3.0;
        bool useSlice = abs(u_src_slice - 0.5) > 0.01;
        int sliceCount = int(u_src_slice_count * 4.0) + 1;
        float sliceSpacing = 0.1 + u_src_slice_dist * 0.8;

        int numTrails = int(u_src_trail_dist * 5.0);
        float trailDecay = 0.3 + u_src_trail_fade * 0.5;

        float t = 0.0, glow = 0.0;
        bool hit = false;
        for (int i = 0; i < 80; i++) {
            vec3 p = ro + rd * t;
            float d = sierpinskiTetraDE(p, iters);
            if (useSlice) {
                if (sliceCount <= 1) {
                    d = max(d, abs(p.z - sliceZ) - 0.02);
                } else {
                    float md = 1e10;
                    for (int s = 0; s < 5; s++) {
                        if (s >= sliceCount) break;
                        float z = sliceZ + float(s - sliceCount/2) * sliceSpacing;
                        md = min(md, abs(p.z - z) - 0.02);
                    }
                    d = max(d, md);
                }
            }
            float glowBase = 1.0 / (1.0 + d * d * 300.0);
            float trailGlow = 0.0;
            if (d < 0.1 && numTrails > 0) {
                for (int tr = 0; tr < 5; tr++) {
                    if (tr >= numTrails) break;
                    float offset = float(tr + 1) * 0.02;
                    vec3 tp = p + rd * offset;
                    float td = sierpinskiTetraDE(tp, iters);
                    float tg = 1.0 / (1.0 + td * td * 300.0);
                    trailGlow += tg * pow(trailDecay, float(tr + 1));
                }
            }
            glow += glowBase + trailGlow * 0.3;
            if (u_src_feedback > 0.01 && d < 0.5) {
                float fbRings = sin(d * u_src_feedback * 100.0) * 0.5 + 0.5;
                glow += fbRings * u_src_feedback * glowBase * 3.0;
            }
            if (d < 0.001) { hit = true; break; }
            t += d;
            if (t > 10.0) break;
        }
        vec3 col = vec3(0.0);
        if (hit) {
            vec3 p = ro + rd * t;
            vec3 n = calcNormal(p, iters);
            vec3 light = normalize(vec3(1, 2, 3));
            float diff = max(dot(n, light), 0.0) * 0.7 + 0.3;
            float hue = u_src_color_shift + dot(abs(n), vec3(0.3, 0.5, 0.2));
            int palIdx = int(u_src_palette * 7.0);
            col = diff * fracPalette(hue, palIdx);
        }
        col += vec3(0.1, 0.15, 0.25) * glow * 0.012;

        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)";

// Apollonian 3D — sphere packing via 3D inversive geometry
inline const char* sourceApollonian3D = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_iterations;
    uniform float u_src_rotation_x;
    uniform float u_src_rotation_y;
    uniform float u_src_color_shift;
    uniform float u_src_scale;
    uniform float u_src_slice;
    uniform float u_src_palette;
    uniform float u_src_zoom;
    uniform float u_src_speed;
    uniform float u_src_trail_dist;
    uniform float u_src_trail_fade;
    uniform float u_src_slice_count;
    uniform float u_src_slice_dist;
    uniform float u_src_feedback;
    uniform float u_rms;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    mat3 rotY(float a) { float c=cos(a),s=sin(a); return mat3(c,0,s, 0,1,0, -s,0,c); }
    mat3 rotX(float a) { float c=cos(a),s=sin(a); return mat3(1,0,0, 0,c,-s, 0,s,c); }

    float apollonian3DDE(vec3 p, int iters, float sc) {
        float s = 1.0;
        float minR = 1e10;
        for (int i = 0; i < 20; i++) {
            if (i >= iters) break;
            p = abs(p);
            // Sort coordinates descending
            if (p.x < p.y) p.xy = p.yx;
            if (p.x < p.z) p.xz = p.zx;
            if (p.y < p.z) p.yz = p.zy;
            // Scale and translate
            p = p * sc - vec3(sc - 1.0);
            s *= sc;
            // Sphere inversion
            float r2 = dot(p, p);
            float k = max(1.0 / r2, 1.0);
            p *= k;
            s *= k;
            minR = min(minR, r2);
        }
        return (length(p) - 0.5) / s;
    }

    vec3 calcNormal(vec3 p, int it, float sc) {
        vec2 e = vec2(0.001, 0.0);
        return normalize(vec3(
            apollonian3DDE(p+e.xyy,it,sc) - apollonian3DDE(p-e.xyy,it,sc),
            apollonian3DDE(p+e.yxy,it,sc) - apollonian3DDE(p-e.yxy,it,sc),
            apollonian3DDE(p+e.yyx,it,sc) - apollonian3DDE(p-e.yyx,it,sc)));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        int iters = int(u_src_iterations * 15.0) + 5;
        float sc = 1.5 + u_src_scale * 2.0;
        float autoSpeed = (u_src_speed - 0.5) * 2.0;
        float rx = u_src_rotation_x * 6.28318 + u_time * autoSpeed;
        float ry = u_src_rotation_y * 6.28318 + u_time * autoSpeed * 0.7;
        mat3 rot = rotY(ry) * rotX(rx);
        float camDist = mix(6.0, 0.3, u_src_zoom);
        vec3 ro = rot * vec3(0, 0, camDist);
        vec3 fwd = normalize(-ro);
        vec3 right = normalize(cross(fwd, vec3(0,1,0)));
        vec3 up = cross(right, fwd);
        vec3 rd = normalize(fwd + uv.x * right + uv.y * up);

        float sliceZ = (u_src_slice - 0.5) * 4.0;
        bool useSlice = abs(u_src_slice - 0.5) > 0.01;
        int sliceCount = int(u_src_slice_count * 4.0) + 1;
        float sliceSpacing = 0.1 + u_src_slice_dist * 0.8;

        int numTrails = int(u_src_trail_dist * 5.0);
        float trailDecay = 0.3 + u_src_trail_fade * 0.5;

        float t = 0.0, glow = 0.0;
        bool hit = false;
        for (int i = 0; i < 80; i++) {
            vec3 p = ro + rd * t;
            float d = apollonian3DDE(p, iters, sc);
            if (useSlice) {
                if (sliceCount <= 1) {
                    d = max(d, abs(p.z - sliceZ) - 0.02);
                } else {
                    float md = 1e10;
                    for (int s = 0; s < 5; s++) {
                        if (s >= sliceCount) break;
                        float z = sliceZ + float(s - sliceCount/2) * sliceSpacing;
                        md = min(md, abs(p.z - z) - 0.02);
                    }
                    d = max(d, md);
                }
            }
            float glowBase = 1.0 / (1.0 + d * d * 200.0);
            float trailGlow = 0.0;
            if (d < 0.1 && numTrails > 0) {
                for (int tr = 0; tr < 5; tr++) {
                    if (tr >= numTrails) break;
                    float offset = float(tr + 1) * 0.02;
                    vec3 tp = p + rd * offset;
                    float td = apollonian3DDE(tp, iters, sc);
                    float tg = 1.0 / (1.0 + td * td * 200.0);
                    trailGlow += tg * pow(trailDecay, float(tr + 1));
                }
            }
            glow += glowBase + trailGlow * 0.3;
            if (u_src_feedback > 0.01 && d < 0.5) {
                float fbRings = sin(d * u_src_feedback * 100.0) * 0.5 + 0.5;
                glow += fbRings * u_src_feedback * glowBase * 3.0;
            }
            if (d < 0.001) { hit = true; break; }
            t += d;
            if (t > 15.0) break;
        }
        vec3 col = vec3(0.0);
        if (hit) {
            vec3 p = ro + rd * t;
            vec3 n = calcNormal(p, iters, sc);
            vec3 light = normalize(vec3(1, 2, 3));
            float diff = max(dot(n, light), 0.0) * 0.7 + 0.3;
            float hue = u_src_color_shift + dot(abs(n), vec3(0.2, 0.3, 0.5));
            int palIdx = int(u_src_palette * 7.0);
            col = diff * fracPalette(hue, palIdx);
        }
        col += vec3(0.1, 0.15, 0.25) * glow * 0.01;

        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)";

// ============================================================
// TORUS SOURCES (7 sources) — Raymarched 3D torus viewed from outside
// VERIFIED via WebGL test (torus_test_4.png). Key parameters:
// R=1.0 (ring), r=0.4 (tube). Camera dist=1.3, Y=0.65, FOV=0.7.
// Camera orbits above the torus looking down at the hole.
// Stripes use theta (toroidal/major angle) for the classic illusion.
// ============================================================

// === TORUS SOURCES — ALL BROWSER-VERIFIED (2026-03-22) ===
// Core camera: dist=1.1, Y=0.05, inward*0.5+tangent*0.5, FOV=2.6 fisheye
// Stripe: step(0.0, sin(N*theta + M*phi)) — integer N,M for seamless wrap
// Checker: abs(step(sin(N*theta)) - step(sin(M*phi))) — XOR pattern
// Twisted: sdTwistedTorus with conservative d*0.4 stepping
// Torus Hole: top-down perspective camera, standard torus SDF

// 1. Striped Torus — BROWSER VERIFIED (test_s1_striped.png)
inline const char* sourceStripedTorus = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_stripe_count; // number of stripes (6-32)
    uniform float u_src_twist;        // spiral twist amount (0=straight, 1=heavy spiral)
    uniform float u_src_speed;        // rotation speed
    uniform float u_src_tube_radius;  // tube thickness
    uniform float u_src_camera;       // camera angle (0=side vortex, 1=top-down hole)
    uniform float u_src_color_shift;  // hue tint (0=B&W)
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_beatPhase;

    float sdTorus(vec3 p, vec2 t){vec2 q=vec2(length(p.xz)-t.x,p.y);return length(q)-t.y;}
    vec3 calcN(vec3 p,vec2 t){float h=0.0001;vec2 k=vec2(1,-1);
        return normalize(k.xyy*sdTorus(p+k.xyy*h,t)+k.yyx*sdTorus(p+k.yyx*h,t)+
        k.yxy*sdTorus(p+k.yxy*h,t)+k.xxx*sdTorus(p+k.xxx*h,t));}
    void main() {
        vec2 uv=(gl_FragCoord.xy-0.5*u_resolution)/u_resolution.y;
        float R=1.0;float r=0.32+u_src_tube_radius*0.2+u_bass*0.05;
        vec2 td=vec2(R,r);
        float speed=0.08+u_src_speed*0.25;
        float N=floor(6.0+u_src_stripe_count*26.0);
        float M=floor(u_src_twist*8.0); // integer twist for seamless wrap
        float camAngle=u_src_camera; // 0=side vortex, 1=top hole view

        float ca=u_time*speed;
        // Camera blends between side view (vortex) and top view (hole)
        float camDist=mix(1.1,1.15,camAngle);
        float camY=mix(0.05,0.55,camAngle);
        vec3 ro=vec3(camDist*cos(ca),camY,camDist*sin(ca));

        vec3 inw=normalize(vec3(-cos(ca),0,-sin(ca)));
        vec3 tan2=normalize(vec3(-sin(ca),0,cos(ca)));
        // Side: look inward+tangent (vortex). Top: look at origin (hole)
        vec3 fwdSide=normalize(inw*0.5+tan2*0.5+vec3(0,0.05,0));
        vec3 fwdTop=normalize(-ro);
        vec3 fwd=normalize(mix(fwdSide,fwdTop,camAngle));

        vec3 ri=normalize(cross(fwd,vec3(0,1,0)));vec3 up=cross(ri,fwd);
        // FOV: wider for side, narrower for top
        float fov=mix(2.6,0.7,camAngle);
        float len=length(uv);

        vec3 rd;
        if(camAngle<0.5){
            // Fisheye for side view
            float ang=len*fov;
            vec2 d2=len>0.001?uv/len:vec2(0,1);
            rd=normalize(fwd*cos(ang)+(ri*d2.x+up*d2.y)*sin(ang));
        } else {
            // Standard perspective for top view
            rd=normalize(fwd*fov+uv.x*ri+uv.y*up);
        }

        float t=0.0;bool hit=false;
        for(int i=0;i<128;i++){float d=abs(sdTorus(ro+rd*t,td));
            if(d<0.0005){hit=true;break;}t+=max(d*0.5,0.001);if(t>10.0)break;}
        vec3 col=vec3(0.0);
        if(hit){
            vec3 p=ro+rd*t;vec3 n=calcN(p,td);
            float theta=atan(p.z,p.x);float phi=atan(p.y,length(p.xz)-R);
            float stripe=step(0.0,sin(N*theta+M*phi));
            // Top view gets subtle shading for depth
            vec3 ld=normalize(vec3(0.3,1.0,0.5));
            float diff=mix(1.0, max(dot(n,ld),0.0)*0.35+0.65, camAngle);
            vec3 c1=vec3(1.0),c2=vec3(0.0);float hue=u_src_color_shift;
            if(hue>0.01)c1=0.5+0.5*cos(6.28318*(hue+vec3(0,0.33,0.67)));
            col=mix(c2,c1,stripe)*diff;col*=0.85+u_rms*0.3;
        }
        fragColor=vec4(col,1.0);
    }
)";

// 2. Spiral Vortex — adjustable twist on torus, fisheye
inline const char* sourceSpiralVortex = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_twist;         // spiral twist amount
    uniform float u_src_stripe_count;  // stripe density
    uniform float u_src_speed;         // rotation speed
    uniform float u_src_color_shift;   // hue tint
    uniform float u_rms;
    uniform float u_mid;
    uniform float u_beatPhase;

    float sdTorus(vec3 p, vec2 t){vec2 q=vec2(length(p.xz)-t.x,p.y);return length(q)-t.y;}
    void main() {
        vec2 uv=(gl_FragCoord.xy-0.5*u_resolution)/u_resolution.y;
        float R=1.0,r=0.42;vec2 td=vec2(R,r);
        float speed=0.08+u_src_speed*0.25;
        float M=floor(1.0+(u_src_twist+u_mid*0.3)*8.0); // integer twist
        float N=floor(6.0+u_src_stripe_count*20.0); // integer stripes
        float ca=u_time*speed;
        float camDist=1.1;
        vec3 ro=vec3(camDist*cos(ca),0.05,camDist*sin(ca));
        vec3 inw=normalize(vec3(-cos(ca),0,-sin(ca)));
        vec3 tan2=normalize(vec3(-sin(ca),0,cos(ca)));
        vec3 fwd=normalize(inw*0.5+tan2*0.5+vec3(0,0.05,0));
        vec3 ri=normalize(cross(fwd,vec3(0,1,0)));vec3 up=cross(ri,fwd);
        float fov=2.6;float len=length(uv);float ang=len*fov;
        vec2 d2=len>0.001?uv/len:vec2(0,1);
        vec3 rd=normalize(fwd*cos(ang)+(ri*d2.x+up*d2.y)*sin(ang));
        float t=0.0;bool hit=false;
        for(int i=0;i<128;i++){float d=abs(sdTorus(ro+rd*t,td));
            if(d<0.0005){hit=true;break;}t+=max(d*0.5,0.001);if(t>10.0)break;}
        vec3 col=vec3(0.0);
        if(hit){
            vec3 p=ro+rd*t;
            float theta=atan(p.z,p.x);float phi=atan(p.y,length(p.xz)-R);
            float stripe=step(0.0,sin(N*theta+M*phi));
            vec3 c1=vec3(1.0),c2=vec3(0.0);float hue=u_src_color_shift;
            if(hue>0.01)c1=0.5+0.5*cos(6.28318*(hue+vec3(0,0.33,0.67)));
            col=mix(c2,c1,stripe);col*=0.85+u_rms*0.3;
        }
        fragColor=vec4(col,1.0);
    }
)";

// 3. Checker Torus — checkerboard pattern on torus
inline const char* sourceCheckerTorus = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_grid_u;        // ring grid density
    uniform float u_src_grid_v;        // tube grid density
    uniform float u_src_speed;         // rotation speed
    uniform float u_src_color_shift;   // hue tint
    uniform float u_rms;
    uniform float u_spectralCentroid;
    uniform float u_beatPhase;

    float sdTorus(vec3 p, vec2 t){vec2 q=vec2(length(p.xz)-t.x,p.y);return length(q)-t.y;}
    void main() {
        vec2 uv=(gl_FragCoord.xy-0.5*u_resolution)/u_resolution.y;
        float R=1.0,r=0.42;vec2 td=vec2(R,r);
        float speed=0.08+u_src_speed*0.25;
        float cn=clamp(u_spectralCentroid/8000.0,0.0,1.0);
        float gu=floor(6.0+(u_src_grid_u+cn*0.1)*26.0);
        float gv=floor(4.0+(u_src_grid_v+cn*0.1)*18.0);
        float ca=u_time*speed;
        float camDist=1.1;
        vec3 ro=vec3(camDist*cos(ca),0.05,camDist*sin(ca));
        vec3 inw=normalize(vec3(-cos(ca),0,-sin(ca)));
        vec3 tan2=normalize(vec3(-sin(ca),0,cos(ca)));
        vec3 fwd=normalize(inw*0.5+tan2*0.5+vec3(0,0.05,0));
        vec3 ri=normalize(cross(fwd,vec3(0,1,0)));vec3 up=cross(ri,fwd);
        float fov=2.6;float len=length(uv);float ang=len*fov;
        vec2 d2=len>0.001?uv/len:vec2(0,1);
        vec3 rd=normalize(fwd*cos(ang)+(ri*d2.x+up*d2.y)*sin(ang));
        float t=0.0;bool hit=false;
        for(int i=0;i<128;i++){float d=abs(sdTorus(ro+rd*t,td));
            if(d<0.0005){hit=true;break;}t+=max(d*0.5,0.001);if(t>10.0)break;}
        vec3 col=vec3(0.0);
        if(hit){
            vec3 p=ro+rd*t;
            float theta=atan(p.z,p.x);float phi=atan(p.y,length(p.xz)-R);
            // Checkerboard: XOR of two step functions
            float s1=step(0.0,sin(gu*theta));
            float s2=step(0.0,sin(gv*phi));
            float check=abs(s1-s2);
            vec3 c1=vec3(1.0),c2=vec3(0.0);float hue=u_src_color_shift;
            if(hue>0.01)c1=0.5+0.5*cos(6.28318*(hue+vec3(0,0.33,0.67)));
            col=mix(c2,c1,check);col*=0.85+u_rms*0.3;
        }
        fragColor=vec4(col,1.0);
    }
)";

// 4. Ribbed Vortex — smooth colored ridges on torus
inline const char* sourceRibbedVortex = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_ridge_count;   // number of ridges
    uniform float u_src_color_mix;     // gradient blend amount
    uniform float u_src_speed;         // rotation speed
    uniform float u_src_color_shift;   // base hue
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_beatPhase;

    float sdTorus(vec3 p, vec2 t){vec2 q=vec2(length(p.xz)-t.x,p.y);return length(q)-t.y;}
    void main() {
        vec2 uv=(gl_FragCoord.xy-0.5*u_resolution)/u_resolution.y;
        float R=1.0,r=0.42+u_bass*0.04;vec2 td=vec2(R,r);
        float speed=0.06+u_src_speed*0.2;
        float ridges=floor(10.0+u_src_ridge_count*30.0);
        float ca=u_time*speed;
        float camDist=1.1;
        vec3 ro=vec3(camDist*cos(ca),0.05,camDist*sin(ca));
        vec3 inw=normalize(vec3(-cos(ca),0,-sin(ca)));
        vec3 tan2=normalize(vec3(-sin(ca),0,cos(ca)));
        vec3 fwd=normalize(inw*0.5+tan2*0.5+vec3(0,0.05,0));
        vec3 ri=normalize(cross(fwd,vec3(0,1,0)));vec3 up=cross(ri,fwd);
        float fov=2.6;float len=length(uv);float ang=len*fov;
        vec2 d2=len>0.001?uv/len:vec2(0,1);
        vec3 rd=normalize(fwd*cos(ang)+(ri*d2.x+up*d2.y)*sin(ang));
        float t=0.0;bool hit=false;
        for(int i=0;i<128;i++){float d=abs(sdTorus(ro+rd*t,td));
            if(d<0.0005){hit=true;break;}t+=max(d*0.5,0.001);if(t>10.0)break;}
        vec3 col=vec3(0.0);
        if(hit){
            vec3 p=ro+rd*t;
            float theta=atan(p.z,p.x);float phi=atan(p.y,length(p.xz)-R);
            // Smooth ridges via sin
            float ridge=0.5+0.5*sin(ridges*phi);ridge=pow(ridge,2.0);
            // Gradient: cyan-orange blend by theta position
            float cmix=u_src_color_mix;float hb=u_src_color_shift;
            vec3 cA=0.5+0.5*cos(6.28318*(hb+0.55+vec3(0,0.33,0.67)));
            vec3 cB=0.5+0.5*cos(6.28318*(hb+0.08+vec3(0,0.33,0.67)));
            float tN=theta/6.28318+0.5;
            vec3 baseCol=mix(cA,cB,tN*cmix+(1.0-cmix)*0.5);
            col=baseCol*(0.3+ridge*0.7);col*=0.85+u_rms*0.3;
        }
        fragColor=vec4(col,1.0);
    }
)";

// 5. Wormhole Tunnel — warped torus with spiral stripes + glow
inline const char* sourceWormholeTunnel = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_warp;          // space distortion amount
    uniform float u_src_morph;         // (reserved)
    uniform float u_src_speed;         // rotation speed
    uniform float u_src_color_shift;   // hue
    uniform float u_rms;
    uniform float u_beatPhase;
    uniform float u_bass;

    float sdTorus2(vec3 p, vec2 t){vec2 q=vec2(length(p.xz)-t.x,p.y);return length(q)-t.y;}
    vec3 calcN2(vec3 p,vec2 t){float h=0.0001;vec2 k=vec2(1,-1);
        return normalize(k.xyy*sdTorus2(p+k.xyy*h,t)+k.yyx*sdTorus2(p+k.yyx*h,t)+
        k.yxy*sdTorus2(p+k.yxy*h,t)+k.xxx*sdTorus2(p+k.xxx*h,t));}
    void main() {
        vec2 uv=(gl_FragCoord.xy-0.5*u_resolution)/u_resolution.y;
        float R=1.0,r=0.38;vec2 td=vec2(R,r);
        float speed=0.08+u_src_speed*0.25;
        float N=8.0,M=4.0;
        float ca=u_time*speed;
        float camDist=1.1;
        vec3 ro=vec3(camDist*cos(ca),0.05,camDist*sin(ca));
        vec3 inw=normalize(vec3(-cos(ca),0,-sin(ca)));
        vec3 tan2=normalize(vec3(-sin(ca),0,cos(ca)));
        vec3 fwd=normalize(inw*0.5+tan2*0.5+vec3(0,0.05,0));
        vec3 ri=normalize(cross(fwd,vec3(0,1,0)));vec3 up=cross(ri,fwd);
        float fov=2.6;float len=length(uv);float ang=len*fov;
        vec2 d2=len>0.001?uv/len:vec2(0,1);
        vec3 rd=normalize(fwd*cos(ang)+(ri*d2.x+up*d2.y)*sin(ang));
        float t=0.0;bool hit=false;
        for(int i=0;i<128;i++){float d=abs(sdTorus2(ro+rd*t,td));
            if(d<0.0005){hit=true;break;}t+=max(d*0.5,0.001);if(t>10.0)break;}
        vec3 col=vec3(0.0);
        if(hit){
            vec3 p=ro+rd*t;vec3 n=calcN2(p,td);
            float theta=atan(p.z,p.x);float phi=atan(p.y,length(p.xz)-R);
            float stripe=step(0.0,sin(N*theta+M*phi));
            float hue=u_src_color_shift+theta/6.28318*0.3+0.6;
            vec3 c1=0.5+0.5*cos(6.28318*(hue+vec3(0,0.33,0.67)));
            vec3 ld=normalize(vec3(0.3,1,0.5));
            float diff=max(dot(n,ld),0.0)*0.5+0.5;
            col=mix(c1*0.1,c1,stripe)*diff;
        }
        col*=0.85+u_rms*0.3;
        fragColor=vec4(col,1.0);
    }
)";

// 6. Twisted Torus (OS17) — Mobius-like twist along tube
inline const char* sourceTwistedTorus = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_twist;         // tube twist amount
    uniform float u_src_speed;         // rotation speed
    uniform float u_src_tube_radius;   // tube thickness
    uniform float u_src_stripe_count;  // stripe density
    uniform float u_src_color_shift;   // hue tint
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_mid;
    uniform float u_beatPhase;

    float sdTwistedTorus(vec3 p,float R,float r,float tw){
        float a=atan(p.z,p.x);float ta=a*tw;float ct=cos(ta),st=sin(ta);
        float md=length(p.xz)-R;vec2 tw2=vec2(md*ct-p.y*st,md*st+p.y*ct);
        return length(tw2)-r;}
    void main() {
        vec2 uv=(gl_FragCoord.xy-0.5*u_resolution)/u_resolution.y;
        float R=1.0;float r=0.25+u_src_tube_radius*0.2+u_bass*0.04;
        float speed=0.08+u_src_speed*0.25;
        float twist=0.5+(u_src_twist+u_mid*0.3)*4.0;
        float N=floor(6.0+u_src_stripe_count*20.0);
        float ca=u_time*speed+u_beatPhase*0.3;
        float camDist=1.1;
        vec3 ro=vec3(camDist*cos(ca),0.05,camDist*sin(ca));
        vec3 inw=normalize(vec3(-cos(ca),0,-sin(ca)));
        vec3 tan2=normalize(vec3(-sin(ca),0,cos(ca)));
        vec3 fwd=normalize(inw*0.5+tan2*0.5+vec3(0,0.05,0));
        vec3 ri=normalize(cross(fwd,vec3(0,1,0)));vec3 up=cross(ri,fwd);
        float fov=2.6;float len=length(uv);float ang=len*fov;
        vec2 d2=len>0.001?uv/len:vec2(0,1);
        vec3 rd=normalize(fwd*cos(ang)+(ri*d2.x+up*d2.y)*sin(ang));
        float t=0.0;bool hit=false;
        for(int i=0;i<128;i++){float d=abs(sdTwistedTorus(ro+rd*t,R,r,twist));
            if(d<0.0005){hit=true;break;}t+=max(d*0.5,0.001);if(t>10.0)break;}
        vec3 col=vec3(0.0);
        if(hit){
            vec3 p=ro+rd*t;
            // Untwist to get UV
            float theta=atan(p.z,p.x);float ta=theta*twist;
            float md=length(p.xz)-R;float ct=cos(ta),st=sin(ta);
            vec2 untw=vec2(md*ct-p.y*st,md*st+p.y*ct);
            float phi=atan(untw.y,untw.x);
            float stripe=step(0.0,sin(N*theta+3.0*phi));
            vec3 c1=vec3(1.0),c2=vec3(0.0);float hue=u_src_color_shift;
            if(hue>0.01)c1=0.5+0.5*cos(6.28318*(hue+vec3(0,0.33,0.67)));
            col=mix(c2,c1,stripe);col*=0.85+u_rms*0.3;
        }
        fragColor=vec4(col,1.0);
    }
)";

// 7. Wormhole (OS18) — warped space torus with glow and morphing
inline const char* sourceWormhole = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_warp;          // space warp intensity
    uniform float u_src_morph;         // cross-section morph
    uniform float u_src_speed;         // rotation speed
    uniform float u_src_glow;          // glow intensity
    uniform float u_src_color_shift;   // hue
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_mid;
    uniform float u_beatPhase;
    uniform float u_onsetStrength;

    float sdTorus3(vec3 p, vec2 t){vec2 q=vec2(length(p.xz)-t.x,p.y);return length(q)-t.y;}
    vec3 calcN3(vec3 p,vec2 t){float h=0.0001;vec2 k=vec2(1,-1);
        return normalize(k.xyy*sdTorus3(p+k.xyy*h,t)+k.yyx*sdTorus3(p+k.yyx*h,t)+
        k.yxy*sdTorus3(p+k.yxy*h,t)+k.xxx*sdTorus3(p+k.xxx*h,t));}
    void main() {
        vec2 uv=(gl_FragCoord.xy-0.5*u_resolution)/u_resolution.y;
        float R=1.0,r=0.4;vec2 td=vec2(R,r);
        float speed=0.08+u_src_speed*0.25;
        float N=6.0,M=3.0;
        float glowAmt=0.3+u_src_glow*0.7;
        float wb=u_onsetStrength*0.04;
        float ca=u_time*speed;
        float camDist=1.1;
        vec3 ro=vec3(camDist*cos(ca),0.05,camDist*sin(ca));
        ro+=vec3(wb*sin(u_time*17.0),wb*cos(u_time*13.0),0.0);
        vec3 inw=normalize(vec3(-cos(ca),0,-sin(ca)));
        vec3 tan2=normalize(vec3(-sin(ca),0,cos(ca)));
        vec3 fwd=normalize(inw*0.5+tan2*0.5+vec3(0,0.05,0));
        vec3 ri=normalize(cross(fwd,vec3(0,1,0)));vec3 up=cross(ri,fwd);
        float fov=2.6;float len=length(uv);float ang=len*fov;
        vec2 d2=len>0.001?uv/len:vec2(0,1);
        vec3 rd=normalize(fwd*cos(ang)+(ri*d2.x+up*d2.y)*sin(ang));
        float t=0.0;bool hit=false;float glow=0.0;
        for(int i=0;i<128;i++){float d=abs(sdTorus3(ro+rd*t,td));
            glow+=1.0/(1.0+d*d*300.0);
            if(d<0.0005){hit=true;break;}t+=max(d*0.5,0.001);if(t>10.0)break;}
        vec3 col=vec3(0.0);
        if(hit){
            vec3 p=ro+rd*t;vec3 n=calcN3(p,td);
            float theta=atan(p.z,p.x);float phi=atan(p.y,length(p.xz)-R);
            float stripe=step(0.0,sin(N*theta+M*phi+u_time*0.5));
            float hue=u_src_color_shift+theta/6.28318*0.5+0.5;
            vec3 c1=0.5+0.5*cos(6.28318*(hue+vec3(0,0.33,0.67)));
            vec3 ld=normalize(vec3(0.3,1,0.5));
            float diff=max(dot(n,ld),0.0)*0.4+0.6;
            col=c1*max(stripe,0.12)*diff;
        }
        float hg=u_src_color_shift+0.5;
        vec3 gc=0.5+0.5*cos(6.28318*(hg+vec3(0,0.33,0.67)));
        col+=gc*glow*0.008*glowAmt;col*=0.85+u_rms*0.3;
        fragColor=vec4(col,1.0);
    }
)";

// 8. Torus Hole — comprehensive 16-param torus with full texture mapping controls
// Camera: orbit, tilt (smooth, no pop), zoom
// Pattern: stripe count, twist, angle, scale, width, offsets, checker mix
// Visual: shading, color
inline const char* sourceTorusHole = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    // Camera
    uniform float u_src_orbit;
    uniform float u_src_tilt;
    uniform float u_src_speed;
    uniform float u_src_zoom;
    // Lens
    uniform float u_src_lens_shape;    // 0=circular fisheye, 1=tubular/elliptical
    uniform float u_src_lens_rotate;   // rotate the lens ellipse
    uniform float u_src_depth_fade;    // thin stripes further from camera
    // Pattern
    uniform float u_src_stripe_count;
    uniform float u_src_twist;
    uniform float u_src_stripe_angle;
    uniform float u_src_stripe_scale;
    uniform float u_src_stripe_width;
    uniform float u_src_phi_offset;
    uniform float u_src_theta_offset;
    // Geometry
    uniform float u_src_tube_radius;
    // Deformation
    uniform float u_src_pinch;
    uniform float u_src_heart;
    // Visual
    uniform float u_src_shading;
    uniform float u_src_color_shift;
    // Audio
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_beatPhase;

    float sdTorus(vec3 p, vec2 t){vec2 q=vec2(length(p.xz)-t.x,p.y);return length(q)-t.y;}
    vec3 calcN(vec3 p,vec2 t){float h=0.0001;vec2 k=vec2(1,-1);
        return normalize(k.xyy*sdTorus(p+k.xyy*h,t)+k.yyx*sdTorus(p+k.yyx*h,t)+
        k.yxy*sdTorus(p+k.yxy*h,t)+k.xxx*sdTorus(p+k.xxx*h,t));}
    void main() {
        vec2 uv=(gl_FragCoord.xy-0.5*u_resolution)/u_resolution.y;
        float R=1.0;
        float r=0.1+u_src_tube_radius*1.0+u_bass*0.04; // 4x larger range: 0.1 to 1.1
        vec2 td=vec2(R,r);

        // Pattern
        float scl=0.5+u_src_stripe_scale*1.5;
        float N=floor((10.0+u_src_stripe_count*50.0)*scl);
        float M=floor(u_src_twist*8.0);
        float stripeAngle=u_src_stripe_angle*3.14159;
        float stripeWidth=u_src_stripe_width;
        float phiOff=u_src_phi_offset*6.28318;
        float thetaOff=u_src_theta_offset*6.28318;
        float shade=u_src_shading;
        float depthFade=u_src_depth_fade;

        // Camera
        float tilt=u_src_tilt;
        float camY=-0.4+tilt*0.81;
        float speed=0.06+u_src_speed*0.15;
        float ca=u_time*speed+u_src_orbit*6.28318;
        float camDist=0.9+u_src_zoom*0.5;
        vec3 ro=vec3(camDist*cos(ca),camY,camDist*sin(ca));

        // Look direction
        vec3 inw=normalize(vec3(-cos(ca),0,-sin(ca)));
        vec3 tan2=normalize(vec3(-sin(ca),0,cos(ca)));
        float inBlend=mix(0.45,0.8,tilt);
        vec3 fwdBase=normalize(inw*inBlend+tan2*(1.0-inBlend)+vec3(0,0.03,0));
        vec3 fwd=normalize(mix(fwdBase,normalize(-ro),tilt*tilt));
        vec3 ri=normalize(cross(fwd,vec3(0,1,0)));
        vec3 up=cross(ri,fwd);

        // === LENS DISTORTION ===
        // Lens shape: circular (0) to tubular/elliptical (1)
        // Stretches the UV in one axis before fisheye mapping
        float lensShape=u_src_lens_shape;
        float lensRot=u_src_lens_rotate*3.14159; // 0 to PI
        // Rotate UV by lens angle
        float lc=cos(lensRot),ls=sin(lensRot);
        vec2 uvRot=vec2(uv.x*lc-uv.y*ls, uv.x*ls+uv.y*lc);
        // Stretch one axis to create tubular shape
        uvRot.x*=1.0+lensShape*2.0; // stretch X by up to 3x
        // Rotate back
        vec2 uvLens=vec2(uvRot.x*lc+uvRot.y*ls, -uvRot.x*ls+uvRot.y*lc);

        // Fisheye with lens-distorted UV
        float fov=mix(2.6,1.2,tilt);
        float len=length(uvLens);float ang=len*fov;
        vec2 d2=len>0.001?uvLens/len:vec2(0,1);
        vec3 rd=normalize(fwd*cos(ang)+(ri*d2.x+up*d2.y)*sin(ang));

        float t=0.0;bool hit=false;
        for(int i=0;i<128;i++){float d=abs(sdTorus(ro+rd*t,td));
            if(d<0.0005){hit=true;break;}t+=max(d*0.5,0.001);if(t>10.0)break;}
        vec3 col=vec3(0.0);
        if(hit){
            vec3 p=ro+rd*t;vec3 n=calcN(p,td);
            float theta=atan(p.z,p.x)+thetaOff;
            float phi=atan(p.y,length(p.xz)-R)+phiOff;

            // Texture deformation
            float pinch=u_src_pinch;
            float heart=u_src_heart;
            // Pinch: range -4 to +4 (slider 0.5 = neutral)
            float pinchVal=(pinch*2.0-1.0)*4.0;
            phi+=pinchVal*sin(phi);
            // Heart: range -1 to +1 (slider 0.5 = neutral)
            float heartVal=heart*2.0-1.0;
            theta+=heartVal*(sin(theta)*2.0+sin(2.0*theta)*0.8);

            // Stripe angle rotation
            float sa=stripeAngle;
            float u1=theta*cos(sa)+phi*sin(sa);
            float u2=-theta*sin(sa)+phi*cos(sa);

            // Depth fade: thin stripes further from camera
            // Modulate the stripe frequency by distance
            float dist=length(p-ro);
            float depthN=N*(1.0+depthFade*dist*0.5);

            float spiralVal=sin(depthN*u1+M*u2);
            float threshold=sin((stripeWidth-0.5)*3.14159);
            float pattern=step(threshold,spiralVal);

            // Shading
            vec3 ld=normalize(vec3(0.3,1.0,0.5));
            float diff=mix(1.0,max(dot(n,ld),0.0)*0.5+0.5,shade);
            vec3 c1=vec3(1.0),c2=vec3(0.0);float hue=u_src_color_shift;
            if(hue>0.01)c1=0.5+0.5*cos(6.28318*(hue+vec3(0,0.33,0.67)));
            col=mix(c2,c1,pattern)*diff;col*=0.85+u_rms*0.3;
        }
        fragColor=vec4(col,1.0);
    }
)";

// ============================================================
// Phase 16: Time Effects (temporal — use u_prev_frame)
// ============================================================

// P16.1: Echo (AE-style) — stacks multiple ghosted copies of moving content.
// The previous frame accumulates: each frame blends current with the accumulated
// buffer, creating stroboscopic trails that fade over time.
// Operator: 0=Add (bright streaks), 0.33=Screen, 0.66=Maximum, 1.0=Blend
static const char* ghostTrails = R"(
#version 410 core
in vec2 v_texCoord;
out vec4 fragColor;
uniform sampler2D u_texture;
uniform sampler2D u_prev_frame;
uniform float u_trail_length;
uniform float u_trail_fade;
void main() {
    vec4 current = texture(u_texture, v_texCoord);
    vec4 prev = texture(u_prev_frame, v_texCoord);

    // Remap slider [0,1] to useful decay range [0.82, 0.995].
    // At slider=0 (decay=0.82) trails are very short. At slider=1 (0.995) near-infinite.
    // The whole slider range produces visible change.
    float decay = mix(0.82, 0.995, u_trail_length);

    // The accumulated buffer = previous output * decay
    // We blend this with the current frame using the selected operator.

    // trail_fade selects the operator:
    // 0.0 = Add (stroboscopic, bright, can blow out)
    // 0.33 = Screen (additive but won't exceed white)
    // 0.66 = Maximum (keeps brightest pixel)
    // 1.0 = Blend (average, subtle ghosting)
    float op = u_trail_fade;

    vec4 trail = prev * decay;

    // Compute each blend mode
    vec4 added = current + trail;                                    // Add
    vec4 screened = 1.0 - (1.0 - current) * (1.0 - trail);         // Screen
    vec4 maxed = max(current, trail);                                // Maximum
    vec4 blended = current * (1.0 - decay) + trail;                 // Blend/Average

    // Interpolate between operators based on u_trail_fade
    vec4 result;
    if (op < 0.33) {
        result = mix(added, screened, op / 0.33);
    } else if (op < 0.66) {
        result = mix(screened, maxed, (op - 0.33) / 0.33);
    } else {
        result = mix(maxed, blended, (op - 0.66) / 0.34);
    }

    fragColor = clamp(result, 0.0, 1.0);
}
)";

// P16.2: Posterize Time — hard frame rate reduction for choppy stop-motion.
// Locks the output to a low frame rate. Between updates, shows the HELD frame
// with zero blending — a hard freeze-then-snap look.
// rate: target fps mapped from slider (0=60fps smooth, 1=2fps very choppy)
// amount: mix between live and posterized (0=live, 1=full stop-motion)
static const char* frameHold = R"(
#version 410 core
in vec2 v_texCoord;
out vec4 fragColor;
uniform sampler2D u_texture;
uniform sampler2D u_prev_frame;
uniform float u_hold_rate;
uniform float u_hold_amount;
uniform float u_time;
void main() {
    vec4 current = texture(u_texture, v_texCoord);
    vec4 held = texture(u_prev_frame, v_texCoord);

    // Map rate slider to target FPS.
    // Slider 0 = 60fps (smooth, no visible effect).
    // Slider 0.5 = ~6fps (clearly choppy).
    // Slider 1 = 1fps (extreme stop-motion).
    // Use exponential curve so the slider feels responsive across the whole range.
    float targetFps = 60.0 * pow(1.0/60.0, u_hold_rate); // 60 → 1 exponentially
    float holdInterval = 1.0 / targetFps;

    // Quantize time to the target frame rate
    float quantizedTime = floor(u_time / holdInterval) * holdInterval;
    float timeSinceUpdate = u_time - quantizedTime;

    // If we're NOT on an update frame, show the held (previous) frame
    // The threshold is half a frame duration — if past that, we're in hold territory
    float isHolding = step(holdInterval * 0.4, timeSinceUpdate);

    // amount controls how much of the posterize effect is applied
    // 0 = fully live, 1 = full stop-motion
    vec4 posterized = mix(current, held, isHolding);
    fragColor = mix(current, posterized, u_hold_amount);
}
)";

// P16.3: Freeze — captures and holds the entire frame.
// amount=1 freezes completely (shows held frame only).
// amount=0 = fully live (bypass).
static const char* timeFreeze = R"(
#version 410 core
in vec2 v_texCoord;
out vec4 fragColor;
uniform sampler2D u_texture;
uniform sampler2D u_prev_frame;
uniform float u_freeze_amount;
void main() {
    vec4 current = texture(u_texture, v_texCoord);
    vec4 frozen = texture(u_prev_frame, v_texCoord);
    fragColor = mix(current, frozen, u_freeze_amount);
}
)";

// P16.4: Screen Split — surveillance wall with per-cell time delay.
// Each cell shows the full image, but cells further from top-left show
// progressively older frames (mixed between current and previous).
// delay: how much time offset between cells (0=all same, 1=max offset)
// mode: 0=sequential L-to-R delay, 0.5=diagonal delay, 1.0=random delay
static const char* screenSplit = R"(
#version 410 core
in vec2 v_texCoord;
out vec4 fragColor;
uniform sampler2D u_texture;
uniform sampler2D u_prev_frame;
uniform float u_screensplit_cols;
uniform float u_screensplit_rows;
uniform float u_screensplit_delay;
uniform float u_screensplit_mode;
uniform float u_time;
float hash21(vec2 p) {
    p = fract(p * vec2(233.34, 851.73));
    p += dot(p, p + 23.45);
    return fract(p.x * p.y);
}
void main() {
    float cols = floor(mix(2.0, 8.0, u_screensplit_cols));
    float rows = floor(mix(2.0, 8.0, u_screensplit_rows));

    vec2 cellIdx = floor(v_texCoord * vec2(cols, rows));
    vec2 cellUV = fract(v_texCoord * vec2(cols, rows));

    // Border
    float borderW = 0.035;
    float inCell = step(borderW, cellUV.x) * step(cellUV.x, 1.0 - borderW)
                 * step(borderW, cellUV.y) * step(cellUV.y, 1.0 - borderW);
    if (inCell < 0.5) {
        fragColor = vec4(0.06, 0.06, 0.06, 1.0);
        return;
    }

    // Each cell shows the full image
    vec2 innerUV = (cellUV - borderW) / (1.0 - 2.0 * borderW);
    innerUV = clamp(innerUV, 0.0, 1.0);

    // Calculate per-cell delay factor [0, 1]
    float totalCells = cols * rows;
    float cellNum = cellIdx.y * cols + cellIdx.x;
    float mode = u_screensplit_mode;

    float cellDelay;
    if (mode < 0.33) {
        // Sequential: left-to-right, top-to-bottom
        cellDelay = cellNum / max(totalCells - 1.0, 1.0);
    } else if (mode < 0.66) {
        // Diagonal: distance from top-left corner
        cellDelay = (cellIdx.x + cellIdx.y) / max(cols + rows - 2.0, 1.0);
    } else {
        // Random per cell
        cellDelay = hash21(cellIdx);
    }

    // delay param controls how strongly cells are time-offset
    // cellDelay * delay = how much of the previous frame this cell shows
    float delayMix = cellDelay * u_screensplit_delay;

    vec4 current = texture(u_texture, innerUV);
    vec4 prev = texture(u_prev_frame, innerUV);

    // Mix between current and delayed frame based on cell's delay
    fragColor = mix(current, prev, delayMix);
}
)";

// P16.5: Frame Stutter — jumps back in time rhythmically.
// Shows a frame from N frames ago, creating a rewind/scratch/stutter effect.
// Handled specially by compositor using the ring buffer (same as Screen Split).
// depth: how many frames back to jump (0=1 frame, 1=max ring frames)
// stutter: rate of jumping (0=smooth delay, 1=rapid stutter)
static const char* frameDelay = R"(
#version 410 core
in vec2 v_texCoord;
out vec4 fragColor;
uniform sampler2D u_texture;
void main() {
    // This shader is a passthrough — the actual Frame Stutter logic
    // is handled by the compositor using the ring buffer.
    fragColor = texture(u_texture, v_texCoord);
}
)";

// ============================================================
// Phase 16: Feedback Shader (layer-level feedback processor)
// ============================================================

static const char* feedbackBlend = R"(
#version 410 core
in vec2 v_texCoord;
out vec4 fragColor;
uniform sampler2D u_currentFrame;
uniform sampler2D u_previousFrame;
uniform float u_feedback_amount;
uniform float u_feedback_scaleX;
uniform float u_feedback_scaleY;
uniform float u_feedback_rotation;
uniform float u_feedback_offsetX;
uniform float u_feedback_offsetY;
uniform float u_feedback_lumaKey;
void main() {
    // Transform UV for previous frame sampling
    vec2 uv = v_texCoord - 0.5;

    // Scale (< 1 = zoom in, > 1 = zoom out)
    uv /= vec2(u_feedback_scaleX, u_feedback_scaleY);

    // Rotate
    float angle = u_feedback_rotation * 3.14159265 / 180.0;
    float c = cos(angle), s = sin(angle);
    uv = mat2(c, -s, s, c) * uv;

    // Offset
    uv += vec2(u_feedback_offsetX, u_feedback_offsetY);
    uv += 0.5;

    vec4 prev = texture(u_previousFrame, uv);
    vec4 curr = texture(u_currentFrame, v_texCoord);

    // Luma key: fade out dark areas of feedback to prevent muddiness
    float luma = dot(prev.rgb, vec3(0.299, 0.587, 0.114));
    float lumaFade = smoothstep(u_feedback_lumaKey * 0.5, u_feedback_lumaKey * 0.5 + 0.1, luma);
    prev *= lumaFade;

    // Blend: current frame on top of transformed previous frame
    fragColor = mix(curr, prev, u_feedback_amount);
}
)";

// ============================================================================
// Phase 17: Creative Sources (19 new procedural sources)
// ============================================================================

// P17.1 — Lissajous Weaver (Math)
// Uses closest-point-on-curve via dense sampling for smooth glowing lines
inline const char* sourceLissajousWeaver = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_freq_x;
    uniform float u_src_freq_y;
    uniform float u_src_phase;
    uniform float u_src_decay;
    uniform float u_src_harmonics;
    uniform float u_src_thickness;
    uniform float u_src_color_shift;
    uniform float u_rms;
    uniform float u_beatPhase;
    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        float fx = 1.0 + u_src_freq_x * 7.0;
        float fy = 1.0 + u_src_freq_y * 7.0;
        float ph = u_src_phase * 6.28318 + u_beatPhase * 6.28318;
        int nh = int(u_src_harmonics * 7.0) + 1;
        float thick = 0.03 + u_src_thickness * 0.12;
        float t = u_time * 0.5;
        float acc = 0.0;
        for (int h = 0; h < 8; h++) {
            if (h >= nh) break;
            float hf = float(h + 1);
            float minDist = 1e9;
            float closestT = 0.0;
            // Dense sampling along the curve
            for (int i = 0; i < 400; i++) {
                float s = float(i) / 400.0;
                float theta = s * 6.28318 * 3.0 + t * hf;
                vec2 lp = vec2(sin(fx * theta + ph * hf) * 0.85,
                               sin(fy * theta) * 0.85);
                float d = length(uv - lp);
                if (d < minDist) { minDist = d; closestT = s; }
            }
            // Glow with trail decay
            float trailFade = 1.0 - closestT * u_src_decay;
            float glow = exp(-minDist / (thick * 0.5)) * max(trailFade, 0.3);
            acc += glow / hf;
        }
        acc = clamp(acc, 0.0, 1.0);
        float hue = u_src_color_shift + acc * 0.3;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= acc * (0.8 + u_rms * 0.4);
        fragColor = vec4(col, 1.0);
    }
)";

// P17.2 — Fermat Spiral Garden (Math)
inline const char* sourceFermatSpiral = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_count;
    uniform float u_src_divergence;
    uniform float u_src_growth;
    uniform float u_src_pulse;
    uniform float u_src_color_spread;
    uniform float u_src_shape;
    uniform float u_rms;
    uniform float u_beatPhase;
    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        int N = int(50.0 + u_src_count * 450.0);
        float goldenAngle = mix(2.0, 3.0, u_src_divergence);  // ~2.399 = golden angle
        float t = u_time * (0.2 + u_src_growth * 2.0);
        float pulseAmt = u_src_pulse * 0.5;
        float acc = 0.0;
        float hueAcc = 0.0;
        for (int i = 0; i < 500; i++) {
            if (i >= N) break;
            float fi = float(i);
            float angle = fi * goldenAngle + t * 0.2;
            float r = sqrt(fi / float(N)) * 0.9;
            vec2 pos = vec2(cos(angle), sin(angle)) * r;
            float d = length(uv - pos);
            float baseSize = 0.02 + 0.03 * (1.0 - r);
            float pulse = 1.0 + pulseAmt * sin(u_beatPhase * 6.28318 + fi * 0.1);
            float size = baseSize * pulse * (0.8 + u_rms * 0.4);
            int shape = int(u_src_shape * 3.0);
            float mask;
            if (shape == 0) { // Circle
                mask = smoothstep(size, size * 0.5, d);
            } else if (shape == 1) { // Square
                vec2 dd = abs(uv - pos);
                mask = smoothstep(size, size * 0.5, max(dd.x, dd.y));
            } else { // Petal
                vec2 lp = uv - pos;
                float a = atan(lp.y, lp.x) - angle;
                float petalR = size * (0.5 + 0.5 * cos(a * 2.0));
                mask = smoothstep(petalR, petalR * 0.5, d);
            }
            acc += mask;
            hueAcc += mask * fi / float(N) * u_src_color_spread;
        }
        acc = clamp(acc, 0.0, 1.0);
        float hue = hueAcc + u_time * 0.05;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col * acc, 1.0);
    }
)";

// P17.3 — Hyperbolic Tiling (Math) — uses noise for SDF
inline const char* sourceHyperbolicTiling = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_p_sides;
    uniform float u_src_q_order;
    uniform float u_src_rotation;
    uniform float u_src_zoom;
    uniform float u_src_color_scheme;
    uniform float u_src_line_width;
    uniform float u_rms;
    uniform float u_beatPhase;

    // Complex multiply
    vec2 cmul(vec2 a, vec2 b) { return vec2(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x); }
    // Möbius transform: (z - a) / (1 - conj(a)*z)
    vec2 mobius(vec2 z, vec2 a) {
        vec2 num = z - a;
        vec2 den = vec2(1.0, 0.0) - cmul(vec2(a.x, -a.y), z);
        float d2 = dot(den, den);
        return cmul(num, vec2(den.x, -den.y)) / max(d2, 1e-8);
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        float zoom = 0.5 + u_src_zoom * 1.5;
        uv /= zoom;

        // Apply rotation via Möbius transform in the disk
        float rotAngle = u_time * (u_src_rotation - 0.5) * 0.5;
        float rotR = 0.3 + u_beatPhase * 0.1;
        vec2 rotCenter = vec2(cos(rotAngle), sin(rotAngle)) * rotR;
        uv = mobius(uv, rotCenter);

        float r = length(uv);
        if (r >= 0.98) {
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }

        int P = int(u_src_p_sides * 5.0) + 3;  // 3-8
        int Q = int(u_src_q_order * 5.0) + 3;   // 3-8

        // Reflect into fundamental domain
        float angle = atan(uv.y, uv.x);
        float sector = 6.28318 / float(P);
        int cellIdx = 0;
        for (int i = 0; i < 30; i++) {
            angle = atan(uv.y, uv.x);
            if (angle < 0.0) angle += 6.28318;
            float sectorId = floor(angle / sector);
            float localAngle = angle - sectorId * sector;
            cellIdx += int(sectorId);

            if (localAngle > sector * 0.5) {
                localAngle = sector - localAngle;
                cellIdx++;
            }
            float cr = length(uv);
            uv = vec2(cos(localAngle), sin(localAngle)) * cr;

            // Circle inversion for hyperbolic reflection
            float invR = cos(3.14159 / float(P)) / cos(3.14159 / float(Q));
            vec2 center = vec2(invR, 0.0);
            vec2 diff = uv - center;
            float d2 = dot(diff, diff);
            float circR = sqrt(abs(invR * invR - 1.0));
            if (d2 < circR * circR) {
                uv = center + diff * (circR * circR / d2);
                cellIdx++;
            } else {
                break;
            }
        }

        float edgeDist = min(abs(uv.y), abs(length(uv) - cos(3.14159/float(P))/cos(3.14159/float(Q))));
        float lineW = 0.005 + u_src_line_width * 0.04;
        float edge = smoothstep(lineW, 0.0, edgeDist);

        float fill = float(cellIdx % 2);
        float colScheme = u_src_color_scheme;
        vec3 col;
        if (colScheme < 0.33) {
            col = mix(vec3(0.1, 0.15, 0.3), vec3(0.8, 0.6, 0.2), fill);
        } else if (colScheme < 0.67) {
            float hue = float(cellIdx) * 0.1 + u_time * 0.05;
            col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        } else {
            float hue = float(cellIdx) * 0.08;
            col = 0.5 + 0.5 * cos(6.28318 * (hue + u_rms * 0.5 + vec3(0.0, 0.33, 0.67)));
        }
        col = mix(col, vec3(1.0), edge);

        // Fade at disk boundary
        float diskFade = smoothstep(0.98, 0.9, r);
        fragColor = vec4(col * diskFade, 1.0);
    }
)";

// P17.4 — Penrose Pulse (Math)
inline const char* sourcePenrosePulse = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_generation;
    uniform float u_src_ripple_speed;
    uniform float u_src_color_mode;
    uniform float u_src_edge_glow;
    uniform float u_src_morph;
    uniform float u_rms;
    uniform float u_beatPhase;
    uniform float u_onsetStrength;

    // Penrose tiling via de Bruijn's method (dual grid)
    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        float scale = 3.0 + u_src_generation * 8.0;
        uv *= scale;

        // 5 directional grid lines for Penrose P3 tiling
        float minEdge = 1e9;
        float tileIdx = 0.0;
        for (int k = 0; k < 5; k++) {
            float angle = float(k) * 3.14159 / 5.0 + u_src_morph * 0.3;
            vec2 dir = vec2(cos(angle), sin(angle));
            float proj = dot(uv, dir);
            float grid = fract(proj);
            float edge = min(grid, 1.0 - grid);
            minEdge = min(minEdge, edge);
            tileIdx += floor(proj);
        }

        float edgeW = 0.02 + u_src_edge_glow * 0.08;
        float edge = smoothstep(edgeW, 0.0, minEdge);

        // Ripple from center
        float dist = length(uv) / scale;
        float ripple = sin(dist * 20.0 - u_time * (1.0 + u_src_ripple_speed * 5.0) + u_onsetStrength * 3.0);
        ripple = ripple * 0.5 + 0.5;

        float colMode = u_src_color_mode;
        vec3 col;
        float tileHash = fract(sin(tileIdx * 127.1) * 43758.5453);
        if (colMode < 0.33) {
            // Two-color
            col = mix(vec3(0.15, 0.1, 0.3), vec3(0.9, 0.7, 0.2), step(0.5, tileHash));
        } else if (colMode < 0.67) {
            // Rainbow
            col = 0.5 + 0.5 * cos(6.28318 * (tileHash + vec3(0.0, 0.33, 0.67)));
        } else {
            // Audio-mapped
            col = 0.5 + 0.5 * cos(6.28318 * (tileHash + u_rms + vec3(0.0, 0.33, 0.67)));
        }

        col *= 0.3 + ripple * 0.7;
        col = mix(col, vec3(1.0), edge * u_src_edge_glow);
        col *= 0.7 + u_rms * 0.5;

        fragColor = vec4(col, 1.0);
    }
)";

// P17.5 — Moire Interference (Geometric)
inline const char* sourceMoireInterference = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_pattern;
    uniform float u_src_frequency;
    uniform float u_src_offset_x;
    uniform float u_src_offset_y;
    uniform float u_src_rotation;
    uniform float u_src_zoom;
    uniform float u_src_color_shift;
    uniform float u_rms;
    uniform float u_spectralCentroid;

    float moirePattern(vec2 p, float freq, int mode) {
        if (mode == 0) { // Lines
            return sin(p.x * freq) * 0.5 + 0.5;
        } else if (mode == 1) { // Dots
            vec2 g = sin(p * freq);
            return (g.x * g.y) * 0.5 + 0.5;
        } else { // Rings
            return sin(length(p) * freq) * 0.5 + 0.5;
        }
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        float z = 0.5 + u_src_zoom * 2.0;
        uv *= z;
        float freq = 10.0 + u_src_frequency * 60.0;
        int mode = int(u_src_pattern * 2.0);
        float t = u_time * 0.3;

        // Layer 1
        float p1 = moirePattern(uv, freq, mode);

        // Layer 2 with offset and rotation
        float rot = (u_src_rotation - 0.5) * 0.5 + t * 0.1;
        float c = cos(rot), s = sin(rot);
        vec2 uv2 = mat2(c, -s, s, c) * uv;
        uv2 += vec2((u_src_offset_x - 0.5) * 2.0 + u_spectralCentroid * 0.0001,
                     (u_src_offset_y - 0.5) * 2.0);
        float p2 = moirePattern(uv2, freq, mode);

        float moire = abs(p1 - p2);
        float hue = u_src_color_shift + moire * 0.3 + u_time * 0.02;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= moire * (0.8 + u_rms * 0.5);
        fragColor = vec4(col, 1.0);
    }
)";

// P17.6 — Crystal Cavern (3D ray-marched)
inline const char* sourceCrystalCavern = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_speed;
    uniform float u_src_crystal_size;
    uniform float u_src_reflectivity;
    uniform float u_src_light_color;
    uniform float u_src_fog;
    uniform float u_src_complexity;
    uniform float u_rms;
    uniform float u_bass;

    float hash(vec3 p) { return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453); }

    float caveDE(vec3 p, float crystalSize) {
        // Menger-like folded cave
        float scale = mix(1.5, 3.0, crystalSize);
        int iters = int(u_src_complexity * 4.0) + 2;
        float d = length(max(abs(p) - vec3(1.0), 0.0));
        float s = 1.0;
        for (int i = 0; i < 6; i++) {
            if (i >= iters) break;
            p = abs(p);
            if (p.x < p.y) p.xy = p.yx;
            if (p.x < p.z) p.xz = p.zx;
            if (p.y < p.z) p.yz = p.zy;
            p = p * scale - vec3(scale - 1.0);
            if (p.z < -0.5 * (scale - 1.0)) p.z += scale - 1.0;
            s *= scale;
        }
        return length(p) / s - 0.01;
    }

    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        float t = u_time * (0.2 + u_src_speed * 0.8);
        vec3 ro = vec3(sin(t * 0.3) * 0.5, cos(t * 0.2) * 0.3, t);
        vec3 rd = normalize(vec3(uv, 0.8));
        // Rotate camera
        float ca = t * 0.1;
        float cc = cos(ca), ss = sin(ca);
        rd.xz = mat2(cc, -ss, ss, cc) * rd.xz;

        float totalDist = 0.0;
        vec3 col = vec3(0.0);
        float crystalSize = u_src_crystal_size;

        for (int i = 0; i < 80; i++) {
            vec3 p = ro + rd * totalDist;
            float d = caveDE(p, crystalSize);
            if (d < 0.002) {
                // Normal via gradient
                vec2 e = vec2(0.001, 0.0);
                vec3 n = normalize(vec3(
                    caveDE(p+e.xyy, crystalSize) - caveDE(p-e.xyy, crystalSize),
                    caveDE(p+e.yxy, crystalSize) - caveDE(p-e.yxy, crystalSize),
                    caveDE(p+e.yyx, crystalSize) - caveDE(p-e.yyx, crystalSize)
                ));
                // Light
                vec3 lightDir = normalize(vec3(sin(t), 0.5, cos(t)));
                float diff = max(dot(n, lightDir), 0.0);
                float spec = pow(max(dot(reflect(rd, n), lightDir), 0.0), 16.0 + u_src_reflectivity * 48.0);
                float hue = u_src_light_color;
                vec3 lightCol = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
                col = lightCol * (diff * 0.6 + spec * u_src_reflectivity + 0.1);
                // Fog
                float fog = exp(-totalDist * (0.1 + u_src_fog * 0.5));
                col *= fog;
                col *= 0.8 + u_bass * 0.4;
                break;
            }
            totalDist += d;
            if (totalDist > 20.0) break;
        }
        fragColor = vec4(col, 1.0);
    }
)";

// P17.7 — Infinite Corridor (3D ray-marched)
inline const char* sourceInfiniteCorridor = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_speed;
    uniform float u_src_width;
    uniform float u_src_wall_pattern;
    uniform float u_src_light_spacing;
    uniform float u_src_light_intensity;
    uniform float u_src_color;
    uniform float u_rms;
    uniform float u_onsetStrength;
    uniform float u_beatPhase;

    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        float t = u_time * (0.5 + u_src_speed * 2.0);
        float w = 0.3 + u_src_width * 0.7;
        float h = 0.3 + u_src_width * 0.5;

        // Ray from camera
        vec3 rd = normalize(vec3(uv, 1.0));
        vec3 ro = vec3(0.0, 0.0, t);

        // Intersect with 4 planes (walls, floor, ceiling)
        float tMin = 1e9;
        vec3 hitNormal = vec3(0.0);
        vec3 hitPos = vec3(0.0);
        // Floor y = -h
        float tf = (-h - ro.y) / rd.y;
        if (tf > 0.0 && tf < tMin) { tMin = tf; hitNormal = vec3(0,1,0); }
        // Ceiling y = h
        float tc = (h - ro.y) / rd.y;
        if (tc > 0.0 && tc < tMin) { tMin = tc; hitNormal = vec3(0,-1,0); }
        // Left wall x = -w
        float tl = (-w - ro.x) / rd.x;
        if (tl > 0.0 && tl < tMin) { tMin = tl; hitNormal = vec3(1,0,0); }
        // Right wall x = w
        float tr = (w - ro.x) / rd.x;
        if (tr > 0.0 && tr < tMin) { tMin = tr; hitNormal = vec3(-1,0,0); }

        if (tMin > 100.0) {
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }

        hitPos = ro + rd * tMin;

        // Wall pattern
        int pattern = int(u_src_wall_pattern * 3.0);
        float patVal = 0.0;
        vec2 wallUV;
        if (abs(hitNormal.y) > 0.5) wallUV = hitPos.xz;
        else wallUV = hitPos.yz;

        if (pattern == 0) { // Grid
            vec2 g = abs(fract(wallUV * 2.0) - 0.5);
            patVal = smoothstep(0.02, 0.05, min(g.x, g.y));
        } else if (pattern == 1) { // Brick
            vec2 brickUV = wallUV * vec2(2.0, 4.0);
            brickUV.x += step(1.0, mod(floor(brickUV.y), 2.0)) * 0.5;
            vec2 g = abs(fract(brickUV) - 0.5);
            patVal = smoothstep(0.02, 0.06, min(g.x, g.y));
        } else if (pattern == 2) { // Ribbed
            patVal = sin(wallUV.x * 20.0) * 0.3 + 0.7;
        } else { // Smooth
            patVal = 0.7;
        }

        // Ceiling lights
        float lightDist = 1.0 + u_src_light_spacing * 4.0;
        float lightZ = mod(hitPos.z, lightDist) - lightDist * 0.5;
        float lightOn = step(abs(hitNormal.y), 0.5) * 0.0 + step(0.5, hitNormal.y) * 0.0 +
                         step(0.5, -hitNormal.y) * 1.0;  // ceiling only
        float lightMask = 0.0;
        if (hitNormal.y < -0.5) { // Ceiling
            lightMask = smoothstep(0.3, 0.0, abs(lightZ)) * smoothstep(w*0.5, 0.0, abs(hitPos.x));
            // Flash on beat
            float beatFlash = 1.0 + u_onsetStrength * 2.0;
            lightMask *= u_src_light_intensity * beatFlash;
        }

        // Lighting
        float ambient = 0.05;
        float depth = tMin;
        float fog = exp(-depth * 0.15);

        float hue = u_src_color;
        vec3 wallCol = vec3(0.15, 0.15, 0.2) * patVal;
        vec3 lightCol = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));

        // Point lighting from ceiling fixtures
        float nearestLight = floor(hitPos.z / lightDist + 0.5) * lightDist;
        vec3 lPos = vec3(0.0, h - 0.01, nearestLight);
        vec3 lDir = normalize(lPos - hitPos);
        float lDist = length(lPos - hitPos);
        float atten = 1.0 / (1.0 + lDist * lDist * 0.3);
        float diff = max(dot(hitNormal, lDir), 0.0);

        vec3 col = wallCol * (ambient + diff * atten * u_src_light_intensity) +
                   lightCol * lightMask;
        col *= fog;
        col *= 0.8 + u_rms * 0.4;

        fragColor = vec4(col, 1.0);
    }
)";

// P17.8 — Orbit Chamber (3D ray-marched)
inline const char* sourceOrbitChamber = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_obj_count;
    uniform float u_src_obj_type;
    uniform float u_src_orbit_speed;
    uniform float u_src_orbit_radius;
    uniform float u_src_material;
    uniform float u_src_light_orbit;
    uniform float u_src_color_shift;
    uniform float u_rms;
    uniform float u_beatPhase;

    float sdSphere(vec3 p, float r) { return length(p) - r; }
    float sdBox3(vec3 p, vec3 b) { vec3 q = abs(p) - b; return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0); }
    float sdTorus3(vec3 p, vec2 t) { vec2 q = vec2(length(p.xz)-t.x, p.y); return length(q)-t.y; }

    float sceneSDF(vec3 p, float t, int nObj, int objType, float orbitR) {
        float d = 1e9;
        for (int i = 0; i < 8; i++) {
            if (i >= nObj) break;
            float angle = float(i) * 6.28318 / float(nObj) + t;
            float yOff = sin(angle * 2.0 + t * 0.5) * 0.3;
            vec3 objPos = vec3(cos(angle) * orbitR, yOff, sin(angle) * orbitR);
            vec3 q = p - objPos;
            float objD;
            if (objType == 0) objD = sdSphere(q, 0.25);
            else if (objType == 1) objD = sdBox3(q, vec3(0.2));
            else if (objType == 2) objD = sdTorus3(q, vec2(0.2, 0.07));
            else { // Mixed
                if (i % 3 == 0) objD = sdSphere(q, 0.25);
                else if (i % 3 == 1) objD = sdBox3(q, vec3(0.2));
                else objD = sdTorus3(q, vec2(0.2, 0.07));
            }
            d = min(d, objD);
        }
        return d;
    }

    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        float t = u_time * (0.3 + u_src_orbit_speed * 1.5);
        int nObj = int(u_src_obj_count * 7.0) + 1;
        int objType = int(u_src_obj_type * 3.0);
        float orbitR = 0.3 + u_src_orbit_radius * 0.7;

        // Camera
        float camAngle = t * 0.2;
        vec3 ro = vec3(cos(camAngle) * 1.5, 0.5, sin(camAngle) * 1.5);
        vec3 target = vec3(0.0);
        vec3 fwd = normalize(target - ro);
        vec3 right = normalize(cross(fwd, vec3(0,1,0)));
        vec3 up = cross(right, fwd);
        vec3 rd = normalize(uv.x * right + uv.y * up + 1.2 * fwd);

        // Light
        float lt = u_time * (0.5 + u_src_light_orbit * 2.0);
        vec3 lightPos = vec3(cos(lt) * 2.0, 1.0, sin(lt) * 2.0);

        float totalDist = 0.0;
        vec3 col = vec3(0.0);
        for (int i = 0; i < 96; i++) {
            vec3 p = ro + rd * totalDist;
            float d = sceneSDF(p, t, nObj, objType, orbitR);
            if (d < 0.001) {
                vec2 e = vec2(0.001, 0.0);
                vec3 n = normalize(vec3(
                    sceneSDF(p+e.xyy, t, nObj, objType, orbitR) - sceneSDF(p-e.xyy, t, nObj, objType, orbitR),
                    sceneSDF(p+e.yxy, t, nObj, objType, orbitR) - sceneSDF(p-e.yxy, t, nObj, objType, orbitR),
                    sceneSDF(p+e.yyx, t, nObj, objType, orbitR) - sceneSDF(p-e.yyx, t, nObj, objType, orbitR)
                ));
                vec3 lDir = normalize(lightPos - p);
                float diff = max(dot(n, lDir), 0.0);
                float spec = pow(max(dot(reflect(rd, n), lDir), 0.0), mix(8.0, 64.0, u_src_material));
                float hue = u_src_color_shift + length(p.xz) * 0.3;
                vec3 baseCol = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
                float chrome = u_src_material;
                col = baseCol * diff * (1.0 - chrome * 0.5) + vec3(1.0) * spec * chrome + baseCol * 0.05;
                col *= 0.8 + u_rms * 0.4;
                break;
            }
            totalDist += d;
            if (totalDist > 10.0) break;
        }
        fragColor = vec4(col, 1.0);
    }
)";

// P17.9 — Astral Grid (Geometric)
inline const char* sourceAstralGrid = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_grid_size;
    uniform float u_src_scroll_speed;
    uniform float u_src_tilt;
    uniform float u_src_warp;
    uniform float u_src_glow;
    uniform float u_src_horizon_color;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_beatPhase;
    void main() {
        vec2 uv = v_texCoord;
        float tiltAmt = 0.1 + u_src_tilt * 0.8;
        // Perspective transform: y maps to depth
        float y = uv.y - (1.0 - tiltAmt);
        if (y < 0.001) {
            // Sky / horizon gradient
            float hue = u_src_horizon_color;
            vec3 skyCol = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
            skyCol *= 0.1 + smoothstep(0.0, -0.3, y - 0.001) * 0.2;
            fragColor = vec4(skyCol, 1.0);
            return;
        }
        float depth = tiltAmt / max(y, 0.001);
        float x = (uv.x - 0.5) * depth;
        float z = depth;
        float t = u_time * (0.5 + u_src_scroll_speed * 3.0);
        z += t;
        // Sine warp
        float warpAmt = u_src_warp * 0.5;
        float yWarp = sin(z * 0.5 + u_time) * warpAmt * (0.5 + u_bass * 1.0);
        // Grid lines
        float gridS = 0.5 + u_src_grid_size * 2.0;
        vec2 gp = vec2(x, z) / gridS;
        vec2 gridDist = abs(fract(gp) - 0.5);
        float lineW = 0.03 * depth;
        float gridLine = smoothstep(lineW, 0.0, min(gridDist.x, gridDist.y));
        // Glow
        float glow = gridLine * (u_src_glow * 2.0 + 0.5);
        // Depth fog
        float fog = exp(-depth * 0.15);
        // Beat pulse
        float pulse = 1.0 + u_beatPhase * 0.3 * u_rms;
        float hue = u_src_horizon_color + depth * 0.02;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= glow * fog * pulse;
        fragColor = vec4(col, 1.0);
    }
)";

// P17.10 — Radial Burst (Geometric)
inline const char* sourceRadialBurst = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_ray_count;
    uniform float u_src_length;
    uniform float u_src_rotation;
    uniform float u_src_width;
    uniform float u_src_taper;
    uniform float u_src_glow;
    uniform float u_src_color_shift;
    uniform float u_rms;
    uniform float u_onsetStrength;
    uniform float u_beatPhase;
    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        int nRays = int(u_src_ray_count * 60.0) + 4;
        float rayLen = 0.2 + u_src_length * 0.8 + u_onsetStrength * 0.3;
        float rot = u_time * (u_src_rotation - 0.5) * 2.0;
        float rayW = 0.01 + u_src_width * 0.06;
        float taper = u_src_taper;
        float r = length(uv);
        float angle = atan(uv.y, uv.x) - rot;
        float sector = 6.28318 / float(nRays);
        float nearestAngle = floor(angle / sector + 0.5) * sector;
        float angleDist = abs(angle - nearestAngle);
        // Taper: width narrows with distance
        float taperW = rayW * mix(1.0, max(1.0 - r / rayLen, 0.0), taper);
        float mask = smoothstep(taperW, taperW * 0.3, angleDist * r);
        mask *= smoothstep(rayLen, rayLen * 0.5, r);
        mask *= smoothstep(0.0, 0.05, r); // hole in center
        // Glow
        float glowMask = mask + exp(-angleDist * r * 20.0) * exp(-r * 3.0) * u_src_glow;
        float hue = u_src_color_shift + angle * 0.05;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= glowMask * (0.7 + u_rms * 0.6);
        fragColor = vec4(col, 1.0);
    }
)";

// P17.11 — Hex Grid (Geometric)
inline const char* sourceHexGrid = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_cell_size;
    uniform float u_src_pattern;
    uniform float u_src_fill;
    uniform float u_src_edge_width;
    uniform float u_src_rotation;
    uniform float u_src_color_mode;
    uniform float u_rms;
    uniform float u_beatPhase;
    uniform float u_bass;
    uniform float u_mid;
    uniform float u_high;

    vec4 hexCoords(vec2 uv) {
        // Returns (hex center, local coords within hex)
        vec2 s = vec2(1.0, 1.7320508);  // 1, sqrt(3)
        vec2 a = mod(uv, s) - s * 0.5;
        vec2 b = mod(uv - s * 0.5, s) - s * 0.5;
        vec2 gv = (dot(a,a) < dot(b,b)) ? a : b;
        vec2 id = uv - gv;
        return vec4(gv, id);
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        float rot = u_src_rotation * 3.14159;
        float c = cos(rot), s = sin(rot);
        uv = mat2(c, -s, s, c) * uv;
        float sz = 0.15 + u_src_cell_size * 0.5;
        uv /= sz;
        vec4 hc = hexCoords(uv);
        vec2 gv = hc.xy;
        vec2 id = hc.zw;
        float cellHash = fract(sin(dot(id, vec2(127.1, 311.7))) * 43758.5453);
        float dist = length(gv);
        float hexR = 0.5;
        // Hex SDF (approximate)
        vec2 q = abs(gv);
        float hexDist = max(q.x * 0.866025 + q.y * 0.5, q.y) - hexR;
        float edgeW = 0.02 + u_src_edge_width * 0.1;
        float edge = smoothstep(edgeW, 0.0, abs(hexDist));
        // Fill pattern
        int pattern = int(u_src_pattern * 3.0);
        float fillVal = 0.0;
        float distFromCenter = length(id * sz);
        if (pattern == 0) { // Random
            fillVal = step(1.0 - u_src_fill, cellHash);
        } else if (pattern == 1) { // Wave
            fillVal = sin(distFromCenter * 3.0 - u_time * 2.0) * 0.5 + 0.5;
            fillVal = step(1.0 - u_src_fill, fillVal);
        } else if (pattern == 2) { // Spiral
            float a = atan(id.y, id.x);
            fillVal = sin(a * 3.0 + distFromCenter * 5.0 - u_time * 2.0) * 0.5 + 0.5;
            fillVal = step(1.0 - u_src_fill, fillVal);
        } else { // Audio
            float ringIdx = floor(distFromCenter / 0.5);
            float bands[3] = float[3](u_bass, u_mid, u_high);
            int bandIdx = int(mod(ringIdx, 3.0));
            fillVal = step(0.3, bands[bandIdx]);
        }
        float mask = fillVal * step(hexDist, 0.0);
        // Color
        float colMode = u_src_color_mode;
        vec3 col;
        if (colMode < 0.33) { // Monochrome
            col = vec3(0.8, 0.9, 1.0);
        } else if (colMode < 0.67) { // Rainbow
            float hue = cellHash + u_time * 0.05;
            col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        } else { // Audio frequency
            float hue = distFromCenter * 0.2;
            col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        }
        col *= mask + edge * u_src_edge_width;
        col *= 0.7 + u_rms * 0.6;
        fragColor = vec4(col, 1.0);
    }
)";

// P17.12 — Sacred Geometry (Geometric)
inline const char* sourceSacredGeometry = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_pattern;
    uniform float u_src_rotation;
    uniform float u_src_breathe;
    uniform float u_src_line_width;
    uniform float u_src_glow;
    uniform float u_src_reveal;
    uniform float u_src_color_shift;
    uniform float u_rms;
    uniform float u_beatPhase;
    uniform float u_phrasePhase;

    float circleSDF(vec2 p, vec2 center, float r) { return abs(length(p - center) - r); }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        float rot = u_time * (u_src_rotation - 0.5) * 0.5;
        float c = cos(rot), s = sin(rot);
        uv = mat2(c, -s, s, c) * uv;
        // Breathe
        float breathe = 1.0 + sin(u_beatPhase * 6.28318) * u_src_breathe * 0.15;
        uv /= breathe;
        float lineW = 0.005 + u_src_line_width * 0.02;
        float reveal = u_src_reveal * 0.99 + 0.01 + u_phrasePhase * (1.0 - u_src_reveal);
        int pattern = int(u_src_pattern * 4.0);
        float minDist = 1e9;
        float totalCircles = 0.0;
        if (pattern == 0) { // Flower of Life
            float r = 0.3;
            totalCircles = 19.0;
            float drawn = totalCircles * reveal;
            int idx = 0;
            // Center circle
            if (float(idx) < drawn) minDist = min(minDist, circleSDF(uv, vec2(0.0), r));
            idx++;
            // 6 surrounding
            for (int i = 0; i < 6; i++) {
                if (float(idx) >= drawn) break;
                float a = float(i) * 6.28318 / 6.0;
                minDist = min(minDist, circleSDF(uv, vec2(cos(a), sin(a)) * r, r));
                idx++;
            }
            // 12 outer ring
            for (int i = 0; i < 12; i++) {
                if (float(idx) >= drawn) break;
                float a = float(i) * 6.28318 / 12.0 + 3.14159/12.0;
                minDist = min(minDist, circleSDF(uv, vec2(cos(a), sin(a)) * r * 1.732, r));
                idx++;
            }
        } else if (pattern == 1) { // Seed of Life
            float r = 0.35;
            totalCircles = 7.0;
            float drawn = totalCircles * reveal;
            if (0.0 < drawn) minDist = min(minDist, circleSDF(uv, vec2(0.0), r));
            for (int i = 0; i < 6; i++) {
                if (float(i+1) >= drawn) break;
                float a = float(i) * 6.28318 / 6.0;
                minDist = min(minDist, circleSDF(uv, vec2(cos(a), sin(a)) * r, r));
            }
        } else if (pattern == 2) { // Metatron's Cube
            float r = 0.5;
            totalCircles = 13.0;
            float drawn = totalCircles * reveal;
            if (0.0 < drawn) minDist = min(minDist, circleSDF(uv, vec2(0.0), 0.01)); // center dot
            // Inner hex
            for (int i = 0; i < 6; i++) {
                if (float(i+1) >= drawn) break;
                float a = float(i) * 6.28318 / 6.0;
                vec2 p1 = vec2(cos(a), sin(a)) * r * 0.5;
                minDist = min(minDist, circleSDF(uv, p1, 0.01));
                // Lines connecting
                for (int j = i+1; j < 6; j++) {
                    float a2 = float(j) * 6.28318 / 6.0;
                    vec2 p2 = vec2(cos(a2), sin(a2)) * r * 0.5;
                    vec2 pa = uv - p1, ba = p2 - p1;
                    float h2 = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
                    minDist = min(minDist, length(pa - ba * h2));
                }
            }
            // Outer hex
            for (int i = 0; i < 6; i++) {
                if (float(i+7) >= drawn) break;
                float a = float(i) * 6.28318 / 6.0 + 3.14159/6.0;
                minDist = min(minDist, circleSDF(uv, vec2(cos(a), sin(a)) * r, 0.01));
            }
        } else if (pattern == 3) { // Sri Yantra (simplified triangles)
            float sz = 0.6;
            totalCircles = 9.0;
            float drawn = totalCircles * reveal;
            for (int i = 0; i < 9; i++) {
                if (float(i) >= drawn) break;
                float t2 = float(i) / 9.0;
                float triSize = sz * (1.0 - t2 * 0.7);
                float yOff = (t2 - 0.5) * 0.3;
                float flip = (i % 2 == 0) ? 1.0 : -1.0;
                // Triangle as 3 line segments
                vec2 p0 = vec2(0.0, triSize * flip) + vec2(0.0, yOff);
                vec2 p1t = vec2(-triSize * 0.866, -triSize * 0.5 * flip) + vec2(0.0, yOff);
                vec2 p2t = vec2( triSize * 0.866, -triSize * 0.5 * flip) + vec2(0.0, yOff);
                // Segments
                for (int seg = 0; seg < 3; seg++) {
                    vec2 sa = (seg==0) ? p0 : (seg==1) ? p1t : p2t;
                    vec2 sb = (seg==0) ? p1t : (seg==1) ? p2t : p0;
                    vec2 pa2 = uv - sa, ba2 = sb - sa;
                    float h2 = clamp(dot(pa2, ba2) / dot(ba2, ba2), 0.0, 1.0);
                    minDist = min(minDist, length(pa2 - ba2 * h2));
                }
            }
        } else { // Fibonacci spiral
            float r2 = 0.0;
            float golden = 1.6180339887;
            totalCircles = 30.0;
            float drawn = totalCircles * reveal;
            for (int i = 0; i < 30; i++) {
                if (float(i) >= drawn) break;
                float fi = float(i);
                r2 = 0.02 * pow(golden, fi * 0.15);
                float a = fi * 2.399963; // golden angle
                vec2 center = vec2(cos(a), sin(a)) * r2;
                minDist = min(minDist, circleSDF(uv, center, r2 * 0.3));
            }
        }

        float mask = smoothstep(lineW + u_src_glow * 0.05, 0.0, minDist);
        float hue = u_src_color_shift + minDist * 3.0 + u_time * 0.02;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= mask * (0.7 + u_rms * 0.6);
        fragColor = vec4(col, 1.0);
    }
)";

// P17.13 — Fire Wall (Nature) — needs noise
inline const char* sourceFireWall = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_height;
    uniform float u_src_turbulence;
    uniform float u_src_speed;
    uniform float u_src_temperature;
    uniform float u_src_density;
    uniform float u_src_wind;
    uniform float u_rms;
    uniform float u_spectralFlux;

    void main() {
        vec2 uv = v_texCoord;
        float t = u_time * (0.5 + u_src_speed * 3.0);
        // Scroll upward
        vec2 noiseUV = uv;
        noiseUV.y -= t;
        noiseUV.x += sin(uv.y * 3.0 + t) * u_src_wind * 0.3;

        // Multi-octave noise for fire shape
        float turb = u_src_turbulence;
        float n = 0.0;
        float amp = 1.0;
        float freq = 3.0 + turb * 5.0;
        for (int i = 0; i < 5; i++) {
            float nx = sin(noiseUV.x * freq + noiseUV.y * freq * 0.7 + t * float(i) * 0.5) *
                       cos(noiseUV.y * freq * 1.3 - t * float(i) * 0.3);
            n += nx * amp;
            amp *= 0.5;
            freq *= 2.0;
        }
        n = n * 0.5 + 0.5;

        // Height mask — fire rises from bottom
        float fireH = u_src_height * 0.9 + 0.1 + u_rms * 0.2;
        float heightMask = smoothstep(fireH, 0.0, uv.y);
        heightMask *= u_src_density * 1.5;
        float fire = n * heightMask;
        fire = clamp(fire, 0.0, 1.0);

        // Fire color palette (temperature controlled)
        vec3 col;
        float temp = u_src_temperature;
        if (temp < 0.5) {
            // Cool fire (blue → purple → white)
            col = mix(vec3(0.0, 0.0, 0.3), vec3(0.5, 0.2, 0.8), fire);
            col = mix(col, vec3(1.0), fire * fire);
        } else {
            // Hot fire (black → red → orange → yellow → white)
            col = mix(vec3(0.0), vec3(0.8, 0.1, 0.0), clamp(fire * 3.0, 0.0, 1.0));
            col = mix(col, vec3(1.0, 0.5, 0.0), clamp(fire * 3.0 - 1.0, 0.0, 1.0));
            col = mix(col, vec3(1.0, 1.0, 0.6), clamp(fire * 3.0 - 2.0, 0.0, 1.0));
        }
        col *= 0.8 + u_spectralFlux * 0.5;
        fragColor = vec4(col, 1.0);
    }
)";

// P17.14 — Water Caustics (Nature)
inline const char* sourceWaterCaustics = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_complexity;
    uniform float u_src_speed;
    uniform float u_src_brightness;
    uniform float u_src_color;
    uniform float u_src_distortion;
    uniform float u_src_scale;
    uniform float u_rms;
    uniform float u_beatPhase;

    void main() {
        vec2 uv = v_texCoord * 2.0 - 1.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        float sc = 1.0 + u_src_scale * 4.0;
        uv *= sc;
        float t = u_time * (0.3 + u_src_speed * 1.5);
        int layers = int(u_src_complexity * 4.0) + 2;
        float distort = u_src_distortion * 0.5;

        // Caustic pattern from overlapping sine waves
        float caustic = 0.0;
        for (int i = 0; i < 6; i++) {
            if (i >= layers) break;
            float fi = float(i);
            float angle = fi * 2.399 + t * 0.1;
            vec2 dir = vec2(cos(angle), sin(angle));
            float freq = 3.0 + fi * 1.5;
            float phase = t * (0.5 + fi * 0.2);
            vec2 distortedUV = uv + dir * sin(dot(uv, dir.yx) * distort) * distort;
            caustic += sin(dot(distortedUV, dir) * freq + phase);
        }
        caustic = caustic / float(layers);
        caustic = caustic * caustic; // sharpen
        caustic *= u_src_brightness * 2.0;
        caustic = clamp(caustic, 0.0, 1.0);

        // Water color
        float hue = u_src_color;
        vec3 waterCol;
        if (hue < 0.33) { // Blue
            waterCol = mix(vec3(0.0, 0.05, 0.15), vec3(0.2, 0.5, 0.8), caustic);
        } else if (hue < 0.67) { // Green
            waterCol = mix(vec3(0.0, 0.08, 0.05), vec3(0.2, 0.7, 0.5), caustic);
        } else { // Warm
            waterCol = mix(vec3(0.1, 0.05, 0.0), vec3(0.8, 0.6, 0.3), caustic);
        }
        waterCol += caustic * 0.5; // bright caustic highlights
        waterCol *= 0.8 + u_rms * 0.4;

        fragColor = vec4(waterCol, 1.0);
    }
)";

// P17.15 — Electric Arc (Nature)
inline const char* sourceElectricArc = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_arc_count;
    uniform float u_src_chaos;
    uniform float u_src_thickness;
    uniform float u_src_branches;
    uniform float u_src_glow;
    uniform float u_src_color;
    uniform float u_rms;
    uniform float u_onsetStrength;

    float hash(float n) { return fract(sin(n) * 43758.5453); }
    float hash2(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

    float lightning(vec2 uv, vec2 a, vec2 b, float chaos, float thick, float seed) {
        // Midpoint displacement lightning
        float minDist = 1e9;
        int steps = 16;
        vec2 prev = a;
        for (int i = 1; i <= 16; i++) {
            float t = float(i) / float(steps);
            vec2 mid = mix(a, b, t);
            // Displace perpendicular
            vec2 dir = normalize(b - a);
            vec2 perp = vec2(-dir.y, dir.x);
            float disp = (hash(seed + float(i) * 17.3 + floor(u_time * 10.0)) - 0.5) * 2.0;
            disp *= chaos * 0.3 * (1.0 - abs(t * 2.0 - 1.0)); // less at endpoints
            mid += perp * disp;
            // Distance to segment prev→mid
            vec2 pa = uv - prev, ba = mid - prev;
            float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
            float d = length(pa - ba * h);
            minDist = min(minDist, d);
            prev = mid;
        }
        return smoothstep(thick, 0.0, minDist);
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        int nArcs = int(u_src_arc_count * 5.0) + 1;
        float chaos = u_src_chaos;
        float thick = 0.005 + u_src_thickness * 0.02;
        float glowR = 0.02 + u_src_glow * 0.1;
        int nBranch = int(u_src_branches * 4.0);
        float intensity = u_onsetStrength * 2.0 + 1.0;

        float acc = 0.0;
        for (int a = 0; a < 6; a++) {
            if (a >= nArcs) break;
            float seed = float(a) * 37.7 + floor(u_time * 5.0 + 1.0);
            float angle = float(a) * 6.28318 / float(nArcs) + hash(float(a) * 13.1 + floor(u_time * 3.0 + 1.0)) * 1.0;
            vec2 start = vec2(cos(angle), sin(angle)) * 0.7;
            vec2 end2 = vec2(cos(angle + 2.5 + float(a) * 0.3), sin(angle + 2.5 + float(a) * 0.3)) * 0.7;

            float bolt = lightning(uv, start, end2, chaos, thick, seed);
            acc += bolt * intensity;

            // Branches
            for (int br = 0; br < 4; br++) {
                if (br >= nBranch) break;
                float bt = hash(seed + float(br) * 7.1) * 0.6 + 0.2;
                vec2 branchStart = mix(start, end2, bt);
                float brAngle = angle + (hash(seed + float(br) * 3.3) - 0.5) * 1.5;
                vec2 branchEnd = branchStart + vec2(cos(brAngle), sin(brAngle)) * 0.3;
                float branch = lightning(uv, branchStart, branchEnd, chaos * 0.7, thick * 0.5, seed + float(br) * 100.0);
                acc += branch * intensity * 0.5;
            }

            // Glow
            float boltDist = length(uv - mix(start, end2, clamp(dot(uv-start, end2-start)/dot(end2-start,end2-start), 0.0, 1.0)));
            acc += exp(-boltDist / glowR) * 0.3;
        }

        acc = clamp(acc, 0.0, 1.0);
        float hue = u_src_color;
        vec3 col;
        if (hue < 0.33) col = vec3(0.9, 0.95, 1.0); // White
        else if (hue < 0.67) col = vec3(0.4, 0.6, 1.0); // Blue
        else col = vec3(0.7, 0.4, 1.0); // Purple
        col *= acc;
        col *= 0.6 + u_rms * 0.8;
        fragColor = vec4(col, 1.0);
    }
)";

// P17.16 — Laser Scanner (Lighting)
inline const char* sourceLaserScanner = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_pattern;
    uniform float u_src_beam_count;
    uniform float u_src_color;
    uniform float u_src_speed;
    uniform float u_src_spread;
    uniform float u_src_flicker;
    uniform float u_rms;
    uniform float u_beatPhase;
    uniform float u_bass;

    float hash(float n) { return fract(sin(n) * 43758.5453); }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        int patt = int(u_src_pattern * 5.0);
        int nBeams = int(u_src_beam_count * 15.0) + 2;
        float t = u_time * (0.5 + u_src_speed * 3.0);
        float spreadAngle = u_src_spread * 3.14159;
        float beamW = 0.006;
        float acc = 0.0;

        for (int i = 0; i < 16; i++) {
            if (i >= nBeams) break;
            float fi = float(i);
            float angle;
            float beamLen = 2.0;

            if (patt == 0) { // Fan
                angle = -spreadAngle * 0.5 + spreadAngle * fi / max(float(nBeams-1), 1.0) + sin(t) * 0.3;
            } else if (patt == 1) { // Tunnel (concentric)
                angle = fi * 6.28318 / float(nBeams) + t * 0.5;
            } else if (patt == 2) { // Cone
                angle = fi * 6.28318 / float(nBeams);
                beamLen = 0.5 + 0.5 * sin(t + fi);
            } else if (patt == 3) { // Wave (audio waveform)
                angle = -1.5 + 3.0 * fi / float(nBeams);
                beamLen = 0.3 + u_rms * 1.0;
            } else if (patt == 4) { // Spiral
                float spiralT = t + fi * 0.5;
                angle = spiralT;
                beamLen = 0.2 + fi * 0.1;
            } else { // Abstract
                angle = sin(t + fi * 1.7) * 3.14159;
                beamLen = 0.5 + sin(t * 2.0 + fi) * 0.5;
            }

            vec2 dir = vec2(cos(angle), sin(angle));
            // Distance from point to ray from origin in direction dir
            float proj = dot(uv, dir);
            vec2 closest = dir * max(proj, 0.0);
            float d = length(uv - closest);
            // Only show if within beam length
            float len = clamp(proj / beamLen, 0.0, 1.0);
            float fade = 1.0 - len * 0.5;
            // Flicker
            float flick = 1.0 - u_src_flicker * 0.3 * hash(fi + floor(t * 10.0));
            float beam = smoothstep(beamW * 3.0, 0.0, d) * fade * flick * step(0.0, proj) * step(proj, beamLen);
            acc += beam;
        }

        acc = clamp(acc, 0.0, 1.0);
        float hue = u_src_color;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= acc * (0.7 + u_bass * 0.6);
        fragColor = vec4(col, 1.0);
    }
)";

// P17.17 — Scroll Plane (3D)
inline const char* sourceScrollPlane = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_speedx;
    uniform float u_src_speedy;
    uniform float u_src_scale;
    uniform float u_src_warp;
    uniform float u_src_color_shift;
    uniform float u_rms;
    uniform float u_beatPhase;
    void main() {
        vec2 uv = v_texCoord;
        float speedX = (u_src_speedx - 0.5) * 2.0;
        float speedY = (u_src_speedy - 0.5) * 2.0;
        float sc = mix(0.5, 4.0, u_src_scale);
        uv = fract(uv * sc + vec2(speedX, speedY) * u_time);
        // Perspective warp
        if (u_src_warp > 0.01) {
            float perspective = mix(1.0, 0.3, uv.y * u_src_warp);
            uv.x = 0.5 + (uv.x - 0.5) / perspective;
        }
        // Procedural grid pattern
        vec2 grid = abs(fract(uv * 8.0) - 0.5);
        float line = smoothstep(0.02, 0.0, min(grid.x, grid.y));
        float checker = step(0.5, mod(floor(uv.x * 8.0) + floor(uv.y * 8.0), 2.0));
        float pattern = mix(checker * 0.3 + 0.1, 1.0, line);
        float hue = u_src_color_shift + uv.y * 0.2 + u_time * 0.02;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= pattern * (0.7 + u_rms * 0.5);
        fragColor = vec4(col, 1.0);
    }
)";

// P17.18 — Rotating Cube Map (3D)
inline const char* sourceRotatingCubeMap = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_rotx;
    uniform float u_src_roty;
    uniform float u_src_scale;
    uniform float u_src_light;
    uniform float u_src_color_shift;
    uniform float u_rms;
    uniform float u_beatPhase;

    vec2 boxIntersect(vec3 ro, vec3 rd) {
        vec3 invRd = 1.0 / rd;
        vec3 t1 = (-vec3(1.0) - ro) * invRd;
        vec3 t2 = ( vec3(1.0) - ro) * invRd;
        vec3 tmin = min(t1, t2);
        vec3 tmax = max(t1, t2);
        float tNear = max(max(tmin.x, tmin.y), tmin.z);
        float tFar  = min(min(tmax.x, tmax.y), tmax.z);
        return vec2(tNear, tFar);
    }

    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        float t = u_time;
        float rx = t * (u_src_rotx - 0.5) * 2.0;
        float ry = t * (u_src_roty - 0.5) * 2.0;
        // Camera inside cube at origin
        vec3 ro = vec3(0.0);
        vec3 rd = normalize(vec3(uv, 1.0));
        // Rotate ray direction (rotates the cube around camera)
        float cx = cos(rx), sx = sin(rx);
        rd.yz = mat2(cx, -sx, sx, cx) * rd.yz;
        float cy = cos(ry), sy = sin(ry);
        rd.xz = mat2(cy, -sy, sy, cy) * rd.xz;

        vec2 hits = boxIntersect(ro, rd);
        float tHit = hits.y; // far intersection (we're inside)
        vec3 hitPos = ro + rd * tHit;

        // Determine which face was hit
        vec3 absHit = abs(hitPos);
        vec2 faceUV;
        vec3 normal;
        if (absHit.x >= absHit.y && absHit.x >= absHit.z) {
            faceUV = hitPos.yz; normal = vec3(sign(hitPos.x), 0, 0);
        } else if (absHit.y >= absHit.z) {
            faceUV = hitPos.xz; normal = vec3(0, sign(hitPos.y), 0);
        } else {
            faceUV = hitPos.xy; normal = vec3(0, 0, sign(hitPos.z));
        }
        faceUV = faceUV * 0.5 + 0.5;

        // Pattern on face
        float sc = mix(1.0, 8.0, u_src_scale);
        vec2 grid = abs(fract(faceUV * sc) - 0.5);
        float line = smoothstep(0.03, 0.0, min(grid.x, grid.y));
        float checker = step(0.5, mod(floor(faceUV.x * sc) + floor(faceUV.y * sc), 2.0));

        // Lighting
        float lt = u_time * u_src_light;
        vec3 lightDir = normalize(vec3(sin(lt), cos(lt * 0.7), sin(lt * 0.5)));
        float diff = max(dot(normal, lightDir), 0.0) * 0.5 + 0.5;

        float hue = u_src_color_shift + dot(normal, vec3(0.1, 0.2, 0.3));
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= (checker * 0.3 + 0.2 + line * 0.5) * diff;
        col *= 0.7 + u_rms * 0.5;

        fragColor = vec4(col, 1.0);
    }
)";

// P17.19 — Dual Plane Drift (3D)
inline const char* sourceDualPlaneDrift = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_speed;
    uniform float u_src_rotation;
    uniform float u_src_distance;
    uniform float u_src_scale;
    uniform float u_src_color_shift;
    uniform float u_rms;
    uniform float u_beatPhase;
    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        float t = u_time * (0.3 + u_src_speed * 2.0);
        float dist = 0.3 + u_src_distance * 1.0;

        // Simple two-plane corridor: camera at origin, looking down Z
        vec3 rd = normalize(vec3(uv, 1.0));
        // Apply rotation
        float rot = u_src_rotation * 3.14159;
        float cr = cos(rot), sr = sin(rot);
        rd.xy = mat2(cr, -sr, sr, cr) * rd.xy;

        vec3 col = vec3(0.0);

        // Top plane at y = dist
        if (rd.y > 0.001) {
            float tHit = dist / rd.y;
            vec3 hitPos = rd * tHit;
            hitPos.z += t;
            vec2 planeUV = hitPos.xz * mix(0.5, 4.0, u_src_scale);
            vec2 grid = abs(fract(planeUV) - 0.5);
            float line = smoothstep(0.03, 0.0, min(grid.x, grid.y));
            float fog = exp(-tHit * 0.2);
            float hue = u_src_color_shift;
            vec3 planeCol = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
            col += planeCol * (line * 0.8 + 0.1) * fog;
        }

        // Bottom plane at y = -dist
        if (rd.y < -0.001) {
            float tHit = -dist / rd.y;
            vec3 hitPos = rd * tHit;
            hitPos.z += t;
            vec2 planeUV = hitPos.xz * mix(0.5, 4.0, u_src_scale);
            vec2 grid = abs(fract(planeUV) - 0.5);
            float line = smoothstep(0.03, 0.0, min(grid.x, grid.y));
            float fog = exp(-tHit * 0.2);
            float hue = u_src_color_shift + 0.5;
            vec3 planeCol = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
            col += planeCol * (line * 0.8 + 0.1) * fog;
        }

        col *= 0.7 + u_rms * 0.5;
        fragColor = vec4(col, 1.0);
    }
)";

// ============================================================
// Phase 18: Audio-Native Effects (10 effects)
// These effects use extended audio uniforms (chromagram, MFCCs,
// structural state, onset, pitch, key detection) — our differentiator.
// ============================================================

// P18.1: Harmonic Displacement — chromagram drives 12 directional UV offsets
static constexpr const char* harmonicDisplace = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform float u_harmdisplace_amount;
    uniform float u_harmdisplace_smooth;
    uniform float u_harmdisplace_color;
    uniform float u_chromagram[12];
    uniform float u_hcdf;
    uniform float u_time;
    void main() {
        float amount = u_harmdisplace_amount * 0.08;
        float sharpness = mix(1.0, 3.0, 1.0 - u_harmdisplace_smooth);
        vec2 disp = vec2(0.0);
        for (int i = 0; i < 12; i++) {
            float angle = float(i) * 0.5236; // 2*PI/12
            float energy = pow(u_chromagram[i], sharpness);
            disp += vec2(cos(angle), sin(angle)) * energy;
        }
        disp *= amount * (0.5 + u_hcdf * 2.0);
        vec2 uv = v_texCoord + disp;
        vec4 col = texture(u_texture, uv);
        if (u_harmdisplace_color > 0.01) {
            // Tint by dominant chroma direction
            float hue = atan(disp.y, disp.x) / 6.28318 + 0.5;
            vec3 tint = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
            col.rgb = mix(col.rgb, col.rgb * tint, u_harmdisplace_color);
        }
        fragColor = col;
    }
)";

// P18.2: Timbral Mosaic — MFCCs control Voronoi tile properties
static constexpr const char* timbralMosaic = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform float u_timbremosaic_amount;
    uniform float u_timbremosaic_size;
    uniform float u_timbremosaic_complexity;
    uniform float u_mfccs[13];
    uniform float u_time;
    vec2 hash2(vec2 p) {
        p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
        return fract(sin(p) * 43758.5453);
    }
    void main() {
        float amount = u_timbremosaic_amount;
        if (amount < 0.01) { fragColor = texture(u_texture, v_texCoord); return; }
        float tileSize = mix(0.02, 0.15, u_timbremosaic_size);
        tileSize *= mix(0.5, 1.5, clamp(u_mfccs[0] * 0.5 + 0.5, 0.0, 1.0));
        vec2 uv = v_texCoord / tileSize;
        vec2 iuv = floor(uv);
        vec2 fuv = fract(uv);
        float minDist = 1e9;
        vec2 nearestCell = vec2(0.0);
        for (int y = -1; y <= 1; y++)
        for (int x = -1; x <= 1; x++) {
            vec2 neighbor = vec2(float(x), float(y));
            vec2 point = hash2(iuv + neighbor);
            point = 0.5 + 0.5 * sin(u_time * 0.3 + 6.28318 * point);
            float d = length(neighbor + point - fuv);
            if (d < minDist) { minDist = d; nearestCell = iuv + neighbor; }
        }
        vec2 cellCenter = (nearestCell + 0.5) * tileSize;
        // Complexity controls how many MFCCs modulate the mosaic
        float complexity = u_timbremosaic_complexity;
        float rotation = u_mfccs[2] * 0.5 * complexity;
        vec2 sampleOffset = vec2(u_mfccs[3], u_mfccs[4]) * 0.02 * complexity;
        vec2 sampleUV = clamp(cellCenter + sampleOffset, 0.0, 1.0);
        vec4 tileCol = texture(u_texture, sampleUV);
        // Edge color influenced by more MFCCs at higher complexity
        float edge = smoothstep(0.0, 0.05, minDist);
        float edgeBright = 0.5 + abs(u_mfccs[1]) * 2.0;
        float hueShift = (u_mfccs[5] + u_mfccs[6]) * complexity;
        vec3 edgeCol = 0.5 + 0.5 * cos(6.28318 * (hueShift + vec3(0.0, 0.33, 0.67)));
        edgeCol *= 0.1 * edgeBright;
        // Tile color rotation by complexity
        float cr = cos(rotation), sr = sin(rotation);
        tileCol.rg = vec2(cr * tileCol.r - sr * tileCol.g, sr * tileCol.r + cr * tileCol.g);
        vec3 result = mix(edgeCol, tileCol.rgb, edge);
        fragColor = vec4(mix(texture(u_texture, v_texCoord).rgb, result, amount), tileCol.a);
    }
)";

// P18.3: Structural Morph — 4 UV distortions blended by structural state
static constexpr const char* structuralMorph = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform float u_structmorph_intensity;
    uniform float u_structmorph_normal;
    uniform float u_structmorph_drop;
    uniform float u_structuralState;
    uniform float u_rms;
    uniform float u_time;
    void main() {
        float intensity = u_structmorph_intensity;
        if (intensity < 0.01) { fragColor = texture(u_texture, v_texCoord); return; }
        vec2 uv = v_texCoord;
        vec2 center = uv - 0.5;
        float state = u_structuralState; // 0=normal, 1=buildup, 2=drop, 3=breakdown
        // Normal: gentle wave — style controls frequency and amplitude
        float normalAmt = u_structmorph_normal * 0.06 * intensity;
        float normalFreq = mix(3.0, 12.0, u_structmorph_normal);
        vec2 normal = vec2(sin(uv.y * normalFreq + u_time * 2.0) * normalAmt,
                           cos(uv.x * normalFreq + u_time * 1.5) * normalAmt);
        // Buildup: increasing swirl
        float swirlAmt = 0.3 * intensity;
        float angle = length(center) * 10.0 * swirlAmt;
        float ca = cos(angle), sa = sin(angle);
        vec2 buildup = vec2(ca * center.x - sa * center.y, sa * center.x + ca * center.y) - center;
        // Drop: explosion outward
        float dropStr = u_structmorph_drop * intensity * 0.15;
        vec2 drop = normalize(center + 0.001) * dropStr * (1.0 + u_rms);
        // Breakdown: slow drift inward
        vec2 breakdown = -center * 0.03 * intensity;
        // Blend by state (smooth transitions)
        float w0 = max(0.0, 1.0 - abs(state));
        float w1 = max(0.0, 1.0 - abs(state - 1.0));
        float w2 = max(0.0, 1.0 - abs(state - 2.0));
        float w3 = max(0.0, 1.0 - abs(state - 3.0));
        vec2 disp = normal * w0 + buildup * w1 + drop * w2 + breakdown * w3;
        fragColor = texture(u_texture, uv + disp);
    }
)";

// P18.4: Pitch Chromatic Shift — hue shift based on detected pitch/key
static constexpr const char* pitchChromaShift = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform float u_pitchcolor_amount;
    uniform float u_pitchcolor_mode;
    uniform float u_pitchcolor_sat;
    uniform float u_dominantPitch;
    uniform float u_detectedKey;
    uniform float u_chromagram[12];
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
        vec4 col = texture(u_texture, v_texCoord);
        float amount = u_pitchcolor_amount;
        if (amount < 0.01) { fragColor = col; return; }
        float hueShift = 0.0;
        if (u_pitchcolor_mode < 0.33) {
            // Note-based: pitch class -> hue position
            float midiNote = 12.0 * log2(max(u_dominantPitch, 20.0) / 440.0) + 69.0;
            hueShift = mod(midiNote, 12.0) / 12.0;
        } else if (u_pitchcolor_mode < 0.66) {
            // Key-based: smooth key palette
            hueShift = u_detectedKey >= 0.0 ? u_detectedKey / 12.0 : 0.0;
        } else {
            // Chromagram-weighted: weighted average of all active notes
            float totalEnergy = 0.0;
            float weightedHue = 0.0;
            for (int i = 0; i < 12; i++) {
                weightedHue += u_chromagram[i] * float(i) / 12.0;
                totalEnergy += u_chromagram[i];
            }
            hueShift = totalEnergy > 0.001 ? weightedHue / totalEnergy : 0.0;
        }
        vec3 hsv = rgb2hsv(col.rgb);
        hsv.x = fract(hsv.x + hueShift * amount);
        hsv.y *= mix(1.0, 1.5, u_pitchcolor_sat * amount);
        hsv.y = clamp(hsv.y, 0.0, 1.0);
        fragColor = vec4(hsv2rgb(hsv), col.a);
    }
)";

// P18.5: Key Palette — auto color remap based on detected key + major/minor
static constexpr const char* keyPalette = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform float u_keypalette_amount;
    uniform float u_keypalette_bright;
    uniform float u_keypalette_sat;
    uniform float u_detectedKey;
    uniform float u_keyIsMajor;
    vec3 keyColor(float key, float isMajor) {
        // Scriabin-inspired: each key has a characteristic hue
        // Major = warm, saturated. Minor = cool, desaturated.
        float hue = key / 12.0;
        float sat = isMajor > 0.5 ? 0.7 : 0.4;
        float val = isMajor > 0.5 ? 0.9 : 0.7;
        return 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67))) * sat * val;
    }
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float amount = u_keypalette_amount;
        if (amount < 0.01 || u_detectedKey < 0.0) { fragColor = col; return; }
        float luma = dot(col.rgb, vec3(0.299, 0.587, 0.114));
        vec3 palette = keyColor(u_detectedKey, u_keyIsMajor);
        palette *= mix(0.5, 1.5, u_keypalette_bright);
        vec3 tinted = palette * luma;
        tinted = mix(vec3(luma), tinted, mix(0.5, 1.5, u_keypalette_sat));
        fragColor = vec4(mix(col.rgb, tinted, amount), col.a);
    }
)";

// P18.6: Transient Flash — onset-triggered visual flash
static constexpr const char* transientFlash = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform float u_transflash_style;
    uniform float u_transflash_intensity;
    uniform float u_transflash_decay;
    uniform float u_onsetDetected;
    uniform float u_onsetStrength;
    uniform float u_time;
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float intensity = u_transflash_intensity;
        if (intensity < 0.01) { fragColor = col; return; }
        // Use onset strength directly — it has fast attack, natural decay
        float flash = u_onsetStrength * intensity;
        flash = pow(flash, mix(0.3, 2.0, u_transflash_decay));
        if (u_transflash_style < 0.25) {
            // White flash
            col.rgb = mix(col.rgb, vec3(1.0), flash);
        } else if (u_transflash_style < 0.5) {
            // Invert flash
            col.rgb = mix(col.rgb, 1.0 - col.rgb, flash);
        } else if (u_transflash_style < 0.75) {
            // Edge glow flash
            vec2 uv = v_texCoord;
            vec2 texel = vec2(1.0) / vec2(textureSize(u_texture, 0));
            float edge = length(
                texture(u_texture, uv + texel).rgb - texture(u_texture, uv - texel).rgb);
            col.rgb += vec3(edge * flash * 3.0);
        } else {
            // Zoom punch
            vec2 center = v_texCoord - 0.5;
            vec2 zoomedUV = 0.5 + center * (1.0 - flash * 0.1);
            col = mix(col, texture(u_texture, zoomedUV), flash);
        }
        fragColor = col;
    }
)";

// P18.7: Beat Ripple — onset drops ripples from random positions
static constexpr const char* beatRipple = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform float u_beatripple_intensity;
    uniform float u_beatripple_decay;
    uniform float u_beatripple_count;
    uniform float u_onsetDetected;
    uniform float u_beatPhase;
    uniform float u_onsetStrength;
    uniform float u_time;
    float hash(float n) { return fract(sin(n) * 43758.5453); }
    void main() {
        float intensity = u_beatripple_intensity;
        if (intensity < 0.01) { fragColor = texture(u_texture, v_texCoord); return; }
        int maxRipples = int(mix(2.0, 8.0, u_beatripple_count));
        float decay = mix(0.3, 3.0, u_beatripple_decay);
        vec2 uv = v_texCoord;
        vec2 totalDisp = vec2(0.0);
        for (int i = 0; i < 8; i++) {
            if (i >= maxRipples) break;
            // Each ripple has a pseudo-random center and birth time
            float seed = float(i) * 7.31;
            vec2 center = vec2(hash(seed), hash(seed + 1.0));
            // Ripple age based on beat phase offset
            float age = fract(u_time * 0.5 + hash(seed + 2.0));
            float strength = exp(-age * decay) * intensity;
            float dist = length(uv - center);
            float wave = sin(dist * 30.0 - age * 20.0) * strength;
            totalDisp += normalize(uv - center + 0.001) * wave * 0.02;
        }
        // Extra impulse on actual onsets
        totalDisp *= 1.0 + u_onsetStrength * 2.0;
        fragColor = texture(u_texture, uv + totalDisp);
    }
)";

// P18.8: Rhythm Slice — horizontal strips shift on beat divisions
static constexpr const char* rhythmSlice = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform float u_rhythmslice_amount;
    uniform float u_rhythmslice_count;
    uniform float u_rhythmslice_sync;
    uniform float u_beatPhase;
    uniform float u_barPhase;
    uniform float u_phrasePhase;
    uniform float u_time;
    float hash(float n) { return fract(sin(n) * 43758.5453); }
    void main() {
        float amount = u_rhythmslice_amount;
        if (amount < 0.01) { fragColor = texture(u_texture, v_texCoord); return; }
        float sliceCount = mix(4.0, 16.0, u_rhythmslice_count);
        float sliceIndex = floor(v_texCoord.y * sliceCount);
        float slicePhase = sliceIndex / sliceCount;
        // Each slice syncs to a different beat division
        float division = mod(sliceIndex, 3.0);
        float phase;
        if (division < 1.0) phase = u_phrasePhase;      // slow: phrase level
        else if (division < 2.0) phase = u_barPhase;     // medium: bar level
        else phase = u_beatPhase;                         // fast: beat level
        // Mix in random offset based on sync parameter
        float randomOffset = hash(sliceIndex * 17.3 + floor(u_time * 2.0)) * (1.0 - u_rhythmslice_sync);
        float offset = sin((phase + randomOffset) * 6.28318) * amount * 0.15;
        vec2 uv = v_texCoord;
        uv.x += offset;
        uv.x = fract(uv.x);
        fragColor = texture(u_texture, uv);
    }
)";

// P18.9: Density Wave — beat-synced compression/rarefaction waves
static constexpr const char* densityWave = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform float u_densitywave_amount;
    uniform float u_densitywave_dir;
    uniform float u_densitywave_wl;
    uniform float u_beatPhase;
    uniform float u_rms;
    uniform float u_time;
    void main() {
        float amount = u_densitywave_amount;
        if (amount < 0.01) { fragColor = texture(u_texture, v_texCoord); return; }
        vec2 uv = v_texCoord;
        float freq = mix(2.0, 12.0, u_densitywave_wl);
        float dir = u_densitywave_dir * 6.28318;
        vec2 axis = vec2(cos(dir), sin(dir));
        float pos = dot(uv - 0.5, axis);
        float wave = sin(pos * freq + u_beatPhase * 6.28318) * amount * 0.1 * (0.5 + u_rms);
        uv += axis * wave;
        fragColor = texture(u_texture, uv);
    }
)";

// P18.10: Chroma Dissolve — dissolve image by note-matched hue
static constexpr const char* chromaDissolve = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform float u_chromadiss_amount;
    uniform float u_chromadiss_softness;
    uniform float u_chromagram[12];
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float amount = u_chromadiss_amount;
        if (amount < 0.01) { fragColor = col; return; }
        // Get pixel hue (0-1)
        float cMax = max(col.r, max(col.g, col.b));
        float cMin = min(col.r, min(col.g, col.b));
        float delta = cMax - cMin;
        float hue = 0.0;
        if (delta > 0.001) {
            if (cMax == col.r) hue = mod((col.g - col.b) / delta, 6.0) / 6.0;
            else if (cMax == col.g) hue = ((col.b - col.r) / delta + 2.0) / 6.0;
            else hue = ((col.r - col.g) / delta + 4.0) / 6.0;
        }
        hue = fract(hue);
        // Map hue to pitch class (0-11)
        float noteFloat = hue * 12.0;
        int note = int(floor(noteFloat)) % 12;
        int noteNext = (note + 1) % 12;
        float blend = fract(noteFloat);
        float energy = mix(u_chromagram[note], u_chromagram[noteNext], blend);
        // Higher energy = more of that hue dissolves
        float softness = mix(0.05, 0.5, u_chromadiss_softness);
        float dissolve = smoothstep(energy * amount - softness, energy * amount + softness, 0.5);
        col.a *= dissolve;
        col.rgb *= dissolve;
        fragColor = col;
    }
)";

// ============================================================
// Phase 18: Audio-Native Sources (8 sources)
// ============================================================

// P18.11: Spectrum Landscape — 7-band FFT as 3D terrain
static constexpr const char* sourceSpectrumLandscape = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_src_height;
    uniform float u_src_camera;
    uniform float u_src_color_mode;
    uniform float u_src_glow;
    uniform float u_src_smoothing;
    uniform float u_bandEnergies[7];
    uniform float u_rms;
    uniform float u_spectralCentroid;
    void main() {
        vec2 uv = gl_FragCoord.xy / u_resolution;
        // Map X to 7 frequency bands
        float bandF = uv.x * 6.0;
        int band = clamp(int(bandF), 0, 6);
        int bandNext = min(band + 1, 6);
        float blendB = fract(bandF);
        // Smooth interpolation between bands
        float energy = mix(u_bandEnergies[band], u_bandEnergies[bandNext], blendB);
        energy = mix(energy, energy, u_src_smoothing); // temporal smoothing placeholder
        float heightScale = mix(0.1, 0.8, u_src_height);
        float barHeight = energy * heightScale;
        // Camera angle: 0=overhead (bar chart), 1=horizon (perspective)
        float camAngle = u_src_camera;
        float y = uv.y;
        // Apply perspective skew
        y = mix(y, y * (0.3 + uv.x * 0.7), camAngle * 0.5);
        float dist = y - (0.1 + barHeight);
        // Color by mode
        vec3 col;
        float hue;
        if (u_src_color_mode < 0.33) {
            // Amplitude heat map
            hue = energy * 0.7;
            col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        } else if (u_src_color_mode < 0.66) {
            // Frequency rainbow
            hue = uv.x;
            col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        } else {
            // Monochrome
            col = vec3(0.8, 0.9, 1.0);
        }
        float bar = smoothstep(0.005, 0.0, dist);
        float glow = exp(-abs(dist) * mix(5.0, 20.0, 1.0 - u_src_glow)) * energy;
        col = col * bar + col * glow * 0.5;
        col *= 0.7 + u_rms * 0.5;
        fragColor = vec4(col, 1.0);
    }
)";

// P18.12: Chromatic Ring — 12-segment circle showing pitch class energy
static constexpr const char* sourceChromaticRing = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_src_ring_width;
    uniform float u_src_glow;
    uniform float u_src_rotation;
    uniform float u_src_ripple;
    uniform float u_chromagram[12];
    uniform float u_detectedKey;
    uniform float u_hcdf;
    uniform float u_pitchConfidence;
    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        float angle = atan(uv.y, uv.x) + 3.14159;
        float rotation = u_src_rotation * 6.28318 + u_time * 0.2;
        angle = mod(angle + rotation, 6.28318);
        float r = length(uv);
        float ringR = 0.35;
        float ringW = mix(0.05, 0.2, u_src_ring_width);
        // Which pitch class
        float noteF = angle / 6.28318 * 12.0;
        int note = int(floor(noteF)) % 12;
        float energy = u_chromagram[note];
        // Ring distance
        float ringDist = abs(r - ringR);
        float inRing = smoothstep(ringW, ringW - 0.01, ringDist);
        // Energy modulates ring width outward
        float energyRing = smoothstep(ringW + energy * 0.1, ringW + energy * 0.1 - 0.01, ringDist);
        // Color per note (rainbow around the circle)
        float hue = float(note) / 12.0;
        vec3 noteColor = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        // Highlight detected key
        float isKey = (u_detectedKey >= 0.0 && abs(float(note) - u_detectedKey) < 0.5) ? 1.0 : 0.0;
        vec3 col = noteColor * energy * (1.0 + isKey * 0.5);
        col *= max(inRing, energyRing);
        // HCDF ripple
        float ripple = sin(r * 30.0 - u_time * 5.0) * u_src_ripple * u_hcdf;
        col += noteColor * max(0.0, ripple) * 0.3;
        // Glow
        float glow = exp(-ringDist * mix(5.0, 20.0, 1.0 - u_src_glow)) * energy * 0.3;
        col += noteColor * glow;
        col *= 0.5 + u_pitchConfidence * 0.5;
        fragColor = vec4(col, 1.0);
    }
)";

// P18.13: Band Tower — 7 vertical towers driven by frequency bands
static constexpr const char* sourceBandTower = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_src_shape;
    uniform float u_src_spacing;
    uniform float u_src_reflection;
    uniform float u_src_color_mode;
    uniform float u_src_smoothing;
    uniform float u_src_rotation_3d;
    uniform float u_bandEnergies[7];
    uniform float u_spectralCentroid;
    void main() {
        vec2 uv = gl_FragCoord.xy / u_resolution;
        float spacing = mix(0.02, 0.06, u_src_spacing);
        float barWidth = (1.0 - spacing * 8.0) / 7.0;
        vec3 col = vec3(0.0);
        // Band colors
        vec3 bandColors[7] = vec3[7](
            vec3(0.8, 0.1, 0.1),   // Sub - red
            vec3(0.9, 0.4, 0.1),   // Bass - orange
            vec3(0.9, 0.8, 0.1),   // LowMid - yellow
            vec3(0.1, 0.8, 0.2),   // Mid - green
            vec3(0.1, 0.5, 0.9),   // HighMid - blue
            vec3(0.4, 0.2, 0.9),   // Presence - indigo
            vec3(0.8, 0.2, 0.8)    // Brilliance - violet
        );
        for (int i = 0; i < 7; i++) {
            float x0 = spacing + float(i) * (barWidth + spacing);
            float x1 = x0 + barWidth;
            float energy = u_bandEnergies[i];
            float barH = energy * 0.8 + 0.02;
            if (uv.x >= x0 && uv.x <= x1) {
                vec3 bandCol;
                if (u_src_color_mode < 0.33) bandCol = bandColors[i];
                else if (u_src_color_mode < 0.66) {
                    float h = float(i) / 7.0;
                    bandCol = 0.5 + 0.5 * cos(6.28318 * (h + u_time * 0.1 + vec3(0.0, 0.33, 0.67)));
                } else bandCol = vec3(0.8);
                // Main bar
                if (uv.y < barH) {
                    float grad = uv.y / barH;
                    col = bandCol * (0.5 + grad * 0.5);
                    col += vec3(energy * 0.3); // peak glow
                }
                // Reflection
                if (u_src_reflection > 0.01) {
                    float reflY = -uv.y;
                    if (reflY > -barH && uv.y < 0.0) {
                        col += bandCol * 0.2 * u_src_reflection;
                    }
                }
            }
        }
        fragColor = vec4(col, 1.0);
    }
)";

// P18.14: Timbral Nebula — 13 MFCCs as particle cloud positions
static constexpr const char* sourceTimbralNebula = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_src_particles;
    uniform float u_src_spread;
    uniform float u_src_trail;
    uniform float u_src_color_source;
    uniform float u_src_glow;
    uniform float u_src_sensitivity;
    uniform float u_mfccs[13];
    uniform float u_spectralCentroid;
    uniform float u_rms;
    uniform float u_hcdf;
    float hash(float n) { return fract(sin(n) * 43758.5453); }
    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        float sensitivity = mix(0.5, 4.0, u_src_sensitivity);
        float spread = mix(0.2, 0.8, u_src_spread);
        int numParticles = int(mix(20.0, 80.0, u_src_particles));
        vec3 col = vec3(0.0);
        for (int i = 0; i < 80; i++) {
            if (i >= numParticles) break;
            // Position driven by MFCCs — each particle mapped to a pair of coefficients
            int mIdx1 = i % 13;
            int mIdx2 = (i * 7 + 3) % 13;
            float m1 = u_mfccs[mIdx1] * sensitivity;
            float m2 = u_mfccs[mIdx2] * sensitivity;
            // Base position from hash, offset by MFCC values
            vec2 basePos = vec2(hash(float(i) * 3.7) - 0.5, hash(float(i) * 5.3) - 0.5);
            vec2 pos = basePos * spread + vec2(m1, m2) * 0.3;
            // Trail: smear in the direction of change
            pos += vec2(sin(u_time * 0.5 + float(i)), cos(u_time * 0.3 + float(i) * 1.3)) * u_src_trail * 0.05;
            float dist = length(uv - pos);
            float size = 0.01 + u_rms * 0.02;
            float particle = exp(-dist * dist / (size * size));
            // Color
            float hue;
            if (u_src_color_source < 0.33) {
                hue = clamp(u_spectralCentroid / 8000.0, 0.0, 1.0);
            } else if (u_src_color_source < 0.66) {
                hue = float(mIdx1) / 13.0;
            } else {
                hue = 0.6;
            }
            vec3 pCol = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
            col += pCol * particle;
        }
        // Glow
        col *= 1.0 + u_src_glow * 0.5;
        col *= 0.7 + u_rms * 0.5;
        fragColor = vec4(col, 1.0);
    }
)";

// P18.15: Structural Landscape — terrain evolving with song structure
static constexpr const char* sourceStructuralLandscape = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_src_scale;
    uniform float u_src_history;
    uniform float u_src_drama;
    uniform float u_src_palette;
    uniform float u_src_fog;
    uniform float u_src_camera;
    uniform float u_structuralState;
    uniform float u_rms;
    uniform float u_spectralFlux;
    uniform float u_bpm;
    float noise(vec2 p) {
        vec2 i = floor(p);
        vec2 f = fract(p);
        f = f * f * (3.0 - 2.0 * f);
        float a = fract(sin(dot(i, vec2(127.1, 311.7))) * 43758.5453);
        float b = fract(sin(dot(i + vec2(1,0), vec2(127.1, 311.7))) * 43758.5453);
        float c = fract(sin(dot(i + vec2(0,1), vec2(127.1, 311.7))) * 43758.5453);
        float d = fract(sin(dot(i + vec2(1,1), vec2(127.1, 311.7))) * 43758.5453);
        return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
    }
    float fbm(vec2 p) {
        float v = 0.0, a = 0.5;
        for (int i = 0; i < 5; i++) {
            v += a * noise(p);
            p *= 2.0;
            a *= 0.5;
        }
        return v;
    }
    void main() {
        vec2 uv = gl_FragCoord.xy / u_resolution;
        float scale = mix(2.0, 8.0, u_src_scale);
        vec2 p = uv * scale;
        p.x += u_time * (u_bpm > 0.0 ? u_bpm / 120.0 : 1.0) * 0.3;
        float state = u_structuralState;
        float drama = u_src_drama;
        // Terrain height modulated by structural state
        float h = fbm(p);
        h += u_spectralFlux * 0.2 * drama;
        // State-specific modulations
        if (state > 0.5 && state < 1.5) {
            // Buildup: terrain rises
            h += 0.2 * drama;
            h += sin(p.x * 3.0 + u_time * 2.0) * 0.1 * drama;
        } else if (state > 1.5 && state < 2.5) {
            // Drop: dramatic peaks
            h *= 1.0 + 0.5 * drama;
            h += u_rms * 0.3 * drama;
        } else if (state > 2.5) {
            // Breakdown: flatten, fog
            h *= 0.5;
        }
        float terrain = smoothstep(uv.y - 0.02, uv.y + 0.02, h * 0.6);
        // Color by palette
        vec3 col;
        float hue;
        if (u_src_palette < 0.33) {
            // Earth
            col = mix(vec3(0.1, 0.3, 0.1), vec3(0.6, 0.4, 0.2), h);
            col = mix(col, vec3(0.9, 0.9, 1.0), max(0.0, h - 0.7));
        } else if (u_src_palette < 0.66) {
            // Neon
            hue = h * 0.5 + u_time * 0.05;
            col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        } else {
            // Minimal
            col = vec3(h * 0.8 + 0.1);
        }
        col *= terrain;
        // Fog
        float fog = (1.0 - uv.y) * u_src_fog;
        if (state > 2.5) fog *= 2.0; // extra fog in breakdowns
        col = mix(col, vec3(0.05, 0.05, 0.1), fog * 0.5);
        col *= 0.7 + u_rms * 0.5;
        fragColor = vec4(col, 1.0);
    }
)";

// P18.16: Cymatics — Chladni figures from dominant pitch
static constexpr const char* sourceCymatics = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_src_resonance;
    uniform float u_src_damping;
    uniform float u_src_color_shift;
    uniform float u_dominantPitch;
    uniform float u_pitchConfidence;
    uniform float u_rms;
    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        // Derive frequency from pitch (normalize to useful range)
        float pitch = clamp(u_dominantPitch, 50.0, 2000.0);
        float freq = pitch / 50.0; // normalize to ~1-40 range
        freq *= mix(0.5, 2.0, u_src_resonance);
        // Chladni figure: cos(n*pi*x)*cos(m*pi*y) - cos(m*pi*x)*cos(n*pi*y) = 0
        float n = floor(freq * 0.5) + 1.0;
        float m = floor(freq * 0.3) + 1.0;
        float pattern = cos(n * 3.14159 * uv.x) * cos(m * 3.14159 * uv.y)
                       - cos(m * 3.14159 * uv.x) * cos(n * 3.14159 * uv.y);
        // Add second mode for richer patterns
        float n2 = n + 1.0, m2 = m + 1.0;
        float pattern2 = cos(n2 * 3.14159 * uv.x) * cos(m2 * 3.14159 * uv.y)
                        - cos(m2 * 3.14159 * uv.x) * cos(n2 * 3.14159 * uv.y);
        pattern = mix(pattern, pattern2, 0.3);
        // Nodal lines (where pattern ≈ 0)
        float sharpness = mix(5.0, 30.0, u_src_resonance);
        float line = exp(-abs(pattern) * sharpness);
        // Damping: blend with smoothed version
        float damping = u_src_damping;
        line = mix(line, smoothstep(0.3, 0.0, abs(pattern)), damping);
        // Color
        float hue = u_src_color_shift + freq * 0.01;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= line;
        col *= 0.3 + u_pitchConfidence * 0.7;
        col *= 0.6 + u_rms * 0.6;
        fragColor = vec4(col, 1.0);
    }
)";

// P18.17: Spectral Waterfall — scrolling spectrogram
// Non-stateful version: simulates history using time-offset noise
static constexpr const char* sourceSpectralWaterfall = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_src_scroll;
    uniform float u_src_color_mode;
    uniform float u_src_log_scale;
    uniform float u_bandEnergies[7];
    uniform float u_rms;
    void main() {
        vec2 uv = gl_FragCoord.xy / u_resolution;
        float scrollSpeed = mix(0.3, 3.0, u_src_scroll);
        // Map X to frequency bands
        float bandF = uv.x * 6.0;
        if (u_src_log_scale > 0.5) {
            bandF = pow(uv.x, 2.0) * 6.0;
        }
        int band = clamp(int(bandF), 0, 6);
        int bandNext = min(band + 1, 6);
        float blend = fract(bandF);
        float energy = mix(u_bandEnergies[band], u_bandEnergies[bandNext], blend);
        // Y position represents time: top = now, bottom = past
        // Current band energy at top, fading/varying downward
        float age = (1.0 - uv.y); // 0 at top (now), 1 at bottom (old)
        float scrollPhase = u_time * scrollSpeed;
        // Simulate past energy: current energy decays + noise for variation
        float noise = fract(sin(dot(vec2(bandF, uv.y * 100.0 + scrollPhase), vec2(127.1, 311.7))) * 43758.5453);
        float pastEnergy = energy * exp(-age * 2.0) + noise * 0.1 * (1.0 - age * 0.5);
        pastEnergy = clamp(pastEnergy, 0.0, 1.0);
        // Add scrolling line pattern for visual movement
        float line = sin((uv.y + scrollPhase * 0.1) * u_resolution.y * 0.3) * 0.03;
        pastEnergy += line;
        pastEnergy = clamp(pastEnergy, 0.0, 1.0);
        // Color modes
        vec3 col;
        if (u_src_color_mode < 0.33) {
            // Thermal: black → blue → red → yellow → white
            col = mix(vec3(0.0, 0.0, 0.2), vec3(1.0, 0.0, 0.0), clamp(pastEnergy * 2.0, 0.0, 1.0));
            col = mix(col, vec3(1.0, 1.0, 0.0), max(0.0, pastEnergy - 0.5) * 2.0);
            col = mix(col, vec3(1.0), max(0.0, pastEnergy - 0.85) * 6.67);
        } else if (u_src_color_mode < 0.66) {
            // Neon rainbow
            float hue = uv.x * 0.8;
            col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
            col *= pastEnergy;
        } else {
            // Monochrome green (classic spectrogram)
            col = vec3(0.0, pastEnergy * 0.9, pastEnergy * 0.3);
        }
        col *= 0.7 + u_rms * 0.5;
        fragColor = vec4(col, 1.0);
    }
)";

// P18.18: Spectral Ring — circular FFT display
static constexpr const char* sourceSpectralRing = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_src_radius;
    uniform float u_src_thickness;
    uniform float u_src_glow;
    uniform float u_src_rotation;
    uniform float u_src_color_shift;
    uniform float u_bandEnergies[7];
    uniform float u_beatPhase;
    uniform float u_onsetStrength;
    uniform float u_rms;
    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        float r = length(uv);
        float angle = atan(uv.y, uv.x) + 3.14159;
        // Beat-driven rotation
        float rot = u_src_rotation * u_time * 2.0 + u_beatPhase * 0.5;
        angle = mod(angle + rot, 6.28318);
        float ringR = mix(0.15, 0.4, u_src_radius);
        float thick = mix(0.01, 0.06, u_src_thickness);
        // Map angle to 7 bands (repeated for symmetry)
        float bandAngle = angle / 6.28318 * 7.0;
        int band = int(floor(bandAngle)) % 7;
        float energy = u_bandEnergies[band];
        // Onset pulse
        float pulse = 1.0 + u_onsetStrength * 0.2;
        float barHeight = energy * 0.2 * pulse;
        // Distance from ring
        float innerR = ringR - thick;
        float outerR = ringR + thick + barHeight;
        float inRing = smoothstep(innerR - 0.005, innerR, r) * smoothstep(outerR, outerR - 0.005, r);
        // Color
        float hue = float(band) / 7.0 + u_src_color_shift;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= inRing;
        // Glow
        float dist = abs(r - ringR);
        float glow = exp(-dist * mix(10.0, 40.0, 1.0 - u_src_glow)) * energy * 0.4;
        col += 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67))) * glow;
        col *= 0.7 + u_rms * 0.5;
        fragColor = vec4(col, 1.0);
    }
)";

// P18.19: Bitmap Font Atlas (embedded as constant data)
// 8x8 pixel grid, 128 ASCII characters, used by Text Wall source.
// The atlas is generated procedurally in the text wall shader using
// a simplified approach — bitmap data for basic characters encoded
// as integer bit patterns.

// P18.20: Scrolling Text Wall — uses procedural bitmap font
static constexpr const char* sourceTextWall = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_src_speed;
    uniform float u_src_density;
    uniform float u_src_size;
    uniform float u_src_color_shift;
    uniform float u_rms;
    uniform float u_beatPhase;
    // Procedural character renderer — simplified bitmap font
    float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
    float character(vec2 p, float charId) {
        // 5x7 bitmap characters encoded procedurally
        // Each character is a unique pattern based on hash
        vec2 cell = floor(p * vec2(5.0, 7.0));
        if (cell.x < 0.0 || cell.x >= 5.0 || cell.y < 0.0 || cell.y >= 7.0) return 0.0;
        float bit = step(0.4, hash(cell + charId * 17.0));
        // Make it look more like text: vertical strokes more likely
        bit *= step(0.3, hash(vec2(cell.x, charId)));
        return bit;
    }
    void main() {
        vec2 uv = gl_FragCoord.xy / u_resolution;
        float cellSize = mix(0.02, 0.08, u_src_size);
        float density = mix(0.3, 1.0, u_src_density);
        float speed = mix(0.2, 3.0, u_src_speed);
        // Scrolling grid of characters
        vec2 scrollUV = uv;
        scrollUV.y += u_time * speed;
        scrollUV.x += sin(uv.y * 3.0 + u_time) * 0.02;
        vec2 cell = floor(scrollUV / cellSize);
        vec2 cellUV = fract(scrollUV / cellSize);
        // Character selection per cell
        float charId = hash(cell);
        // Skip some cells for density
        float visible = step(1.0 - density, hash(cell * 7.3));
        float ch = character(cellUV, charId * 100.0) * visible;
        // Beat-reactive brightness
        float brightness = 0.5 + u_rms * 0.5 + u_beatPhase * 0.2;
        // Color
        float hue = u_src_color_shift + cell.x * 0.01 + u_time * 0.05;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= ch * brightness;
        // Slight glow
        col += col * 0.3;
        fragColor = vec4(col, 1.0);
    }
)";

// ============================================================
// Phase 19: Complex Effects (8 shaders)
// ============================================================

inline const char* lumaTerrain = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform float u_lumaterrain_height;
    uniform float u_lumaterrain_segments;
    uniform float u_lumaterrain_angle;
    void main() {
        float numLines = floor(u_lumaterrain_segments * 80.0) + 5.0;
        float lineSpacing = 1.0 / numLines;
        float lineY = floor(v_texCoord.y * numLines) / numLines + lineSpacing * 0.5;
        vec4 lineColor = texture(u_texture, vec2(v_texCoord.x, lineY));
        float luma = dot(lineColor.rgb, vec3(0.299, 0.587, 0.114));
        float height = u_lumaterrain_height * 0.4;
        float offsetY = luma * height;
        // Perspective tilt
        float tilt = mix(0.0, 0.5, u_lumaterrain_angle);
        float perspY = v_texCoord.y + (v_texCoord.y - 0.5) * tilt;
        float distToLine = abs(perspY - lineY - offsetY);
        float lineWidth = lineSpacing * 0.3;
        float line = smoothstep(lineWidth, lineWidth * 0.3, distToLine);
        // Darken lines behind (simple depth)
        float depth = 1.0 - (1.0 - v_texCoord.y) * 0.4 * u_lumaterrain_angle;
        fragColor = mix(texture(u_texture, v_texCoord), lineColor * depth, line);
    }
)";

inline const char* voxelMatrix = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform float u_voxel_size;
    uniform float u_voxel_height;
    uniform float u_voxel_rotation;
    void main() {
        float blockSize = mix(0.008, 0.06, u_voxel_size);
        vec2 blockPos = floor(v_texCoord / blockSize) * blockSize;
        vec2 blockCenter = blockPos + blockSize * 0.5;
        vec4 blockColor = texture(u_texture, blockCenter);
        float luma = dot(blockColor.rgb, vec3(0.299, 0.587, 0.114));
        vec2 inBlock = (v_texCoord - blockPos) / blockSize;
        // 3D extrusion shading
        float raised = luma * u_voxel_height;
        float topFace = step(raised * 0.3, 1.0 - inBlock.y);
        float shade = 1.0 - raised * 0.5 * (1.0 - inBlock.y);
        // Side face darkening
        float sideDark = 1.0;
        if (inBlock.x < 0.15) sideDark = 0.7 + 0.3 * (inBlock.x / 0.15);
        if (inBlock.x > 0.85) sideDark = 0.7 + 0.3 * ((1.0 - inBlock.x) / 0.15);
        // Border
        float borderW = 0.06;
        float border = step(borderW, inBlock.x) * step(inBlock.x, 1.0 - borderW) *
                       step(borderW, inBlock.y) * step(inBlock.y, 1.0 - borderW);
        // Rotation effect — shift sampling angle
        float angle = u_voxel_rotation * 6.28318;
        float s = sin(angle) * 0.02;
        vec2 rotOffset = vec2(s * (inBlock.y - 0.5), -s * (inBlock.x - 0.5));
        vec4 rotColor = texture(u_texture, blockCenter + rotOffset);
        vec4 finalColor = mix(blockColor, rotColor, u_voxel_rotation);
        fragColor = finalColor * shade * sideDark * border;
    }
)";

inline const char* monitorWall = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform float u_monwall_cols;
    uniform float u_monwall_rows;
    uniform float u_monwall_border;
    uniform float u_monwall_glow;
    void main() {
        float cols = floor(u_monwall_cols * 6.0) + 2.0;
        float rows = floor(u_monwall_rows * 4.0) + 2.0;
        vec2 cell = fract(v_texCoord * vec2(cols, rows));
        float borderW = u_monwall_border * 0.12 + 0.01;
        float mask = step(borderW, cell.x) * step(cell.x, 1.0 - borderW) *
                     step(borderW, cell.y) * step(cell.y, 1.0 - borderW);
        vec4 content = texture(u_texture, v_texCoord);
        // Screen glow near edges
        float edgeDist = min(min(cell.x, 1.0 - cell.x), min(cell.y, 1.0 - cell.y));
        float glow = smoothstep(borderW * 3.0, borderW, edgeDist) * u_monwall_glow;
        content.rgb += content.rgb * glow * 0.5;
        // Bezel color (dark gray)
        vec3 bezel = vec3(0.05);
        fragColor = vec4(mix(bezel, content.rgb, mask), 1.0);
    }
)";

inline const char* dropShadow = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform vec2 u_resolution;
    uniform float u_shadow_ox;
    uniform float u_shadow_oy;
    uniform float u_shadow_blur;
    uniform float u_shadow_opacity;
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        vec2 shadowOffset = (vec2(u_shadow_ox, u_shadow_oy) - 0.5) * 0.15;
        float blurSize = u_shadow_blur * 0.015;
        // Blurred shadow sample
        float shadow = 0.0;
        float total = 0.0;
        for (int x = -3; x <= 3; x++) {
            for (int y = -3; y <= 3; y++) {
                float w = exp(-float(x*x + y*y) / 8.0);
                vec2 offset = vec2(float(x), float(y)) * blurSize;
                vec4 s = texture(u_texture, v_texCoord - shadowOffset + offset);
                shadow += dot(s.rgb, vec3(0.299, 0.587, 0.114)) * w;
                total += w;
            }
        }
        shadow /= total;
        // Apply shadow behind content
        float contentLuma = dot(col.rgb, vec3(0.299, 0.587, 0.114));
        float shadowMask = shadow * u_shadow_opacity * (1.0 - contentLuma * 0.5);
        vec3 result = col.rgb * (1.0 - shadowMask * 0.7);
        result = mix(result, col.rgb, contentLuma);
        fragColor = vec4(result, col.a);
    }
)";

inline const char* channelDelay = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform sampler2D u_prev_frame;
    uniform float u_delay_r;
    uniform float u_delay_g;
    uniform float u_delay_b;
    void main() {
        vec4 current = texture(u_texture, v_texCoord);
        vec4 prev = texture(u_prev_frame, v_texCoord);
        // Each channel blends between current and previous based on delay amount
        float rMix = u_delay_r;
        float gMix = u_delay_g;
        float bMix = u_delay_b;
        float r = mix(current.r, prev.r, rMix);
        float g = mix(current.g, prev.g, gMix);
        float b = mix(current.b, prev.b, bMix);
        fragColor = vec4(r, g, b, current.a);
    }
)";

inline const char* topoLines = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform float u_topo_amount;
    uniform float u_topo_levels;
    uniform float u_topo_thickness;
    uniform float u_topo_color;
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float luma = dot(col.rgb, vec3(0.299, 0.587, 0.114));
        float numLevels = mix(4.0, 24.0, u_topo_levels);
        float contour = fract(luma * numLevels);
        float thickness = mix(0.02, 0.45, u_topo_thickness);
        float line = smoothstep(thickness, thickness * 0.3, abs(contour - 0.5));
        // Color modes: 0=white, 0.5=height-mapped, 1.0=overlay on original
        vec3 lineColor;
        if (u_topo_color < 0.33) {
            lineColor = vec3(1.0);
        } else if (u_topo_color < 0.66) {
            float hue = luma * 0.8;
            lineColor = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        } else {
            lineColor = col.rgb * 1.5;
        }
        fragColor = mix(col, vec4(lineColor, col.a), line * u_topo_amount);
    }
)";

inline const char* dataCorrupt = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform float u_time;
    uniform float u_datacorrupt_amount;
    uniform float u_datacorrupt_block;
    uniform float u_datacorrupt_color;
    float hash(vec2 p) { return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453); }
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float blockSize = mix(0.01, 0.12, u_datacorrupt_block);
        vec2 blockIdx = floor(v_texCoord / blockSize);
        float blockRand = hash(blockIdx + floor(u_time * 3.0));
        vec2 uv = v_texCoord;
        // Block displacement
        if (blockRand < u_datacorrupt_amount * 0.4) {
            float displaceX = (hash(blockIdx * 3.7) - 0.5) * 0.2 * u_datacorrupt_amount;
            uv.x = fract(uv.x + displaceX);
        }
        vec4 sample_col = texture(u_texture, uv);
        // Posterize (JPEG-like compression artifact)
        float quant = mix(256.0, 4.0, u_datacorrupt_amount);
        vec3 posterized = floor(sample_col.rgb * quant) / quant;
        // Chroma subsampling damage
        float chromaBlock = blockSize * mix(1.0, 4.0, u_datacorrupt_color);
        vec2 chromaUV = floor(uv / chromaBlock) * chromaBlock + chromaBlock * 0.5;
        vec3 chromaSample = texture(u_texture, chromaUV).rgb;
        // Mix damaged chroma with posterized luma
        float luma = dot(posterized, vec3(0.299, 0.587, 0.114));
        vec3 damaged = vec3(luma) + (chromaSample - vec3(dot(chromaSample, vec3(0.299, 0.587, 0.114)))) * (1.0 - u_datacorrupt_color * 0.5);
        // Random color channel swap
        if (hash(blockIdx * 5.1 + floor(u_time * 5.0)) < u_datacorrupt_amount * 0.2) {
            damaged = damaged.gbr;
        }
        fragColor = vec4(mix(col.rgb, damaged, u_datacorrupt_amount), col.a);
    }
)";

inline const char* glitchSort = R"(#version 410 core
    out vec4 fragColor;
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_glitchsort_amount;
    uniform float u_glitchsort_threshold;
    uniform float u_glitchsort_dir;
    float hash(vec2 p) { return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453); }
    void main() {
        vec4 col = texture(u_texture, v_texCoord);
        float luma = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));
        float threshold = u_glitchsort_threshold;
        // Direction: 0=horizontal, 0.5=vertical, 1.0=diagonal
        vec2 sortDir;
        if (u_glitchsort_dir < 0.33) {
            sortDir = vec2(1.0 / u_resolution.x, 0.0);
        } else if (u_glitchsort_dir < 0.66) {
            sortDir = vec2(0.0, 1.0 / u_resolution.y);
        } else {
            sortDir = vec2(1.0 / u_resolution.x, 1.0 / u_resolution.y);
        }
        // Pixel sort approximation: scan neighbors and shift bright pixels
        vec4 result = col;
        if (luma > threshold) {
            int steps = int(u_glitchsort_amount * 40.0) + 1;
            float bestLuma = luma;
            vec2 bestUV = v_texCoord;
            for (int i = 1; i <= 40; i++) {
                if (i > steps) break;
                vec2 sampleUV = v_texCoord - sortDir * float(i);
                if (sampleUV.x < 0.0 || sampleUV.x > 1.0 || sampleUV.y < 0.0 || sampleUV.y > 1.0) break;
                vec4 s = texture(u_texture, sampleUV);
                float sl = dot(s.rgb, vec3(0.2126, 0.7152, 0.0722));
                if (sl > threshold && sl < bestLuma) {
                    bestLuma = sl;
                    bestUV = sampleUV;
                }
            }
            result = texture(u_texture, bestUV);
        }
        // Row-based glitch streaks for extra visual interest
        float rowHash = hash(vec2(floor(v_texCoord.y * u_resolution.y), floor(u_time * 4.0)));
        if (rowHash < u_glitchsort_amount * 0.15 && luma > threshold) {
            float streak = hash(vec2(floor(v_texCoord.y * u_resolution.y), 0.0)) * 0.1;
            result = texture(u_texture, v_texCoord + sortDir * streak * u_glitchsort_amount);
        }
        fragColor = mix(col, result, u_glitchsort_amount);
    }
)";

// ============================================================
// Phase 19: Remaining Sources (12 shaders)
// ============================================================

inline const char* sourceSuperformula = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_beatPhase;
    uniform float u_src_m;
    uniform float u_src_n1;
    uniform float u_src_n2;
    uniform float u_src_n3;
    uniform float u_src_size;
    uniform float u_src_color_shift;
    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        float theta = atan(uv.y, uv.x) + u_time * 0.3;
        float r = length(uv);
        float m = mix(3.0, 10.0, u_src_m);
        float n1 = mix(0.2, 4.0, u_src_n1);
        float n2 = mix(0.2, 4.0, u_src_n2);
        float n3 = mix(0.2, 4.0, u_src_n3);
        float t = m * theta / 4.0;
        float sf = pow(pow(abs(cos(t)), n2) + pow(abs(sin(t)), n3), -1.0 / n1);
        float scale = mix(0.15, 0.6, u_src_size);
        // Fill shape interior
        float fill = smoothstep(0.02, 0.0, r - sf * scale) * 0.6;
        // Edge glow
        float edge = 0.0;
        for (int i = 0; i < 3; i++) {
            float layerScale = scale * (1.0 + float(i) * 0.15);
            float dist = abs(r - sf * layerScale);
            float thickness = 0.015 + float(i) * 0.005;
            edge += smoothstep(thickness, 0.0, dist) * (1.0 - float(i) * 0.25);
        }
        float shape = max(fill, edge);
        float hue = u_src_color_shift + theta / 6.28318 * 0.5;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= shape * (1.0 + u_rms * 0.5);
        fragColor = vec4(col, 1.0);
    }
)";

inline const char* sourceTruchet = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_beatPhase;
    uniform float u_barPhase;
    uniform float u_src_density;
    uniform float u_src_style;
    uniform float u_src_thickness;
    uniform float u_src_glow;
    uniform float u_src_color_shift;
    float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
    void main() {
        vec2 uv = gl_FragCoord.xy / u_resolution;
        float density = mix(4.0, 20.0, u_src_density);
        vec2 gridUV = uv * density;
        vec2 cell = floor(gridUV);
        vec2 f = fract(gridUV);
        // Tile rotation based on hash + time
        float h = hash(cell + floor(u_time * 0.5) * 0.1);
        float rot = floor(h * 4.0); // 0, 1, 2, 3
        // Rotate UV within cell
        if (rot >= 1.0) { f = vec2(1.0 - f.y, f.x); }
        if (rot >= 2.0) { f = vec2(1.0 - f.y, f.x); }
        if (rot >= 3.0) { f = vec2(1.0 - f.y, f.x); }
        float pattern = 0.0;
        float thickness = mix(0.02, 0.2, u_src_thickness);
        if (u_src_style < 0.33) {
            // Quarter circles
            float d1 = abs(length(f) - 1.0);
            float d2 = abs(length(f - 1.0) - 1.0);
            pattern = smoothstep(thickness, thickness * 0.3, d1) +
                      smoothstep(thickness, thickness * 0.3, d2);
        } else if (u_src_style < 0.66) {
            // Diagonal lines
            float d1 = abs(f.x + f.y - 1.0) / 1.414;
            float d2 = abs(f.x - f.y) / 1.414;
            pattern = smoothstep(thickness, thickness * 0.3, min(d1, d2));
        } else {
            // Triangles
            float d = abs(f.x - f.y);
            pattern = smoothstep(thickness, thickness * 0.3, d);
            float d2 = abs(f.x + f.y - 1.0);
            pattern += smoothstep(thickness * 0.7, thickness * 0.2, d2) * 0.5;
        }
        pattern = clamp(pattern, 0.0, 1.0);
        // Glow
        pattern *= (0.6 + u_src_glow * 0.8);
        float hue = u_src_color_shift + hash(cell) * 0.2 + u_time * 0.05;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= pattern * (1.2 + u_rms * 0.5);
        fragColor = vec4(col, 1.0);
    }
)";

inline const char* sourceParticleNebula = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_spectralCentroid;
    uniform float u_src_density;
    uniform float u_src_scale;
    uniform float u_src_speed;
    uniform float u_src_color_shift;
    // Simplex-like noise
    float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
    float noise(vec2 p) {
        vec2 i = floor(p);
        vec2 f = fract(p);
        f = f * f * (3.0 - 2.0 * f);
        float a = hash(i);
        float b = hash(i + vec2(1.0, 0.0));
        float c = hash(i + vec2(0.0, 1.0));
        float d = hash(i + vec2(1.0, 1.0));
        return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
    }
    float fbm(vec2 p) {
        float v = 0.0;
        float amp = 0.5;
        for (int i = 0; i < 5; i++) {
            v += noise(p) * amp;
            p *= 2.1;
            amp *= 0.5;
        }
        return v;
    }
    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        float scale = mix(1.0, 5.0, u_src_scale);
        float speed = mix(0.1, 0.6, u_src_speed);
        uv *= scale;
        uv += u_time * speed * vec2(0.3, 0.2);
        // Multi-layer nebula
        float n1 = fbm(uv);
        float n2 = fbm(uv * 1.5 + 3.7);
        float n3 = fbm(uv + vec2(n1, n2) * 0.5);
        float combined = n3 * mix(0.5, 2.0, u_src_density);
        combined = pow(combined, mix(1.5, 0.5, u_bass));
        // Color: nebula gradient
        float hue = u_src_color_shift + combined * 0.3 + u_time * 0.02;
        vec3 col1 = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        vec3 col2 = 0.5 + 0.5 * cos(6.28318 * (hue + 0.5 + vec3(0.0, 0.33, 0.67)));
        vec3 col = mix(col1, col2, combined);
        col *= combined * (0.7 + u_rms * 0.6);
        fragColor = vec4(col, 1.0);
    }
)";

inline const char* sourceLightning = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_onsetStrength;
    uniform float u_onsetDetected;
    uniform float u_spectralCentroid;
    uniform float u_src_intensity;
    uniform float u_src_branches;
    uniform float u_src_glow;
    uniform float u_src_color_shift;
    float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
    float noise(vec2 p) {
        vec2 i = floor(p);
        vec2 f = fract(p);
        f = f * f * (3.0 - 2.0 * f);
        return mix(mix(hash(i), hash(i + vec2(1, 0)), f.x),
                   mix(hash(i + vec2(0, 1)), hash(i + vec2(1, 1)), f.x), f.y);
    }
    float bolt(vec2 uv, float seed, float branches) {
        float brightness = 0.0;
        float segments = 20.0;
        vec2 prev = vec2(hash(vec2(seed, 0.0)) * 0.8 - 0.4 + 0.5, 1.0);
        for (float i = 1.0; i <= 20.0; i++) {
            float t = i / segments;
            float nx = noise(vec2(seed * 10.0 + i * 0.5, u_time * 8.0)) * 0.6 - 0.3;
            vec2 curr = vec2(prev.x + nx * 0.15, 1.0 - t);
            // Line segment distance
            vec2 pa = uv - prev;
            vec2 ba = curr - prev;
            float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
            float d = length(pa - ba * h);
            brightness += smoothstep(0.02, 0.0, d) * (1.0 - t * 0.5);
            // Branch
            if (branches > 0.3 && hash(vec2(seed + i, 3.0)) < branches * 0.3) {
                vec2 branchEnd = curr + vec2(hash(vec2(seed + i, 5.0)) * 0.2 - 0.1, -0.05);
                vec2 pb = uv - curr;
                vec2 bb = branchEnd - curr;
                float hb = clamp(dot(pb, bb) / dot(bb, bb), 0.0, 1.0);
                float db = length(pb - bb * hb);
                brightness += smoothstep(0.015, 0.0, db) * 0.5 * (1.0 - t);
            }
            prev = curr;
        }
        return brightness;
    }
    void main() {
        vec2 uv = gl_FragCoord.xy / u_resolution;
        float intensity = mix(0.3, 2.0, u_src_intensity);
        // Trigger bolts on onset or periodically
        float trigger = max(u_onsetStrength, sin(u_time * 2.0) * 0.3 + 0.3);
        float brightness = 0.0;
        int numBolts = int(trigger * 3.0) + 1;
        for (int b = 0; b < 3; b++) {
            if (b >= numBolts) break;
            float seed = floor(u_time * 6.0) + float(b) * 7.3;
            brightness += bolt(uv, seed, u_src_branches) * intensity;
        }
        // Glow
        brightness += brightness * u_src_glow * 0.5;
        // Color
        float hue = u_src_color_shift;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        // White core + colored glow
        brightness *= 2.0;
        vec3 result = mix(col * brightness, vec3(brightness), min(brightness * 0.5, 1.0));
        fragColor = vec4(result, 1.0);
    }
)";

inline const char* sourceFire = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_onsetStrength;
    uniform float u_src_height;
    uniform float u_src_turbulence;
    uniform float u_src_speed;
    uniform float u_src_color_shift;
    float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
    float noise(vec2 p) {
        vec2 i = floor(p);
        vec2 f = fract(p);
        f = f * f * (3.0 - 2.0 * f);
        return mix(mix(hash(i), hash(i + vec2(1, 0)), f.x),
                   mix(hash(i + vec2(0, 1)), hash(i + vec2(1, 1)), f.x), f.y);
    }
    float fbm(vec2 p) {
        float v = 0.0;
        float amp = 0.5;
        mat2 rot = mat2(0.8, -0.6, 0.6, 0.8);
        for (int i = 0; i < 5; i++) {
            v += noise(p) * amp;
            p = rot * p * 2.1;
            amp *= 0.5;
        }
        return v;
    }
    void main() {
        vec2 uv = gl_FragCoord.xy / u_resolution;
        float speed = mix(1.0, 5.0, u_src_speed);
        float turb = mix(1.0, 6.0, u_src_turbulence);
        float height = mix(0.3, 1.0, u_src_height) + u_bass * 0.3;
        // Fire shape: strongest at bottom, fading up
        vec2 fireUV = vec2(uv.x * 3.0, uv.y * 2.0 - u_time * speed);
        float n = fbm(fireUV * turb);
        float n2 = fbm(fireUV * turb * 1.5 + 5.0);
        // Flame envelope
        float flameShape = (1.0 - uv.y) * height;
        flameShape *= smoothstep(0.0, 0.3, 0.5 - abs(uv.x - 0.5));
        float fire = n * n2 * flameShape * 3.0;
        fire = pow(max(fire, 0.0), 1.5);
        // Fire palette: black → red → orange → yellow → white
        vec3 col;
        if (u_src_color_shift < 0.33) {
            // Orange fire
            col = mix(vec3(0.0), vec3(1.0, 0.2, 0.0), clamp(fire, 0.0, 1.0));
            col = mix(col, vec3(1.0, 0.6, 0.0), clamp(fire - 0.3, 0.0, 1.0));
            col = mix(col, vec3(1.0, 1.0, 0.5), clamp(fire - 0.7, 0.0, 1.0));
        } else if (u_src_color_shift < 0.66) {
            // Blue fire
            col = mix(vec3(0.0), vec3(0.0, 0.2, 1.0), clamp(fire, 0.0, 1.0));
            col = mix(col, vec3(0.2, 0.5, 1.0), clamp(fire - 0.3, 0.0, 1.0));
            col = mix(col, vec3(0.7, 0.9, 1.0), clamp(fire - 0.7, 0.0, 1.0));
        } else {
            // Green fire
            col = mix(vec3(0.0), vec3(0.0, 0.8, 0.2), clamp(fire, 0.0, 1.0));
            col = mix(col, vec3(0.3, 1.0, 0.3), clamp(fire - 0.3, 0.0, 1.0));
            col = mix(col, vec3(0.8, 1.0, 0.7), clamp(fire - 0.7, 0.0, 1.0));
        }
        // Onset flare
        col += col * u_onsetStrength * 0.5;
        fragColor = vec4(col, 1.0);
    }
)";

inline const char* sourceStarfield = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_beatPhase;
    uniform float u_onsetStrength;
    uniform float u_src_speed;
    uniform float u_src_density;
    uniform float u_src_streak;
    uniform float u_src_color_shift;
    float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        float speed = mix(0.3, 2.0, u_src_speed);
        float t = u_time * speed;
        vec3 result = vec3(0.0);
        for (int layer = 0; layer < 5; layer++) {
            float depth = 0.5 + float(layer) * 0.5;
            float layerDensity = mix(6.0, 20.0, u_src_density);
            vec2 starUV = uv * layerDensity / depth;
            starUV.y -= t / depth;
            vec2 cell = floor(starUV);
            vec2 f = fract(starUV) - 0.5;
            float h = hash(cell + float(layer) * 100.0);
            if (h > 0.3) {
                vec2 starPos = vec2(h * 0.6 - 0.3, hash(cell.yx + float(layer) * 50.0) * 0.6 - 0.3);
                float d = length(f - starPos);
                float starSize = 0.05 + h * 0.08;
                float star = smoothstep(starSize, 0.0, d);
                // Glow halo
                float glow = smoothstep(starSize * 3.0, 0.0, d) * 0.3;
                star += glow;
                // Streak
                if (u_src_streak > 0.01) {
                    vec2 streakDir = vec2(0.0, starSize * u_src_streak * 4.0);
                    float streakD = length(f - starPos - streakDir * 0.5);
                    star = max(star, smoothstep(starSize * 2.0, 0.0, min(d, streakD)) * 0.6);
                }
                float twinkle = sin(u_time * 3.0 + h * 50.0) * 0.2 + 0.8;
                float brightness = star * twinkle / depth;
                float hue = u_src_color_shift + h * 0.3;
                vec3 starCol = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
                starCol = mix(starCol, vec3(1.0), star * 0.5);
                result += starCol * brightness;
            }
        }
        result *= (1.0 + u_rms * 0.5);
        fragColor = vec4(result, 1.0);
    }
)";

inline const char* sourceDNAHelix = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_beatPhase;
    uniform float u_chromagram[12];
    uniform float u_src_speed;
    uniform float u_src_zoom;
    uniform float u_src_glow;
    uniform float u_src_color_shift;
    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        float speed = mix(0.5, 3.0, u_src_speed);
        float zoom = mix(0.3, 1.5, u_src_zoom);
        uv *= zoom;
        float rotAngle = u_time * speed;
        // Double helix parameters
        float helixY = uv.y;
        float phase1 = helixY * 8.0 + rotAngle;
        float phase2 = phase1 + 3.14159;
        float strand1X = sin(phase1) * 0.25;
        float strand2X = sin(phase2) * 0.25;
        float strand1Z = cos(phase1);
        float strand2Z = cos(phase2);
        // Draw strands as tubes
        // Glow around strands
        float glowR = 0.25;
        float d1 = length(vec2(uv.x - strand1X, 0.0));
        float d2 = length(vec2(uv.x - strand2X, 0.0));
        float tubeR = 0.05;
        float tube1 = smoothstep(tubeR, tubeR * 0.2, d1) * (strand1Z * 0.3 + 0.7);
        float tube2 = smoothstep(tubeR, tubeR * 0.2, d2) * (strand2Z * 0.3 + 0.7);
        // Add glow halos
        tube1 += smoothstep(glowR, 0.0, d1) * 0.4;
        tube2 += smoothstep(glowR, 0.0, d2) * 0.4;
        // Rungs connecting strands (12 per turn = pitch classes)
        float rungBrightness = 0.0;
        for (int i = 0; i < 12; i++) {
            float rungPhase = float(i) * 3.14159 * 2.0 / 12.0;
            float rungY = fract((helixY * 8.0 + rotAngle - rungPhase) / (2.0 * 3.14159));
            if (abs(rungY - 0.5) < 0.01) {
                float rungX1 = sin(rungPhase + rotAngle) * 0.25;
                float rungX2 = sin(rungPhase + rotAngle + 3.14159) * 0.25;
                float minX = min(rungX1, rungX2);
                float maxX = max(rungX1, rungX2);
                if (uv.x > minX - 0.01 && uv.x < maxX + 0.01) {
                    float chromaVal = u_chromagram[i];
                    rungBrightness += chromaVal * 0.8;
                }
            }
        }
        float glowMult = 1.0 + u_src_glow * 0.8;
        float hue = u_src_color_shift + helixY * 0.1;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        vec3 col2 = 0.5 + 0.5 * cos(6.28318 * (hue + 0.5 + vec3(0.0, 0.33, 0.67)));
        vec3 result = (col * tube1 + col2 * tube2 + vec3(0.8, 0.9, 1.0) * rungBrightness) * glowMult;
        result *= (1.0 + u_rms * 0.5);
        fragColor = vec4(result, 1.0);
    }
)";

inline const char* sourceFibonacci = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_beatPhase;
    uniform float u_src_elements;
    uniform float u_src_size;
    uniform float u_src_spread;
    uniform float u_src_glow;
    uniform float u_src_color_shift;
    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        int maxElements = int(mix(20.0, 200.0, u_src_elements));
        float elementSize = mix(0.02, 0.08, u_src_size);
        float spread = mix(0.015, 0.06, u_src_spread);
        float goldenAngle = 2.39996323;
        vec3 result = vec3(0.0);
        for (int i = 0; i < 200; i++) {
            if (i >= maxElements) break;
            float fibAngle = float(i) * goldenAngle + u_time * 0.3;
            float fibRadius = sqrt(float(i)) * spread;
            vec2 pos = vec2(cos(fibAngle), sin(fibAngle)) * fibRadius;
            float d = length(uv - pos);
            float elem = smoothstep(elementSize, elementSize * 0.1, d);
            // Glow halo
            elem += smoothstep(elementSize * 3.0, 0.0, d) * 0.15;
            // Pulse individual elements
            float pulse = sin(u_time * 2.0 + float(i) * 0.3) * 0.3 + 0.7;
            float hue = u_src_color_shift + float(i) / float(maxElements);
            vec3 elemCol = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
            result += elemCol * elem * pulse;
        }
        result *= (0.8 + u_src_glow * 1.0) * (1.0 + u_rms * 0.5);
        fragColor = vec4(result, 1.0);
    }
)";

inline const char* sourceRadar = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_beatPhase;
    uniform float u_bandEnergies[7];
    uniform float u_onsetStrength;
    uniform float u_src_speed;
    uniform float u_src_decay;
    uniform float u_src_grid;
    uniform float u_src_color_shift;
    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        float radius = length(uv);
        float angle = atan(uv.y, uv.x);
        float PI = 3.14159265;
        // Sweep line
        float sweepAngle = u_time * mix(1.0, 5.0, u_src_speed);
        float sweepMod = mod(sweepAngle, 2.0 * PI);
        float angleDiff = mod(angle - sweepMod + 2.0 * PI, 2.0 * PI);
        float sweepTrail = mix(0.3, 1.5, u_src_decay);
        float sweep = exp(-angleDiff / sweepTrail) * 0.8;
        // Sweep line itself
        sweep += smoothstep(0.05, 0.0, angleDiff) * 1.5;
        // Grid rings
        float gridIntensity = u_src_grid;
        float rings = smoothstep(0.005, 0.0, abs(mod(radius, 0.1) - 0.05)) * gridIntensity * 0.4;
        // Cross lines
        float cross = smoothstep(0.003, 0.0, abs(uv.x)) * gridIntensity * 0.2;
        cross += smoothstep(0.003, 0.0, abs(uv.y)) * gridIntensity * 0.2;
        // Band energy blips
        float blips = 0.0;
        for (int i = 0; i < 7; i++) {
            float blipR = float(i + 1) * 0.06;
            float blipAngle = float(i) * PI * 2.0 / 7.0 + sweepMod;
            vec2 blipPos = vec2(cos(blipAngle), sin(blipAngle)) * blipR;
            float d = length(uv - blipPos);
            blips += smoothstep(0.02, 0.0, d) * u_bandEnergies[i] * 2.0;
        }
        // Circular mask
        float mask = smoothstep(0.5, 0.48, radius);
        float total = (sweep + rings + cross + blips) * mask;
        // Color
        float hue = u_src_color_shift;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        vec3 result = col * total;
        // White-hot blips
        result += vec3(blips * 0.5);
        fragColor = vec4(result, 1.0);
    }
)";

inline const char* sourceGlitchGrid = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_onsetStrength;
    uniform float u_beatPhase;
    uniform float u_src_grid;
    uniform float u_src_chaos;
    uniform float u_src_flicker;
    uniform float u_src_color_shift;
    float hash(vec2 p) { return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453); }
    void main() {
        vec2 uv = gl_FragCoord.xy / u_resolution;
        float cellCount = mix(4.0, 16.0, u_src_grid);
        vec2 cellIdx = floor(uv * cellCount);
        vec2 cellUV = fract(uv * cellCount);
        float cellRand = hash(cellIdx);
        float timeStep = floor(u_time * mix(2.0, 12.0, u_src_flicker));
        float flickerRand = hash(cellIdx + timeStep);
        // Glitch: shuffle some cells
        vec2 displayIdx = cellIdx;
        if (cellRand < u_src_chaos) {
            displayIdx = vec2(
                mod(cellIdx.x + floor(hash(vec2(timeStep, cellIdx.y)) * cellCount), cellCount),
                mod(cellIdx.y + floor(hash(vec2(cellIdx.x, timeStep)) * cellCount), cellCount)
            );
        }
        // Cell content: colored rectangle with random fill
        float fill = hash(displayIdx + 0.1);
        float brightness = flickerRand;
        brightness *= step(0.2, fill);
        // Border
        float border = step(0.05, cellUV.x) * step(cellUV.x, 0.95) *
                       step(0.05, cellUV.y) * step(cellUV.y, 0.95);
        float hue = u_src_color_shift + hash(displayIdx * 3.0) * 0.4;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= brightness * border;
        // Onset flash
        col += col * u_onsetStrength * 0.5;
        col *= (0.6 + u_rms * 0.6);
        fragColor = vec4(col, 1.0);
    }
)";

inline const char* sourceRoseCurves = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_dominantPitch;
    uniform float u_beatPhase;
    uniform float u_src_k;
    uniform float u_src_thickness;
    uniform float u_src_layers;
    uniform float u_src_glow;
    uniform float u_src_color_shift;
    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        float theta = atan(uv.y, uv.x) + u_time * 0.2;
        float r = length(uv);
        float k = mix(2.0, 12.0, u_src_k);
        float thickness = mix(0.003, 0.04, u_src_thickness);
        float layerCount = mix(1.0, 5.0, u_src_layers);
        vec3 result = vec3(0.0);
        for (int i = 0; i < 5; i++) {
            if (float(i) >= layerCount) break;
            float layerK = k + float(i) * 0.7;
            float layerRot = float(i) * 0.3;
            float roseR = abs(cos(layerK * (theta + layerRot))) * 0.35;
            float d = abs(r - roseR);
            float curve = smoothstep(thickness, thickness * 0.1, d);
            // Fill interior slightly
            float fill = smoothstep(0.01, 0.0, r - roseR) * 0.15;
            float brightness = (curve + fill) * (1.0 - float(i) * 0.15);
            float hue = u_src_color_shift + float(i) * 0.12 + theta / 6.28318 * 0.2;
            vec3 layerCol = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
            result += layerCol * brightness;
        }
        result *= (0.8 + u_src_glow * 1.0) * (1.0 + u_rms * 0.5);
        fragColor = vec4(result, 1.0);
    }
)";

inline const char* sourceDotMatrixWave = R"(#version 410 core
    out vec4 fragColor;
    uniform vec2 u_resolution;
    uniform float u_time;
    uniform float u_rms;
    uniform float u_beatPhase;
    uniform float u_onsetStrength;
    uniform float u_bandEnergies[7];
    uniform float u_src_density;
    uniform float u_src_speed;
    uniform float u_src_damping;
    uniform float u_src_size;
    uniform float u_src_color;
    uniform float u_src_sources;
    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        float density = mix(8.0, 48.0, u_src_density);
        float dotSize = mix(0.01, 0.04, u_src_size);
        float speed = mix(2.0, 10.0, u_src_speed);
        float damping = mix(0.5, 3.0, u_src_damping);
        int numSources = int(mix(1.0, 5.0, u_src_sources));
        // Grid of dots
        vec2 gridUV = uv * density;
        vec2 cell = floor(gridUV);
        vec2 f = fract(gridUV) - 0.5;
        vec2 dotPos = cell / density;
        // Wave sources
        float waveAmp = 0.0;
        for (int s = 0; s < 5; s++) {
            if (s >= numSources) break;
            // Source positions spread across the field
            float angle = float(s) * 6.28318 / float(numSources) + u_time * 0.2;
            vec2 srcPos = vec2(cos(angle), sin(angle)) * 0.2;
            float dist = length(dotPos - srcPos);
            float wave = sin(dist * speed * 6.28318 - u_time * speed) / (1.0 + damping * dist);
            waveAmp += wave * 0.5;
        }
        // Dot rendering with wave-modulated size
        float modSize = dotSize * (1.0 + waveAmp);
        modSize = max(modSize, 0.002);
        float dot = smoothstep(modSize, modSize * 0.3, length(f / density));
        // Color
        float hue = u_src_color + waveAmp * 0.2;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        float brightness = dot * (0.5 + abs(waveAmp)) * (0.7 + u_rms * 0.5);
        fragColor = vec4(col * brightness, 1.0);
    }
)";

// === Phase 20: Text Animator Source ===
inline const char* sourceTextAnimator = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_beatPhase;
    uniform float u_onsetStrength;
    uniform float u_src_font_size;
    uniform float u_src_text_r;
    uniform float u_src_text_g;
    uniform float u_src_text_b;
    uniform float u_src_anim;
    uniform float u_src_speed;
    uniform float u_src_columns;
    uniform float u_src_spacing;

    // Simple pseudo-random
    float hash(vec2 p) {
        return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
    }

    // 7-segment digit renderer
    float segment(vec2 p, vec2 a, vec2 b, float w) {
        vec2 pa = p - a, ba = b - a;
        float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
        return smoothstep(w, w * 0.3, length(pa - ba * h));
    }

    // Render a pseudo-character glyph (block-based like pixel art text)
    float renderGlyph(vec2 uv, float charId, float strokeW) {
        float w = strokeW;
        float s = 0.0;
        int bits = int(hash(vec2(charId, 0.0)) * 127.0);
        // Top horizontal
        if ((bits & 1) != 0) s += segment(uv, vec2(0.1, 0.9), vec2(0.9, 0.9), w);
        // Middle horizontal
        if ((bits & 2) != 0) s += segment(uv, vec2(0.1, 0.5), vec2(0.9, 0.5), w);
        // Bottom horizontal
        if ((bits & 4) != 0) s += segment(uv, vec2(0.1, 0.1), vec2(0.9, 0.1), w);
        // Top-left vertical
        if ((bits & 8) != 0) s += segment(uv, vec2(0.1, 0.5), vec2(0.1, 0.9), w);
        // Top-right vertical
        if ((bits & 16) != 0) s += segment(uv, vec2(0.9, 0.5), vec2(0.9, 0.9), w);
        // Bottom-left vertical
        if ((bits & 32) != 0) s += segment(uv, vec2(0.1, 0.1), vec2(0.1, 0.5), w);
        // Bottom-right vertical
        if ((bits & 64) != 0) s += segment(uv, vec2(0.9, 0.1), vec2(0.9, 0.5), w);
        return clamp(s, 0.0, 1.0);
    }

    void main() {
        vec2 uv = v_texCoord;
        float speed = mix(0.2, 3.0, u_src_speed);
        float fontSize = mix(4.0, 40.0, u_src_font_size);
        float cols = mix(4.0, 60.0, u_src_columns);
        float spacing = mix(0.8, 2.0, u_src_spacing);
        float anim = u_src_anim;

        // Grid of characters
        float aspect = u_resolution.x / u_resolution.y;
        float rows = cols / aspect * spacing;
        vec2 grid = vec2(cols, rows);

        // Animation modes
        if (anim < 0.25) {
            // Scroll up
            uv.y += u_time * speed * 0.2;
        } else if (anim < 0.5) {
            // Scroll left
            uv.x += u_time * speed * 0.2;
        } else if (anim < 0.75) {
            // Wave
            uv.y += sin(uv.x * 6.28318 * 2.0 + u_time * speed) * 0.02;
        }
        // else: typewriter (handled per-character below)

        vec2 cellId = floor(uv * grid);
        vec2 cellUv = fract(uv * grid);

        // Character ID based on cell position + time animation
        float charId;
        if (anim >= 0.75) {
            // Typewriter: reveal characters left-to-right
            float reveal = fract(u_time * speed * 0.3) * (cols + 5.0);
            if (cellId.x > reveal) {
                fragColor = vec4(0.0);
                return;
            }
            charId = hash(cellId) * 100.0;
        } else {
            charId = hash(cellId + floor(vec2(u_time * speed * 0.5))) * 100.0;
        }

        // Render glyph
        // fontSize controls stroke width: thin glyphs at 0, thick at 1
        float strokeWidth = mix(0.02, 0.12, u_src_font_size);
        float glyph = renderGlyph(cellUv, charId, strokeWidth);

        // Color with audio reactivity
        vec3 textColor = vec3(u_src_text_r, u_src_text_g, u_src_text_b);
        float brightness = glyph * (0.7 + u_rms * 0.5);

        // Onset flash
        brightness += u_onsetStrength * 0.3 * glyph;

        // Beat pulse on random characters
        if (hash(cellId * 3.7) > 0.7)
            brightness *= 1.0 + u_beatPhase * 0.3;

        fragColor = vec4(textColor * brightness, brightness > 0.01 ? 1.0 : 0.0);
    }
)";

// === Phase 20: Strange Attractor Field Source ===
inline const char* sourceStrangeAttractor = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;  // Previous state (for trail persistence)
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_beatPhase;
    uniform float u_onsetStrength;
    uniform float u_src_attractor;
    uniform float u_src_speed;
    uniform float u_src_trail;
    uniform float u_src_rotation;
    uniform float u_src_glow;
    uniform float u_src_color_mode;

    // Lorenz attractor
    vec3 lorenz(vec3 p) {
        float sigma = 10.0, rho = 28.0, beta = 8.0 / 3.0;
        return vec3(sigma * (p.y - p.x), p.x * (rho - p.z) - p.y, p.x * p.y - beta * p.z);
    }

    // Rossler attractor
    vec3 rossler(vec3 p) {
        float a = 0.2, b = 0.2, c = 5.7;
        return vec3(-p.y - p.z, p.x + a * p.y, b + p.z * (p.x - c));
    }

    // Halvorsen attractor
    vec3 halvorsen(vec3 p) {
        float a = 1.89;
        return vec3(-a * p.x - 4.0 * p.y - 4.0 * p.z - p.y * p.y,
                    -a * p.y - 4.0 * p.z - 4.0 * p.x - p.z * p.z,
                    -a * p.z - 4.0 * p.x - 4.0 * p.y - p.x * p.x);
    }

    // Thomas attractor
    vec3 thomas(vec3 p) {
        float b = 0.208186;
        return vec3(sin(p.y) - b * p.x, sin(p.z) - b * p.y, sin(p.x) - b * p.z);
    }

    // Aizawa attractor
    vec3 aizawa(vec3 p) {
        float a = 0.95, b = 0.7, c = 0.6, d = 3.5, e = 0.25, f = 0.1;
        return vec3((p.z - b) * p.x - d * p.y,
                    d * p.x + (p.z - b) * p.y,
                    c + a * p.z - p.z * p.z * p.z / 3.0 - (p.x * p.x + p.y * p.y) * (1.0 + e * p.z) + f * p.z * p.x * p.x * p.x);
    }

    // Dadras attractor
    vec3 dadras(vec3 p) {
        float a = 3.0, b = 2.7, c = 1.7, d = 2.0, e = 9.0;
        return vec3(p.y - a * p.x + b * p.y * p.z,
                    c * p.y - p.x * p.z + p.z,
                    d * p.x * p.y - e * p.z);
    }

    vec3 computeAttractor(vec3 p, int type) {
        if (type == 0) return lorenz(p);
        if (type == 1) return rossler(p);
        if (type == 2) return halvorsen(p);
        if (type == 3) return thomas(p);
        if (type == 4) return aizawa(p);
        return dadras(p);
    }

    void main() {
        vec2 uv = v_texCoord;

        // Read previous frame for trail persistence
        vec4 prev = texture(u_texture, uv);
        float trail = mix(0.85, 0.995, u_src_trail);
        vec4 faded = prev * trail;

        float speed = mix(0.001, 0.01, u_src_speed) * (0.7 + u_rms * 0.5);
        int attractorType = int(u_src_attractor * 5.0 + 0.5);

        // Rotation for viewing angle
        float rotSpeed = mix(-1.0, 1.0, u_src_rotation);
        float angle = u_time * rotSpeed * 0.5;
        float ca = cos(angle), sa = sin(angle);

        // Simulate multiple particles using pixel-seeded initial conditions
        float brightness = 0.0;
        vec3 particleColor = vec3(0.0);
        float dt = speed;

        for (int i = 0; i < 8; i++) {
            // Seed particles from pixel neighborhood + offsets
            vec3 p;
            if (attractorType == 0) p = vec3(1.0, 1.0, 1.0) + vec3(float(i) * 0.1);
            else if (attractorType == 1) p = vec3(0.1, 0.1, 0.1) + vec3(float(i) * 0.05);
            else if (attractorType == 2) p = vec3(-1.0, -1.0, -2.0) + vec3(float(i) * 0.1);
            else if (attractorType == 3) p = vec3(1.0, 0.0, 0.0) + vec3(float(i) * 0.1);
            else if (attractorType == 4) p = vec3(0.1, 0.0, 0.0) + vec3(float(i) * 0.05);
            else p = vec3(1.0, 1.0, 0.0) + vec3(float(i) * 0.1);

            // Integrate forward to time-dependent position
            int steps = int(u_time * 100.0 * speed * 10.0) + i * 50;
            steps = min(steps, 500);
            for (int s = 0; s < 500; s++) {
                if (s >= steps) break;
                vec3 dp = computeAttractor(p, attractorType);
                p += dp * dt;
            }

            // Project 3D -> 2D with rotation
            vec3 rp = vec3(ca * p.x - sa * p.z, p.y, sa * p.x + ca * p.z);

            // Scale to screen coordinates
            float scale;
            if (attractorType == 0) scale = 0.02;      // Lorenz
            else if (attractorType == 1) scale = 0.04;  // Rossler
            else if (attractorType == 2) scale = 0.04;  // Halvorsen
            else if (attractorType == 3) scale = 0.15;  // Thomas
            else if (attractorType == 4) scale = 0.2;   // Aizawa
            else scale = 0.02;                           // Dadras

            vec2 projected = rp.xy * scale + 0.5;
            float dist = length(uv - projected);

            // Point size with glow
            float glowAmount = mix(0.005, 0.03, u_src_glow);
            float point = exp(-dist * dist / (glowAmount * glowAmount));

            // Velocity-based coloring
            vec3 dp = computeAttractor(p, attractorType);
            float vel = length(dp);

            vec3 col;
            float cm = u_src_color_mode;
            if (cm < 0.33) {
                // Velocity-mapped color
                col = 0.5 + 0.5 * cos(6.28318 * (vel * 0.01 + vec3(0.0, 0.33, 0.67)));
            } else if (cm < 0.67) {
                // Position-mapped color
                col = 0.5 + 0.5 * cos(6.28318 * (length(p) * 0.05 + vec3(0.0, 0.33, 0.67)));
            } else {
                // Fixed hue cycling with time
                col = 0.5 + 0.5 * cos(6.28318 * (u_time * 0.1 + vec3(0.0, 0.33, 0.67)));
            }

            brightness += point;
            particleColor += col * point;
        }

        // Combine with faded previous frame
        vec3 newColor = brightness > 0.001 ? particleColor / max(brightness, 0.001) : vec3(0.0);
        vec3 result = max(faded.rgb, newColor * brightness);

        // Onset creates burst
        result += newColor * u_onsetStrength * 0.5;

        fragColor = vec4(result, 1.0);
    }
)";

// === Phase 20: Gravity Well Source ===
inline const char* sourceGravityWell = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;  // Previous state (particle field)
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_beatPhase;
    uniform float u_onsetStrength;
    uniform float u_onsetDetected;
    uniform float u_spectralCentroid;
    uniform float u_src_gravity;
    uniform float u_src_scatter;
    uniform float u_src_trail;
    uniform float u_src_wells;
    uniform float u_src_color_mode;
    uniform float u_src_particle_density;

    float hash(vec2 p) {
        return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
    }

    vec2 hash2(vec2 p) {
        return fract(sin(vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)))) * 43758.5453);
    }

    void main() {
        vec2 uv = v_texCoord;
        vec2 pos = (uv - 0.5) * 2.0; // [-1, 1]

        // Previous frame for trail persistence
        vec4 prev = texture(u_texture, uv);
        float trailFade = mix(0.90, 0.995, u_src_trail);

        // Number of gravity wells (1-4)
        int numWells = int(u_src_wells * 3.0 + 1.5);
        float gravity = mix(0.5, 5.0, u_src_gravity);

        // Scatter force on onset
        float scatterForce = u_onsetDetected * mix(0.0, 2.0, u_src_scatter);

        // Compute gravitational field at this pixel
        vec2 totalForce = vec2(0.0);
        float totalPotential = 0.0;

        for (int i = 0; i < 4; i++) {
            if (i >= numWells) break;

            // Well positions (orbit slowly, affected by audio)
            float angle = float(i) * 6.28318 / float(numWells) + u_time * 0.3;
            float radius = 0.3 + 0.1 * sin(u_time * 0.5 + float(i));
            vec2 wellPos;
            if (numWells == 1) {
                wellPos = vec2(0.0);
            } else {
                wellPos = vec2(cos(angle), sin(angle)) * radius;
            }

            // Bass makes well pulse
            wellPos *= 1.0 + u_bass * 0.2;

            vec2 dir = wellPos - pos;
            float dist = length(dir) + 0.01;
            vec2 force = normalize(dir) * gravity / (dist * dist + 0.1);

            // Scatter: reverse force on onset
            force -= normalize(dir) * scatterForce / (dist + 0.1);

            totalForce += force;
            totalPotential += 1.0 / (dist + 0.1);
        }

        // Particle generation — use force field to create visual
        // Particles are implicitly represented by the force field
        float density = mix(10.0, 80.0, u_src_particle_density);
        vec2 cellId = floor(uv * density);
        vec2 cellUv = fract(uv * density) - 0.5;

        // Each cell has a particle
        vec2 particleOffset = hash2(cellId) - 0.5;
        particleOffset *= 0.8;

        // Apply force to particle position (advection)
        vec2 advectedPos = cellUv - particleOffset;
        advectedPos += totalForce * 0.01;

        float dist = length(advectedPos);
        float particle = smoothstep(0.15, 0.0, dist);

        // Color based on mode
        vec3 col;
        float cm = u_src_color_mode;
        if (cm < 0.33) {
            // Velocity-based
            float vel = length(totalForce);
            col = 0.5 + 0.5 * cos(6.28318 * (vel * 0.1 + vec3(0.0, 0.33, 0.67)));
        } else if (cm < 0.67) {
            // Distance-based
            col = 0.5 + 0.5 * cos(6.28318 * (totalPotential * 0.1 + vec3(0.0, 0.33, 0.67)));
        } else {
            // Fixed warm
            col = vec3(1.0, 0.6, 0.2);
        }

        // Orbital streaks from force field
        float streak = abs(dot(normalize(totalForce + 0.001), normalize(advectedPos + 0.001)));
        streak = pow(streak, 3.0);

        float brightness = (particle + streak * 0.3) * (0.5 + u_rms * 0.8);

        // Combine with trail
        vec3 newColor = col * brightness;
        vec3 result = max(prev.rgb * trailFade, newColor);

        // Onset explosion: bright flash at well centers
        if (u_onsetDetected > 0.5) {
            for (int i = 0; i < 4; i++) {
                if (i >= numWells) break;
                float angle = float(i) * 6.28318 / float(numWells) + u_time * 0.3;
                float radius = (numWells == 1) ? 0.0 : 0.3;
                vec2 wellPos = vec2(cos(angle), sin(angle)) * radius;
                float d = length(pos - wellPos);
                result += col * exp(-d * d * 5.0) * u_onsetStrength;
            }
        }

        fragColor = vec4(result, 1.0);
    }
)";

// === Phase 20: Fluid Dynamics Source ===
inline const char* sourceFluidDynamics = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;  // Previous state: RG = velocity, BA = dye
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_rms;
    uniform float u_bass;
    uniform float u_mid;
    uniform float u_high;
    uniform float u_beatPhase;
    uniform float u_onsetStrength;
    uniform float u_onsetDetected;
    uniform float u_spectralCentroid;
    uniform float u_src_viscosity;
    uniform float u_src_diffusion;
    uniform float u_src_inject_radius;
    uniform float u_src_color_mode;
    uniform float u_src_curl;
    uniform float u_src_decay;

    vec4 sampleState(vec2 uv) {
        return texture(u_texture, clamp(uv, 0.0, 1.0));
    }

    void main() {
        vec2 uv = v_texCoord;
        vec2 texel = 1.0 / u_resolution;

        // Read current state: RG = velocity field, BA = dye (hue, brightness)
        vec4 state = sampleState(uv);
        vec2 vel = state.rg * 2.0 - 1.0; // Decode from [0,1] to [-1,1]
        vec2 dye = state.ba;

        // === Advection: move quantities along velocity field ===
        float visc = mix(0.999, 0.95, u_src_viscosity);
        vec2 advectUV = uv - vel * texel * 2.0;
        vec4 advected = sampleState(advectUV);
        vec2 advVel = advected.rg * 2.0 - 1.0;
        vec2 advDye = advected.ba;

        // === Diffusion: smooth the velocity field ===
        float diff = mix(0.0, 0.3, u_src_diffusion);
        vec2 velL = sampleState(uv + vec2(-texel.x, 0.0)).rg * 2.0 - 1.0;
        vec2 velR = sampleState(uv + vec2( texel.x, 0.0)).rg * 2.0 - 1.0;
        vec2 velU = sampleState(uv + vec2(0.0,  texel.y)).rg * 2.0 - 1.0;
        vec2 velD = sampleState(uv + vec2(0.0, -texel.y)).rg * 2.0 - 1.0;
        vec2 velAvg = (velL + velR + velU + velD) * 0.25;

        // Apply viscosity (damping) and diffusion (smoothing)
        vel = mix(advVel, velAvg, diff) * visc;

        // === Vorticity confinement ===
        float curlStrength = mix(0.0, 0.5, u_src_curl);
        float wL = length(velL), wR = length(velR);
        float wU = length(velU), wD = length(velD);
        float curl = (wR - wL) - (wU - wD);
        vec2 curlForce = normalize(vec2(abs(wU) - abs(wD), abs(wR) - abs(wL)) + 0.0001);
        curlForce *= sign(curl) * curlStrength;
        vel += curlForce * texel.x;

        // === Pressure projection (simple relaxation) ===
        float divL = sampleState(uv + vec2(-texel.x, 0.0)).r * 2.0 - 1.0;
        float divR = sampleState(uv + vec2( texel.x, 0.0)).r * 2.0 - 1.0;
        float divU = sampleState(uv + vec2(0.0,  texel.y)).g * 2.0 - 1.0;
        float divD = sampleState(uv + vec2(0.0, -texel.y)).g * 2.0 - 1.0;
        float div = ((divR - divL) + (divU - divD)) * 0.5;
        vel -= vec2(divR - divL, divU - divD) * 0.25;

        // === Audio injection ===
        float injectR = mix(0.02, 0.15, u_src_inject_radius);
        vec2 pos = uv - 0.5;

        // Bass: inject from bottom center (blue dye, upward velocity)
        float bassDist = length(pos - vec2(0.0, -0.4));
        if (bassDist < injectR) {
            float strength = u_bass * smoothstep(injectR, 0.0, bassDist);
            vel += vec2(0.0, strength * 2.0);
            dye.x = mix(dye.x, 0.6, strength); // blue hue
            dye.y = max(dye.y, strength);
        }

        // Mid: inject from sides (green dye)
        for (int side = -1; side <= 1; side += 2) {
            float midDist = length(pos - vec2(float(side) * 0.4, 0.0));
            if (midDist < injectR) {
                float strength = u_mid * smoothstep(injectR, 0.0, midDist);
                vel += vec2(float(-side) * strength * 2.0, 0.0);
                dye.x = mix(dye.x, 0.33, strength); // green hue
                dye.y = max(dye.y, strength);
            }
        }

        // High: inject from top (magenta dye, downward velocity)
        float highDist = length(pos - vec2(0.0, 0.4));
        if (highDist < injectR) {
            float strength = u_high * smoothstep(injectR, 0.0, highDist);
            vel += vec2(0.0, -strength * 2.0);
            dye.x = mix(dye.x, 0.85, strength); // magenta hue
            dye.y = max(dye.y, strength);
        }

        // Onset: burst at center with spectral color
        if (u_onsetDetected > 0.5) {
            float onsetDist = length(pos);
            if (onsetDist < injectR * 2.0) {
                float strength = u_onsetStrength * smoothstep(injectR * 2.0, 0.0, onsetDist);
                // Radial burst
                vec2 dir = normalize(pos + 0.0001);
                vel += dir * strength * 3.0;
                // Spectral centroid maps to hue
                dye.x = u_spectralCentroid / 8000.0;
                dye.y = max(dye.y, strength);
            }
        }

        // === Dye advection and decay ===
        vec2 dyeAdvUV = uv - vel * texel * 2.0;
        vec2 advDyeFull = sampleState(dyeAdvUV).ba;
        dye = mix(advDyeFull, dye, 0.3);
        float decay = mix(0.998, 0.98, u_src_decay);
        dye.y *= decay;

        // Clamp velocity to [-1,1] range
        vel = clamp(vel, -1.0, 1.0);

        // Encode: RG = velocity (mapped to [0,1]), BA = dye
        vec2 velEncoded = vel * 0.5 + 0.5;

        // Store state
        // But also output visible color for the render
        // The state is stored in the ping-pong buffer; we output color
        float hue = dye.x;
        float brightness = dye.y;

        vec3 col;
        float cm = u_src_color_mode;
        if (cm < 0.33) {
            // Audio frequency colors
            col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        } else if (cm < 0.67) {
            // Complementary
            col = 0.5 + 0.5 * cos(6.28318 * (hue * 2.0 + vec3(0.0, 0.5, 0.25)));
        } else {
            // Mono (white/blue)
            col = mix(vec3(0.1, 0.2, 0.5), vec3(1.0), brightness);
        }

        // Mix visual output with state storage
        // We need the state in RGBA, but also want visual output
        // Solution: store state, but output will be reinterpreted visually
        // For stateful ping-pong, we store the actual state data
        fragColor = vec4(velEncoded, dye);
    }
)";

// === Phase 20: Layer Router — passthrough shader (just copies input) ===
// The Layer Router doesn't need its own shader since it returns another layer's texture directly.
// But for consistency with the ProceduralSource system, we provide a passthrough.
inline const char* sourceLayerRouter = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    void main() {
        fragColor = texture(u_texture, v_texCoord);
    }
)";

} // namespace EmbeddedShaders
