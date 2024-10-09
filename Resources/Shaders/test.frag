#version 300 es 

precision mediump float;    

#include "../../external/lygia/generative/voronoise.glsl"
#include "../../external/lygia/math/mod289.glsl"
#include "../../external/lygia/math/taylorInvSqrt.glsl"
#include "../../external/lygia/math/quintic.glsl"
#include "cracks.glsl"
                 
in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform float u_time;
uniform float u_perlin_multiplier = 0.2;
uniform float u_mixture = 0.3;
uniform float u_cracks_scale = 10.0;
uniform vec3 u_color;
uniform vec3 u_cracks_color;
uniform sampler2D u_texture;


float pnoise(in vec2 P, in vec2 rep) {
    vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
    vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
    Pi = mod(Pi, rep.xyxy); // To create noise with explicit period
    Pi = mod289(Pi);        // To avoid truncation effects in permutation
    vec4 ix = Pi.xzxz;
    vec4 iy = Pi.yyww;
    vec4 fx = Pf.xzxz;
    vec4 fy = Pf.yyww;

    vec4 i = permute(permute(ix) + iy);

    vec4 gx = fract(i * (1.0 / 41.0)) * 2.0 - 1.0 ;
    vec4 gy = abs(gx) - 0.5 ;
    vec4 tx = floor(gx + 0.5);
    gx = gx - tx;

    vec2 g00 = vec2(gx.x,gy.x);
    vec2 g10 = vec2(gx.y,gy.y);
    vec2 g01 = vec2(gx.z,gy.z);
    vec2 g11 = vec2(gx.w,gy.w);

    vec4 norm = taylorInvSqrt(vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
    g00 *= norm.x;
    g01 *= norm.y;
    g10 *= norm.z;
    g11 *= norm.w;

    float n00 = dot(g00, vec2(fx.x, fy.x));
    float n10 = dot(g10, vec2(fx.y, fy.y));
    float n01 = dot(g01, vec2(fx.z, fy.z));
    float n11 = dot(g11, vec2(fx.w, fy.w));

    vec2 fade_xy = quintic(Pf.xy);
    vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
    float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
    return 2.3 * n_xy;
}

void main()                                  
{             
    float m = u_perlin_multiplier * 100.;                               
    //float voronoi_profile = voronoise(v_tex_coord, u_voronoi_u, u_voronoi_v);
    float perlin_profile = pnoise(m*v_tex_coord, vec2(5.0,3.));
    float cracks_profile = cracks(v_tex_coord , u_cracks_scale, 1.); //0.01*sin(2.*3.141592 * v_tex_coord.x/20.)

    vec3 result = mix(u_color*(perlin_profile),  u_cracks_color * cracks_profile, u_mixture);
    FragColor = vec4(result , 1);      
}                                            