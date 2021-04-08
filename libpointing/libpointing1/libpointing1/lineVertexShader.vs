#version 330 core
layout (location = 0) in vec3 aPosLine;
layout (location = 1) in vec4 aColorLine;
out vec4 outColorLine;

void main()
{
    gl_Position = vec4(aPosLine, 1.0);
    outColorLine = aColorLine; 
}
