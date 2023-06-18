#version 330 core

layout(location = 0) in vec4 position;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_project;

void main()
{
    gl_Position = u_project*u_view*u_model*position;
};