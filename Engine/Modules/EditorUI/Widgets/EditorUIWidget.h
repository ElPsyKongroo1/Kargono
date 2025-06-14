#pragma once

#include "Kargono/Core/FixedString.h"

#include "cstdint"
#include "limits"

namespace Kargono::EditorUI
{
	using WidgetID = uint32_t;
	constexpr inline WidgetID k_InvalidWidgetID{ std::numeric_limits<WidgetID>::max() };
	using WidgetFlags = uint8_t;

	constexpr WidgetID k_ChildWidgetMax{ 0x400'000 /*2^24*/ };

	class WidgetIDCounter
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		WidgetIDCounter() = default;
		~WidgetIDCounter() = default;
	public:
		//==============================
		// Get New ID
		//==============================
		WidgetID GetID()
		{
			m_Counter++;
			return m_Counter * k_ChildWidgetMax;
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		WidgetID m_Counter{ 1 };
	};

	class WidgetCounterService // TODO: EWWWWWWWWWW REMOVE THIS
	{
	public:
		static WidgetIDCounter& GetActiveContext()
		{
			return s_Counter;
		}

	private:
		static inline WidgetIDCounter s_Counter{};
	};

	class Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Widget() : m_WidgetID(WidgetCounterService::GetActiveContext().GetID())  
		{
			// Set up string version of widget ID
			// TODO: Remove this once all widgets can be identified w/ integer ID
			m_WidgetIDString = "##";
			m_WidgetIDString.AppendInteger(m_WidgetID);
		}
		virtual ~Widget() = default;

	protected:
		//==============================
		// Manage Child ID's
		//==============================
		WidgetID GetNextChildID()
		{
			return ++m_ChildID;
		}

		void ResetChildID()
		{
			m_ChildID = m_WidgetID;
		}

	protected:
		//==============================
		// Internal Fields
		//==============================
		WidgetID m_WidgetID{};
		WidgetID m_ChildID{};
		FixedString16 m_WidgetIDString{};
	};
}