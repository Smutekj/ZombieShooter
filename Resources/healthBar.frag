#version 300 es 

precision mediump float;    

in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform float u_tex_multiplier = 100.0;
uniform float u_time_multiplier = 0.9;
uniform float u_health_percentage = 1.0;
uniform vec3 u_color = vec3(1.,0.2,0.);
uniform vec3 u_color_fire = vec3(1.,1.,0.);
uniform vec3 u_color_edge = vec3(0.,0.,1.);
uniform float u_time;


uniform sampler2D u_texture;

float squareSDF(vec2 tex, vec2 center, vec2 size)
{
    return min(tex.x - center.x - size.x, tex.y - center.y - size.y);
}

float sdBox( vec2 p, vec2 b )
{
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

void main()                                  
{   

    float sdf = sdBox(v_tex_coord - vec2(0.5, 0.5), vec2(0.51, 0.47));
    float rect_shape = 1. - smoothstep(-0.05, 0.0, sdf) ;

    float alpha = 1. - smoothstep(u_health_percentage, u_health_percentage + 0.05, v_tex_coord.x);
    if(rect_shape < 0.5)
    {
        FragColor = vec4(u_color_edge,  1.0);
    }    
    else{
        FragColor = vec4(1.0 - v_tex_coord.x, alpha*v_tex_coord.x,  0., alpha);
    }       
}                                   