#version 330 core
layout (location = 0) in vec3 position;
out vec3 TexCoords;

uniform mat4 u_project;
uniform mat4 u_view;

void main()
{
    gl_Position =   u_project * u_view * vec4(position, 1.0);  
    TexCoords = position;
}