#version 410 core

in vec2 v_texCoord;
out vec4 fragColor;

uniform sampler2D u_texture;
uniform float u_time;
uniform vec2  u_resolution;

// Effect parameters — all [0, 1]
uniform float u_ripple_intensity;  // How much the UV is displaced
uniform float u_ripple_freq;       // Number of ripple rings
uniform float u_ripple_speed;      // Animation speed

void main()
{
    vec2 uv = v_texCoord;
    vec2 center = vec2(0.5);
    float dist = distance(uv, center);

    // Map [0,1] params to useful ranges
    float intensity = u_ripple_intensity * 0.05;  // max 5% UV displacement
    float freq = 5.0 + u_ripple_freq * 25.0;      // 5 to 30 rings
    float speed = 1.0 + u_ripple_speed * 5.0;      // 1x to 6x speed

    // Ripple displacement
    float wave = sin(dist * freq - u_time * speed) * intensity;
    vec2 dir = normalize(uv - center + vec2(0.0001)); // avoid div-by-zero
    uv += dir * wave;

    fragColor = texture(u_texture, uv);
}
