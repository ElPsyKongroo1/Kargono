#include "../../Library/Rendering/Includes.h"
#include "../../Library/Rendering/Library.h"
#include "Rendering1.h"

/*============================================================================================================================================================================================
 * Rendering Functions
 *============================================================================================================================================================================================*/


void FRendering1()
{
    // Render Objects
    for (Object object : Resources::currentRenderer->objectRenderBuffer)
    {
        object.renderer->render();
    }
}

