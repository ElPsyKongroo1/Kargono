#pragma once
#include "../../Rendering/Includes.h"
#include "../../Rendering/Library.h"
#include "../ApplicationInterface/Application.h"

class Game : public Application
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
    Game(unsigned int width, unsigned int height) {}
    ~Game() {}
    // initialize game state (load all shaders/textures/levels)
    void Init() {}
    // game loop
    void ProcessInput(float dt) override {}
    void Update(float dt) override {}
};