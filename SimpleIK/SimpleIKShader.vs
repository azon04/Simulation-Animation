// shadertype=glsl
#version 330 core
layout (location=0) in vec2 pos;

uniform mat3 modelMatrix;

uniform float width;
uniform float height;

void main()
{
	gl_Position = vec4( vec2(modelMatrix * vec3(pos, 1.0)), 0.0, 1.0 );	
	gl_Position.x = 2 * ( gl_Position.x / width ) - 1.0;
	gl_Position.y = 2 * ( gl_Position.y / height ) - 1.0;
}