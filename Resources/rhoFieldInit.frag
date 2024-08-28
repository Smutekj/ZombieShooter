#version 300 es 

precision mediump float;    

in vec2 v_tex_coord;                          
in vec4 v_color;       
uniform float u_speed = 1.0;

out vec4 FragColor;

void main(void)
{

	vec2 dr1 = v_tex_coord - vec2(0.5, 0.5);
	vec2 dr2 = v_tex_coord - vec2(0.65, 0.5);
	float r2 = dot(dr1, dr1); 
	float r = sqrt(r2); 

	float rho1 = 0.0;
	float rho2 = 0.0;
	if(length(dr1)  < 0.2  )
	{
		rho1 = 0.5;
	}
	if(length(dr2) < 0.3 )
	{
		rho2 = 0.5;
	}

	FragColor =vec4(rho1, 0.0, 0.0, 1.0);
}
