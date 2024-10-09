#version 300 es 

precision mediump float;    

in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform sampler2D u_input;

// uniform vec2 u_texel_multiplier = vec2(1., 1.);

void main()
{            

	ivec2 tex_sizei = textureSize(u_input, 0);
	vec2 texel_size = vec2(1./float(tex_sizei.x), 1./float(tex_sizei.y));


    mat3 kernel;
    kernel[0] = vec3(1., 0., -1.);
    kernel[1] = vec3(1., 0., -1.);
    kernel[2] = vec3(1., 0., -1.);

    vec4 result = vec4(0.);
    for(int i = -1; i < 2; ++i)
    {
        for(int j = -1; j < 2; ++j)
        {
            float dx = float(i)*texel_size.x;
            float dy = float(j)*texel_size.y;
            result += kernel[i+1][j+1] * texture(u_input, v_tex_coord + vec2(dx, dy));
        }
    }

    FragColor = result;   
}


