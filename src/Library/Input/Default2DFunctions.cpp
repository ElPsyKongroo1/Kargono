#include "Input.h"
#include "../Library.h"
#include "InputFunctions.h"

namespace Default2DFunctions {

    void CAMERA_FOV_MOUSE(double xoffset, double yoffset)
    {
        Resources::currentApplication->currentCamera->fov -= (float)yoffset;
        if (Resources::currentApplication->currentCamera->fov < 1.0f)
            Resources::currentApplication->currentCamera->fov = 1.0f;
        if (Resources::currentApplication->currentCamera->fov > 45.0f)
            Resources::currentApplication->currentCamera->fov = 45.0f;
    }
}