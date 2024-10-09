#version 300 es 

precision mediump float;    

in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform sampler2D u_vel_field;
uniform sampler2D u_rho_field;

uniform float u_time_factor = 0.0005;  
uniform float u_diff_factor = 0.0;  

vec2 unpackVelocity(vec4 vel_field_data)
{
	float n = vel_field_data.x;
	float cos_a = (vel_field_data.y * 2. - 1.);
	float sin_a = (vel_field_data.z * 2. - 1.);
	return n * vec2(vel_field_data.y, vel_field_data.z); 
}

void main(void)
{

	ivec2 tex_sizei = textureSize(u_vel_field, 0);
	vec2 tex_size = vec2(float(tex_sizei.x), float(tex_sizei.y));

	vec2 dr = v_tex_coord - vec2(0.5, 0.5);

	vec4 vel_field_data = texture(u_vel_field, v_tex_coord);
	vec4 rho_field_data = texture(u_rho_field, v_tex_coord);
	
	vec4 vel_field_data_up = texture(u_vel_field, v_tex_coord + vec2(0., 1./tex_size.y));
	vec4 vel_field_data_down = texture(u_vel_field, v_tex_coord+ vec2(0., -1./tex_size.y));
	vec4 vel_field_data_left = texture(u_vel_field, v_tex_coord+ vec2(-1./tex_size.x, 0.));
	vec4 vel_field_data_right = texture(u_vel_field, v_tex_coord+ vec2(1./tex_size.x, 0.));

	vec4 rho_field_data_up = texture(u_rho_field, v_tex_coord + vec2(0., 1./tex_size.y));
	vec4 rho_field_data_down = texture(u_rho_field, v_tex_coord + vec2(0., -1./tex_size.y));
	vec4 rho_field_data_left = texture(u_rho_field, v_tex_coord + vec2(-1./tex_size.x, 0.));
	vec4 rho_field_data_right = texture(u_rho_field, v_tex_coord + vec2(1./tex_size.x, 0.));

	vec2 vel_field = unpackVelocity(vel_field_data);
	vec2 vel_up = unpackVelocity(vel_field_data_up);
	vec2 vel_down = unpackVelocity(vel_field_data_down);
	vec2 vel_left = unpackVelocity(vel_field_data_left);
	vec2 vel_right = unpackVelocity(vel_field_data_right);

	float rho1 = rho_field_data.x;
	float rho2 = rho_field_data.y;
	vec2 rho1v = rho1 * vel_field;
	vec2 rho2v = rho2 * vel_field;

	float rho1_up = rho_field_data_up.x;
	float rho1_down = rho_field_data_down.x;
	float rho1_left = rho_field_data_left.x;
	float rho1_right = rho_field_data_right.x;

	float rho2_up = rho_field_data_up.y;
	float rho2_down = rho_field_data_down.y;
	float rho2_left = rho_field_data_left.y;
	float rho2_right = rho_field_data_right.y;

	float delta_rho_diff = 0.;
	delta_rho_diff += (rho1_up - rho1)*(rho1_up + rho1)/2. ;
	delta_rho_diff += (rho1_down - rho1)*(rho1_down+ rho1)/2. ;
	delta_rho_diff += (rho1_left - rho1)*(rho1_left+ rho1)/2.;
	delta_rho_diff += (rho1_right - rho1)*(rho1_right+ rho1)/2.;

	vec2 a_plus = max(vel_field, vec2(0.,0.));
	vec2 a_minus = min(vel_field, vec2(0.,0.));
	vec2 drho_minus = vec2((rho1 - rho1_left), (rho1 - rho1_down));
	vec2 drho_plus = vec2( (rho1_right - rho1),  (rho1_up - rho1));
	//float delta_rho1 = + u_time_factor * (a_minus.x * drho_plus.x + a_plus.x * drho_minus.x 
	//									+ a_minus.y * drho_plus.y + a_plus.y * drho_minus.y);
	float delta_rho1 = u_time_factor * (vel_field.x * (drho_minus.x + drho_plus.x) +
										vel_field.y * (drho_minus.y + drho_plus.y) );



	float rho1_next = rho1 + delta_rho1;
	FragColor = vec4(rho1_next, 0., 0.0, 1.0);
}
