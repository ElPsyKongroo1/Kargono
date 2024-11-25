#pragma once
#include "ImGuiNotify.hpp"
#include "IconsFontAwesome6.h"
#include "fa-solid-900.h"

namespace Kargono::EditorUI
{
	inline void RenderImGuiNotify()
	{
		// Notifications style setup
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f); // Disable round borders
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f); // Disable borders

		// Notifications color setup
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f)); // Background color

		// Main rendering function
		ImGui::RenderNotifications();

		//——————————————————————————————— WARNING ———————————————————————————————
		// Argument MUST match the amount of ImGui::PushStyleVar() calls 
		ImGui::PopStyleVar(2);
		// Argument MUST match the amount of ImGui::PushStyleColor() calls 
		ImGui::PopStyleColor(1);
	}
}


