#pragma once

#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"

namespace Kargono::RuntimeUI
{
	class Window
	{
	public:
		//============================
		// Constructors/Destructors
		//============================
		Window()
		{

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
		Math::vec3 CalculateSize(uint32_t viewportWidth, uint32_t viewportHeight);
		Math::vec3 CalculateWorldPosition(uint32_t viewportWidth, uint32_t viewportHeight);
		Math::vec3 CalculateScreenPosition(Math::vec2 worldPosition, uint32_t viewportWidth, uint32_t viewportHeight);
	public:
		//============================
		// Public Fields
		//============================
		std::string m_Tag{ "None" };
		int32_t m_ID{ k_InvalidWindowID };
		Math::vec3 m_ScreenPosition{};
		Math::vec2 m_Size{ 1.0f, 1.0f };
		Math::vec4 m_BackgroundColor{ 0.3f };
		int32_t m_DefaultActiveWidget{ k_InvalidWidgetID };
		Ref<Widget> m_DefaultActiveWidgetRef{ nullptr };
		std::vector<Ref<Widget>> m_Widgets{};

	private:
		//============================
		// Internal Fields
		//============================
		bool m_WindowDisplayed{ false };
	};
}