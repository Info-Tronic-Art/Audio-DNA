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
    uniform float u_src_power;
    uniform float u_src_color_speed;
    uniform float u_src_color_shift;
    uniform float u_src_dive_speed;

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        // Auto-dive: continuously zoom into an interesting point
        float diveSpeed = u_src_dive_speed * u_src_dive_speed * 8.0;
        float autoZoom = diveSpeed > 0.01 ? u_time * diveSpeed : 0.0;

        // Exponential zoom (manual + auto-dive)
        float zoom = exp(-(u_src_zoom * 10.0 + autoZoom));

        // Interesting dive targets (cycle between them)
        vec2 diveTargets[4];
        diveTargets[0] = vec2(-0.7436439, 0.1318259);  // Seahorse valley
        diveTargets[1] = vec2(-0.1011, 0.9563);         // Spiral arm
        diveTargets[2] = vec2(-1.7497, 0.0);            // Mini-brot antenna
        diveTargets[3] = vec2(0.2501, 0.0);              // Elephant valley

        vec2 center;
        if (diveSpeed > 0.01) {
            // Pick dive target based on time cycle (switch every ~25s of zoom)
            int targetIdx = int(mod(u_time * 0.04, 4.0));
            center = diveTargets[targetIdx];
        } else {
            center = vec2(-0.5 + u_src_center_x * 2.0 - 0.5,
                           u_src_center_y * 2.0 - 1.0);
        }
        uv = uv * zoom + center;

        // Auto-increase iterations with zoom depth for detail
        int baseIter = 32 + int(u_src_max_iter * 224.0);
        int autoIter = int(autoZoom * 15.0);
        int maxIter = min(baseIter + autoIter, 400);
        float power = 2.0 + u_src_power * 6.0;

        // Julia / Mandelbrot mode
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

            if (power < 2.5) {
                // Standard z^2 (fastest path)
                z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
            } else {
                // Multibrot z^n via polar
                float r = length(z);
                float theta = atan(z.y, z.x);
                float rn = pow(r, power);
                z = rn * vec2(cos(power * theta), sin(power * theta)) + c;
            }
            iter = i;
        }

        // Smooth iteration count for anti-banding
        float si = float(iter);
        if (iter < maxIter - 1) {
            si = float(iter) - log2(log2(dot(z,z))) + 4.0;
        }

        vec3 col = vec3(0.0);
        if (iter < maxIter - 1) {
            float t = si * (0.01 + u_src_color_speed * 0.08) + u_src_color_shift * 6.28 + u_time * 0.02;
            col = 0.5 + 0.5 * cos(6.28318 * (t + vec3(0.0, 0.33, 0.67)));
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

// Julia Set — animate c for morphing fractals
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
    uniform float u_rms;
    uniform float u_beatPhase;
    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        // Auto-dive zoom
        float diveSpeed = u_src_dive_speed * u_src_dive_speed * 8.0;
        float autoZoom = diveSpeed > 0.01 ? u_time * diveSpeed : 0.0;
        float zoom = exp(u_src_zoom * 8.0 - 2.0 + autoZoom);
        uv /= zoom;
        // Julia c-value: animate with time and audio, morph between presets during dive
        float cx, cy;
        if (diveSpeed > 0.01) {
            // Slowly morph c through beautiful regions
            float phase = u_time * 0.08;
            cx = -0.7 + 0.3 * sin(phase) + 0.1 * sin(phase * 2.7);
            cy = 0.27 + 0.2 * cos(phase * 0.9) + 0.05 * cos(phase * 3.1);
        } else {
            cx = (u_src_cx - 0.5) * 2.0 + sin(u_time * 0.2) * 0.05 * u_rms;
            cy = (u_src_cy - 0.5) * 2.0 + cos(u_time * 0.15) * 0.05 * u_rms;
        }
        vec2 z = uv;
        vec2 c = vec2(cx, cy);
        int baseIter = int(u_src_iterations * 200.0) + 20;
        int maxIter = min(baseIter + int(autoZoom * 10.0), 400);
        int i;
        for (i = 0; i < maxIter; i++) {
            z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
            if (dot(z,z) > 4.0) break;
        }
        if (i == maxIter) { fragColor = vec4(0,0,0,1); return; }
        float si = float(i) - log2(log2(dot(z,z))) + 4.0;
        float t = si * (0.02 + u_src_color_speed * 0.1) + u_src_color_shift * 6.28;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (t + vec3(0.0, 0.33, 0.67)));
        fragColor = vec4(col, 1.0);
    }
)";

// Burning Ship — aggressive flame-like fractal
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
    uniform float u_rms;
    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        float diveSpeed = u_src_dive_speed * u_src_dive_speed * 8.0;
        float autoZoom = diveSpeed > 0.01 ? u_time * diveSpeed : 0.0;
        float zoom = exp(u_src_zoom * 8.0 - 1.0 + autoZoom);
        vec2 center;
        if (diveSpeed > 0.01) {
            // Dive into the ship's smokestack
            center = vec2(-1.762, -0.028);
        } else {
            center = vec2((u_src_center_x - 0.5) * 4.0 - 0.5,
                           (u_src_center_y - 0.5) * 4.0 - 0.5);
        }
        uv = uv / zoom + center;
        vec2 z = vec2(0.0);
        vec2 c = uv;
        int baseIter = int(u_src_iterations * 200.0) + 20;
        int maxIter = min(baseIter + int(autoZoom * 15.0), 400);
        int i;
        for (i = 0; i < maxIter; i++) {
            z = abs(z); // The burning ship twist
            z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
            if (dot(z,z) > 4.0) break;
        }
        if (i == maxIter) { fragColor = vec4(0,0,0,1); return; }
        float si = float(i) - log2(log2(dot(z,z))) + 4.0;
        float t = si * (0.02 + u_src_color_speed * 0.1) + u_src_color_shift * 6.28;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (t * 0.7 + vec3(0.0, 0.1, 0.2)));
        col *= 0.85 + u_rms * 0.3;
        fragColor = vec4(col, 1.0);
    }
)";

// Newton Fractal — psychedelic basins of attraction
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
    uniform float u_rms;
    vec2 cmul(vec2 a, vec2 b) { return vec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x); }
    vec2 cdiv(vec2 a, vec2 b) { return cmul(a, vec2(b.x,-b.y)) / dot(b,b); }
    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        float diveSpeed = u_src_dive_speed * u_src_dive_speed * 8.0;
        float autoZoom = diveSpeed > 0.01 ? u_time * diveSpeed : 0.0;
        float zoom = exp(u_src_zoom * 6.0 - 1.0 + autoZoom);
        // Dive into root boundary
        if (diveSpeed > 0.01) {
            uv = uv / zoom + vec2(0.5, 0.866); // boundary between roots
        } else {
            uv /= zoom;
        }
        int n = int(u_src_power * 5.0) + 3; // power 3-8
        float damp = 0.5 + u_src_damping * 1.0; // relaxation
        vec2 z = uv;
        int maxIter = 50;
        int i;
        float minDist = 1e10;
        int closestRoot = 0;
        for (i = 0; i < maxIter; i++) {
            // Compute z^n and z^(n-1) using polar form
            float r = length(z);
            if (r < 0.0001) break;
            float theta = atan(z.y, z.x);
            vec2 zn = pow(r, float(n)) * vec2(cos(float(n)*theta), sin(float(n)*theta));
            vec2 zn1 = pow(r, float(n-1)) * vec2(cos(float(n-1)*theta), sin(float(n-1)*theta));
            // Newton step: z - damp * (z^n - 1) / (n * z^(n-1))
            vec2 fz = zn - vec2(1.0, 0.0);
            vec2 fpz = float(n) * zn1;
            z -= damp * cdiv(fz, fpz);
            // Check convergence to roots (nth roots of unity)
            for (int k = 0; k < n; k++) {
                float ra = 6.28318 * float(k) / float(n);
                vec2 root = vec2(cos(ra), sin(ra));
                float d = length(z - root);
                if (d < minDist) { minDist = d; closestRoot = k; }
            }
            if (minDist < 0.001) break;
        }
        float hue = float(closestRoot) / float(n) + u_src_color_shift;
        float brightness = 1.0 - float(i) / float(maxIter);
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= brightness * (0.8 + u_rms * 0.4);
        fragColor = vec4(col, 1.0);
    }
)";

// Sierpinski — triangle and carpet modes via folding
inline const char* sourceSierpinski = R"(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_mode;       // 0=triangle, 1=carpet
    uniform float u_src_zoom;
    uniform float u_src_iterations;
    uniform float u_src_rotation;
    uniform float u_src_color_shift;
    uniform float u_rms;
    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        float zoom = exp(u_src_zoom * 6.0);
        float rot = u_time * (u_src_rotation - 0.5) * 1.0;
        float c = cos(rot), s = sin(rot);
        uv = vec2(uv.x*c - uv.y*s, uv.x*s + uv.y*c);
        uv *= zoom;
        int maxIter = int(u_src_iterations * 12.0) + 3;
        bool isCarpet = u_src_mode > 0.5;
        float val = 1.0;
        if (isCarpet) {
            // Sierpinski Carpet
            vec2 p = fract(uv * 0.5 + 0.5);
            for (int i = 0; i < maxIter; i++) {
                p *= 3.0;
                vec2 cell = floor(p);
                if (cell.x == 1.0 && cell.y == 1.0) { val = 0.0; break; }
                p = fract(p);
            }
        } else {
            // Sierpinski Triangle (folding)
            vec2 p = uv * 0.5 + vec2(0.5, 0.25);
            for (int i = 0; i < maxIter; i++) {
                p *= 2.0;
                if (p.x + p.y > 1.5) p = 2.0 - p;
                if (p.x > 1.0) p.x -= 1.0;
                if (p.y > 1.0) p.y -= 1.0;
            }
            float d = length(p - vec2(0.5, 0.35));
            val = smoothstep(0.5, 0.3, d);
        }
        float hue = u_src_color_shift;
        vec3 col;
        if (hue < 0.01) col = vec3(val);
        else col = val * (0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67))));
        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)";

// Apollonian Gasket — circle packing fractal
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
    uniform float u_rms;
    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        float zoom = 1.0 + u_src_zoom * 8.0;
        float rot = u_time * (u_src_rotation - 0.5) * 0.5;
        float ca = cos(rot), sa = sin(rot);
        uv = vec2(uv.x*ca - uv.y*sa, uv.x*sa + uv.y*ca);
        uv *= zoom;

        // Apollonian gasket via Kleinian group / IFS
        vec2 p = uv;
        float minDist = 1e10;
        int maxIter = int(u_src_iterations * 20.0) + 8;

        for (int i = 0; i < 30; i++) {
            if (i >= maxIter) break;
            p = abs(p);
            if (p.x < p.y) p = p.yx;  // fold
            p -= vec2(1.0, 1.0);
            if (p.y < -0.5 * p.x) p = vec2(p.x - p.y, p.y + p.x) * 0.7071; // 45 degree fold
            float d = length(p);
            minDist = min(minDist, d);
            p *= 2.0;
            p -= vec2(1.5, 0.5);
        }

        // Distance-based coloring
        float glow = exp(-minDist * 3.0);
        float hue = u_src_color_shift + log(minDist + 1.0) * 0.5;
        vec3 col = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
        col *= glow * (0.8 + u_rms * 0.4);
        fragColor = vec4(col, 1.0);
    }
)";

// ============================================================
// 3D FRACTAL SOURCES — Ray Marched
// ============================================================

// Mandelbulb — 3D Mandelbrot analog
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
    uniform float u_rms;

    mat3 rotY(float a) { float c=cos(a),s=sin(a); return mat3(c,0,s, 0,1,0, -s,0,c); }
    mat3 rotX(float a) { float c=cos(a),s=sin(a); return mat3(1,0,0, 0,c,-s, 0,s,c); }

    float mandelbulbDE(vec3 pos, float power) {
        vec3 z = pos;
        float dr = 1.0, r = 0.0;
        for (int i = 0; i < 12; i++) {
            r = length(z);
            if (r > 2.0) break;
            float theta = acos(z.z / r);
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
        return normalize(vec3(
            mandelbulbDE(p+e.xyy,pw) - mandelbulbDE(p-e.xyy,pw),
            mandelbulbDE(p+e.yxy,pw) - mandelbulbDE(p-e.yxy,pw),
            mandelbulbDE(p+e.yyx,pw) - mandelbulbDE(p-e.yyx,pw)));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        float power = 2.0 + u_src_power * 12.0;
        float rx = u_time * (u_src_rotation_x - 0.5) * 1.0;
        float ry = u_time * (u_src_rotation_y - 0.5) * 1.0;
        mat3 rot = rotY(ry) * rotX(rx);
        vec3 ro = rot * vec3(0, 0, 2.5);
        vec3 target = vec3(0);
        vec3 fwd = normalize(target - ro);
        vec3 right = normalize(cross(fwd, vec3(0,1,0)));
        vec3 up = cross(right, fwd);
        vec3 rd = normalize(fwd + uv.x * right + uv.y * up);
        float t = 0.0;
        float glow = 0.0;
        bool hit = false;
        for (int i = 0; i < 80; i++) {
            vec3 p = ro + rd * t;
            float d = mandelbulbDE(p, power);
            glow += 1.0 / (1.0 + d * d * 500.0);
            if (d < 0.001) { hit = true; break; }
            t += d;
            if (t > 10.0) break;
        }
        vec3 col = vec3(0.0);
        if (hit) {
            vec3 p = ro + rd * t;
            vec3 n = calcNormal(p, power);
            vec3 light = normalize(vec3(1, 2, 3));
            float diff = max(dot(n, light), 0.0) * 0.7 + 0.3;
            float hue = u_src_color_shift + length(p) * 0.5;
            col = diff * (0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67))));
        }
        col += vec3(0.1, 0.15, 0.3) * glow * 0.015;
        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)";

// Menger Sponge — 3D Sierpinski cube
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
    uniform float u_rms;

    mat3 rotY(float a) { float c=cos(a),s=sin(a); return mat3(c,0,s, 0,1,0, -s,0,c); }
    mat3 rotX(float a) { float c=cos(a),s=sin(a); return mat3(1,0,0, 0,c,-s, 0,s,c); }

    float sdBox(vec3 p, vec3 b) {
        vec3 q = abs(p) - b;
        return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
    }

    float mengerDE(vec3 p, int iters, float twistAmt) {
        float d = sdBox(p, vec3(1.0));
        float s = 1.0;
        for (int i = 0; i < iters; i++) {
            // Optional twist per iteration
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
        float rx = u_time * (u_src_rotation_x - 0.5) * 1.0;
        float ry = u_time * (u_src_rotation_y - 0.5) * 1.0;
        mat3 rot = rotY(ry) * rotX(rx);
        vec3 ro = rot * vec3(0, 0, 3.0);
        vec3 fwd = normalize(-ro);
        vec3 right = normalize(cross(fwd, vec3(0,1,0)));
        vec3 up = cross(right, fwd);
        vec3 rd = normalize(fwd + uv.x * right + uv.y * up);
        float t = 0.0;
        float glow = 0.0;
        bool hit = false;
        for (int i = 0; i < 80; i++) {
            vec3 p = ro + rd * t;
            float d = mengerDE(p, iters, twistAmt);
            glow += 1.0 / (1.0 + d * d * 200.0);
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
            col = diff * (0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67))));
        }
        col += vec3(0.15, 0.1, 0.25) * glow * 0.01;
        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)";

// KIFS — Kaleidoscopic IFS, most versatile 3D fractal
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
    uniform float u_rms;

    mat3 rotY(float a) { float c=cos(a),s=sin(a); return mat3(c,0,s, 0,1,0, -s,0,c); }
    mat3 rotX(float a) { float c=cos(a),s=sin(a); return mat3(1,0,0, 0,c,-s, 0,s,c); }
    mat3 rotZ(float a) { float c=cos(a),s=sin(a); return mat3(c,-s,0, s,c,0, 0,0,1); }

    float kifsDE(vec3 p, float sc, int iters, float foldType, float off) {
        vec3 offset = vec3(1.0, 1.0, 1.0) * off * 2.0;
        float iterRot = u_time * 0.1;
        for (int i = 0; i < iters; i++) {
            p = abs(p);
            // Fold types
            if (foldType < 0.33) {
                // Tetrahedral fold
                if (p.x - p.y < 0.0) p.xy = p.yx;
                if (p.x - p.z < 0.0) p.xz = p.zx;
                if (p.y - p.z < 0.0) p.yz = p.zy;
            } else if (foldType < 0.67) {
                // Menger-style fold
                if (p.x - p.y < 0.0) p.xy = p.yx;
                if (p.x - p.z < 0.0) p.xz = p.zx;
                // Rotation fold
                float a = iterRot + float(i) * 0.5;
                float rc = cos(a), rs = sin(a);
                p.yz = vec2(p.y*rc - p.z*rs, p.y*rs + p.z*rc);
            } else {
                // Octahedral fold
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
        float rx = u_time * (u_src_rotation_x - 0.5) * 1.0;
        float ry = u_time * (u_src_rotation_y - 0.5) * 1.0;
        mat3 rot = rotY(ry) * rotX(rx);
        vec3 ro = rot * vec3(0, 0, 3.5);
        vec3 fwd = normalize(-ro);
        vec3 right = normalize(cross(fwd, vec3(0,1,0)));
        vec3 up = cross(right, fwd);
        vec3 rd = normalize(fwd + uv.x * right + uv.y * up);
        float t = 0.0;
        float glow = 0.0;
        bool hit = false;
        for (int i = 0; i < 80; i++) {
            vec3 p = ro + rd * t;
            float d = kifsDE(p, sc, iters, foldType, off);
            glow += 1.0 / (1.0 + d * d * 300.0);
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
            col = diff * (0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67))));
        }
        col += vec3(0.1, 0.2, 0.3) * glow * 0.012;
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

} // namespace EmbeddedShaders
