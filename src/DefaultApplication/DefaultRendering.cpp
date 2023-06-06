#include "../Library/Includes.h"
#include "../Library/Library.h"
#include "Rendering2.h"

/*============================================================================================================================================================================================
 * Rendering Functions
 *============================================================================================================================================================================================*/


void FRendering()
{
    // Render Objects
    for (ModelRenderer object : Resources::currentApplication->allSimpleObjects)
    {
        object.model->Draw(&object);
    }
}