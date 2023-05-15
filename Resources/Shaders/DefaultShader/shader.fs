#version 460 core

struct Material {
    sampler2D diffuse[5];
}; 

in vec2 fTextureCoord;
out vec4 FragColor;

uniform Material material;

void main()
{
    FragColor = mix(texture(material.diffuse[0], fTextureCoord), texture(material.diffuse[1], fTextureCoord), 0.2);
}