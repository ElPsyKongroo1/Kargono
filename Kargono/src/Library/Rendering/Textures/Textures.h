#pragma once
#include "../../Includes.h"

/*============================================================================================================================================================================================
 * Texture Struct that represents indivudal texture object
 *============================================================================================================================================================================================*/

class Texture {
public:
	unsigned int id;
	std::string type;
	std::string path;  // we store the path of the texture to compare with other textures
public:
	Texture(const char* textureSource, const char* textureType);
	Texture() {}
	~Texture();
private:
	void loadImage(const char* source);
};


/*============================================================================================================================================================================================
 * Textures class that manages 


 objects
 *============================================================================================================================================================================================*/

class TextureManager
{
public:
	Texture* smileyFace;
	Texture* smileyFaceSpec;
	Texture* crate;
	Texture* crateSpec;
public:
	void CreateTextures();
	void DestroyTextures();
	

private:
	void loadImage(std::string& source);


};