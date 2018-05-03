// shadertype=glsl
#version 330 core
layout (location=0) in vec3 pos;
layout (location=1) in int index;

uniform mat4 viewMat;
uniform mat4 projMat;
uniform mat4 modelMat[100];

out vec4 outColor;

void main()
{
	gl_Position = projMat * viewMat * modelMat[int(index)] * vec4(pos, 1.0);
	outColor = vec4(1.0, 1.0, 0.0 , 1.0);
}

