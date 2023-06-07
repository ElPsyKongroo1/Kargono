#include "../Library/Includes.h"
#include "../Library/Library.h"
#include "Rendering2.h"

/*============================================================================================================================================================================================
 * Rendering Functions
 *============================================================================================================================================================================================*/


void FRendering()
{
    // Render Objects
    for (Object object : Resources::currentApplication->allSimpleObjects)
    {
        static_cast<ModelRenderer*>(object.renderer)->model->Draw(&object);
    }
}