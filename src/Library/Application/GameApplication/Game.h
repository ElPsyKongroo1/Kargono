#pragma once
#include "../../Includes.h"
#include "../ApplicationInterface/Application.h"
#include "../../../Applications/Breakout/Classes/ResourceManager.h"
#include "../../../Applications/Breakout/Classes/GameObject.h"

class GameApplication : public Application
{
    enum GameState {
        GAME_ACTIVE,
        GAME_MENU,
        GAME_WIN,
        NONE
    };

public:
    // game state
    GameState State;
    ResourceManager* resourceManager = nullptr;
    GameObject* focusedObject = nullptr;
    // constructor/destructor
    GameApplication() {}
    GameApplication(unsigned int width, unsigned int height) : Application(width, height), 
                                                               State{GameApplication::NONE} {}
    ~GameApplication() 
    {
        State = GameApplication::NONE;
    }
    // initialize game state (load all shaders/textures/levels)
    void Init() {}
    // game loop
    void ProcessInput(float dt) override {}
    void Update(float dt) override {}
};