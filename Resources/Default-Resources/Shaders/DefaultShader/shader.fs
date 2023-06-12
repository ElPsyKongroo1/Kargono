#version 460 core
#define MAXIMUM_TEXTURE_UNITS 5


struct Material {
    int nDiffuseTexture;
    sampler2D diffuse[MAXIMUM_TEXTURE_UNITS];
}; 

in vec2 fTextureCoord;
out vec4 FragColor;

uniform Material material;

void processTextureUnits (out vec3 diffuseColor );

void main()
{
    vec3 diffuseColor;
    processTextureUnits(diffuseColor);
    FragColor = vec4(diffuseColor, 1.0f);
}

void processTextureUnits (out vec3 diffuseColor )
{
    if (material.nDiffuseTexture == 1)
    {
       diffuseColor = vec3(texture(material.diffuse[0], fTextureCoord));
    }
    else 
    {
        vec4 diffuseMix = mix(texture(material.diffuse[0], fTextureCoord), texture(material.diffuse[1], fTextureCoord), 0.2);
        for (int i = 0; i < material.nDiffuseTexture - 2; i++)
        {
            diffuseMix = mix(diffuseMix, texture(material.diffuse[i + 2], fTextureCoord), 0.2);
        }
        diffuseColor = vec3(diffuseMix);
    }
}
