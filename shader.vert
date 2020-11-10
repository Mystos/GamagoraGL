#version 450
uniform float time;
uniform vec2 mousePos;

in vec3 position;
in vec3 color;
in vec3 speed;
in float size;

out vec3 particule_color;

void main()
{
    float r = length(position.xy);

    gl_PointSize = size * 40;


    //float x = cos(time*r)*r;
    //float y = sin(time*r)*r;

    gl_Position = vec4(position,1.0) ;

    //gl_Position = vec4(position * sin(time), 1.0);

    particule_color = vec3(color);
}