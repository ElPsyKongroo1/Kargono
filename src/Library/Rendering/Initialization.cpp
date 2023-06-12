#include "Includes.h"
#include "Library.h"
#include "../Game/Objects/Objects.h"


void UKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


/*============================================================================================================================================================================================
 * Initialization
 *============================================================================================================================================================================================*/

void FInitializeRenderer()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Resources::currentRenderer->GLFWVersion[0]);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Resources::currentRenderer->GLFWVersion[1]);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    Resources::currentRenderer->window = glfwCreateWindow(Resources::currentRenderer->screenDimension.x, Resources::currentRenderer->screenDimension.y, Resources::currentRenderer->programName, glfwGetPrimaryMonitor(), NULL);
    if (Resources::currentRenderer->window == NULL)
    {
        std::cout << "Failed to Initialize Window due to: " << std::endl;
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        throw std::runtime_error("ERROR::FAILED_TO_INITIALIZE_GLFW");
    }
    glfwMakeContextCurrent(Resources::currentRenderer->window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to Initialize Window due to: " << std::endl;
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        throw std::runtime_error("ERROR::FAILED_TO_INITIALIZE_GLFW");
    }

    glViewport(0, 0, Resources::currentRenderer->screenDimension.x, Resources::currentRenderer->screenDimension.y);
    glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_ALWAYS); // Everything is displaying out of order. Hell yea
    glfwSwapInterval(1);
    

    glfwSetFramebufferSizeCallback(Resources::currentRenderer->window, framebuffer_size_callback);
    glfwSetScrollCallback(Resources::currentRenderer->window, scroll_callback);
    glfwSetInputMode(Resources::currentRenderer->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(Resources::currentRenderer->window, mouse_callback);
    glfwSetKeyCallback(Resources::currentRenderer->window, UKeyCallback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui_ImplGlfw_InitForOpenGL(Resources::currentRenderer->window, true);
    std::string version = "#version " + std::to_string(Resources::currentRenderer->GLFWVersion[0]) + std::to_string(Resources::currentRenderer->GLFWVersion[1]) + std::to_string(0);
    ImGui_ImplOpenGL3_Init(version.c_str());

    return;
}

void FInitializeLibraryResources()
{
    Resources::windowManager.CreateUIWindows();
    Resources::textureManager.CreateTextures();
    Resources::meshManager.CreateMeshes();
    Resources::shaderManager.CreateShaders();
    Resources::inputManager.CreateInputs();
    Resources::cameraManager.CreateCameras();
    Resources::modelManager.CreateModels();

}



/*============================================================================================================================================================================================
 * Input/Callback
 *============================================================================================================================================================================================*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    Resources::currentRenderer->currentInput->processMouseMovement(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Resources::currentRenderer->currentInput->processScroll(xoffset, yoffset);
}

void FProcessInputHold(GLFWwindow* window)
{
    Resources::currentRenderer->currentInput->processKeyboardHold(window);
    int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
    if (!present)
    {
        // Nothing, the controller is not connected
    }
    else
    {
        Resources::currentRenderer->currentInput->processGamePad();
    }

}

void UKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Resources::currentRenderer->currentInput->processKeyboardClick(window, key, scancode, action, mods);
}