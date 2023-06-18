#version 330 core
layout(location = 0) out vec4 color;

uniform vec4 u_light_color;
uniform vec3 u_light_position;
uniform vec3 u_camera_position;

float ambient = 0.2f;
in vec3 normal;
in vec3 object_position;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;


void main(){
    vec3 u_object_color = texture(u_Texture, v_TexCoord).rgb;
    //light_radius
    float light_radius = distance(object_position, u_light_position);
    //light_direction
    vec3 light_direction = normalize(u_light_position - object_position);
    //camera_diretion
    vec3 camera_diretion = normalize(u_camera_position - object_position);

    //diffuse light
    //lighting by object accepted  
    vec3 light_accepted = vec3(u_light_color);
    //cos light directon and normal;
    vec3 norm = normalize(normal);
    float cos_nl = dot(norm,light_direction );

    //specular highlight
    vec3 h = (light_direction + camera_diretion) / length(light_direction + camera_diretion);
    float cos_nh = dot(norm, h);
    float p = 64;

    color = vec4(light_accepted * max(0.0f, cos_nl) * u_object_color + ambient * light_accepted * u_object_color + light_accepted * max(0.0f, pow(cos_nh, p)) * u_object_color,1.0);
    //color = vec4(ambient * light_accepted * u_object_color,1.0);
}