#include "kgpch.h"

#include "Modules/RuntimeUI/RuntimeUIWindow.h"

#include "Modules/RuntimeUI/RuntimeUIContext.h"

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
		i_ParentUI->m_WindowsState.RevalidateDisplayedWindows();

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
		i_ParentUI->m_WindowsState.RevalidateDisplayedWindows();
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

		ContainerData* data = currentWidget->GetContainerData();
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

	Math::vec3 Window::GetSize(ViewportData viewportData)
	{
		return Math::vec3
		(
			viewportData.m_Width * m_Size.x, 
			viewportData.m_Height * m_Size.y, 
			1.0f
		);
	}

	Math::vec3 Window::GetLowerCornerPosition(ViewportData viewportData)
	{
		return Math::vec3
		(
			viewportData.m_Width * m_ScreenPosition.x, 
			viewportData.m_Height * m_ScreenPosition.y, 
			m_ScreenPosition.z
		);
	}

	Math::vec3 Window::GetCenterPosition(Math::vec3 lowerCorner, Math::vec3 size)
	{
		return Math::vec3
		(
			lowerCorner.x + (size.x / 2),
			lowerCorner.y + (size.y / 2),
			lowerCorner.z
		);
	}

	Math::vec3 Window::GetRelativeViewportPosition(Math::vec2 worldPosition, ViewportData viewportData)
	{
		return Math::vec3
		(
			worldPosition.x / viewportData.m_Width, 
			worldPosition.y / viewportData.m_Height, 
			m_ScreenPosition.z
		);
	}

	void Window::AddWidget(Ref<Widget> newWidget)
	{
		KG_ASSERT(newWidget);

		// Add new widget to buffer
		m_Widgets.push_back(newWidget);

		i_ParentUI->m_WindowsState.RevalidateIDToLocationMap();
		newWidget->RevalidateTextDimensions();
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

		i_ParentUI->m_WindowsState.RevalidateIDToLocationMap();
	}
}