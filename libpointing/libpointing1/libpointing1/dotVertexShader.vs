#version 330 core
layout (location = 0) in vec3 aPosC;
layout (location = 1) in vec3 aColorC;
out vec3 outColorC;

void main()
{
    gl_Position = vec4(aPosC, 1.0);
    outColorC = aColorC; 
}
