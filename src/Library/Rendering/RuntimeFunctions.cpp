#include "Includes.h"
#include "Library.h"
#include "Initialization.h"

void FPreRendering()
{
    // Update delta time and use fps counter
    Resources::currentFrame = (float)glfwGetTime();
    Resources::deltaTime = Resources::currentFrame - Resources::lastFrame;
    Resources::lastFrame = Resources::currentFrame;
    Resources::framesCounter += Resources::deltaTime;
    Resources::framesPerSecond++;
    if (Resources::framesCounter > 1.0f)
    {
        Resources::framesCounter -= 1.0f;
        std::cout << Resources::framesPerSecond << "FPS" << std::endl;
        Resources::framesPerSecond = 0;
    }
     //Clear and Reset Background Color
    glClearColor(Resources::currentRenderer->backgroundColor.r, Resources::currentRenderer->backgroundColor.g, Resources::currentRenderer->backgroundColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

}

void FWindowRendering()
{
    for (int i = 0; i < Resources::windowManager.allWindowsSize; i++)
    {
        if (Resources::windowManager.allWindows[i]->isRendering)
        {
            Resources::windowManager.allWindows[i]->renderWindow();
        }
    }
}




void FPostRendering()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // Process Input
    FProcessInputHold(Resources::currentRenderer->window);
    // Swap Buffers for current frame
    glfwSwapBuffers(Resources::currentRenderer->window);
    // Check for any GLFW events
    glfwPollEvents();
}