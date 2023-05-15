#include "../Library/Includes.h"
#include "../Library/Library.h"
#include "Rendering1.h"

/*============================================================================================================================================================================================
 * Rendering Functions
 *============================================================================================================================================================================================*/


void FRendering1()
{
    // Render Objects
    for (Object object : Resources::currentApplication->allSimpleObjects)
    {
        object.model->Draw(&object);
    }
}

