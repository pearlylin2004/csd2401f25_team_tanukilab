#version 450 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec3 aColor;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;


out vec3 vColor;

void main()
{
    gl_Position = P * V * M * vec4(aPosition, 0.0, 1.0);
    //gl_Position = vec4(aPosition, 0, 1.0);
    vColor = aColor;
}
