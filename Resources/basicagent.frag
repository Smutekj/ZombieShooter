#version 300 es 

precision mediump float;    

#include 

in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform float u_time_multiplier = 5.5;
uniform float u_time;

uniform vec2 u_shape_vec = vec2(1.,1.);
uniform vec3 u_color = vec3(5.,0.5,0.);
uniform vec3 u_color_edge = vec3(0.,0.5,12.);

uniform sampler2D u_texture;

#if !defined(FNC_SATURATE) && !defined(saturate)
#define FNC_SATURATE
#define saturate(V) clamp(V, 0.0, 1.0)
#endif

float capsuleSDF( vec3 p, vec3 a, vec3 b, float r ) {
    vec3 pa = p-a, ba = b-a;
    float h = saturate( dot(pa,ba)/dot(ba,ba) );
    return length( pa - ba*h ) - r;
}


void main()                                  
{            
    float a = 0.05;
    vec2 scale = vec2(u_shape_vec.x +a*sin(u_time*u_time_multiplier),
                     u_shape_vec.y - a*sin(u_time*u_time_multiplier));
    vec2 center = vec2(0.5*scale.x, 0.5 * scale.y);
    vec2 left_eye_center = center  + vec2(-0.18, 0.2);
    vec2 right_eye_center = center  + vec2(+0.18, 0.2);
    vec2 tex = vec2(v_tex_coord.x*scale.x, v_tex_coord.y * scale.y);
    float d_center = distance(center, tex);
    float left_eye_sdf = distance(left_eye_center, v_tex_coord);
    float right_eye_sdf = distance(right_eye_center, v_tex_coord);
    
    float shape_factor = smoothstep(0.35, 0.37, d_center) - smoothstep(0.37, 0.4, d_center);
    float left_eye = 1.0 - smoothstep(0.05, 0.1, left_eye_sdf);
    float right_eye = 1.0 - smoothstep(0.05, 0.1, right_eye_sdf);

    vec3 result = u_color_edge *shape_factor + u_color*(right_eye + left_eye);
    FragColor = vec4(result, clamp(shape_factor + right_eye + left_eye, 0., 1.));
}                                          