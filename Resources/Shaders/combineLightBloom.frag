#version 300 es 

precision mediump float;    

uniform sampler2D u_source;
uniform sampler2D u_bloom;

uniform float exposure = 0.5;

in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

void main(void)
{

    const float gamma = 2.2;
    
    vec3 source_color = texture(u_source, v_tex_coord).rgb;      
    vec3 bloom_color = texture(u_bloom, v_tex_coord).rgb;
    float source_alpha = texture(u_source, v_tex_coord).a;
    float bloom_alpha = texture(u_bloom, v_tex_coord).a;

    vec3 hdr_color = source_color + bloom_color;             // additive blending
    vec3 result = vec3(1.0) - exp(-hdr_color * exposure);    // tone mapping
    result = pow(result, vec3(1.0 / gamma));                // gamme correction      

    FragColor = vec4(result, bloom_alpha);
}
