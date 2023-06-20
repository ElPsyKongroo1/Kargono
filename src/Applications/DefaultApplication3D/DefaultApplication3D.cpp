#include "../../Library/Includes.h"
#include "../../Library/Library.h"
#include "../../Library/Rendering/Shaders/Shaders.h"
#include "../../Library/Rendering/Mesh/Meshes.h"
#include "../../Library/Rendering/Textures/Textures.h"
#include "../../Library/Application/Objects/Objects.h"
#include "../../Library/Rendering/UserInterface/WindowFunctions.h"


/*============================================================================================================================================================================================
 * Application Entry Point
 *============================================================================================================================================================================================*/


int DefaultApplication3D()

{
    // Initialize GLFW context, Meshes, Shaders, and Textures
    Resources::currentApplication->renderer->init();
    Resources::currentApplication->recentInput = Resources::inputManager.default3DInput;
    Resources::currentApplication->currentInput = Resources::currentApplication->recentInput;
    Resources::currentApplication->renderer->setDefaultValues(Resources::modelManager.simpleBackpack,
        Resources::meshManager.cubeMesh,
        Resources::shaderManager.lightingShader,
        Resources::cameraManager.flyCamera);
    
    // Initialize Objects
    Orientation orientation1{ glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
                                    glm::vec3(2.0f, 0.0f, 0.0f),
                                    glm::vec3(2.0f, 2.0f, 2.0f) };
    ModelRenderer* renderer1{ new ModelRenderer(orientation1) };
    Object* object1{ new Object(orientation1, renderer1) };
    Resources::currentApplication->renderer->objectRenderBuffer.push_back(object1);
    
    

    Orientation orientation2{ glm::vec4(1.0f, 0.0f, 0.0f, 270.0f),
                              glm::vec3(3.0f, -2.0f, 4.0f),
                              glm::vec3(50.0f, 50.0f, 2.0f) };
    ModelRenderer* renderer2{ new ModelRenderer(orientation2,
        Resources::currentApplication->renderer->defaultModel,
        Resources::currentApplication->renderer->defaultShader) };
    Object* object2{ new Object(orientation2, renderer2) };
    Resources::currentApplication->renderer->objectRenderBuffer.push_back(object2);
    

    Orientation orientation4{ glm::vec4(0.0f, 1.0f, 0.0f, 45.0f),
                              glm::vec3(7.0f, 5.0f, -4.0f),
                              glm::vec3(0.5f, 0.5f, 0.5f) };
    ModelRenderer* renderer4{ new ModelRenderer(orientation4,
        Resources::currentApplication->renderer->defaultModel,
        Resources::currentApplication->renderer->defaultShader) };
    renderer4->addLightSource(glm::vec3(0.6f, 0.5f, 0.85f));
    Object* object4{ new Object(orientation4, renderer4) };
    Resources::currentApplication->renderer->objectRenderBuffer.push_back(object4);
    

    Orientation orientation5{ glm::vec4(0.0f, 1.0f, 0.0f, 45.0f),
                              glm::vec3(-21.0f, 5.0f, 12.0f),
                              glm::vec3(0.5f, 0.5f, 0.5f) };
    ModelRenderer* renderer5{ new ModelRenderer(orientation5,
        Resources::currentApplication->renderer->defaultModel,
        Resources::currentApplication->renderer->defaultShader) };
    renderer5->addLightSource(glm::vec3(0.9f, 0.8f, 0.5f));
    Object* object5{ new Object(orientation5, renderer5) };
    Resources::currentApplication->renderer->objectRenderBuffer.push_back(object5);
    

    Orientation orientation7{ glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
                              glm::vec3(8.0f, 0.0f, -10.0f),
                              glm::vec3(1.0f, 1.0f, 1.0f) };
    ModelRenderer* renderer7{ new ModelRenderer(orientation7,
        Resources::modelManager.human,
        Resources::currentApplication->renderer->defaultShader) };
    renderer7->addLightSource(glm::vec3(0.6f, 0.5f, 0.85f));
    Object* object7{ new Object(orientation7, renderer7) };
    Resources::currentApplication->renderer->objectRenderBuffer.push_back(object7);
    

    Orientation orientation8{ glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
                              glm::vec3(-8.0f, 0.0f, 30.0f),
                              glm::vec3(1.0f, 1.0f, 1.0f) };
    ModelRenderer* renderer8{ new ModelRenderer(orientation8,
        Resources::modelManager.house,
        Resources::currentApplication->renderer->defaultShader) };
    Object* object8{ new Object(orientation8, renderer8) };
    Resources::currentApplication->renderer->objectRenderBuffer.push_back(object8);
    

    Orientation orientation9{ glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
                              glm::vec3(20.0f, 0.0f, 30.0f),
                              glm::vec3(1.0f, 1.0f, 1.0f) };
    ModelRenderer* renderer9{ new ModelRenderer(orientation9,
        Resources::modelManager.house2,
        Resources::currentApplication->renderer->defaultShader) };
    Object* object9{ new Object(orientation9, renderer9) };
    Resources::currentApplication->renderer->objectRenderBuffer.push_back(object9);

    Orientation orientation10{ glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
                              glm::vec3(0.0f, 10.0f, 0.0f),
                              glm::vec3(1.0f, 1.0f, 1.0f) };
    ShapeRenderer* renderer10{ new ShapeRenderer(orientation10) };
    Object* object10{ new Object(orientation10, renderer10) };
    Resources::currentApplication->renderer->objectRenderBuffer.push_back(object10);

    //Object object6 = Object(glm::vec4(0.0f, 1.0f, 0.0f, 45.0f),
    //    glm::vec3(0.0f, 40.0f, -40.0f),
    //    glm::vec3(0.5f, 0.5f, 0.5f),
    //    &Resources::meshManager.cubeMesh,
    //    Resources::currentApplication->defaultShader);
    //object6.createLightSource(glm::vec3(0.3f, 0.1f, 0.85f));
    //object6.lightSource.lightType = DIRECTIONAL;
    //object6.lightSource.lightDirection = glm::vec3(0.0f, -1.0f, 1.0f);
    //Resources::currentApplication->allObjects.push_back(object6);

    // Main running Loop
    while (!glfwWindowShouldClose(Resources::currentApplication->renderer->window))
    {
        Resources::currentApplication->renderer->render();

    }
    

    Resources::currentApplication->renderer->close();

    return 0;
}
