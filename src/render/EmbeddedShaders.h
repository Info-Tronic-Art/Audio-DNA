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
        float flash = step(0.9, fract(u_time * rate)) * u_strobe_intensity;
        color.rgb += flash;
        color.rgb = clamp(color.rgb, 0.0, 1.0);
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

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        float zoom = 0.5 + u_src_zoom * 2.0;
        uv *= zoom;

        int iters = 4 + int(u_src_iterations * 12.0);
        float angle = 0.5 + u_src_fold_angle * 2.5;
        float rot = u_src_rotation * 6.28318 + u_time * 0.1;

        // Apply rotation
        float cs = cos(rot), sn = sin(rot);
        uv = vec2(uv.x * cs - uv.y * sn, uv.x * sn + uv.y * cs);

        float d = 1e10;
        for (int i = 0; i < 16; i++) {
            if (i >= iters) break;
            uv = abs(uv) - angle;
            // Rotate each iteration
            float a = 0.7853 + float(i) * 0.1 + u_time * 0.02;
            float c = cos(a), s = sin(a);
            uv = vec2(uv.x * c - uv.y * s, uv.x * s + uv.y * c);
            d = min(d, length(uv));
        }

        // Coloring
        float brightness = 0.7 + u_rms * 0.5;
        vec3 col = 0.5 + 0.5 * cos(d * 8.0 + u_time * 0.3 + vec3(0.0, 1.0, 2.0));
        col *= exp(-d * 1.5) * brightness;

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

    vec3 palette(float t) {
        return 0.5 + 0.5 * cos(6.28318 * (t + vec3(0.0, 0.33, 0.67)));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        // Zoom: exponential for smooth zooming
        float zoom = exp(-u_src_zoom * 6.0);
        vec2 center = vec2(-0.5 + u_src_center_x * 1.0, u_src_center_y * 1.0);
        uv = uv * zoom + center;

        int maxIter = 32 + int(u_src_max_iter * 224.0);

        // Julia / Mandelbrot blend
        vec2 c, z;
        float juliaMix = u_src_julia_mix;
        vec2 juliaC = vec2(-0.7 + 0.3 * sin(u_time * 0.1), 0.27 + 0.15 * cos(u_time * 0.13));

        if (juliaMix > 0.5) {
            z = uv;
            c = juliaC;
        } else {
            z = vec2(0.0);
            c = uv;
        }

        int iter = 0;
        for (int i = 0; i < 256; i++) {
            if (i >= maxIter) break;
            if (dot(z, z) > 4.0) break;
            z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
            iter = i;
        }

        float t = float(iter) / float(maxIter);
        // Smooth iteration count
        if (iter < maxIter - 1) {
            float sl = float(iter) - log2(log2(dot(z,z))) + 4.0;
            t = sl / float(maxIter);
        }

        vec3 col = palette(t + u_time * 0.02);
        if (iter >= maxIter - 1) col = vec3(0.0); // Inside set = black

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

        // Visual output: color from chemicals
        vec3 col;
        col.r = newA - newB;
        col.g = newA * 0.5;
        col.b = newB * 2.0;

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
    uniform float u_src_rule_threshold;
    uniform float u_src_birth_low;
    uniform float u_src_birth_high;
    uniform float u_src_survival_low;

    void main() {
        vec2 pixel = 1.0 / u_resolution;
        vec2 uv = v_texCoord;

        vec4 state = texture(u_texture, uv);
        float alive = step(0.5, state.r);

        // Count neighbors (Moore neighborhood)
        float neighbors = 0.0;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                vec2 neighbor = uv + vec2(float(dx), float(dy)) * pixel;
                neighbors += step(0.5, texture(u_texture, neighbor).r);
            }
        }

        // Configurable birth/survival rules (default: Conway's Game of Life)
        float birthLow = 2.5 + u_src_birth_low * 2.0;   // default ~3
        float birthHigh = 3.5 + u_src_birth_high * 2.0;  // default ~3
        float survLow = 1.5 + u_src_survival_low * 2.0;  // default ~2

        float newState;
        if (alive > 0.5) {
            // Survival: 2 or 3 neighbors (configurable)
            newState = step(survLow, neighbors) * step(neighbors, birthHigh);
        } else {
            // Birth: exactly 3 neighbors (configurable)
            newState = step(birthLow, neighbors) * step(neighbors, birthHigh);
        }

        // Seed on strong onsets
        if (u_onsetStrength > 0.6) {
            float seedDist = length(uv - vec2(0.5 + 0.4 * sin(u_time * 1.3), 0.5 + 0.4 * cos(u_time)));
            if (seedDist < 0.08) {
                // Random pattern from time-based hash
                float h = fract(sin(dot(uv * u_resolution + u_time * 100.0, vec2(12.9898, 78.233))) * 43758.5453);
                newState = step(0.4, h);
            }
        }

        // Initialize if nearly empty
        if (state.a < 0.1) {
            float h = fract(sin(dot(uv * u_resolution, vec2(12.9898, 78.233))) * 43758.5453);
            newState = step(0.6, h); // ~40% alive initially
        }

        // Trail effect: slowly fade old cells
        float trail = state.g * 0.95;
        if (newState > 0.5) trail = 1.0;

        // Color: alive = bright, trail = dim colored
        vec3 col;
        col.r = newState;
        col.g = trail;
        col.b = trail * 0.5 + newState * 0.3;

        fragColor = vec4(col, 1.0);
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

} // namespace EmbeddedShaders
