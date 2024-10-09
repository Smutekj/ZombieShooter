#version 300 es 

precision mediump float;    

in vec2 v_tex_coord;                          
in vec4 v_color;       
uniform float u_speed = 1.0;

out vec4 FragColor;

float hash(float x)
{
	return fract(x*325759.659318);
}

void main(void)
{

	vec2 dr = v_tex_coord - vec2(0.5, 0.5);

	float r2 = dot(dr, dr); 
	float r = sqrt(r2); 

	vec2 vel_field = vec2(1.0, 0.0); //vec2(-dr.y * dr.x, dr.y*dr.y/2.);
	float norm_field = sqrt(dot(vel_field, vel_field));

	FragColor =vec4(norm_field, (vel_field.x/norm_field + 1.0)/2., (vel_field.y / norm_field + 1.)/2., 1.0);
}