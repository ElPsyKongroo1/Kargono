#include "../../Library/Rendering/Includes.h"
#include "../../Library/Rendering/Library.h"
#include "Rendering2.h"

/*============================================================================================================================================================================================
 * Rendering Functions
 *============================================================================================================================================================================================*/


void FRendering()
{
    // Render Objects
    for (Object object : Resources::currentRenderer->objectRenderBuffer)
    {
        object.renderer->render();
    }
}