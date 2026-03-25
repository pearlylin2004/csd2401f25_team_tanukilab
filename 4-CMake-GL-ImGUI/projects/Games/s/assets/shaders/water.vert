#version 450 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 V;
uniform mat4 P;
uniform float uTime;
uniform float uWaveAmplitude;
uniform float uWaveFrequency;
uniform float uWaveSpeed;

struct InstanceData
{
    mat4 model;
    vec4 color;
    vec4 texParams;
};

layout(std430, binding = 0) buffer TranslationBuffer 
{
    InstanceData translations[];
};

out vec4 vColor;
out vec2 vTexCoord;
out float vWaveHeight;

void main()
{
    vec4 worldPos = translations[gl_InstanceID].model * vec4(aPosition, 0.0, 1.0);
    
    // Apply wave displacement only to top vertices (y > 0)
    if (aPosition.y > 0.0) {
        // Calculate wave using sine function
        float wave1 = sin(worldPos.x * uWaveFrequency + uTime * uWaveSpeed) * uWaveAmplitude;
        float wave2 = sin(worldPos.x * uWaveFrequency * 1.5 + uTime * uWaveSpeed * 0.8) * uWaveAmplitude * 0.5;
        
        float waveHeight = wave1 + wave2;
        worldPos.y += waveHeight;
        vWaveHeight = waveHeight;
    } else {
        vWaveHeight = 0.0;
    }
    
    gl_Position = P * V * worldPos;
    vColor = translations[gl_InstanceID].color;
    vTexCoord = aTexCoord * translations[gl_InstanceID].texParams.zw + translations[gl_InstanceID].texParams.xy;
}
