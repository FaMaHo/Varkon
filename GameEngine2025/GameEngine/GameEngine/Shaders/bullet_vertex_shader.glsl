#version 400

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normals;

uniform mat4 MVP;
uniform mat4 model;

void main()
{
    gl_Position = MVP * vec4(pos, 1.0f);
}
