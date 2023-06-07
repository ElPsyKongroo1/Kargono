#include "../Library/Includes.h"
#include "../Library/Initialization.h"
#include "Rendering2.h"
#include "../Library/Library.h"
#include "../Library/Shaders/Shaders.h"
#include "../Library/Mesh/Meshes.h"
#include "../Library/Textures/Textures.h"
#include "../Library/Objects/Objects.h"
#include "../Library/UserInterface/WindowFunctions.h"
//#include <windows.h>

/*============================================================================================================================================================================================
 * Application Entry Point
 *============================================================================================================================================================================================*/


int DefaultApplication()

{
    // Initialize GLFW context, Meshes, Shaders, and Textures
    FInitializeRenderer();
    FInitializeLibraryResources();
    
    // Initialize Objects
    Orientation orientation1{ glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
                                    glm::vec3(2.0f, 0.0f, 0.0f),
                                    glm::vec3(2.0f, 2.0f, 2.0f) };
    ModelRenderer renderer1{ ModelRenderer(orientation1) };
    Object object1{ orientation1, &renderer1 };
    Resources::currentApplication->allSimpleObjects.push_back(object1);
    
    

    Orientation orientation2{ glm::vec4(1.0f, 0.0f, 0.0f, 270.0f),
                              glm::vec3(3.0f, -2.0f, 4.0f),
                              glm::vec3(50.0f, 50.0f, 2.0f) };
    ModelRenderer renderer2{ ModelRenderer(orientation2,
        Resources::currentApplication->defaultModel,
        Resources::currentApplication->defaultShader) };
    Object object2{ orientation2, &renderer2 };
    Resources::currentApplication->allSimpleObjects.push_back(object2);
    

    Orientation orientation4{ glm::vec4(0.0f, 1.0f, 0.0f, 45.0f),
                              glm::vec3(7.0f, 5.0f, -4.0f),
                              glm::vec3(0.5f, 0.5f, 0.5f) };
    ModelRenderer renderer4{ ModelRenderer(orientation4,
        Resources::currentApplication->defaultModel,
        Resources::currentApplication->defaultShader) };
    renderer4.addLightSource(glm::vec3(0.6f, 0.5f, 0.85f));
    Object object4{ orientation4, &renderer4 };
    Resources::currentApplication->allSimpleObjects.push_back(object4);
    

    Orientation orientation5{ glm::vec4(0.0f, 1.0f, 0.0f, 45.0f),
                              glm::vec3(-21.0f, 5.0f, 12.0f),
                              glm::vec3(0.5f, 0.5f, 0.5f) };
    ModelRenderer renderer5{ ModelRenderer(orientation5,
        Resources::currentApplication->defaultModel,
        Resources::currentApplication->defaultShader) };
    renderer5.addLightSource(glm::vec3(0.9f, 0.8f, 0.5f));
    Object object5{ orientation5, &renderer5 };
    Resources::currentApplication->allSimpleObjects.push_back(object5);
    

    Orientation orientation7{ glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
                              glm::vec3(8.0f, 0.0f, -10.0f),
                              glm::vec3(1.0f, 1.0f, 1.0f) };
    ModelRenderer renderer7{ ModelRenderer(orientation7,
        &Resources::modelManager.human,
        Resources::currentApplication->defaultShader) };
    renderer7.addLightSource(glm::vec3(0.6f, 0.5f, 0.85f));
    Object object7{ orientation7, &renderer7 };
    Resources::currentApplication->allSimpleObjects.push_back(object7);
    

    Orientation orientation8{ glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
                              glm::vec3(-8.0f, 0.0f, 30.0f),
                              glm::vec3(1.0f, 1.0f, 1.0f) };
    ModelRenderer renderer8{ ModelRenderer(orientation8,
        &Resources::modelManager.house,
        Resources::currentApplication->defaultShader) };
    Object object8{ orientation8, &renderer8 };
    Resources::currentApplication->allSimpleObjects.push_back(object8);
    

    Orientation orientation9{ glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
                              glm::vec3(20.0f, 0.0f, 30.0f),
                              glm::vec3(1.0f, 1.0f, 1.0f) };
    ModelRenderer renderer9{ ModelRenderer(orientation9,
        &Resources::modelManager.house2,
        Resources::currentApplication->defaultShader) };
    Object object9{ orientation9, &renderer9 };
    Resources::currentApplication->allSimpleObjects.push_back(object9);
    

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
    while (!glfwWindowShouldClose(Resources::currentApplication->window))
    {
        // PreRendering
        FPreRendering();

        // Render
        FRendering();
        
        FWindowRendering();

        // Post Rendering
        FPostRendering();

    }
    

    FTerminateLibraryResources();
    
    //glfwSetWindowShouldClose(Resources::currentApplication->window, GL_FALSE);
    //glfwSetWindowMonitor(Resources::currentApplication->window, NULL, 0, 0, Resources::currentApplication->screenDimension.x, Resources::currentApplication->screenDimension.y, 0);
    //glfwSetWindowShouldClose(Resources::currentApplication->window, GL_TRUE);
    //Sleep(3000);
    
    FTerminateRenderer();

    return 0;
}
