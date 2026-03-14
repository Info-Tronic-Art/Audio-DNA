#version 410 core

in vec2 v_texCoord;
out vec4 fragColor;

uniform sampler2D u_texture;

// Effect parameters — all [0, 1]
uniform float u_vignette_int;   // Intensity of edge darkening
uniform float u_vignette_soft;  // Softness of the falloff

void main()
{
    vec4 color = texture(u_texture, v_texCoord);

    // Distance from center, normalized so corners are ~1.0
    vec2 uv = v_texCoord * 2.0 - 1.0;
    float dist = length(uv) * 0.707; // normalize so corner = 1.0

    // Softness controls the falloff curve
    float softness = 0.2 + u_vignette_soft * 0.8; // 0.2 to 1.0
    float vignette = smoothstep(1.0, 1.0 - softness, dist);

    // Mix: at full intensity, corners go to black
    float strength = u_vignette_int;
    color.rgb *= mix(1.0, vignette, strength);

    fragColor = color;
}
