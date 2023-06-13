#pragma once
#include "../../Rendering/Includes.h"
#include "../../Rendering/Library.h"
#include "../ApplicationInterface/Application.h"

class DefaultApplication : public Application
{
protected:
    DefaultApplication(unsigned int width, unsigned int height) : renderer{ nullptr }, input{ nullptr }, Width{ 1 }, Height{ 1 } {}
public:
    RendererState* renderer;
    GLInput* input;
    unsigned int Width, Height;
    // constructor/destructor
    ~DefaultApplication() {}
    // initialize game state (load all shaders/textures/levels)
    virtual void Init() override {};
    // game loop
    virtual void ProcessInput(float dt) override {};
    virtual void Update(float dt) override {};
};