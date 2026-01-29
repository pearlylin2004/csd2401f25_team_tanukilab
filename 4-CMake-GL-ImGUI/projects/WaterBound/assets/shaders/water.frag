#version 450 core

in vec4 vColor;
in vec2 vTexCoord;
in float vWaveHeight;
out vec4 FragColor;

uniform sampler2D uTex2d;
uniform bool uUseTexture;
uniform float uWaterAlpha;
uniform vec4 uWaterColor;

void main()
{
    vec4 baseColor;
    
    if (uUseTexture) {
        baseColor = texture(uTex2d, vTexCoord);
    } else {
        baseColor = uWaterColor;
    }
    
    float highlight = vWaveHeight * 0.5 + 0.5; // Normalize to 0-1
    baseColor.rgb += vec3(0.1, 0.15, 0.2) * highlight;
    
    baseColor.a = uWaterAlpha;
    
    FragColor = baseColor * vColor;
   
    // float edgeFade = abs(vTexCoord.y - 0.5) * 2.0;
    // FragColor.a *= (1.0 - edgeFade * 0.3);
}
