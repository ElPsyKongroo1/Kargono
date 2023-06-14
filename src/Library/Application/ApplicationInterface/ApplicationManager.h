#pragma once
#include "../DefaultApplication/DefaultApplication.h"
#include "../GameApplication/Game.h"

class ApplicationManager
{
public:
    ApplicationManager() {};
    
public:
    DefaultApplication* default3DApplication{nullptr};
    DefaultApplication* default2DApplication{nullptr};
    GameApplication* breakout{nullptr};
public:
    void CreateApplications();
    void DestroyApplications();
private:
    void CreateDefault3DApplication();
    void CreateDefault2DApplication();
    void CreateBreakout();

};