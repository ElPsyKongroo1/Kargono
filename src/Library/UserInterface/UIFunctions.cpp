#include "../Includes.h"
#include "../Library.h"

void renderMainMenu()
{

	bool isDemoOpen = false;
	ImGui::Begin("Main Menu");
	if (ImGui::Button("Demo Button"))
	{
		UIWindow* window = &Resources::windowManager.demoWindow;
		if (!window->isRendering)
		{
			window->isRendering = true;
			Resources::currentApplication->currentWindow = window;
			for (int i = 0; i < Resources::windowManager.mainMenu.children.size(); i++)
			{
				if (window != Resources::windowManager.mainMenu.children.at(i))
				{
					Resources::windowManager.mainMenu.children.at(i)->closeChildren();
				}
			}
		}
		else
		{
			window->isRendering = false;
			Resources::currentApplication->currentWindow = window->parent;
		}
	}
	if (ImGui::Button("Dev Tools"))
	{
		UIWindow* window = &Resources::windowManager.devTools;
		if (!window->isRendering)
		{
			window->isRendering = true;
			Resources::currentApplication->currentWindow = window;
			for (int i = 0; i < Resources::windowManager.mainMenu.children.size(); i++)
			{
				if (window != Resources::windowManager.mainMenu.children.at(i))
				{
					Resources::windowManager.mainMenu.children.at(i)->closeChildren();
				}
			}
		}
		else
		{
			window->isRendering = false;
			Resources::currentApplication->currentWindow = window->parent;
		}
	}
	if (ImGui::Button("Exit Application"))
	{
		Resources::inputManager.accessClickFunction(EXIT_APPLICATION, nullptr);
	}
	ImGui::End();
}

void renderDemo()
{
	ImGui::ShowDemoWindow();
}

void renderDevTool()
{
	ImGui::Begin("Dev Tools");
	if (ImGui::CollapsingHeader("Application Resources"))
	{
		if (ImGui::CollapsingHeader("Objects"))
		{
			for (int i = 0; i < Resources::currentApplication->allSimpleObjects.size(); i++)
			{
				ModelRenderer* object = &Resources::currentApplication->allSimpleObjects.at(i);
				if (ImGui::TreeNode((void*)(intptr_t)i, "Object %d", i + 1))
				{
					ImGui::Text("Translation: %fx, %fy, %fz", object->translation.x, object->translation.y, object->translation.z);
					ImGui::Text("Rotation: %fx, %fy, %fz, %fdeg", object->rotation.x, object->rotation.y, object->rotation.z, object->rotation.w);
					ImGui::Text("Scale: %fx, %fy, %fz", object->scale.x, object->scale.y, object->scale.z);
					ImGui::Separator();
					ImGui::TreePop();
				}
			}
		}
	}
	ImGui::End();
}
