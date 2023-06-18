#version 330 core
layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 anormal;

out vec2 TexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;

uniform mat4 u_project;
uniform mat4 u_view;
uniform mat4 u_model;
uniform mat4 lightSpaceMatrix;

void main()
{
    vs_out.FragPos = vec3(u_model * position);
    vs_out.Normal = transpose(inverse(mat3(u_model))) * anormal;
    vs_out.TexCoords = texCoord;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    gl_Position = u_project * u_view * u_model * position;
}