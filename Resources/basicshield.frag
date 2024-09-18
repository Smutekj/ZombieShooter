#version 300 es 

precision mediump float;    

                 
in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform float u_time_multiplier = 0.5;
uniform float u_time;

uniform vec2 u_shape_vec = vec2(1.,0.7);
uniform vec3 u_color = vec3(60.,0.5.,0.);

uniform sampler2D u_texture;

void main()                                  
{            

    vec2 center = vec2(0.5*u_shape_vec.x, 0.5 * u_shape_vec.y);
    vec2 tex = vec2(v_tex_coord.x*u_shape_vec.x, v_tex_coord.y * u_shape_vec.y);
    float d_center = distance(center, tex);
    float shape_factor =  (smoothstep(0.33, 0.35, d_center)- smoothstep(0.36, 0.38, d_center));

    vec3 result = u_color ;
    FragColor = vec4(result*shape_factor, shape_factor);
}                                          