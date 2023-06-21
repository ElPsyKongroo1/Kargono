#pragma once
#include "../../../Library/Includes.h"
#include "GameObject.h"
class GameLevel
{
public:
    // level state
    std::vector<GameBrick*> currentMapBricks;
    int levelWidth;
    int levelHeight;
    
    std::vector<int> initMap;
    // constructor
    GameLevel(int width, int height) : levelWidth{ width }, levelHeight{height} { }
    ~GameLevel() 
    {
        levelWidth = -1;
        levelHeight = -1;
        initMap.clear();
        std::for_each(currentMapBricks.begin(), currentMapBricks.end(), [](GameBrick* brick) 
            {
                delete brick;
            });
        currentMapBricks.clear();
    }
    // loads level from file
    void Load(const char* file);
    // check if the level is completed (all non-solid tiles are destroyed)
    void RemoveBrick(GameBrick* brick);
    bool IsCompleted();
private:
    // initialize level from tile data
    void init(int cameraViewWidth, int cameraViewHeight);
};