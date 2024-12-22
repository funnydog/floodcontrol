#version 330 core

in vec2 fragUV;
in vec4 fragColor;

uniform sampler2D image;

layout (location = 0) out vec4 outColor;

void main()
{
	outColor = fragColor * texture(image, fragUV);
}
