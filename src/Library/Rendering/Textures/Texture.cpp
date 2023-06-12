#include "Textures.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../../External_Libraries/stb_image.h"

Texture::Texture(const char* textureSource, const char* textureType) 
{
    type = textureType;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    loadImage(textureSource); // Loads image into texture on GLFW context
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() 
{
    glDeleteTextures(1, &id);
    id = -1;
    type = "";
    path = "";
}

void Texture::loadImage(const char* source)
{
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(source, &width, &height, &nrChannels, 0);
    if (data)
    {
        if (nrChannels == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else if (nrChannels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }
    else
    {
        std::cout << "ERROR::FAILED_TO_LOAD_TEXTURE" << std::endl;
        throw std::runtime_error("Check Error Logs");
    }
    stbi_image_free(data);
}
