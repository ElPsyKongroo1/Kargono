#pragma once

#include "imgui.h"

namespace Kargono::UI
{
	struct ScopedStyleColor
	{
		ScopedStyleColor() = default;

		ScopedStyleColor(ImGuiCol idx, ImVec4 color, bool predicate = true)
			:m_Set(predicate)
		{
			if (predicate) { ImGui::PushStyleColor(idx, color); }
		}

		ScopedStyleColor(ImGuiCol idx, ImU32 color, bool predicate = true)
			:m_Set(predicate)
		{
			if (predicate) { ImGui::PushStyleColor(idx, color); }
		}

		~ScopedStyleColor()
		{
			if (m_Set) { ImGui::PopStyleColor(); }
			
		}
	private:
		bool m_Set = false;
	};
}
