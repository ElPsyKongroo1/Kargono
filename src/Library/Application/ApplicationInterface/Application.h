#pragma once
#include "../../Includes.h"
#include "../../Library.h"

class Application
{
protected:
    Application() : renderer{ nullptr }, input{ nullptr }, Width{ 1 }, Height{1} {}
public:
    RendererState* renderer;
    GLInput* input;
    unsigned int Width, Height;
    // constructor/destructor
    ~Application() {}
    // initialize game state (load all shaders/textures/levels)
    virtual void Init() = 0;
    // game loop
    virtual void ProcessInput(float dt) = 0;
    virtual void Update(float dt) = 0;
};