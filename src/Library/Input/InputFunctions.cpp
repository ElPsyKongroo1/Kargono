#include "Input.h"
#include "../Library.h"

/*============================================================================================================================================================================================
 * Input Functionality
 *============================================================================================================================================================================================*/


bool updateType = false;
GLInput* typeChange[2];
GLButton* oldButton = nullptr;

void updateInputType()
{
    Resources::currentApplication->currentInput = typeChange[1];
    if (oldButton != nullptr)
    {
        for (int i = 0; i < Resources::currentApplication->currentInput->gamePadClickSize[0]; i++)
        {
            if (oldButton->glfwValue == Resources::currentApplication->currentInput->gamePadClick[0][i].glfwValue)
            {
                Resources::currentApplication->currentInput->gamePadClick[0][i].previousState = oldButton->previousState;
                break;
            }
        }
    }
}
