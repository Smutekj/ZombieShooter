#version 300 es 

precision mediump float;    

#include

in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform float u_time_multiplier = 5.5;
uniform float u_time;

uniform vec2 u_shape_vec = vec2(1.,0.7);
uniform vec3 u_color = vec3(60.,0.5,0.);

uniform sampler2D u_texture;

void main()                                  
{            
    float a = 0.05;
    vec2 scale = vec2(u_shape_vec.x +a*sin(u_time*u_time_multiplier),
                     u_shape_vec.y - a*sin(u_time*u_time_multiplier));
    vec2 center = vec2(0.5*scale.x, 0.5 * scale.y);
    vec2 tex = vec2(v_tex_coord.x*scale.x, v_tex_coord.y * scale.y);
    float d_center = distance(center, tex);
    
    
    float shape_factor =  (smoothstep(0.22 + 0.2*a*sin(u_time*u_time_multiplier), 0.27, d_center)- smoothstep(0.28, 0.3, d_center));


    vec3 result = u_color ;
    FragColor = vec4(result*shape_factor, shape_factor);
}                                          