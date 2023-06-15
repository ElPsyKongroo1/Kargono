#include "Application.h"

Application::~Application() 
{
    renderer = nullptr;
    currentInput = nullptr;
    defaultInput = nullptr;
    width = 0;
    height = 0;
}