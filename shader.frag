#version 450
uniform float time;

out vec4 color;

in vec3 particule_color;
void main()
{
    color = vec4(particule_color,0.f);
}