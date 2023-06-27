#pragma once
#include "../../../Includes.h"
class GameLevel
{
public:
    // level state
    int levelWidth;
    int levelHeight;
    
    std::vector<int> initMap;
    // constructor
    GameLevel(int width, int height) : levelWidth{ width }, levelHeight{ height } {}
    ~GameLevel() 
    {
        levelWidth = -1;
        levelHeight = -1;
        initMap.clear();
    }
    // loads level from file
    virtual void Load(const char* file) = 0;
    // check if the level is completed (all non-solid tiles are destroyed)
private:
    // initialize level from tile data
    virtual void init(int cameraViewWidth, int cameraViewHeight) = 0;
};