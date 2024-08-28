#version 300 es 

precision mediump float;    

#include "../external/lygia/generative/pnoise.glsl"
                 
in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform float u_tex_multiplier;
uniform float u_time_multiplier = 0.1;
uniform vec2 u_resolution;
uniform vec3 u_color = vec3(0.,0.2.,1.);

uniform float u_time;
uniform sampler2D u_texture;

void main()                                  
{            

    vec3 st_time = vec3(v_tex_coord, u_time*u_time_multiplier);
    float n = pnoise(st_time*(u_tex_multiplier+5.), vec3(2,2,5));

    FragColor = vec4((1. - n)*u_color,1.);
}                                          