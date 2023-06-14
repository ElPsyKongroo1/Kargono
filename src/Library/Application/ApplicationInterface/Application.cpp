#include "Application.h"

Application::~Application() 
{
    renderer = nullptr;
    input = nullptr;
    width = 0;
    height = 0;
}