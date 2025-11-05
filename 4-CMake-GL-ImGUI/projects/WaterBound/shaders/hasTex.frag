#version 450 core

in vec3 vColor;
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D uTex2d;

void main()
{
    // Test 1: Show texture coordinates
    FragColor = vec4(vTex, 0.0, 1.0);  // Should show red-green gradient
    
    vec4 texColor1 = texture(uTex2d, vTex);
    if(texColor1.a <= 0.01)
    {
        discard;
    }
    FragColor = texColor1;
}
