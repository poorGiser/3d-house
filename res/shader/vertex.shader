#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 anormal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_project;

out vec2 v_TexCoord;


void main()
{
    gl_Position = u_project*u_view*u_model*position;
    v_TexCoord = texCoord;
};