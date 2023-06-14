#pragma once
#include "../../Includes.h"
#include "../Textures/Textures.h"
#include "../Shaders/Shaders.h"


/*============================================================================================================================================================================================
 * GLMesh Object
 *============================================================================================================================================================================================*/
struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	Vertex()
	{
		Position = glm::vec3(0.0f, 0.0f, 0.0f);
		Normal = glm::vec3(0.0f, 0.0f, 0.0f);
		TexCoords = glm::vec2(0.0f, 0.0f);
	}
	Vertex(glm::vec3 Position, glm::vec3 Normal, glm::vec2 TexCoords)
	{
		this->Position = Position;
		this->Normal = Normal;
		this->TexCoords = TexCoords;
	}
	~Vertex() 
	{
		Position = glm::vec3();
		Normal = glm::vec3();
		TexCoords = glm::vec2();
	}
};

class GLMesh
{
public:
	enum OUTPUTTYPE { DRAWARRAYS, DRAWELEMENTS, NOOUTPUT };
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture*> textures;
	GLuint vao;
	GLuint vbos[2];
	OUTPUTTYPE output;
public:
	GLMesh() 
	{
	}
	GLMesh(GLMesh::OUTPUTTYPE outputType, std::vector<unsigned int>& indices, std::vector<Vertex>& vertices);
	~GLMesh();
	GLMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture*> textures, GLMesh::OUTPUTTYPE type)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;
		this->output = type;

		setupMesh();
	}
public:
	void Draw(void* object);
private:
	void setupMesh();
};

/*============================================================================================================================================================================================
 * MeshManager Class
 *============================================================================================================================================================================================*/

class MeshManager 
{
public:
	GLMesh* cubeMesh;
public:
	void CreateMeshes();
	void DestroyMeshes();
	
private:
	void CreateCubeMesh(GLMesh& mesh);
	void DestroyMesh(GLMesh& mesh);
};