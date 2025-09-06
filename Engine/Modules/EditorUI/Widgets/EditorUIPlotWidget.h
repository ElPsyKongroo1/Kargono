#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"

#include <functional>

namespace Kargono::EditorUI
{
	enum PlotFlags : WidgetFlags
	{
		Plot_None = 0,
		Plot_Indented = BIT(0)
	};

	struct PlotWidget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		PlotWidget() : Widget() {}
		~PlotWidget() = default;
	public:
		//==============================
		// Rendering
		//==============================
		void RenderPlot();
	public:
		//==============================
		// Configure Plot
		//==============================
		void SetBufferSize(size_t newSize);
		void SetYAxisLabel(std::string_view text);
	public:
		//==============================
		// Modify Plot Values
		//==============================
		void AddValue(float yValue);
		void UpdateValue(float yValue, size_t offset = 0);
		void Clear();
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label;
		WidgetFlags m_Flags{ PlotFlags::Plot_None };
		float m_MaxYVal{ 50.0f };
		Ref<void> m_ProvidedData{ nullptr };
	private:
		//==============================
		// Internal Fields
		//==============================
		FixedString16 m_YAxisLabel{ "##" };
		std::vector<float> m_XValues;
		std::vector<float> m_YValues;
		size_t m_BufferSize{ 0 };
		size_t m_Offset{ 0 };
	};
}

