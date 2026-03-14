#version 410 core

in vec2 v_texCoord;
out vec4 fragColor;

uniform sampler2D u_texture;

// Effect parameters — all [0, 1]
uniform float u_rgb_split;   // Amount of RGB channel separation
uniform float u_rgb_angle;   // Direction angle of the split

void main()
{
    // Map [0,1] to useful range
    float amount = u_rgb_split * 0.03; // max 3% UV offset
    float angle = u_rgb_angle * 6.28318; // 0 to 2*PI

    vec2 dir = vec2(cos(angle), sin(angle)) * amount;

    float r = texture(u_texture, v_texCoord + dir).r;
    float g = texture(u_texture, v_texCoord).g;
    float b = texture(u_texture, v_texCoord - dir).b;
    float a = texture(u_texture, v_texCoord).a;

    fragColor = vec4(r, g, b, a);
}
