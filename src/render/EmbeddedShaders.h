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

} // namespace EmbeddedShaders
