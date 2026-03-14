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

} // namespace EmbeddedShaders
