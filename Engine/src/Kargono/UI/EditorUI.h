#pragma once

#include "Kargono/Core/Base.h"

#include "imgui.h"

namespace Kargono { class Texture2D; }

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

	class EditorEngine
	{
	public:
		static void Init();
		static Ref<Texture2D> s_IconPlay, s_IconPause, s_IconStop,
			s_IconStep, s_IconSimulate, s_IconAddItem,
			s_IconSettings, s_IconDelete;

		static Ref<Texture2D> s_DirectoryIcon, s_GenericFileIcon, s_BackIcon,
			s_AudioIcon, s_ImageIcon, s_BinaryIcon,
			s_SceneIcon, s_RegistryIcon, s_ScriptProjectIcon,
			s_UserInterfaceIcon, s_FontIcon, s_InputIcon;
	};
}
