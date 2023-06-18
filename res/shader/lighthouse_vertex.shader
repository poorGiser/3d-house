#version 330 core
layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 anormal;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_project;

out vec3  normal;
out vec3 object_position;
out vec2 v_TexCoord;


void main(){
    vec3 Normal = transpose(inverse(mat3(u_model))) * anormal;
	gl_Position= u_project*u_view*u_model*position;
	object_position = vec3(u_model*position);
	normal = Normal;
	v_TexCoord = texCoord;
}