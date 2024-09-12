#version 300 es 

precision mediump float;    

uniform float u_posx = 0.;
uniform float u_posy = 0.;
uniform float u_max_radius = 150.;

in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

void main()                                  
{            
    vec4 result = v_color;
    
    vec2 dr = vec2(v_tex_coord - vec2(0.5));
    float r = distance(v_tex_coord, vec2(u_posx, u_posy));

    float r_norm = r / u_max_radius;
    
    result.a = 1.0 * (1.-smoothstep(1.0, 1.15f, r_norm));
    
    FragColor = result; 
}                                            