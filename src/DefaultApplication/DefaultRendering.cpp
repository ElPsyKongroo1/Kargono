#include "../Library/Includes.h"
#include "../Library/Library.h"
#include "Rendering2.h"

/*============================================================================================================================================================================================
 * Rendering Functions
 *============================================================================================================================================================================================*/


void FRendering()
{
    // Render Objects
    for (ModelRenderer renderable : Resources::currentApplication->allSimpleObjects)
    {
        renderable.model->Draw(&renderable);
    }
}