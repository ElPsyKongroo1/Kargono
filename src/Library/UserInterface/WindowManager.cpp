#include "../Includes.h"
#include "WindowFunctions.h"
#include "../Library.h"
#include "UIFunctions.h"

/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * WindowManager Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/

 /*============================================================================================================================================================================================
   * Instantiate/Delete UIWindow Objects
   *============================================================================================================================================================================================*/


void WindowManager::CreateUIWindows()
{
	CreateMainMenu(mainMenu);
	CreateDemoWindow(demoWindow);
	CreateDevTools(devTools);
	allWindows[0] = &mainMenu;
	allWindows[1] = &demoWindow;
	allWindows[2] = &devTools;
}
void WindowManager::DestroyUIWindows()
{

}

/*============================================================================================================================================================================================
  * Initialize specific UIWindow Objects
  *============================================================================================================================================================================================*/


void WindowManager::CreateMainMenu(UIWindow& window)
{
	window.type = UIWindow::MAIN_MENU;
	window.isRendering = false;
	window.parent = nullptr;
	window.children = std::vector<UIWindow*>();
	window.children.push_back(&Resources::windowManager.demoWindow);
	window.children.push_back(&Resources::windowManager.devTools);
}
void WindowManager::DestroyUIWindow(UIWindow& window)
{

}

void WindowManager::CreateDemoWindow(UIWindow& window)
{
	window.type = UIWindow::DEMO_WINDOW;
	window.isRendering = false;
	window.parent = &Resources::windowManager.mainMenu;
	window.children = std::vector<UIWindow*>();
}

void WindowManager::CreateDevTools(UIWindow& window)
{
	window.type = UIWindow::DEV_TOOLS;
	window.isRendering = false;
	window.parent = &Resources::windowManager.mainMenu;
	window.children = std::vector<UIWindow*>();
}