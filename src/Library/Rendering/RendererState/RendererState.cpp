#include "RendererState.h"
#include "../../Library.h"
#include "../../Application/Input/InputCallback.h"
/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * RendererState Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/

RendererState::RendererState(const char* programName, int GLFWVersion[2], glm::vec2 screenDimension, glm::vec3 backgroundColor)
{
    this->programName = programName;
    this->GLFWVersion[0] = 4;
    this->GLFWVersion[1] = 6;
    this->screenDimension = screenDimension;
    this->backgroundColor = backgroundColor;

    this->objectRenderBuffer = std::vector<Object*>();
    this->lightSourceRenderBuffer = std::vector<LightSource*>();
    this->currentWindow = nullptr;
}
RendererState::~RendererState() 
{
    programName = "";
    int GLFWVersionSize = sizeof(GLFWVersion) / sizeof(int);
    for (int i = 0; i < GLFWVersionSize; i++)
    {
        GLFWVersion[i] = 0;
    }
    screenDimension = glm::vec3();
    backgroundColor = glm::vec3();
    for (Object* object : objectRenderBuffer) 
    {
        delete object;
        object = nullptr;
    }
    objectRenderBuffer.clear();
    lightSourceRenderBuffer.clear();
    currentCamera = nullptr;
    defaultMesh = nullptr;
    defaultModel = nullptr;
    defaultShader = nullptr;
    window = nullptr;
    currentWindow = nullptr;
}


void RendererState::init() 
{
	InitializeRenderer(); // GLFW, GLEW, and IMGui contexts
	InitializeDefaultResources();
}

void RendererState::render() 
{
    PreRendering();
    for (Object* object : objectRenderBuffer)
    {
        object->renderer->render();
    }
    WindowRendering();
    PostRendering();
}

void RendererState::close() 
{
    closeLibraryResources();
    terminate();
}

void RendererState::setDefaultValues(Model* model, GLMesh* mesh, GLShader* shader, GLCamera* camera)
{
	defaultModel = model;
	defaultMesh = mesh;
	defaultShader = shader;
	currentCamera = camera;
}

void RendererState::InitializeDefaultResources()
{
	Resources::textureManager.CreateTextures();
	Resources::shaderManager.CreateShaders();
	Resources::cameraManager.CreateCameras();
	Resources::inputManager.CreateInputs();
	Resources::windowManager.CreateUIWindows();
	Resources::meshManager.CreateMeshes();
	Resources::modelManager.CreateModels();

}

void RendererState::InitializeRenderer()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GLFWVersion[0]);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GLFWVersion[1]);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    window = glfwCreateWindow(screenDimension.x, screenDimension.y, programName, glfwGetPrimaryMonitor(), NULL);
    if (window == NULL)
    {
        std::cout << "Failed to Initialize Window due to: " << std::endl;
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        throw std::runtime_error("ERROR::FAILED_TO_INITIALIZE_GLFW");
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to Initialize Window due to: " << std::endl;
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        throw std::runtime_error("ERROR::FAILED_TO_INITIALIZE_GLFW");
    }

    glViewport(0, 0, screenDimension.x, screenDimension.y);
    glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_ALWAYS); // Everything is displaying out of order. Hell yea
    glfwSwapInterval(1);


    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, UKeyCallback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    std::string version = "#version " + std::to_string(GLFWVersion[0]) + std::to_string(GLFWVersion[1]) + std::to_string(0);
    ImGui_ImplOpenGL3_Init(version.c_str());

    return;
}

void RendererState::PreRendering()
{
    // Update delta time and use fps counter
    Resources::currentFrame = (float)glfwGetTime();
    Resources::deltaTime = Resources::currentFrame - Resources::lastFrame;
    Resources::lastFrame = Resources::currentFrame;
    Resources::framesCounter += Resources::deltaTime;
    Resources::framesPerSecond++;
    Resources::runtime += Resources::deltaTime;
    if (Resources::framesCounter > 1.0f)
    {
        //std::cout << Resources::runtime << " Runtime" << std::endl; //FIXME
        Resources::framesCounter -= 1.0f;
        //std::cout << Resources::framesPerSecond << "FPS" << std::endl; //FIXME
        Resources::framesPerSecond = 0;
    }
    //Clear and Reset Background Color
    glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

}

void RendererState::WindowRendering()
{
    for (int i = 0; i < Resources::windowManager.allWindowsSize; i++)
    {
        if (Resources::windowManager.allWindows[i]->isRendering)
        {
            Resources::windowManager.allWindows[i]->renderWindow();
        }
    }
}

void RendererState::PostRendering()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap Buffers for current frame
    glfwSwapBuffers(window);
    // Process Input
    FProcessInputHold(window);
    // Check for any GLFW events
    glfwPollEvents();
}


void RendererState::closeLibraryResources()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    Resources::meshManager.DestroyMeshes();
    Resources::shaderManager.DestroyShaders();
    Resources::textureManager.DestroyTextures();
    Resources::currentApplication->renderer->objectRenderBuffer.clear();
    Resources::currentApplication->renderer->lightSourceRenderBuffer.clear();
    Resources::cameraManager.DestroyCameras();
    Resources::inputManager.DestroyInputs();
    Resources::windowManager.DestroyUIWindows();
    Resources::modelManager.DestroyModels();

    Resources::deltaTime = 0.0f;
    Resources::lastFrame = 0.0f;
    Resources::currentFrame = 0.0f;
    Resources::framesPerSecond = 0;
    Resources::framesCounter = 0.0f;
}
void RendererState::terminate()
{
    glfwTerminate();
}

