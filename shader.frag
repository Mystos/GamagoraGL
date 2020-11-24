#version 450
uniform float time;
in vec2 uv;
in vec3 v_position;
layout(binding=0) uniform sampler2D tex;
out vec4 color;


in vec3 particule_color;
void main()
{
    vec3 lightPosition = vec3(10, 10, 10);
    vec3 lightDirection = normalize(lightPosition - v_position);
    vec3 normal = normalize(cross(dFdx(v_position), dFdy(v_position)));

    float coeff = abs(dot(lightDirection, normal));

    color = coeff * texture(tex, uv);
}