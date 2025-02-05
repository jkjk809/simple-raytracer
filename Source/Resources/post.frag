#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D originTexture;
uniform sampler2D bloomTexture;
float gamma = 2.2f;

void main()
{
	vec3 fragment = texture(originTexture, TexCoord).rgb;
	//vec3 bloom = texture(bloomTexture, TexCoord).rgb;

	vec3 color = fragment;

	//float exposure = 0.8f;
	//vec3 toneMap = vec3(1.0f) - exp(-color * exposure);
	//FragColor.rgb = fragment + bloom;
	//FragColor.rgb = pow(color.rgb, vec3(1.0/gamma));
	//FragColor.rgb = pow(toneMap, vec3(1.0f / gamma));

	FragColor.rgb = color;
}