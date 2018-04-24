// shadertype=glsl
#version 330 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec3 color;

uniform float width;
uniform float height;

out vec4 outColor;

void main()
{
	vec3 projPos = vec3(2.0 * pos.x / width - 1.0, 2.0 * pos.y / height - 1.0, 0.0);
	gl_Position = vec4(projPos, 1.0);
	gl_PointSize = 2.0;
	outColor = vec4(color, 1.0);
}