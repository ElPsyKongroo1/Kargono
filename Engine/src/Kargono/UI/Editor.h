#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Events/ApplicationEvent.h"

#include "imgui.h"
#include "ImGuizmo.h"

#include <functional>
#include <vector>
#include <unordered_map>


namespace Kargono { class Texture2D; }

namespace Kargono::UI
{
	
	struct ScopedStyleColor
	{
		ScopedStyleColor() = default;

		ScopedStyleColor(ImGuiCol idx, ImVec4 color, bool predicate = true)
			:m_Set(predicate)
		{
			if (predicate)
			{
				ImGui::PushStyleColor(idx, color);
			}
		}

		ScopedStyleColor(ImGuiCol idx, ImU32 color, bool predicate = true)
			:m_Set(predicate)
		{
			if (predicate)
			{
				ImGui::PushStyleColor(idx, color);
			}
		}

		~ScopedStyleColor()
		{
			if (m_Set)
			{
				ImGui::PopStyleColor();
			}

		}
	private:
		bool m_Set = false;
	};

	using OptionsList = std::unordered_map<std::string, std::vector<std::string>>;

	struct SelectOptionSpec
	{
	public:
		UUID WidgetID;
		std::string Label;
		std::string CurrentOption;
		uint32_t LineCount{ 3 };
		std::function<void(const std::string&)> ConfirmAction {nullptr};
		std::function<void(SelectOptionSpec&)> PopupAction {nullptr};
		void ClearOptionsList()
		{
			OptionsList.clear();
		}
		void AddToOptionsList(const std::string& title, const std::string& option)
		{
			if (!OptionsList.contains(title))
			{
				std::vector<std::string> newVector {};
				newVector.push_back(option);
				OptionsList.insert_or_assign(title, newVector);
				return;
			}

			OptionsList.at(title).push_back(option);
		}
		OptionsList& GetOptionsList()
		{
			return OptionsList;
		}
	private:
		OptionsList OptionsList {};
	};

	struct CheckboxSpec
	{
	public:
		UUID WidgetID;
		std::string Label;
		bool ToggleBoolean;
		std::function<void(bool)> ConfirmAction;
	};

	struct TextInputSpec
	{
	public:
		UUID WidgetID;
		std::string Label;
		std::string CurrentOption;
		std::function<void(const std::string&)> ConfirmAction;
	};

	enum class SpacingAmount
	{
		None = 0,
		Small,
		Medium,
		Large
	};

	class Editor
	{
	public:
		static void Init();
		static void Terminate();

		static void Begin();
		static void End();

		static void OnEvent(Events::Event& e);

		static void Spacing(float space);
		static void Spacing(SpacingAmount space);

		static void SelectOption(SelectOptionSpec& spec);
		static void Checkbox(CheckboxSpec& spec);
		static void Text(const std::string& Label, const std::string& Text);
		static void TextInputModal(TextInputSpec& spec);

		static uint32_t GetActiveWidgetID();

		static void BlockEvents(bool block)
		{
			s_BlockEvents = block;
		}

		static ImFont* s_AntaRegular;
		static ImFont* s_AntaSmall;
		static ImFont* s_PlexBold;
		static ImFont* s_OpenSansRegular;
		static ImFont* s_OpenSansBold;
		static ImFont* s_RobotoRegular;

		static Ref<Texture2D> s_IconPlay, s_IconPause, s_IconStop,
			s_IconStep, s_IconSimulate, s_IconAddItem,
			s_IconSettings, s_IconDelete, s_IconEdit, s_IconEdit_Active, s_IconCancel,
			s_IconConfirm, s_IconSearch, s_IconCheckbox_Empty_Disabled,
			s_IconCheckbox_Check_Disabled, s_IconCheckbox_Empty_Enabled,
			s_IconCheckbox_Check_Enabled;

		static Ref<Texture2D> s_DirectoryIcon, s_GenericFileIcon, s_BackIcon,
			s_AudioIcon, s_ImageIcon, s_BinaryIcon,
			s_SceneIcon, s_RegistryIcon, s_ScriptProjectIcon,
			s_UserInterfaceIcon, s_FontIcon, s_InputIcon;

		inline static ImVec4 s_PureWhite {1.0f, 1.0f, 1.0f, 1.0f};
		inline static ImVec4 s_PearlBlue {38.0f / 255.0f, 212.0f / 255.0f, 212.0f / 255.0f, 1.0f};
		inline static ImVec4 s_DarkPurple {0.27843f, 0.011764f, 0.4f, 1.0f};
		inline static ImVec4 s_LightPurple_Thin { 182.0f / 255.0f, 103.0f / 255.0f, 219.0f / 255.0f, 0.35f };
	private:
		inline static bool s_BlockEvents = true;
		inline static bool s_Running = false;
	};
}
