attribute vec4 position;                      
uniform vec2 scale;
varying vec3 color;       


void main()                                   
{                                             
    gl_Position = vec4(position.xy*scale + vec2(0.5,0), position.z, 1.0);    
    color = gl_Position.xyz + vec3(0.5);      
}                                             