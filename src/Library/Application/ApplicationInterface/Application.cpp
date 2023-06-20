#include "Application.h"

Application::~Application() 
{
    renderer = nullptr;
    currentInput = nullptr;
    recentInput = nullptr;
    width = 0;
    height = 0;
}