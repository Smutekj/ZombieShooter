#version 300 es 

precision mediump float;    
                 
in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform sampler2D u_texture1;
uniform sampler2D u_texture2;

void main()                                  
{                                            
    FragColor = mix(texture(u_texture1, v_tex_coord), texture(u_texture2, v_tex_coord), 0.5);      
}                                            