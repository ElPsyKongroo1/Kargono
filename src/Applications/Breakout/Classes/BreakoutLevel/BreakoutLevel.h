#pragma once
#include "../../../Library/Application/GameApplication/GameLevel/GameLevel.h"
#include "../BreakoutObject/BreakoutObject.h"
#include "../../../Library/Application/Particles/ParticleGenerator.h"

class BreakoutLevel : public GameLevel
{
public:
    // level state
    std::vector<GameBrick*> currentMapBricks;
    std::vector<ParticleGenerator*> currentParticleGenerators;

    GameObject* paddle = nullptr;
    GameBall* ball = nullptr;

    // constructor
    BreakoutLevel(int width, int height) : GameLevel{width, height}, currentParticleGenerators{ std::vector<ParticleGenerator*>() } { }
    ~BreakoutLevel()
    {
        std::for_each(currentMapBricks.begin(), currentMapBricks.end(), [](GameBrick* brick)
            {
                delete brick;
            });
        currentMapBricks.clear();
    }
    // loads level from file
    void Load(const char* file) override;
    // check if the level is completed (all non-solid tiles are destroyed)
    void RemoveBrick(GameBrick* brick);
private:
    // initialize level from tile data
    void init(int cameraViewWidth, int cameraViewHeight) override;
};