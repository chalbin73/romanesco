#version 440 core

out vec4 FragColor;
in vec3 pos;

uniform sampler2D tex;

float map(float v, float v_min, float v_max, float out_min, float out_max);
vec3 hsv2rgb(vec3 c);

void main()
{
    FragColor = texture(tex, vec2(map(pos.x, -1.0, 1.0, 0.0, 1.0), map(pos.y, -1.0, 1.0, 0.0, 1.0)));
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float map(float v, float v_min, float v_max, float out_min, float out_max)
{
    return (((v - v_min) / (v_max - v_min)) * (out_max - out_min)) + out_min;
}