#version 300 es 

precision mediump float;    

#include "../external/lygia/draw/arrows.glsl"

in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform sampler2D u_vel_field;

uniform vec3 u_arrow_color = vec3(0., 0., 0.);  

vec2 unpackVelocity(vec4 vel_field_data)
{
	float n = vel_field_data.x;
	float cos_a = (vel_field_data.y * 2. - 1.);
	float sin_a = (vel_field_data.z * 2. - 1.);
	return n * vec2(cos_a, sin_a); 
}

void main(void)
{

	ivec2 tex_sizei = textureSize(u_vel_field, 0);
	vec2 tex_size = vec2(float(tex_sizei.x), float(tex_sizei.y));

	vec2 dr = v_tex_coord - vec2(0.5, 0.5);

	vec4 vel_field_data = texture(u_vel_field, v_tex_coord);
	vec2 vel_field = unpackVelocity(vel_field_data); 


	float arrow = arrows(v_tex_coord, vel_field, tex_size);

	//FragColor = vec4(vec3(1.) - arrow*(vec3(1.) - u_arrow_color), 1.0);
	FragColor = vec4(vec3(arrow), 1.0);
}
