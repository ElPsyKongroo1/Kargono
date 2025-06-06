#include "kgpch.h"

#include "Modules/RuntimeUI/Widgets/RuntimeUIWidget.h"

namespace Kargono::RuntimeUI
{
	Math::vec3 Widget::CalculateWidgetSize(const Math::vec3& windowSize)
	{
		return Math::vec3
		(
			m_SizeType == PixelOrPercent::Percent ? windowSize.x * m_PercentSize.x : m_PixelSize.x,
			m_SizeType == PixelOrPercent::Percent ? windowSize.y * m_PercentSize.y : m_PixelSize.y,
			1.0f
		);
	}

	Math::vec3 Widget::CalculateWorldPosition(const Math::vec3& windowTranslation, const Math::vec3& windowSize)
	{
		float widgetXPos{ m_XPositionType == PixelOrPercent::Percent ? windowSize.x * m_PercentPosition.x : (float)m_PixelPosition.x };
		float widgetYPos{ m_YPositionType == PixelOrPercent::Percent ? windowSize.y * m_PercentPosition.y : (float)m_PixelPosition.y };
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);
		if (m_XRelativeOrAbsolute == RelativeOrAbsolute::Relative && m_XConstraint != Constraint::None)
		{
			// Handle relative code
			switch (m_XConstraint)
			{
			case Constraint::Center:
				widgetXPos = (windowSize.x * 0.5f) + widgetXPos - (widgetSize.x / 2.0f);
				break;
			case Constraint::Right:
				widgetXPos = windowSize.x + widgetXPos - (widgetSize.x);
				break;
			case Constraint::Left:
				break;
			default:
				KG_ERROR("Invalid constraint {} provided while calculating widget's position", (uint16_t)m_XConstraint);
				break;
			}
		}

		if (m_YRelativeOrAbsolute == RelativeOrAbsolute::Relative && m_YConstraint != Constraint::None)
		{
			// Handle relative code
			switch (m_YConstraint)
			{
			case Constraint::Center:
				widgetYPos = (windowSize.y * 0.5f) + widgetYPos - (widgetSize.y / 2.0f);;
				break;
			case Constraint::Top:
				widgetYPos = windowSize.y + widgetYPos - widgetSize.y;
				break;
			case Constraint::Bottom:
				break;
			default:
				KG_ERROR("Invalid constraint {} provided while calculating widget's position", (uint16_t)m_YConstraint);
				break;
			}
		}


		// Calculate final widget position on screen
		return Math::vec3(windowTranslation.x + widgetXPos, windowTranslation.y + widgetYPos, windowTranslation.z);
	}

	Math::vec3 Widget::CalculateWindowPosition(Math::vec2 worldPosition, const Math::vec3& windowTranslation, const Math::vec3& windowSize)
	{
		float widgetXPos{ worldPosition.x - windowTranslation.x };
		float widgetYPos{ worldPosition.y - windowTranslation.y };
		Math::vec3 widgetSize = CalculateWidgetSize(windowSize);

		if (m_XRelativeOrAbsolute == RelativeOrAbsolute::Relative && m_XConstraint != Constraint::None)
		{
			// Handle relative code
			switch (m_XConstraint)
			{
			case Constraint::Center:
				widgetXPos = widgetXPos - (windowSize.x * 0.5f) + (widgetSize.x / 2.0f);
				break;
			case Constraint::Right:
				widgetXPos = widgetXPos - windowSize.x + (widgetSize.x);
				break;
			case Constraint::Left:
				break;
			default:
				KG_ERROR("Invalid constraint {} provided while calculating widget's position", (uint16_t)m_XConstraint);
				break;
			}
		}

		if (m_YRelativeOrAbsolute == RelativeOrAbsolute::Relative && m_YConstraint != Constraint::None)
		{
			// Handle relative code
			switch (m_YConstraint)
			{
			case Constraint::Center:
				widgetYPos = widgetYPos - (windowSize.y * 0.5f) + (widgetSize.y / 2.0f);
				break;
			case Constraint::Top:
				widgetYPos = widgetYPos - windowSize.y + (widgetSize.y);
				break;
			case Constraint::Bottom:
				break;
			default:
				KG_ERROR("Invalid constraint {} provided while calculating widget's position", (uint16_t)m_YConstraint);
				break;
			}
		}
		// Calculate final widget position on screen
		widgetXPos = m_XPositionType == PixelOrPercent::Percent ? widgetXPos / windowSize.x : widgetXPos;
		widgetYPos = m_YPositionType == PixelOrPercent::Percent ? widgetYPos / windowSize.y : widgetYPos;
		return Math::vec3(widgetXPos, widgetYPos, 0.0f);
	}
}