#version 330 core
layout(location = 0) in vec4 position;

uniform mat4 lightSpaceMatrix;
uniform mat4 u_model;

void main()
{
    gl_Position = lightSpaceMatrix  * u_model * position;
}