#include "kgpch.h"

#include "Kargono/EditorUI/Editor.h"
#include "Kargono/Core/EngineCore.h"
#include "Kargono/Input/InputMode.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/Utility/Operations.h"

#define IMGUI_IMPL_API
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include "ImGuizmo.h"
#include "imgui_internal.h"

namespace Kargono::EditorUI
{
	ImFont* Editor::s_AntaLarge{ nullptr };
	ImFont* Editor::s_AntaRegular{ nullptr };
	ImFont* Editor::s_AntaSmall{ nullptr };
	ImFont* Editor::s_PlexBold{ nullptr };
	ImFont* Editor::s_PlexRegular{ nullptr };
	ImFont* Editor::s_OpenSansRegular{ nullptr };
	ImFont* Editor::s_OpenSansBold{ nullptr };
	ImFont* Editor::s_RobotoRegular{ nullptr };
	ImFont* Editor::s_RobotoMono{ nullptr };
	ImFont* Editor::s_AnonymousRegular{ nullptr };

	Ref<Texture2D> Editor::s_IconPlay{};
	Ref<Texture2D> Editor::s_IconPause{};
	Ref<Texture2D> Editor::s_IconStop{};
	Ref<Texture2D> Editor::s_IconStep{};
	Ref<Texture2D> Editor::s_IconSimulate{};
	Ref<Texture2D> Editor::s_IconAddItem{};
	Ref<Texture2D> Editor::s_IconSettings{};
	Ref<Texture2D> Editor::s_IconDelete{};
	Ref<Texture2D> Editor::s_IconDeleteActive{};
	Ref<Texture2D> Editor::s_IconEdit{};
	Ref<Texture2D> Editor::s_IconEdit_Active{};
	Ref<Texture2D> Editor::s_IconCancel{};
	Ref<Texture2D> Editor::s_IconConfirm{};
	Ref<Texture2D> Editor::s_IconSearch{};
	Ref<Texture2D> Editor::s_IconCheckbox_Empty_Disabled{};
	Ref<Texture2D> Editor::s_IconCheckbox_Check_Disabled{};
	Ref<Texture2D> Editor::s_IconCheckbox_Empty_Enabled{};
	Ref<Texture2D> Editor::s_IconCheckbox_Check_Enabled{};
	Ref<Texture2D> Editor::s_IconOptions{};
	Ref<Texture2D> Editor::s_IconDown{};
	Ref<Texture2D> Editor::s_IconRight{};
	Ref<Texture2D> Editor::s_IconDash{};

	Ref<Texture2D> Editor::s_DirectoryIcon{};
	Ref<Texture2D> Editor::s_GenericFileIcon{};
	Ref<Texture2D> Editor::s_BackIcon{};
	Ref<Texture2D> Editor::s_BackInactiveIcon{};
	Ref<Texture2D> Editor::s_ForwardIcon{};
	Ref<Texture2D> Editor::s_ForwardInactiveIcon{};
	Ref<Texture2D> Editor::s_AudioIcon{};
	Ref<Texture2D> Editor::s_ImageIcon{};
	Ref<Texture2D> Editor::s_BinaryIcon{};
	Ref<Texture2D> Editor::s_SceneIcon{};
	Ref<Texture2D> Editor::s_RegistryIcon{};
	Ref<Texture2D> Editor::s_ScriptProjectIcon{};
	Ref<Texture2D> Editor::s_UserInterfaceIcon{};
	Ref<Texture2D> Editor::s_FontIcon{};
	Ref<Texture2D> Editor::s_InputIcon{};

	void SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = Editor::s_LightPurple_Thin;
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	}

	static InlineButtonSpec s_TableEditButton {};
	static InlineButtonSpec s_TableLinkButton {};
	static InlineButtonSpec s_TableExpandButton {};

	static void InitializeTableResources()
	{
		s_TableEditButton = Editor::s_SmallEditButton;
		s_TableEditButton.YPosition = -5.5f;

		s_TableLinkButton = Editor::s_SmallLinkButton;
		s_TableLinkButton.YPosition = -5.5f;

		s_TableExpandButton = Editor::s_SmallExpandButton;
	}

	void Editor::Init()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		s_OpenSansBold = io.Fonts->AddFontFromFileTTF("Resources/fonts/opensans/static/OpenSans-Bold.ttf", 18.0f);
		s_OpenSansRegular = io.Fonts->AddFontFromFileTTF("Resources/fonts/opensans/static/OpenSans-Regular.ttf", 18.0f);
		s_AntaLarge = io.Fonts->AddFontFromFileTTF("Resources/fonts/Anta-Regular.ttf", 23.0f);
		s_AntaRegular = io.Fonts->AddFontFromFileTTF("Resources/fonts/Anta-Regular.ttf", 20.0f);
		s_AntaSmall = io.Fonts->AddFontFromFileTTF("Resources/fonts/Anta-Regular.ttf", 18.0f);
		s_PlexBold = io.Fonts->AddFontFromFileTTF("Resources/fonts/IBMPlexMono-Bold.ttf", 29.0f);
		s_PlexRegular = io.Fonts->AddFontFromFileTTF("Resources/fonts/IBMPlexMono-Bold.ttf", 22.0f);
		s_RobotoRegular = io.Fonts->AddFontFromFileTTF("Resources/fonts/Roboto-Regular.ttf", 18.0f);
		s_RobotoMono = io.Fonts->AddFontFromFileTTF("Resources/fonts/RobotoMono-SemiBold.ttf", 17.0f);
		s_AnonymousRegular = io.Fonts->AddFontFromFileTTF("Resources/fonts/AnonymousPro-Regular.ttf", 16.0f);
		io.FontDefault = s_RobotoMono;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();
		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		SetDarkThemeColors();

		// Setup Platform/Renderer backends
		EngineCore& app = EngineCore::GetCurrentEngineCore();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		// Set Up Editor Resources
		s_IconPlay = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/play_icon.png").string());
		s_IconPause = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/pause_icon.png").string());
		s_IconSimulate = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/simulate_icon.png").string());
		s_IconStop = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/stop_icon.png").string());
		s_IconStep = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/step_icon.png").string());
		s_IconAddItem = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/add_item.png").string());
		s_IconSettings = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/settings_icon.png").string());
		s_IconDelete = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/delete_icon.png").string());
		s_IconDeleteActive = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/delete_active_icon.png").string());
		s_IconEdit = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/edit_icon.png").string());
		s_IconEdit_Active = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/edit_active_icon.png").string());
		s_IconCancel = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/cancel_icon.png").string());
		s_IconConfirm = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/confirm_icon.png").string());
		s_IconSearch = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/search_icon.png").string());
		s_IconOptions = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/options_icon.png").string());
		s_IconDown = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/down_icon.png").string());
		s_IconRight = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/right_icon.png").string());
		s_IconDash = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/dash_icon.png").string());

		s_IconCheckbox_Empty_Disabled = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/checkbox/checkbox_disabled_empty_icon.png").string());
		s_IconCheckbox_Check_Disabled = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/checkbox/checkbox_disabled_check_icon.png").string());
		s_IconCheckbox_Empty_Enabled = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/checkbox/checkbox_enabled_empty_icon.png").string());
		s_IconCheckbox_Check_Enabled = Texture2D::CreateEditorTexture((EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/checkbox/checkbox_enabled_check_icon.png").string());

		s_DirectoryIcon = Texture2D::CreateEditorTexture(EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/content_browser/directory_icon.png");
		s_GenericFileIcon = Texture2D::CreateEditorTexture(EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/content_browser/generic_file_icon.png");
		s_BackIcon = Texture2D::CreateEditorTexture(EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/content_browser/back_icon.png");
		s_BackInactiveIcon = Texture2D::CreateEditorTexture(EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/content_browser/back_inactive_icon.png");
		s_ForwardIcon = Texture2D::CreateEditorTexture(EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/content_browser/forward_icon.png");
		s_ForwardInactiveIcon = Texture2D::CreateEditorTexture(EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/content_browser/forward_inactive_icon.png");
		s_AudioIcon = Texture2D::CreateEditorTexture(EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/content_browser/audio_icon.png");
		s_ImageIcon = Texture2D::CreateEditorTexture(EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/content_browser/image_icon.png");
		s_BinaryIcon = Texture2D::CreateEditorTexture(EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/content_browser/binary_icon.png");
		s_SceneIcon = Texture2D::CreateEditorTexture(EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/content_browser/scene_icon.png");
		s_RegistryIcon = Texture2D::CreateEditorTexture(EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/content_browser/registry_icon.png");
		s_ScriptProjectIcon = Texture2D::CreateEditorTexture(EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/content_browser/script_project.png");
		s_FontIcon = Texture2D::CreateEditorTexture(EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/content_browser/font.png");
		s_UserInterfaceIcon = Texture2D::CreateEditorTexture(EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/content_browser/user_interface_icon.png");
		s_InputIcon = Texture2D::CreateEditorTexture(EngineCore::GetCurrentEngineCore().GetWorkingDirectory() / "Resources/icons/content_browser/input_icon.png");

		s_SmallEditButton = {
				390.0f,
				0.0f,
				13.0f,
				EditorUI::Editor::s_IconEdit_Active,
				EditorUI::Editor::s_IconEdit,
				"Cancel Editing",
				"Edit",
				PositionType::Absolute
		};

		s_SmallExpandButton = {
				-1.2f,
				4.0f,
				14.0f,
				EditorUI::Editor::s_IconDown,
				EditorUI::Editor::s_IconRight,
				"Collapse",
				"Expand",
				PositionType::Inline
		};

		s_SmallOptionsButton = {
				386.0f,
				1.0f,
				19.0f,
				EditorUI::Editor::s_IconOptions,
				EditorUI::Editor::s_IconOptions,
				"Options",
				"Options",
				PositionType::Absolute
		};

		s_SmallCheckboxButton = {
				0.0,
				0.0f,
				14.0f,
				EditorUI::Editor::s_IconCheckbox_Check_Enabled,
				EditorUI::Editor::s_IconCheckbox_Empty_Enabled,
				"Uncheck",
				"Check",
				PositionType::Inline
		};

		s_SmallLinkButton = {
				390.0,
				0.0f,
				14.0f,
				EditorUI::Editor::s_ForwardInactiveIcon,
				EditorUI::Editor::s_ForwardInactiveIcon,
				"Open",
				"Open",
				PositionType::Absolute
		};

		s_LargeDeleteButton = {
				-112.0f,
				-0.6f,
				28.0f,
				EditorUI::Editor::s_IconDeleteActive,
				EditorUI::Editor::s_IconDeleteActive,
				"Delete",
				"Delete",
				PositionType::Relative
		};

		s_LargeCancelButton = {
				-75.0f,
				-0.6f,
				28.0f,
				EditorUI::Editor::s_IconCancel,
				EditorUI::Editor::s_IconCancel,
				"Cancel",
				"Cancel",
				PositionType::Relative
		};

		s_LargeConfirmButton = {
				-38.0f,
				-0.6f,
				28.0f,
				EditorUI::Editor::s_IconConfirm,
				EditorUI::Editor::s_IconConfirm,
				"Confirm",
				"Confirm",
				PositionType::Relative
		};

		s_LargeSearchButton = {
				-112.0f,
				-0.6f,
				28.0f,
				EditorUI::Editor::s_IconSearch,
				EditorUI::Editor::s_IconSearch,
				"Cancel Search",
				"Search",
				PositionType::Relative
		};

		InitializeTableResources();

		s_Running = true;
	}

	void Editor::Terminate()
	{
		if (s_Running)
		{
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			s_Running = false;
		}
	}

	void Editor::StartRendering()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void Editor::EndRendering()
	{
		ImGuiIO& io = ImGui::GetIO();
		EngineCore& app = EngineCore::GetCurrentEngineCore();
		io.DisplaySize = ImVec2(static_cast<float>(app.GetWindow().GetWidth()), static_cast<float>(app.GetWindow().GetHeight()));

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void Editor::StartWindow(const std::string& label, bool* closeWindow, int32_t flags)
	{
		ImGui::Begin(label.c_str(), closeWindow, flags);
	}

	void Editor::EndWindow()
	{
		ImGui::End();
	}

	uint32_t Editor::GetActiveWidgetID()
	{
		return GImGui->ActiveId;
	}

	void Editor::OnEvent(Events::Event& e)
	{
		KG_PROFILE_FUNCTION();

		if (s_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(Events::Mouse) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(Events::Keyboard) & io.WantCaptureKeyboard;
		}
	}

	uint32_t WidgetIterator(uint32_t& count)
	{
		return ++count;
	}

	void Editor::Spacing(float space)
	{
		ImGui::Dummy(ImVec2(0.0f, space));
	}

	void Editor::TitleText(const std::string& text)
	{
		ImGui::PushFont(EditorUI::Editor::s_PlexBold);
		ImGui::TextColored(s_PearlBlue, text.c_str());
		ImGui::PopFont();
	}

	void Editor::Spacing(SpacingAmount space)
	{
		switch (space)
		{
			case SpacingAmount::Small:
			{
				ImGui::Dummy(ImVec2(0.0f, 3.0f));
				return;
			}
			case SpacingAmount::Medium:
			{
				ImGui::Dummy(ImVec2(0.0f, 8.0f));
				return;
			}
			case SpacingAmount::Large:
			{
				ImGui::Dummy(ImVec2(0.0f, 15.0f));
				return;
			}
			case SpacingAmount::None:
			default:
			{
				KG_WARN("Invalid SpacingAmount provided to Spacing() enum function");
				return;
			}
		}
	}

	void Editor::Separator()
	{
		ImGui::Separator();
	}

	static OptionList GenerateSearchCache(OptionList& originalList, const std::string& searchQuery)
	{
		OptionList returnList{};
		for (auto& [title, options] : originalList)
		{
			std::vector<OptionEntry> returnOptions {};
			for (auto& option : options)
			{
				if (!Utility::Regex::GetMatchSuccess(option.Label, searchQuery, false))
				{
					continue;
				}
				returnOptions.push_back(option);
			}
			if (!returnOptions.empty())
			{
				returnList.insert_or_assign(title, returnOptions);
			}
		}
		return returnList;
	}

	static void CreateImage(Ref<Texture2D> image, float size)
	{
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 4.0f, ImGui::GetCursorPosY() + 3.2f));
		ImGui::Image((ImTextureID)(uint64_t)image->GetRendererID(), ImVec2(size, size),
			ImVec2{ 0, 1 }, ImVec2(1, 0), Editor::s_PureWhite,
			Editor::s_PureEmpty);
	}

	static float SmallButtonAbsoluteLocation(uint32_t slot)
	{
		return 390.0f - (22.0f * slot);
	}

	static void CreateInlineButton(ImGuiID widgetID, std::function<void()> onPress, const InlineButtonSpec& spec, bool active = false)
	{
		switch (spec.XPositionType)
		{
			case PositionType::Inline:
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + spec.XPosition);
				break;
			}
			case PositionType::Absolute:
			{
				ImGui::SetCursorPosX(spec.XPosition);
				break;
			}
			case PositionType::Relative:
			{
				ImGui::SetCursorPosX(ImGui::GetWindowWidth() + spec.XPosition);
				break;
			}
			default:
			{
				break;	
			}
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spec.YPosition);
		ImGui::PushStyleColor(ImGuiCol_Button, Editor::s_PureEmpty);
		Ref<Texture2D> iconChoice = active ? spec.ActiveIcon : spec.InactiveIcon;
		if (ImGui::ImageButtonEx(widgetID,
			(ImTextureID)(uint64_t)iconChoice->GetRendererID(),
			ImVec2(spec.IconSize, spec.IconSize), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
			Editor::s_PureEmpty,
			EditorUI::Editor::s_PureWhite, 0))
		{
			onPress();
		}
		ImGui::PopStyleColor();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::TextColored(EditorUI::Editor::s_PearlBlue, active ? spec.ActiveTooltip.c_str() : spec.InactiveTooltip.c_str());
			ImGui::EndTooltip();
		}
	}

	static void WriteMultilineText(const std::string& text, float leftHandOffset, const uint32_t charPerLine, float yOffset = 0)
	{
		std::string previewOutput{};
		std::string previewRemainder{ text };
		uint32_t iteration{ 0 };
		do
		{
			if (previewRemainder.size() > charPerLine)
			{
				previewOutput = previewRemainder.substr(0, std::min(charPerLine, (uint32_t)previewRemainder.size()));
				previewRemainder = previewRemainder.substr(std::min(charPerLine, (uint32_t)previewRemainder.size()), std::string::npos);
			}
			else
			{
				previewOutput = previewRemainder.c_str();
				previewRemainder.clear();
			}

			if (leftHandOffset > 0)
			{
				ImGui::SameLine(leftHandOffset);
			}
			
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + yOffset + (iteration * 20.0f));
			
			ImGui::Text(previewOutput.c_str());
			iteration++;
		} while (!previewRemainder.empty());
	}

	static void TruncateText(const std::string& text, uint32_t maxTextSize)
	{
		if (text.size() > maxTextSize)
		{
			std::string outputText = text.substr(0, maxTextSize - 2) + "..";
			ImGui::TextColored(EditorUI::Editor::s_PureWhite, outputText.c_str());
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::Editor::s_PearlBlue, text.c_str());
				ImGui::EndTooltip();
			}
		}
		else
		{
			ImGui::Text(text.c_str());
		}
	}

	void Editor::GenericPopup(GenericPopupSpec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };

		if (spec.PopupActive)
		{
			ImGui::OpenPopup(id.c_str());
			spec.PopupActive = false;

			if (spec.PopupAction)
			{
				spec.PopupAction(spec);
			}
		}

		// Display Popup
		ImGui::SetNextWindowSize(ImVec2(spec.PopupWidth, 0.0f));
		if (ImGui::BeginPopupModal(id.c_str(), NULL, ImGuiWindowFlags_NoTitleBar))
		{
			EditorUI::Editor::TitleText(spec.Label);

			ImGui::PushFont(EditorUI::Editor::s_AntaRegular);
			if (spec.DeleteAction)
			{
				// Optional Delete Tool Bar Button
				ImGui::SameLine();
				CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
				{
					if (spec.DeleteAction)
					{
						spec.DeleteAction();
					}
					ImGui::CloseCurrentPopup();
				}, s_LargeDeleteButton);
			}

			// Cancel Tool Bar Button
			ImGui::SameLine();
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				ImGui::CloseCurrentPopup();
			}, s_LargeCancelButton);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
				ImGui::CloseCurrentPopup();
			}, s_LargeConfirmButton);

			ImGui::Separator();

			spec.PopupContents();

			ImGui::PopFont();
			ImGui::EndPopup();
		}
	}

	void Editor::NewItemScreen(const std::string& label1, std::function<void()> onPress1, const std::string& label2, std::function<void()> onPress2)
	{
		ImGui::PushFont(EditorUI::Editor::s_AntaLarge);
		ImVec2 screenDimensions = ImGui::GetContentRegionAvail();
		ImVec2 originalLocation = ImGui::GetCursorScreenPos();
		if (screenDimensions.x > 700.0f)
		{
			ImVec2 buttonDimensions {2.75f, 5.0f};
			ImVec2 screenLocation = ImVec2(originalLocation.x + screenDimensions.x / 2 - (screenDimensions.x / buttonDimensions.x), originalLocation.y + screenDimensions.y / 2 - (screenDimensions.y / (buttonDimensions.y * 2)));
			ImGui::SetCursorScreenPos(screenLocation);
			if (ImGui::Button(label1.c_str(), ImVec2(screenDimensions.x / buttonDimensions.x, screenDimensions.y / buttonDimensions.y)))
			{
				onPress1();
			}
			ImGui::SameLine();
			if (ImGui::Button(label2.c_str(), ImVec2(screenDimensions.x / buttonDimensions.x, screenDimensions.y / buttonDimensions.y)))
			{
				onPress2();
			}
		}
		else
		{
			ImVec2 buttonDimensions{ 1.5f, 8.0f };
			ImVec2 screenLocation = ImVec2(originalLocation.x + screenDimensions.x / 2 - (screenDimensions.x / (buttonDimensions.x * 2)), originalLocation.y + screenDimensions.y / 2 - (screenDimensions.y / buttonDimensions.y));
			ImGui::SetCursorScreenPos(screenLocation);
			if (ImGui::Button(label1.c_str(), ImVec2(screenDimensions.x / buttonDimensions.x, screenDimensions.y / buttonDimensions.y)))
			{
				onPress1();
			}
			Editor::Spacing(SpacingAmount::Small);
			ImGui::SetCursorScreenPos(ImVec2(screenLocation.x, ImGui::GetCursorScreenPos().y));
			if (ImGui::Button(label2.c_str(), ImVec2(screenDimensions.x / buttonDimensions.x, screenDimensions.y / buttonDimensions.y)))
			{
				onPress2();
			}
		}
		ImGui::PopFont();
	}


	void Editor::SelectOption(SelectOptionSpec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };

		if (spec.Flags & SelectOption_PopupOnly)
		{
			if (spec.StartPopup)
			{
				ImGui::OpenPopup(id.c_str());
				spec.StartPopup = false;
				if (spec.PopupAction)
				{
					spec.PopupAction(spec);
				}
				spec.CachedSelection = spec.CurrentOption;
			}
		}
		else
		{
			// Display Menu Item
			if (spec.Flags & SelectOption_Indented)
			{
				ImGui::SetCursorPosX(30.5f);
			}
			TruncateText(spec.Label, spec.Flags & SelectOption_Indented ? 20: 23);

			ImGui::PushStyleColor(ImGuiCol_Text, s_PearlBlue);
			WriteMultilineText(spec.CurrentOption.Label, 200.0f, 23);
			ImGui::PopStyleColor();

			ImGui::SameLine();
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				ImGui::OpenPopup(id.c_str());
				if (spec.PopupAction)
				{
					spec.PopupAction(spec);
				}
				spec.CachedSelection = spec.CurrentOption;
			},
			Editor::s_SmallEditButton);

		}
		
		// Display Popup
		ImGui::SetNextWindowSize(ImVec2(700.0f, 0.0f));
		if (ImGui::BeginPopupModal(id.c_str(), NULL, ImGuiWindowFlags_NoTitleBar))
		{
			static char searchBuffer[256];

			EditorUI::Editor::TitleText(spec.Label);

			ImGui::PushFont(EditorUI::Editor::s_AntaRegular);
			if (spec.Searching)
			{
				ImGui::SameLine(ImGui::GetWindowWidth() - 124.0f - 200.0f);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.6f);
				ImGui::SetNextItemWidth(200.0f);

				ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData* data)
				{
					std::string currentData = std::string(data->Buf);
					SelectOptionSpec* providedSpec = (SelectOptionSpec*)data->UserData;
					providedSpec->CachedSearchResults = GenerateSearchCache(providedSpec->GetAllOptions(), currentData);
					return 0;
				};

				ImGui::InputText((id + "InputText").c_str(), searchBuffer, sizeof(searchBuffer), ImGuiInputTextFlags_CallbackEdit, callback, (void*)&spec);
			}

			// Search Tool Bar Button
			ImGui::SameLine();
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				if (spec.Searching)
				{
					spec.Searching = false;
				}
				else
				{
					spec.Searching = true;
					spec.CachedSearchResults = GenerateSearchCache(spec.ActiveOptions, searchBuffer);
				}
			}, s_LargeSearchButton, spec.Searching);

			// Cancel Tool Bar Button
			ImGui::SameLine();
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				spec.Searching = false;
				memset(searchBuffer, 0, sizeof(searchBuffer));
				ImGui::CloseCurrentPopup();
			}, s_LargeCancelButton);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				spec.CurrentOption = spec.CachedSelection;
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction(spec.CurrentOption);
				}

				spec.Searching = false;
				memset(searchBuffer, 0, sizeof(searchBuffer));
				ImGui::CloseCurrentPopup();
			}, s_LargeConfirmButton);

			ImGui::Separator();

			for (auto& [title, options] :
				spec.Searching ? spec.CachedSearchResults : spec.GetAllOptions())
			{
				ImGui::Text(title.c_str());
				uint32_t iteration{ 1 };
				bool selectedButton = false;
				for (auto& option : options)
				{
					if (spec.CachedSelection == option)
					{
						selectedButton = true;
					}

					if (selectedButton)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, s_DarkPurple);
					}

					if (ImGui::Button((option.Label + id).c_str()))
					{
						spec.CachedSelection = option;
					}
					if (iteration % spec.LineCount != 0 && iteration != 0 && iteration != options.size())
					{
						ImGui::SameLine();
					}

					if (selectedButton)
					{
						ImGui::PopStyleColor();
						selectedButton = false;
					}
					iteration++;
				}
				Editor::Spacing(SpacingAmount::Medium);
			}

			ImGui::PopFont();
			ImGui::EndPopup();
		}
	}

	void Editor::EditVariable(EditVariableSpec& spec)
	{
		// Local Variables
		uint32_t widgetCount{ 0 };
		std::string id = "##" + std::to_string(spec.WidgetID);

		ImGui::TextColored(s_PureWhite, spec.Label.c_str());
		ImGui::PushStyleColor(ImGuiCol_Text, s_PearlBlue);
		ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_CallbackEdit;
		if (spec.VariableType != WrappedVarType::String)
		{
			inputFlags |= ImGuiInputTextFlags_CharsDecimal;
		}
		ImGui::SameLine();
		ImGui::SetCursorPosX(200.0f);
		ImGui::SetNextItemWidth(170.0f);
		ImGuiInputTextCallback typeCallback = [](ImGuiInputTextCallbackData* data)
		{
			return 0;
		};
		ImGui::InputText(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(),
			spec.FieldBuffer.As<char>(), spec.FieldBuffer.Size, inputFlags, typeCallback );
		ImGui::PopStyleColor();
	}

	void Editor::Checkbox(CheckboxSpec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };

		// Display Item
		if (spec.Flags & Checkbox_LeftLean)
		{
			TruncateText(spec.Label, 23);
			ImGui::SameLine(197.5f);
		}
		else
		{
			ImGui::Text(spec.Label.c_str());
			ImGui::SameLine(360.0f);
		}

		if (spec.Editing)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Editor::s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, Editor::s_PureEmpty);
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				if (spec.ToggleBoolean)
				{
					spec.ConfirmAction(false);
				}
				else
				{
					spec.ConfirmAction(true);
				}
			}, s_SmallCheckboxButton, spec.ToggleBoolean);
			ImGui::PopStyleColor(2);
		}
		else
		{
			CreateImage(spec.ToggleBoolean ?
				EditorUI::Editor::s_IconCheckbox_Check_Disabled :
				EditorUI::Editor::s_IconCheckbox_Empty_Disabled,
				14);
		}

		ImGui::SameLine();
		CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.Editing);
		},
		Editor::s_SmallEditButton,
		spec.Editing);
	}

	void Editor::RadioSelector(RadioSelectorSpec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };

		// Display Item
		TruncateText(spec.Label, 23);
		ImGui::SameLine(200.0f);

		if (spec.Editing)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, s_PureEmpty);
			TruncateText(spec.FirstOptionLabel, 12);
			ImGui::SameLine();
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				if (spec.SelectedOption == 0)
				{
					spec.SelectedOption = 1;
				}
				else
				{
					spec.SelectedOption = 0;
				}
				spec.SelectAction(spec.SelectedOption);
			}, s_SmallCheckboxButton, spec.SelectedOption == 0);

			ImGui::SameLine(300.0f);
			TruncateText(spec.SecondOptionLabel, 12);
			ImGui::SameLine();
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				if (spec.SelectedOption == 1)
				{
					spec.SelectedOption = 0;
				}
				else
				{
					spec.SelectedOption = 1;
				}
				spec.SelectAction(spec.SelectedOption);
			}, s_SmallCheckboxButton, spec.SelectedOption == 1);
			ImGui::PopStyleColor(2);
		}
		else
		{
			TruncateText(spec.FirstOptionLabel, 12);
			ImGui::SameLine();
			CreateImage(spec.SelectedOption == 0 ? 
				EditorUI::Editor::s_IconCheckbox_Check_Disabled : 
				EditorUI::Editor::s_IconCheckbox_Empty_Disabled,
				14);

			ImGui::SameLine(300.0f);
			TruncateText(spec.SecondOptionLabel, 12);
			ImGui::SameLine();
			CreateImage(spec.SelectedOption == 1 ?
				EditorUI::Editor::s_IconCheckbox_Check_Disabled :
				EditorUI::Editor::s_IconCheckbox_Empty_Disabled,
				14);
		}

		ImGui::SameLine();
		CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.Editing);
		},
		Editor::s_SmallEditButton,
		spec.Editing);
	}

	void Editor::Table(TableSpec& spec)
	{
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };
		uint32_t smallButtonCount{ 0 };

		if (spec.Flags & Table_Indented)
		{
			ImGui::SetCursorPosX(30.5f);
		}
		else
		{
			ImGui::PushFont(Editor::s_AntaLarge);
		}
		TruncateText(spec.Label, 40);

		if (spec.Flags & Table_UnderlineTitle)
		{
			ImGui::Separator();
		}

		if (!(spec.Flags & Table_Indented))
		{
			ImGui::PopFont();
		}
		s_TableExpandButton.IconSize = (spec.Flags & Table_Indented) ? 12.0f : 14.0f;
		s_TableExpandButton.YPosition = spec.Flags & Table_Indented ? -0.0f : 4.5f;
		ImGui::SameLine();
		CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.Expanded);
		}, 
		s_TableExpandButton ,spec.Expanded);
		
		if (spec.Expanded)
		{
			if (!spec.EditTableSelectionList.empty())
			{
				ImGui::SameLine();
				CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
				{
					ImGui::OpenPopup(spec.WidgetID - 1);
				}, s_SmallOptionsButton);

				if (ImGui::BeginPopupEx(spec.WidgetID - 1, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
				{
					for (auto& [label, func] : spec.EditTableSelectionList)
					{
						if (ImGui::Selectable((label + id).c_str()))
						{
							func();
						}
					}
					ImGui::EndPopup();
				}
			}

			if (!spec.TableValues.empty())
			{
				// Column Titles
				ImGui::SetCursorPosX(spec.Flags & Table_Indented ? 61.0f: 30.5f);
				TruncateText(spec.Column1Title, 12);
				ImGui::SameLine();
				ImGui::SetCursorPosX(200.0f);
				TruncateText(spec.Column2Title, 12);
				Spacing(SpacingAmount::Small);
			}

			for (auto& tableEntry : spec.TableValues)
			{
				smallButtonCount = 0;
				ImGui::SetCursorPosX(spec.Flags & Table_Indented ? 42.5f : 12.0f);
				CreateImage(s_IconDash, 8);
				ImGui::SameLine();
				ImGui::SetCursorPosX(spec.Flags & Table_Indented ? 61.0f : 30.5f);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.2f);
				TruncateText(tableEntry.Label, 16);
				ImGui::PushStyleColor(ImGuiCol_Text, s_PearlBlue);
				WriteMultilineText(tableEntry.Value,200.0f, 21, -5.2f);
				ImGui::PopStyleColor();
				
				if (tableEntry.OnLink)
				{
					s_TableLinkButton.XPosition = SmallButtonAbsoluteLocation(smallButtonCount++);
					ImGui::SameLine();
					CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
					{
						if (tableEntry.OnLink)
						{
							tableEntry.OnLink(tableEntry);
						}
					}, s_TableLinkButton);
				}

				if (tableEntry.OnEdit)
				{
					s_TableEditButton.XPosition = SmallButtonAbsoluteLocation(smallButtonCount++);
					ImGui::SameLine();
					CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
						{
							if (tableEntry.OnEdit)
							{
								tableEntry.OnEdit(tableEntry);
							}
						}, s_TableEditButton);
				}
				
			}
		}
	}

	void Editor::SelectorHeader(SelectorHeaderSpec& spec)
	{
		std::string id = "##" + std::to_string(spec.WidgetID);
		ImGui::PushFont(Editor::s_AntaLarge);
		ImGui::TextColored(spec.EditColorActive ? Editor::s_PearlBlue : Editor::s_PureWhite , spec.Label.c_str());
		ImGui::PopFont();

		ImGui::SameLine();
		CreateInlineButton(spec.WidgetID, [&]()
		{
			ImGui::OpenPopup(id.c_str());
		}, s_SmallOptionsButton);
		
		if (ImGui::BeginPopup(id.c_str()))
		{
			for (auto& [label, func] : spec.GetSelectionList())
			{
				if (ImGui::Selectable((label + id).c_str()))
				{
					func();
				}
			}
			ImGui::EndPopup();
		}
		EditorUI::Editor::Spacing(0.2f);
		EditorUI::Editor::Separator();
	}

	void Editor::CollapsingHeader(CollapsingHeaderSpec& spec)
	{
		uint32_t widgetCount{ 0 };
		ImGui::PushFont(Editor::s_AntaLarge);
		ImGui::Text(spec.Label.c_str());
		ImGui::PopFont();
		ImGui::SameLine();
		CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.Expanded);
		},
		s_SmallExpandButton, spec.Expanded);

		if ((spec.Flags & CollapsingHeader_UnderlineTitle) && spec.Expanded)
		{
			ImGui::Separator();
		}

		if (spec.Expanded && spec.OnExpand)
		{
			spec.OnExpand();
		}
	}

	void Editor::LabeledText(const std::string& label, const std::string& text)
	{
		// Display Menu Item
		TruncateText(label, 23);
		//ImGui::TextColored(s_PureWhite, label.c_str());
		ImGui::PushStyleColor(ImGuiCol_Text, s_PearlBlue);
		WriteMultilineText(text, 200.0f, 23);
		ImGui::PopStyleColor();
	}
	void Editor::Text(const std::string& Text)
	{
		ImGui::Text(Text.c_str());
	}
	void Editor::TextInputPopup(TextInputSpec& spec)
	{
		// Local Variables
		static char stringBuffer[256];
		uint32_t widgetCount{ 0 };
		std::string id = "##" + std::to_string(spec.WidgetID);
		std::string popUpLabel = spec.Label;

		if (spec.Flags & TextInput_PopupOnly)
		{
			if (spec.StartPopup)
			{
				ImGui::OpenPopup(id.c_str());
				spec.StartPopup = false;
				memset(stringBuffer, 0, sizeof(stringBuffer));
				memcpy_s(stringBuffer, sizeof(stringBuffer), spec.CurrentOption.data(), spec.CurrentOption.size());
			}
		}
		else
		{
			ImGui::TextColored(s_PureWhite, spec.Label.c_str());
			ImGui::PushStyleColor(ImGuiCol_Text, s_PearlBlue);
			WriteMultilineText(spec.CurrentOption, 200.0f, 23);
			ImGui::PopStyleColor();

			ImGui::SameLine();
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount),[&]()
			{
				ImGui::OpenPopup(id.c_str());
				memset(stringBuffer, 0, sizeof(stringBuffer));
				memcpy_s(stringBuffer, sizeof(stringBuffer), spec.CurrentOption.data(), spec.CurrentOption.size());
			},
			Editor::s_SmallEditButton);
		}

		ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f));
		if (ImGui::BeginPopupModal(id.c_str(), NULL, ImGuiWindowFlags_NoTitleBar))
		{
			EditorUI::Editor::TitleText(popUpLabel);

			ImGui::PushFont(EditorUI::Editor::s_AntaRegular);

			// Cancel Tool Bar Button
			ImGui::SameLine();
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				memset(stringBuffer, 0, sizeof(stringBuffer));
				ImGui::CloseCurrentPopup();
			}, s_LargeCancelButton);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				spec.CurrentOption = std::string(stringBuffer);
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction(stringBuffer);
				}
				memset(stringBuffer, 0, sizeof(stringBuffer));
				ImGui::CloseCurrentPopup();
			}, s_LargeConfirmButton);

			ImGui::SetNextItemWidth(583.0f);
			ImGui::InputText((id + "InputText").c_str(), stringBuffer, sizeof(stringBuffer));
			ImGui::PopFont();
			ImGui::EndPopup();
		}
	}
}