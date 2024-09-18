#version 300 es 

precision mediump float;    

#include "../external/lygia/generative/pnoise.glsl"
#include "../external/lygia/generative/voronoise.glsl"
                 
in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform float u_tex_multiplier = 100.0;
uniform float u_time_multiplier = 0.5;
uniform vec2 u_voronoi_uv = vec2(0.1, 1.0);
uniform vec2 u_resolution;
uniform vec3 u_color = vec3(60.,0.5.,0.);
uniform float u_time;

uniform sampler2D u_texture;

void main()                                  
{            

    vec3 st_time = vec3(v_tex_coord*u_tex_multiplier, u_time*u_time_multiplier);
    float n = pnoise(st_time, vec3(u_voronoi_uv, 1.0));

    vec2 center = vec2(0.5, 0.5);
    float shape_factor = 1. - smoothstep(0.45, 0.5, distance(center, v_tex_coord));
    vec3 result = u_color ;
    FragColor = vec4(result*shape_factor, shape_factor);
}                                          