#include "ApplicationManager.h"



void ApplicationManager::CreateApplications() 
{
	CreateDefault3DApplication();
	CreateDefault2DApplication();
	CreateBreakout();
}
void ApplicationManager::DestroyApplications()
{
	delete default2DApplication;
	delete default3DApplication;
	delete breakout;

	default2DApplication = nullptr;
	default3DApplication = nullptr;
	breakout = nullptr;
}

void ApplicationManager::CreateDefault3DApplication()
{
	default3DApplication = new DefaultApplication(1920.0f, 1080.0f);
}
void ApplicationManager::CreateDefault2DApplication()
{
	default2DApplication = new DefaultApplication(1920.0f, 1080.0f);
}
void ApplicationManager::CreateBreakout()
{
	breakout = new GameApplication(1920.0f, 1080.0f, GameApplication::GAME_MENU);
}