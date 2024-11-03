#version 300 es 

precision mediump float;    

in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform float u_freq = 0.2;
uniform float u_time;

uniform vec2 u_shape_vec = vec2(1.,0.7);
uniform vec3 u_color = vec3(1.,0.5,20.);

uniform sampler2D u_texture;

void main()                                  
{            
    float amplitude = 0.02;
    vec2 scale = vec2(u_shape_vec.x +amplitude*sin(u_time*u_freq),
                     u_shape_vec.y - amplitude*sin(u_time*u_freq));
    vec2 center = vec2(0.5*scale.x, 0.5 * scale.y);
    vec2 tex = vec2(v_tex_coord.x*scale.x, v_tex_coord.y * scale.y);
    float d_center = distance(center, tex);

    float all_shape_factor =  1. -  smoothstep(0.3, 0.305, d_center);
    float eye_factor =  smoothstep(0.24, 0.25, d_center) -  smoothstep(0.29, 0.3, d_center);
    float center_shape_factor =  1. -  smoothstep(0.07, 0.1, d_center);
    float color_part =  smoothstep(0.09, 0.1, d_center) -  smoothstep(0.24, 0.25, d_center);

    vec3 result = center_shape_factor*vec3(0,0,0) + color_part*v_color.rgb + eye_factor * vec3(1,1,1) ;
    FragColor = vec4(result*all_shape_factor, all_shape_factor * v_color.a);
}                                          