#pragma once
#include "../../Includes.h"
#include "../ApplicationInterface/Application.h"

class DefaultApplication : public Application
{
public:
    DefaultApplication() {};
public:
    DefaultApplication(unsigned int width, unsigned int height) : Application(width, height) {}
public:
    // constructor/destructor
    ~DefaultApplication() {}
    // initialize game state (load all shaders/textures/levels)
    virtual void Init() override {};
    // game loop
    virtual void ProcessInput(float dt) override {};
    virtual void Update(float dt) override {};
};