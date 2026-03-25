#version 450 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexPos;

//layout(location = 3) in mat4 iModel;
//layout(location = 7) in vec4 iColor;

uniform mat4 V;
uniform mat4 P;

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

void main()
{
    //gl_Position = P * V * translations[gl_InstanceID].model * vec4(aPosition, 0.0, 1.0);
    gl_Position = P * V * translations[gl_InstanceID].model * vec4(aPosition, 0.0, 1.0);
    vColor = translations[gl_InstanceID].color;
}
