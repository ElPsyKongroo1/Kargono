#pragma once
#include "../../Includes.h"
#include "../../Library.h"
#include "../ApplicationInterface/Application.h"

class GameApplication : public Application
{
    enum GameState {
        GAME_ACTIVE,
        GAME_MENU,
        GAME_WIN
    };

public:
    // game state
    GameState    State;
    // constructor/destructor
    GameApplication(unsigned int width, unsigned int height) {}
    ~GameApplication() {}
    // initialize game state (load all shaders/textures/levels)
    void Init() {}
    // game loop
    void ProcessInput(float dt) override {}
    void Update(float dt) override {}
};