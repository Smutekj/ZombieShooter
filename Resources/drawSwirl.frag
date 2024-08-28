#version 300 es 

precision mediump float;    

in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform sampler2D u_field_data;


void main(void)
{

	vec4 field_data = texture(u_field_data, v_tex_coord);

	FragColor = vec4(field_data.x, field_data.y, 0.0, 1.0);
}
