#pragma once
#include "Includes.h"
// Initialization
void FInitializeRenderer();
void FInitializeLibraryResources();

void FPreRendering();
void FPostRendering();
void FWindowRendering();

// Termination
void FTerminateLibraryResources();
void FTerminateRenderer();

// Input
void FProcessInputHold(GLFWwindow* window);