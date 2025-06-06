#include "kgpch.h"

#include "Modules/RuntimeUI/RuntimeUIWindow.h"

#include "Modules/RuntimeUI/RuntimeUI.h"

namespace Kargono::RuntimeUI
{
	void Window::DisplayWindow()
	{
		// Return if the window is already displayed
		if (m_WindowDisplayed)
		{
			return;
		}

		// Set window as displayed and revalidate displayed windows for current user interface
		m_WindowDisplayed = true;
		RuntimeUIService::GetActiveContext().RevalidateDisplayedWindows();

	}

	void Window::HideWindow()
	{
		// Return if the window is not displayed
		if (!m_WindowDisplayed)
		{
			return;
		}

		// Set window as hidden and revalidate displayed windows for current user interface
		m_WindowDisplayed = false;
		RuntimeUIService::GetActiveContext().RevalidateDisplayedWindows();
	}

	std::vector<Ref<Widget>> RuntimeUI::Window::GetAllChildWidgets()
	{
		std::vector<Ref<Widget>> returnVector{ m_Widgets };

		for (Ref<Widget> currentWidget : m_Widgets)
		{
			GetChildWidget(returnVector, currentWidget);
		}

		return returnVector;
	}

	void RuntimeUI::Window::GetChildWidget(std::vector<Ref<Widget>>& returnVector, Ref<Widget> currentWidget)
	{
		returnVector.push_back(currentWidget);

		ContainerData* data = RuntimeUIService::GetActiveContext().GetContainerDataFromWidget(currentWidget.get());
		if (data)
		{
			for (Ref<Widget> containedWidget : data->m_ContainedWidgets)
			{
				GetChildWidget(returnVector, containedWidget);
			}
		}
	}

	bool Window::GetWindowDisplayed()
	{
		return m_WindowDisplayed;
	}

	Math::vec3 Window::CalculateSize(uint32_t viewportWidth, uint32_t viewportHeight)
	{
		return Math::vec3(viewportWidth * m_Size.x, viewportHeight * m_Size.y, 1.0f);
	}

	Math::vec3 Window::CalculateWorldPosition(uint32_t viewportWidth, uint32_t viewportHeight)
	{
		return Math::vec3((viewportWidth * m_ScreenPosition.x), (viewportHeight * m_ScreenPosition.y), m_ScreenPosition.z);
	}

	Math::vec3 Window::CalculateScreenPosition(Math::vec2 worldPosition, uint32_t viewportWidth, uint32_t viewportHeight)
	{
		return Math::vec3(worldPosition.x / viewportWidth, worldPosition.y / viewportHeight, m_ScreenPosition.z);
	}

	void Window::AddWidget(Ref<Widget> newWidget)
	{
		KG_ASSERT(newWidget);

		// Add new widget to buffer
		m_Widgets.push_back(newWidget);

		RuntimeUI::RuntimeUIService::GetActiveContext().RevalidateWidgetIDToLocationMap();
		RuntimeUIService::GetActiveContext().RecalculateTextData(newWidget.get());
	}

	void Window::DeleteWidget(std::size_t widgetLocation)
	{
		// Return if the widget location is out of bounds
		if (widgetLocation >= m_Widgets.size())
		{
			KG_WARN("Attempt to delete a widget, however, the provided index is out of bounds");
			return;
		}

		// Delete the widget
		m_Widgets.erase(m_Widgets.begin() + widgetLocation);

		RuntimeUI::RuntimeUIService::GetActiveContext().RevalidateWidgetIDToLocationMap();
	}
}