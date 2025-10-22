#version 450 core

in vec3 vColor;
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D uTex2d;
uniform bool uHasTex;
uniform bool uIsWireframe;
uniform bool uIsBackground;

void main()
{
    
    if (uIsBackground)
    {
        FragColor = vec4(1, 0, 0, 1);
        return;
    }
    if (uIsWireframe)
    {
        FragColor = vec4(0, 1, 0, 1);
        return;
    }

    vec4 texColor1 = texture(uTex2d, vTex);
    FragColor = uHasTex? texColor1 : vec4(vColor, 1.0);
}
