#include "../Library/Includes.h"
#include "../Library/Library.h"
#include "Rendering1.h"

/*============================================================================================================================================================================================
 * Rendering Functions
 *============================================================================================================================================================================================*/


void FRendering1()
{
    // Render Objects
    for (Object object : Resources::currentApplication->objectRenderBuffer)
    {
        static_cast<ModelRenderer*>(object.renderer)->model->Draw(&object);
    }
}

