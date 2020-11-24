#version 450
uniform float time;
uniform vec2 mousePos;
uniform mat4 trans;

in vec3 pos;
in vec2 vertexUV;

out vec2 uv;
out  vec3 v_position;
out vec3 particule_color;

void main()
{
    uv = vertexUV;
    gl_PointSize = 5;

    gl_Position = trans * vec4(pos,1.0);
    //v_position = gl_Position;
    particule_color = vec3(1.0,0.,0.);
}