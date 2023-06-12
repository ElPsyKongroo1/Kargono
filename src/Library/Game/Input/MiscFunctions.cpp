#include "Input.h"
#include "../../Rendering/Library.h"

/*============================================================================================================================================================================================
 * Input Functionality
 *============================================================================================================================================================================================*/


bool updateType = false;
GLInput* typeChange[2];
GLInputLink* oldButton = nullptr;

void updateInputType()
{
    Resources::currentRenderer->currentInput = typeChange[1];
    if (oldButton != nullptr)
    {
        for (int i = 0; i < Resources::currentRenderer->currentInput->gamePadClickSize[0]; i++)
        {
            if (oldButton->glfwValue == Resources::currentRenderer->currentInput->gamePadClick[0][i].glfwValue)
            {
                Resources::currentRenderer->currentInput->gamePadClick[0][i].previousState = oldButton->previousState;
                break;
            }
        }
    }
}
