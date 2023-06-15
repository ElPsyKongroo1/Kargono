#pragma once
#include "../../Includes.h"
#include "../Input/Input.h"
#include "../../Rendering/RendererState/RendererState.h"

class Application
{
protected:
    Application() : renderer{ nullptr }, currentInput{ nullptr }, width{ 1 }, height{1} {}
    Application(unsigned int width, unsigned int height) : renderer{ nullptr }, currentInput{ nullptr }, width{ width }, height{ height } {};
public:
    RendererState* renderer;
    GLInput* currentInput;
    GLInput* defaultInput;

    unsigned int width, height;
    // constructor/destructor
    ~Application();
    // initialize game state (load all shaders/textures/levels)
    virtual void Init() = 0;
    // game loop
    virtual void ProcessInput(float dt) = 0;
    virtual void Update(float dt) = 0;
};