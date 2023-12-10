#pragma once

#include "Kargono/Core/Base.h"

#include "imgui.h"

namespace Kargono
{
	class Texture2D;
	namespace UI
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
	
	class EditorUI
	{
	public:
		static void Init();
		//static void SlotSelectorTable(const std::string& tableIdentifier, uint32_t numberOfColumns, const std::vector<std::string>& namesOfColumns);
		static Ref<Texture2D> s_IconPlay, s_IconPause, s_IconStop, s_IconStep, s_IconSimulate, s_IconAddItem, s_IconSettings, s_IconDelete;
	};

	

}
