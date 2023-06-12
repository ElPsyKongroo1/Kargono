#include "RendererState.h"
#include "../Library.h"

/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * RendererState Manager Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/

 /*============================================================================================================================================================================================
  * Instantiate/Delete Application objects
  *============================================================================================================================================================================================*/
void RendererStateManager::CreateDefaultRenderers()
{
	CreateSample3DRenderer(Sample3DRenderer);
	CreateSample2DRenderer(Sample2DRenderer);

}

void RendererStateManager::DestroyDefaultRenderers()
{
	DestroyRenderer(Sample3DRenderer);
	DestroyRenderer(Sample2DRenderer);

}

void RendererStateManager::DestroyRenderer(RendererState& application)
{
	application.programName = "";
	int GLFWVersionSize = sizeof(application.GLFWVersion) / sizeof(int);
	for (int i = 0; i < GLFWVersionSize; i++)
	{
		application.GLFWVersion[i] = 0;
	}
	application.screenDimension = glm::vec3();
	application.backgroundColor = glm::vec3();

	application.objectRenderBuffer.clear();
	application.lightSourceRenderBuffer.clear();
	application.currentInput = nullptr;
	application.currentCamera = nullptr;
	application.defaultMesh = nullptr;
	application.defaultModel = nullptr;
	application.defaultShader = nullptr;
}

/*============================================================================================================================================================================================
 * Initializes RendererState Objects with specified values
 *============================================================================================================================================================================================*/

void RendererStateManager::CreateSample3DRenderer(RendererState& application)
{
	application.programName = "3D Renderer";
	application.GLFWVersion[0] = 4;
	application.GLFWVersion[1] = 6;
	application.screenDimension = glm::vec2(1920.0f, 1080.0f);
	application.backgroundColor = glm::vec3(0.0f, 0.0f, 0.0f);

	application.objectRenderBuffer = std::vector<Object>();
	application.lightSourceRenderBuffer = std::vector<LightSource*>();
	application.currentWindow = nullptr;

}

void RendererStateManager::CreateSample2DRenderer(RendererState& application)
{
	application.programName = "2D Renderer";
	application.GLFWVersion[0] = 4;
	application.GLFWVersion[1] = 6;
	application.screenDimension = glm::vec2(1920.0f, 1080.0f);
	application.backgroundColor = glm::vec3(0.0f, 0.11f, 0.1f);

	application.objectRenderBuffer = std::vector<Object>();
	application.lightSourceRenderBuffer = std::vector<LightSource*>();
	application.currentWindow = nullptr;

}