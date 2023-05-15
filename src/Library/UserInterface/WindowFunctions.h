#pragma once

/*============================================================================================================================================================================================
 * UIWindow Class
 *============================================================================================================================================================================================*/

class UIWindow
{
public:
	enum WindowType
	{
		MAIN_MENU, DEMO_WINDOW, DEV_TOOLS
	};
public:
	WindowType type;
	std::vector<UIWindow*> children;
	UIWindow* parent;
	bool isRendering;
public:
	void renderWindow();
	void closeChildren();

};

/*============================================================================================================================================================================================
 * WindowManager Class
 *============================================================================================================================================================================================*/


class WindowManager
{
public:

	UIWindow mainMenu;
	UIWindow demoWindow;
	UIWindow devTools;
	UIWindow* allWindows[3];
	int allWindowsSize = 3;
public:
	void CreateUIWindows();
	void DestroyUIWindows();
private:
	void CreateMainMenu(UIWindow& window);
	void CreateDemoWindow(UIWindow& window);
	void CreateDevTools(UIWindow& window);
	void DestroyUIWindow(UIWindow& window);
};