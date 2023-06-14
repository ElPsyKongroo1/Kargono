#pragma once
#include "Application.h"
#include "../DefaultApplication/DefaultApplication.h"
#include "../GameApplication/Game.h"

class ApplicationManager
{
public:
    ApplicationManager() {};
public:
    DefaultApplication* default3DInput;
    DefaultApplication* debugMenuInput;
    GameApplication* default2DInput;
public:
    void CreateApplications();
    void DestroyApplications();
private:


};