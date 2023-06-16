#pragma once
#include "../../../Library/Includes.h"
#include "GameObject.h"
class GameLevel
{
public:
    // level state
    std::vector<GameObject*> currentMap;
    int levelWidth;
    int levelHeight;
    
    std::vector<int> initMap;
    // constructor
    GameLevel(int width, int height) : levelWidth{ width }, levelHeight{height} { }
    // loads level from file
    void Load(const char* file);
    // render level
    void Draw();
    // check if the level is completed (all non-solid tiles are destroyed)
    bool IsCompleted();
private:
    // initialize level from tile data
    void init(int cameraViewWidth, int cameraViewHeight);
};