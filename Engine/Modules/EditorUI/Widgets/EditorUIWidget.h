#pragma once

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
			return m_Counter * k_ChildWidgetMax; // 2^22
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
		Widget() : m_WidgetID(WidgetCounterService::GetActiveContext().GetID())  {}
	protected:
		WidgetID m_WidgetID{};
	};
}