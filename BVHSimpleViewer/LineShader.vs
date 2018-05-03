// shadertype=glsl
#version 330 core
layout (location=0) in vec3 pos;
layout (location=1) in vec3 color;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;

out vec4 outColor;

void main()
{
	gl_Position = projMat * viewMat * modelMat * vec4(pos, 1.0);
	outColor = vec4(color, 1.0);
}