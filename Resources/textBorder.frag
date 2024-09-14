#version 300 es

precision mediump float;                     
in vec2 v_tex_coord;
in vec4 v_color;                          

uniform vec4 u_edge_color = vec4(1., 0.,0.,1.);

out vec4 FragColor;

uniform sampler2D u_texture;

void main()                                  
{        

    float glyph_region = texture(u_texture, v_tex_coord).a;         
    float x = smoothstep(0.4, 0.45, glyph_region);  
    float edge = smoothstep(0.45, 0.49, glyph_region)  - smoothstep(0.51, 0.55, glyph_region);  

    FragColor = vec4(v_color.rgb*x*(1.-edge) + edge*u_edge_color.rgb, x); //vec4(vec3(1. - glyph_region), 1.); 
}                                            