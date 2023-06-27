#pragma once
#include "../../Includes.h"
#include "../ApplicationInterface/Application.h"
#include "../../../Applications/Breakout/Classes/BreakoutResourceManager/BreakoutResourceManager.h"
#include "../../../Applications/Breakout/Classes/BreakoutObject/BreakoutObject.h"
#include "../../../Applications/Breakout/Classes/BreakoutLevel/BreakoutLevel.h"

class GameApplication : public Application
{
public:
    enum GameState {
        GAME_ACTIVE,
        GAME_MENU,
        GAME_WIN,
        NONE
    };

public:
    // game state
    GameState State;
    BreakoutResourceManager* resourceManager = nullptr;
    // constructor/destructor
    GameApplication() {}
    GameApplication(unsigned int width, unsigned int height, GameState state) : Application(width, height), 
                                                               State{state} {}
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