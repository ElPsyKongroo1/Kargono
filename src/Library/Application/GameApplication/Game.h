#pragma once
#include "../../Includes.h"
#include "../ApplicationInterface/Application.h"
#include "../../../Applications/Breakout/Classes/ResourceManager.h"
#include "../../../Applications/Breakout/Classes/GameObject.h"
#include "../../../Applications/Breakout/Classes/GameLevel.h"

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
    ResourceManager* resourceManager = nullptr;
    GameObject* paddle = nullptr;
    GameBall* ball = nullptr;
    GameLevel* currentLevel = nullptr;
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