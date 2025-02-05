#version 330 core

// Input vertex attributes
layout(location = 0) in vec3 aPos;    // Vertex position
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
    // Pass the color to the fragment shader
    gl_Position = vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}