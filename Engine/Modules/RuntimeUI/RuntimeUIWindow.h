#pragma once

#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"

#include "Kargono/Core/Window.h"

namespace Kargono::RuntimeUI
{
	class UserInterface;

	class Window
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		Window()
		{
			KG_ERROR("Should not be default constructing windows"); // TODO: Maybe use an Init() function for default creation/injection
		}
		Window(UserInterface* parentUI) : i_ParentUI(parentUI)
		{
			KG_ASSERT(parentUI);

			m_ID = Utility::STLRandomService::GetActiveRandom().GenerateRandomInteger((int32_t)0, k_InvalidWindowID - 1);
		}

	public:
		//============================
		// Interact w/ Window
		//============================
		void DisplayWindow();
		void HideWindow();

	public:
		//============================
		// Modify Window
		//============================
		void AddWidget(Ref<Widget> newWidget);
		void DeleteWidget(std::size_t widgetLocation);

	public:
		//============================
		// Query Window State
		//============================
		bool GetWindowDisplayed();
		std::vector<Ref<Widget>> GetAllChildWidgets();

	private:
		void GetChildWidget(std::vector<Ref<Widget>>& returnVector, Ref<Widget> currentWidget);

	public:
		//============================
		// Supporting Methods
		//============================
		Math::vec3 GetSize(ViewportData viewportData);
		Math::vec3 GetLowerCornerPosition(ViewportData viewportData);
		Math::vec3 GetCenterPosition(Math::vec3 lowerCorner, Math::vec3 size);
		Math::vec3 GetRelativeViewportPosition(Math::vec2 worldPosition, ViewportData viewportData);
	public:
		//============================
		// Public Fields
		//============================
		std::string m_Tag{ "None" };
		WindowID m_ID{ k_InvalidWindowID };
		Math::vec3 m_ScreenPosition{};
		Math::vec2 m_Size{ 1.0f, 1.0f };
		Math::vec4 m_BackgroundColor{ 0.3f };
		WidgetID m_DefaultActiveWidget{ k_InvalidWidgetID };
		Ref<Widget> m_DefaultActiveWidgetRef{ nullptr };
		std::vector<Ref<Widget>> m_Widgets{};

	private:
		//============================
		// Internal Fields
		//============================
		bool m_WindowDisplayed{ false };
	private:
		//============================
		// Injected Dependencies
		//============================
		UserInterface* i_ParentUI{ nullptr };
	};
}