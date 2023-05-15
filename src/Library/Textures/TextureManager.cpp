#include "Textures.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../External_Libraries/stb_image.h"

/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * Textures Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/

 /*============================================================================================================================================================================================
  * Instantiate/Delete 
  
  
  objects
  *============================================================================================================================================================================================*/

void TextureManager::CreateTextures()
{
    CreateSmiley(smileyFace);
    CreateSmileySpec(smileyFaceSpec);
    CreateCrate(crate);
    CreateCrateSpec(crateSpec);
}


void TextureManager::DestroyTextures()
{
    DestroyTexture(smileyFace);
    DestroyTexture(crate);
    DestroyTexture(crateSpec);
    DestroyTexture(smileyFaceSpec);
}

void TextureManager::DestroyTexture(Texture& texture)
{

    glDeleteTextures(1, &texture.id);
    texture.id = -1;
    texture.type = "";
    texture.path = "";
}

/*============================================================================================================================================================================================
 * Initializes GLTexture Structs with specified values
 *============================================================================================================================================================================================*/


void TextureManager::CreateSmiley(Texture& texture)
{
    std::string textureSource = "Resources/Textures/Original/awesomeface.png";
    texture.type = "texture_diffuse";

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    loadImage(textureSource); // Loads image into texture on GLFW context
    glBindTexture(GL_TEXTURE_2D, 0);
}
void TextureManager::CreateSmileySpec(Texture& texture)
{
    std::string textureSource = "Resources/Textures/SpecularMap/awesomeface.png";
    texture.type = "texture_specular";

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    loadImage(textureSource); // Loads image into texture on GLFW context
    glBindTexture(GL_TEXTURE_2D, 0);
}
void TextureManager::CreateCrate(Texture& texture)
{
    std::string textureSource = "Resources/Textures/Original/Metal_Box.png";
    texture.type = "texture_diffuse";

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    loadImage(textureSource); // Loads image into texture on GLFW context
    glBindTexture(GL_TEXTURE_2D, 0);
}
void TextureManager::CreateCrateSpec(Texture& texture)
{
    std::string textureSource = "Resources/Textures/SpecularMap/Metal_Box.png";
    texture.type = "texture_specular";

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    loadImage(textureSource); // Loads image into texture on GLFW context
    glBindTexture(GL_TEXTURE_2D, 0);
}

/*============================================================================================================================================================================================
 * Internal Functionality
 *============================================================================================================================================================================================*/

void TextureManager::loadImage(std::string& source)
{
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(source.c_str(), &width, &height, &nrChannels, 0);
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