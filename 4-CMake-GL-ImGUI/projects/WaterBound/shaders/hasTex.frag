#version 450 core

in vec3 vColor;
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D uTex2d;

void main()
{
    vec4 texColor1 = texture(uTex2d, vTex);
    if(texColor1.a <= 0.01)
    {
        discard;
    }
    FragColor = texColor1;
}
