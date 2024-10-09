#version 300 es 

precision mediump float;    

in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform float u_freq = 0.2;
uniform float u_time;

uniform vec2 u_radii = vec2(1.,1.0);
uniform float u_inner_radius = 0.46;
uniform float u_outer_radius = 0.49;
uniform vec3 u_color = vec3(1.,0.,1.); 

uniform sampler2D u_texture;

void main()                                  
{            
    vec2 center = vec2(0.5*u_radii.x, 0.5 * u_radii.y);
    vec2 tex = vec2(v_tex_coord.x*u_radii.x, v_tex_coord.y * u_radii.y);
    float d_center = distance(center, tex);
    
    float shape_factor =  smoothstep(u_inner_radius, u_inner_radius+0.03, d_center) 
                        - smoothstep(u_inner_radius+0.03, u_outer_radius, d_center);

    vec3 result = u_color ;
    FragColor = vec4(result*shape_factor, shape_factor);
}                                          