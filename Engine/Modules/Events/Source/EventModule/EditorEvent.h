#pragma once

#include "EventModule/Event.h"
#include "AssetModule/Asset.h"
#include "Kargono/Math/Math.h"

#include <sstream>
#include <variant>

namespace Kargono::Editors { class Editor; }

namespace Kargono::Events
{
	enum class ManageEditorAction : uint16_t
	{
		None = 0,
		AddDebugLine,
		AddDebugPoint,
		ClearDebugLines,
		ClearDebugPoints
	};

	struct DebugLineData
	{
		Math::vec3 m_StartPoint;
		Math::vec3 m_EndPoint;
	};

	struct DebugPointData
	{
		Math::vec3 m_Point;
	};

	//============================================================
	// Manage Editor
	//============================================================
	class ManageEditor : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		ManageEditor(ManageEditorAction action)
			: m_Action(action) 
		{
			KG_ASSERT(action != ManageEditorAction::AddDebugLine && action != ManageEditorAction::AddDebugPoint);
		}
		ManageEditor(DebugLineData lineData)
			: m_Action(ManageEditorAction::AddDebugLine), m_ProvidedData(lineData) {}
		ManageEditor(DebugPointData pointData)
			: m_Action(ManageEditorAction::AddDebugPoint), m_ProvidedData(pointData) {}

		//==============================
		// Getters/Setters
		//==============================
		ManageEditorAction GetAction() const 
		{
			return m_Action;
		}
		virtual EventType GetEventType() const override 
		{ 
			return EventType::ManageEditor; 
		}
		virtual int GetCategoryFlags() const override 
		{ 
			return EventCategory::Editor; 
		}

		DebugPointData GetDebugPointData()
		{
			DebugPointData* pointData = std::get_if<DebugPointData>(&m_ProvidedData);
			KG_ASSERT(pointData);
			return *pointData;
		}
		DebugLineData GetDebugLineData()
		{
			DebugLineData* lineData = std::get_if<DebugLineData>(&m_ProvidedData);
			KG_ASSERT(lineData);
			return *lineData;
		}
	private:
		std::variant<DebugPointData, DebugLineData> m_ProvidedData;
		ManageEditorAction m_Action;
	};
}
