#include "Includes.h"
#include "Library.h"
#include "Objects/Objects.h"


void UKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void FRenderObject(ModelRenderer& object);


/*============================================================================================================================================================================================
 * Initialization
 *============================================================================================================================================================================================*/

void FInitializeRenderer()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Resources::currentApplication->GLFWVersion[0]);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Resources::currentApplication->GLFWVersion[1]);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    Resources::currentApplication->window = glfwCreateWindow(Resources::currentApplication->screenDimension.x, Resources::currentApplication->screenDimension.y, Resources::currentApplication->programName, glfwGetPrimaryMonitor(), NULL);
    if (Resources::currentApplication->window == NULL)
    {
        std::cout << "Failed to Initialize Window due to: " << std::endl;
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        throw std::runtime_error("ERROR::FAILED_TO_INITIALIZE_GLFW");
    }
    glfwMakeContextCurrent(Resources::currentApplication->window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to Initialize Window due to: " << std::endl;
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        throw std::runtime_error("ERROR::FAILED_TO_INITIALIZE_GLFW");
    }

    glViewport(0, 0, Resources::currentApplication->screenDimension.x, Resources::currentApplication->screenDimension.y);
    glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_ALWAYS); // Everything is displaying out of order. Hell yea
    glfwSwapInterval(1);
    

    glfwSetFramebufferSizeCallback(Resources::currentApplication->window, framebuffer_size_callback);
    glfwSetScrollCallback(Resources::currentApplication->window, scroll_callback);
    glfwSetInputMode(Resources::currentApplication->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(Resources::currentApplication->window, mouse_callback);
    glfwSetKeyCallback(Resources::currentApplication->window, UKeyCallback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui_ImplGlfw_InitForOpenGL(Resources::currentApplication->window, true);
    std::string version = "#version " + std::to_string(Resources::currentApplication->GLFWVersion[0]) + std::to_string(Resources::currentApplication->GLFWVersion[1]) + std::to_string(0);
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
    Resources::currentApplication->currentInput->processMouseMovement(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Resources::currentApplication->currentInput->processScroll(xoffset, yoffset);
}

void FProcessInputHold(GLFWwindow* window)
{
    Resources::currentApplication->currentInput->processKeyboardHold(window);
    int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
    if (!present)
    {
        // Nothing, the controller is not connected
    }
    else
    {
        Resources::currentApplication->currentInput->processGamePad();
    }

}

void UKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Resources::currentApplication->currentInput->processKeyboardClick(window, key, scancode, action, mods);
}