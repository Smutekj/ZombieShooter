#version 300 es 

precision mediump float;    
                 
in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform vec3 u_color;
uniform float u_tex_multiplier;
uniform sampler2D u_texture;

float line(vec2 tex_coord, float width, vec2 point, vec2 dir)
{

    dir /= sqrt(dot(dir, dir));

    vec2 in_dir = dot(tex_coord - point,dir)*dir;

    float dist_from_line = sqrt(dot(tex_coord - point - in_dir, tex_coord - point - in_dir));  

    return 1. - step(width/2., dist_from_line);
}


uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x*34.0)+1.0)*x); }

float snoise(vec2 v) {
    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                        -0.577350269189626,  // -1.0 + 2.0 * C.x
                        0.024390243902439); // 1.0 / 41.0
    vec2 i  = floor(v + dot(v, C.yy) );
    vec2 x0 = v -   i + dot(i, C.xx);
    vec2 i1;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
        + i.x + vec3(0.0, i1.x, 1.0 ));

    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

float liney(vec2 tex_coord, float width, float center_x)
{
    float x_min = center_x - width/2.;
    float x_max = center_x + width/2.;
    return smoothstep(x_min, x_min + 0.01, tex_coord.x)*(1. - smoothstep(x_max - 0.01, x_max, tex_coord.x));
}
void main()                                  
{            

    float n = snoise(v_tex_coord*(0.5 + u_tex_multiplier));

    FragColor = vec4(vec3(n), 1);// nvec4(vec3(1. - line(fract(5.*v_tex_coord), u_width, vec2(0.5, 0.5), vec2(1., 1.))),  1);    
}                                            