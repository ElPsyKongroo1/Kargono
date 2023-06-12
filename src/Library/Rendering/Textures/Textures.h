#pragma once
#include "../Includes.h"

/*============================================================================================================================================================================================
 * Texture Struct that represents indivudal texture object
 *============================================================================================================================================================================================*/

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;  // we store the path of the texture to compare with other textures
};


/*============================================================================================================================================================================================
 * Textures class that manages 


 objects
 *============================================================================================================================================================================================*/

class TextureManager
{
public:
	Texture smileyFace;
	Texture smileyFaceSpec;
	Texture crate;
	Texture crateSpec;
public:
	void CreateTextures();
	void DestroyTextures();
	
private:
	void CreateSmiley(Texture& texture);
	void CreateSmileySpec(Texture& texture);
	void CreateCrate(Texture& texture);
	void CreateCrateSpec(Texture& texture);
	void DestroyTexture(Texture& texture);
private:
	void loadImage(std::string& source);


};