#include "kgpch.h"

#include "Kargono/EditorUI/EditorUI.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/Utility/Operations.h"
#include "Kargono/Input/InputService.h"
#include "Kargono/Input/InputMode.h"

#include "API/EditorUI/ImGuiBackendAPI.h"
#include "API/Platform/GlfwAPI.h"
#include "API/Platform/gladAPI.h"
#include "Kargono/Utility/FileDialogs.h"


namespace Kargono::EditorUI
{
	ImFont* EditorUIService::s_AntaLarge{ nullptr };
	ImFont* EditorUIService::s_AntaRegular{ nullptr };
	ImFont* EditorUIService::s_AntaSmall{ nullptr };
	ImFont* EditorUIService::s_PlexBold{ nullptr };
	ImFont* EditorUIService::s_PlexRegular{ nullptr };
	ImFont* EditorUIService::s_OpenSansRegular{ nullptr };
	ImFont* EditorUIService::s_OpenSansBold{ nullptr };
	ImFont* EditorUIService::s_RobotoRegular{ nullptr };
	ImFont* EditorUIService::s_RobotoMono{ nullptr };
	ImFont* EditorUIService::s_AnonymousRegular{ nullptr };

	Ref<Rendering::Texture2D> EditorUIService::s_IconPlay{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconPause{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconStop{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconGrid{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconStep{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconSimulate{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconAddItem{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconEntity{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconDisplay{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconDisplayActive{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconCamera{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconCameraActive{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconPlayActive{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconStopActive{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconPauseActive{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconStepActive{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconSimulateActive{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconSettings{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconDelete{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconDeleteActive{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconEdit{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconEdit_Active{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconCancel{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconCancel2{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconConfirm{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconSearch{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconCheckbox_Empty_Disabled{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconCheckbox_Check_Disabled{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconCheckbox_Empty_Enabled{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconCheckbox_Check_Enabled{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconOptions{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconDown{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconRight{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconDash{};

	Ref<Rendering::Texture2D> EditorUIService::s_DirectoryIcon{};
	Ref<Rendering::Texture2D> EditorUIService::s_GenericFileIcon{};
	Ref<Rendering::Texture2D> EditorUIService::s_BackIcon{};
	Ref<Rendering::Texture2D> EditorUIService::s_BackInactiveIcon{};
	Ref<Rendering::Texture2D> EditorUIService::s_ForwardIcon{};
	Ref<Rendering::Texture2D> EditorUIService::s_ForwardInactiveIcon{};
	Ref<Rendering::Texture2D> EditorUIService::s_AudioIcon{};
	Ref<Rendering::Texture2D> EditorUIService::s_ImageIcon{};
	Ref<Rendering::Texture2D> EditorUIService::s_BinaryIcon{};
	Ref<Rendering::Texture2D> EditorUIService::s_SceneIcon{};
	Ref<Rendering::Texture2D> EditorUIService::s_RegistryIcon{};
	Ref<Rendering::Texture2D> EditorUIService::s_ScriptProjectIcon{};
	Ref<Rendering::Texture2D> EditorUIService::s_UserInterfaceIcon{};
	Ref<Rendering::Texture2D> EditorUIService::s_FontIcon{};
	Ref<Rendering::Texture2D> EditorUIService::s_InputIcon{};

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
		colors[ImGuiCol_ButtonHovered] = EditorUIService::s_LightPurple_Thin;
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = EditorUIService::s_PureEmpty;
		colors[ImGuiCol_FrameBgHovered] = EditorUIService::s_PureEmpty;
		colors[ImGuiCol_FrameBgActive] = EditorUIService::s_PureEmpty;

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
		s_TableEditButton = EditorUIService::s_SmallEditButton;
		s_TableEditButton.YPosition = -5.5f;

		s_TableLinkButton = EditorUIService::s_SmallLinkButton;
		s_TableLinkButton.YPosition = -5.5f;

		s_TableExpandButton = EditorUIService::s_SmallExpandButton;

	}

	static InlineButtonSpec s_CheckboxDisabledButton {};

	static void InitializeCheckboxResources ()
	{
		s_CheckboxDisabledButton = EditorUIService::s_SmallCheckboxButton;
		s_CheckboxDisabledButton.Disabled = true;
		s_CheckboxDisabledButton.ActiveIcon = EditorUI::EditorUIService::s_IconCheckbox_Check_Disabled;
		s_CheckboxDisabledButton.InactiveIcon = EditorUI::EditorUIService::s_IconCheckbox_Empty_Disabled;
	}

	void EditorUIService::Init()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.IniFilename = NULL;
		ImGui::LoadIniSettingsFromDisk("./Resources/EditorConfig.ini");
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

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
		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		style.WindowMenuButtonPosition = -1;
		SetDarkThemeColors();

		// Setup Platform/Renderer backends
		Engine& core = EngineService::GetActiveEngine();
		GLFWwindow* window = static_cast<GLFWwindow*>(core.GetWindow().GetNativeWindow());
		KG_ASSERT(window, "No window active when initializing EditorUI");
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		// Set Up Editor Resources
		s_IconPlay = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/play_icon.png").string());
		s_IconPause = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/pause_icon.png").string());
		s_IconSimulate = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/simulate_icon.png").string());
		s_IconGrid = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/grid_icon.png").string());
		s_IconStop = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/stop_icon.png").string());
		s_IconStep = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/step_icon.png").string());
		s_IconAddItem = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/add_item.png").string());
		s_IconDisplay = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/display_icon.png").string());
		s_IconDisplayActive = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/display_icon_active.png").string());
		s_IconCamera = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/camera_icon.png").string());
		s_IconCameraActive = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/camera_icon_active.png").string());
		s_IconPlayActive = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/play_icon_active.png").string());
		s_IconStopActive = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/stop_icon_active.png").string());
		s_IconPauseActive = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/pause_icon_active.png").string());
		s_IconStepActive = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/step_icon_active.png").string());
		s_IconSimulateActive = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/simulate_icon_active.png").string());
		s_IconSettings = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/settings_icon.png").string());
		s_IconDelete = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/delete_icon.png").string());
		s_IconDeleteActive = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/delete_active_icon.png").string());
		s_IconEdit = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/edit_icon.png").string());
		s_IconEdit_Active = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/edit_active_icon.png").string());
		s_IconCancel = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/cancel_icon.png").string());
		s_IconCancel2 = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/cancel_icon_2.png").string());
		s_IconConfirm = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/confirm_icon.png").string());
		s_IconSearch = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/search_icon.png").string());
		s_IconOptions = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/options_icon.png").string());
		s_IconDown = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/down_icon.png").string());
		s_IconRight = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/right_icon.png").string());
		s_IconDash = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/dash_icon.png").string());
		s_IconEntity = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/entity_icon.png").string());

		s_IconCheckbox_Empty_Disabled = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/checkbox/checkbox_disabled_empty_icon.png").string());
		s_IconCheckbox_Check_Disabled = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/checkbox/checkbox_disabled_check_icon.png").string());
		s_IconCheckbox_Empty_Enabled = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/checkbox/checkbox_enabled_empty_icon.png").string());
		s_IconCheckbox_Check_Enabled = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/checkbox/checkbox_enabled_check_icon.png").string());

		s_DirectoryIcon = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/content_browser/directory_icon.png");
		s_GenericFileIcon = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/content_browser/generic_file_icon.png");
		s_BackIcon = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/content_browser/back_icon.png");
		s_BackInactiveIcon = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/content_browser/back_inactive_icon.png");
		s_ForwardIcon = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/content_browser/forward_icon.png");
		s_ForwardInactiveIcon = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/content_browser/forward_inactive_icon.png");
		s_AudioIcon = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/content_browser/audio_icon.png");
		s_ImageIcon = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/content_browser/image_icon.png");
		s_BinaryIcon = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/content_browser/binary_icon.png");
		s_SceneIcon = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/content_browser/scene_icon.png");
		s_RegistryIcon = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/content_browser/registry_icon.png");
		s_ScriptProjectIcon = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/content_browser/script_project.png");
		s_FontIcon = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/content_browser/font.png");
		s_UserInterfaceIcon = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/content_browser/user_interface_icon.png");
		s_InputIcon = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/icons/content_browser/input_icon.png");

		s_SmallEditButton = {
				390.0f,
				0.0f,
				13.0f,
				EditorUI::EditorUIService::s_IconEdit_Active,
				EditorUI::EditorUIService::s_IconEdit,
				"Cancel Editing",
				"Edit",
				PositionType::Absolute
		};

		s_SmallExpandButton = {
				-1.2f,
				4.0f,
				14.0f,
				EditorUI::EditorUIService::s_IconDown,
				EditorUI::EditorUIService::s_IconRight,
				"Collapse",
				"Expand",
				PositionType::Inline
		};

		s_SmallOptionsButton = {
				386.0f,
				1.0f,
				19.0f,
				EditorUI::EditorUIService::s_IconOptions,
				EditorUI::EditorUIService::s_IconOptions,
				"Options",
				"Options",
				PositionType::Absolute
		};

		s_SmallCheckboxButton = {
				0.0,
				0.0f,
				14.0f,
				EditorUI::EditorUIService::s_IconCheckbox_Check_Enabled,
				EditorUI::EditorUIService::s_IconCheckbox_Empty_Enabled,
				"Uncheck",
				"Check",
				PositionType::Inline
		};

		s_SmallLinkButton = {
				390.0,
				0.0f,
				14.0f,
				EditorUI::EditorUIService::s_ForwardInactiveIcon,
				EditorUI::EditorUIService::s_ForwardInactiveIcon,
				"Open",
				"Open",
				PositionType::Absolute
		};

		s_LargeDeleteButton = {
				-112.0f,
				-0.6f,
				28.0f,
				EditorUI::EditorUIService::s_IconDeleteActive,
				EditorUI::EditorUIService::s_IconDeleteActive,
				"Delete",
				"Delete",
				PositionType::Relative
		};

		s_LargeCancelButton = {
				-75.0f,
				-0.6f,
				28.0f,
				EditorUI::EditorUIService::s_IconCancel,
				EditorUI::EditorUIService::s_IconCancel,
				"Cancel",
				"Cancel",
				PositionType::Relative
		};

		s_LargeConfirmButton = {
				-38.0f,
				-0.6f,
				28.0f,
				EditorUI::EditorUIService::s_IconConfirm,
				EditorUI::EditorUIService::s_IconConfirm,
				"Confirm",
				"Confirm",
				PositionType::Relative
		};

		s_LargeSearchButton = {
				-112.0f,
				-0.6f,
				28.0f,
				EditorUI::EditorUIService::s_IconCancel2,
				EditorUI::EditorUIService::s_IconSearch,
				"Cancel Search",
				"Search",
				PositionType::Relative
		};

		InitializeTableResources();
		InitializeCheckboxResources();

		s_Running = true;

		KG_VERIFY(s_Running && ImGui::GetCurrentContext(), "Editor UI Initiated")
	}

	void EditorUIService::Terminate()
	{
		if (s_Running)
		{
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			s_Running = false;
		}

		s_IconPlay.reset();
		s_IconPause.reset();
		s_IconStop.reset();
		s_IconStep.reset();
		s_IconSimulate.reset();
		s_IconGrid.reset();
		s_IconAddItem.reset();
		s_IconDisplay.reset();
		s_IconDisplayActive.reset();
		s_IconCamera.reset();
		s_IconCameraActive.reset();
		s_IconPlayActive.reset();
		s_IconStopActive.reset();
		s_IconPauseActive.reset();
		s_IconStepActive.reset();
		s_IconSimulateActive.reset();
		s_IconSettings.reset();
		s_IconDelete.reset();
		s_IconDeleteActive.reset();
		s_IconEdit.reset();
		s_IconEdit_Active.reset();
		s_IconCancel.reset();
		s_IconCancel2.reset();
		s_IconConfirm.reset();
		s_IconSearch.reset();
		s_IconCheckbox_Empty_Disabled.reset();
		s_IconCheckbox_Check_Disabled.reset();
		s_IconCheckbox_Empty_Enabled.reset();
		s_IconCheckbox_Check_Enabled.reset();
		s_IconOptions.reset();
		s_IconDown.reset();
		s_IconRight.reset();
		s_IconDash.reset();

		s_DirectoryIcon.reset();
		s_GenericFileIcon.reset();
		s_BackIcon.reset();
		s_BackInactiveIcon.reset();
		s_ForwardIcon.reset();
		s_ForwardInactiveIcon.reset();
		s_AudioIcon.reset();
		s_ImageIcon.reset();
		s_BinaryIcon.reset();
		s_SceneIcon.reset();
		s_RegistryIcon.reset();
		s_ScriptProjectIcon.reset();
		s_UserInterfaceIcon.reset();
		s_FontIcon.reset();
		s_InputIcon.reset();
		s_IconEntity.reset();

		s_SmallEditButton = {};
		s_SmallExpandButton = {};
		s_SmallOptionsButton = {};
		s_SmallCheckboxButton = {};
		s_SmallLinkButton = {};
		s_LargeDeleteButton = {};
		s_LargeCancelButton = {};
		s_LargeConfirmButton = {};
		s_LargeSearchButton = {};

		s_TableEditButton = {};
		s_TableLinkButton = {};
		s_TableExpandButton = {};
		s_CheckboxDisabledButton = {};

		KG_VERIFY(!s_Running && !ImGui::GetCurrentContext(), "Editor UI Terminated")
	}

	void EditorUIService::StartRendering()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void EditorUIService::EndRendering()
	{
		ImGuiIO& io = ImGui::GetIO();
		Engine& app = EngineService::GetActiveEngine();
		io.DisplaySize = ImVec2(static_cast<float>(app.GetWindow().GetWidth()), static_cast<float>(app.GetWindow().GetHeight()));
		if (s_DisableLeftClick)
		{
			ImGui::GetIO().MouseClicked[0] = false;
			s_DisableLeftClick = false;
		}
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

	void EditorUIService::StartWindow(const std::string& label, bool* closeWindow, int32_t flags)
	{
		ImGui::Begin(label.c_str(), closeWindow, flags);
	}

	void EditorUIService::EndWindow()
	{
		ImGui::End();
	}

	uint32_t EditorUIService::GetActiveWidgetID()
	{
		return GImGui->ActiveId;
	}

	std::string EditorUIService::GetFocusedWindowName()
	{
		if (GImGui->NavWindow)
		{
			return GImGui->NavWindow->Name;
		}
		return {};
	}

	void EditorUIService::SetFocusedWindow(const std::string& windowName)
	{
		ImGui::SetWindowFocus(windowName.c_str());
	}

	void EditorUIService::BringWindowToFront(const std::string& windowName)
	{
		ImGui::BringWindowToFront(windowName.c_str());
	}

	void EditorUIService::ClearWindowFocus()
	{
		ImGui::FocusWindow(NULL);
	}

	void EditorUIService::HighlightFocusedWindow()
	{
		ImGuiWindow* window = GImGui->NavWindow;
		if (window)
		{
			ImVec2 windowPos = window->Pos;
			ImVec2 windowSize = window->Size;
			ImGui::GetForegroundDrawList(window)->AddRect(windowPos,
				ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y), IM_COL32(193, 249, 255, 132));
		}
	}

	void EditorUIService::SetDisableLeftClick(bool option)
	{
		s_DisableLeftClick = option;
	}

	void EditorUIService::BlockMouseEvents(bool block)
	{
		s_BlockMouseEvents = block;
	}

	void EditorUIService::OnEvent(Events::Event& e)
	{
		KG_PROFILE_FUNCTION();
		ImGuiIO& io = ImGui::GetIO();
		if (!s_BlockMouseEvents)
		{
			e.Handled |= e.IsInCategory(Events::Mouse) & io.WantCaptureMouse;
		}
		e.Handled |= e.IsInCategory(Events::Keyboard) & io.WantCaptureKeyboard;
	}

	uint32_t WidgetIterator(uint32_t& count)
	{
		return ++count;
	}

	void EditorUIService::Spacing(float space)
	{
		ImGui::Dummy(ImVec2(0.0f, space));
	}

	void EditorUIService::TitleText(const std::string& text)
	{
		ImGui::PushFont(EditorUI::EditorUIService::s_PlexBold);
		ImGui::TextColored(s_PearlBlue, text.c_str());
		ImGui::PopFont();
	}

	void EditorUIService::Spacing(SpacingAmount space)
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

	static void CreateImage(Ref<Rendering::Texture2D> image, float size)
	{
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 4.0f, ImGui::GetCursorPosY() + 3.2f));

		ImGui::Image((ImTextureID)(uint64_t)image->GetRendererID(), ImVec2(size, size),
			ImVec2{ 0, 1 }, ImVec2(1, 0), EditorUIService::s_PureWhite,
			EditorUIService::s_PureEmpty);
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
		ImGui::PushStyleColor(ImGuiCol_Button, EditorUIService::s_PureEmpty);
		if (spec.Disabled)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorUIService::s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, EditorUIService::s_PureEmpty);
		}
		Ref<Rendering::Texture2D> iconChoice = active ? spec.ActiveIcon : spec.InactiveIcon;
		if (ImGui::ImageButtonEx(widgetID,
			(ImTextureID)(uint64_t)iconChoice->GetRendererID(),
			ImVec2(spec.IconSize, spec.IconSize), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
			EditorUIService::s_PureEmpty,
			EditorUI::EditorUIService::s_PureWhite, 0))
		{
			if (onPress)
			{
				onPress();
			}
		}
		ImGui::PopStyleColor(spec.Disabled ? 3 : 1);

		if (!spec.Disabled)
		{
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_PearlBlue, active ? spec.ActiveTooltip.c_str() : spec.InactiveTooltip.c_str());
				ImGui::EndTooltip();
			}
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
			ImGui::TextColored(EditorUI::EditorUIService::s_PureWhite, outputText.c_str());
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_PearlBlue, text.c_str());
				ImGui::EndTooltip();
			}
		}
		else
		{
			ImGui::Text(text.c_str());
		}
	}

	void EditorUIService::GenericPopup(GenericPopupSpec& spec)
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
				spec.PopupAction();
			}
		}

		// Display Popup
		ImGui::SetNextWindowSize(ImVec2(spec.PopupWidth, 0.0f));
		if (ImGui::BeginPopupModal(id.c_str(), NULL, ImGuiWindowFlags_NoTitleBar))
		{
			EditorUI::EditorUIService::TitleText(spec.Label);

			ImGui::PushFont(EditorUI::EditorUIService::s_AntaRegular);
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

			if (spec.PopupContents)
			{
				spec.PopupContents();
			}

			ImGui::PopFont();
			ImGui::EndPopup();
		}
	}

	void EditorUIService::NewItemScreen(const std::string& label1, std::function<void()> onPress1, const std::string& label2, std::function<void()> onPress2)
	{
		ImGui::PushFont(EditorUI::EditorUIService::s_AntaLarge);
		ImVec2 screenDimensions = ImGui::GetContentRegionAvail();
		ImVec2 originalLocation = ImGui::GetCursorScreenPos();
		if (screenDimensions.x > 700.0f)
		{
			ImVec2 buttonDimensions {2.75f, 5.0f};
			ImVec2 screenLocation = ImVec2(originalLocation.x + screenDimensions.x / 2 - (screenDimensions.x / buttonDimensions.x), originalLocation.y + screenDimensions.y / 2 - (screenDimensions.y / (buttonDimensions.y * 2)));
			ImGui::SetCursorScreenPos(screenLocation);
			if (ImGui::Button(label1.c_str(), ImVec2(screenDimensions.x / buttonDimensions.x, screenDimensions.y / buttonDimensions.y)))
			{
				if (onPress1)
				{
					onPress1();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button(label2.c_str(), ImVec2(screenDimensions.x / buttonDimensions.x, screenDimensions.y / buttonDimensions.y)))
			{
				if (onPress2)
				{
					onPress2();
				}
				
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
			EditorUIService::Spacing(SpacingAmount::Small);
			ImGui::SetCursorScreenPos(ImVec2(screenLocation.x, ImGui::GetCursorScreenPos().y));
			if (ImGui::Button(label2.c_str(), ImVec2(screenDimensions.x / buttonDimensions.x, screenDimensions.y / buttonDimensions.y)))
			{
				onPress2();
			}
		}
		ImGui::PopFont();
	}


	void EditorUIService::SelectOption(SelectOptionSpec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };

		if (spec.Flags & SelectOption_PopupOnly)
		{
			if (spec.PopupActive)
			{
				ImGui::OpenPopup(id.c_str());
				spec.PopupActive = false;
				if (spec.PopupAction)
				{
					spec.PopupAction();
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
					spec.PopupAction();
				}
				spec.CachedSelection = spec.CurrentOption;
			},
			EditorUIService::s_SmallEditButton);

		}
		
		// Display Popup
		ImGui::SetNextWindowSize(ImVec2(700.0f, 0.0f));
		if (ImGui::BeginPopupModal(id.c_str(), NULL, ImGuiWindowFlags_NoTitleBar))
		{
			static char searchBuffer[256];

			EditorUI::EditorUIService::TitleText(spec.Label);

			ImGui::PushFont(EditorUI::EditorUIService::s_AntaRegular);
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

				ImGui::PushStyleColor(ImGuiCol_FrameBg, s_LightPurple_Thin);
				ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, s_LightPurple_Thin);
				ImGui::PushStyleColor(ImGuiCol_FrameBgActive, s_LightPurple_Thin);

				ImGui::InputText((id + "InputText").c_str(), searchBuffer, sizeof(searchBuffer), ImGuiInputTextFlags_CallbackEdit, callback, (void*)&spec);
				ImGui::PopStyleColor(3);
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

					if (ImGui::Button((option.Label + id + std::string(option.Handle)).c_str()))
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
				EditorUIService::Spacing(SpacingAmount::Medium);
			}

			ImGui::PopFont();
			ImGui::EndPopup();
		}
	}

	void EditorUIService::EditVariable(EditVariableSpec& spec)
	{
		// Local Variables
		uint32_t widgetCount{ 0 };
		std::string id = "##" + std::to_string(spec.WidgetID);
		static ImGuiInputTextFlags inputFlags {};

		ImGui::TextColored(s_PureWhite, spec.Label.c_str());
		switch (spec.VariableType)
		{
			case WrappedVarType::None:
			case WrappedVarType::Void:
			{
				KG_WARN("Invalid Type used for Edit Variable");
				return;
			}

			case WrappedVarType::Bool:
			{
				ImGui::SameLine(200.0f);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, s_PureEmpty);
				ImGui::PushStyleColor(ImGuiCol_Button, s_PureEmpty);
				TruncateText("True", 12);
				ImGui::SameLine();
				CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
					{
						if (spec.FieldBuffer.As<char>() == "True")
						{
							spec.FieldBuffer.SetString("False");
						}
						else
						{
							spec.FieldBuffer.SetString("True");
						}
					}, s_SmallCheckboxButton, std::string(spec.FieldBuffer.As<char>()) == "True");

				ImGui::SameLine(300.0f);
				TruncateText("False", 12);
				ImGui::SameLine();
				CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
					{
						if (spec.FieldBuffer.As<char>() == "False")
						{
							spec.FieldBuffer.SetString("True");
						}
						else
						{
							spec.FieldBuffer.SetString("False");
						}
					}, s_SmallCheckboxButton, std::string(spec.FieldBuffer.As<char>()) == "False");
				ImGui::PopStyleColor(2);
				break;
			}
			case WrappedVarType::String:
			{
				inputFlags = ImGuiInputTextFlags_CallbackEdit;
				ImGui::PushStyleColor(ImGuiCol_Text, s_PearlBlue);
				ImGui::SameLine();
				ImGui::SetCursorPosX(200.0f);
				ImGui::SetNextItemWidth(170.0f);
				ImGuiInputTextCallback typeCallback = [](ImGuiInputTextCallbackData* data)
				{
					return 0;
				};
				ImGui::InputText(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(),
					spec.FieldBuffer.As<char>(), spec.FieldBuffer.Size, inputFlags, typeCallback);
				ImGui::PopStyleColor();
				break;
			}
			case WrappedVarType::Float:
			case WrappedVarType::Integer32:
			case WrappedVarType::UInteger16:
			case WrappedVarType::UInteger32:
			case WrappedVarType::UInteger64:
			default:
			{
				inputFlags = ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CharsDecimal;
				ImGui::PushStyleColor(ImGuiCol_Text, s_PearlBlue);
				ImGui::SameLine();
				ImGui::SetCursorPosX(200.0f);
				ImGui::SetNextItemWidth(170.0f);
				ImGuiInputTextCallback typeCallback = [](ImGuiInputTextCallbackData* data)
				{
					return 0;
				};
				ImGui::InputText(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(),
					spec.FieldBuffer.As<char>(), spec.FieldBuffer.Size, inputFlags, typeCallback);
				ImGui::PopStyleColor();
				break;
			}
		}
		
	}

	void EditorUIService::Checkbox(CheckboxSpec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };

		if (spec.Flags & Checkbox_Indented)
		{
			ImGui::SetCursorPosX(30.5f);
		}

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
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorUIService::s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, EditorUIService::s_PureEmpty);
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{

				if (spec.ToggleBoolean)
				{
					spec.ToggleBoolean = false;
					if (!spec.ConfirmAction)
					{
						return;
					}
					spec.ConfirmAction(spec.ToggleBoolean);
				}
				else
				{
					spec.ToggleBoolean = true;
					if (!spec.ConfirmAction)
					{
						return;
					}
					spec.ConfirmAction(spec.ToggleBoolean);
				}
			}, s_SmallCheckboxButton, spec.ToggleBoolean);
			ImGui::PopStyleColor(2);
		}
		else
		{
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), nullptr,
			s_CheckboxDisabledButton,
			spec.ToggleBoolean);
		}

		ImGui::SameLine();
		CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.Editing);
		},
		EditorUIService::s_SmallEditButton,
		spec.Editing);
	}

	void EditorUIService::EditFloat(EditFloatSpec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };
		// Display Item
		if (spec.Flags & EditFloat_Indented)
		{
			ImGui::SetCursorPosX(30.5f);
		}
		TruncateText(spec.Label, 20);
		ImGui::SameLine(197.5f);

		if (spec.Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, s_PearlBlue);
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetNextItemWidth(50.0f);
			if (ImGui::DragFloat(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.CurrentFloat), 0.5f,
				0.0f, 0.0f,
				"%.2f"))
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
			}
			ImGui::PopStyleColor();

			ImGui::PopStyleVar();

		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetCursorPos({ 197.5f, yPosition });
			ImGui::Text(Utility::Conversions::FloatToString(spec.CurrentFloat).c_str());
		}

		ImGui::SameLine();
		CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				Utility::Operations::ToggleBoolean(spec.Editing);
			},
			EditorUIService::s_SmallEditButton,
			spec.Editing);
	}

	void EditorUIService::EditVec2(EditVec2Spec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };
		// Display Item
		if (spec.Flags & EditVec2_Indented)
		{
			ImGui::SetCursorPosX(30.5f);
		}
		TruncateText(spec.Label, 20);
		ImGui::SameLine(197.5f);

		if (spec.Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, s_PearlBlue);
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetNextItemWidth(50.0f);
			if (ImGui::DragFloat(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.CurrentVec2.x), 0.5f,
				0.0f, 0.0f,
				"%.2f"))
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
			}
			ImGui::PopStyleColor();

			// y value
			ImGui::PushStyleColor(ImGuiCol_Text, s_LightPurple);
			ImGui::SetCursorPos({ 260.0f, yPosition });
			ImGui::SetNextItemWidth(50.0f);
			if (ImGui::DragFloat(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.CurrentVec2.y), 0.5f,
				0.0f, 0.0f,
				"%.2f"))
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
			}
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();

		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetCursorPos({ 197.5f, yPosition });
			ImGui::Text(Utility::Conversions::FloatToString(spec.CurrentVec2.x).c_str());
			ImGui::SetCursorPos({ 260.0f, yPosition });
			ImGui::Text(Utility::Conversions::FloatToString(spec.CurrentVec2.y).c_str());
		}

		ImGui::SameLine();
		CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				Utility::Operations::ToggleBoolean(spec.Editing);
			},
			EditorUIService::s_SmallEditButton,
			spec.Editing);
	}

	void EditorUIService::EditVec3(EditVec3Spec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };
		// Display Item
		if (spec.Flags & EditVec3_Indented)
		{
			ImGui::SetCursorPosX(30.5f);
		}
		TruncateText(spec.Label, 20);
		ImGui::SameLine(197.5f);
		
		if (spec.Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, s_PearlBlue);
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetNextItemWidth(50.0f);
			if (ImGui::DragFloat(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.CurrentVec3.x), 0.5f,
				0.0f, 0.0f,
				"%.2f"))
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
			}
			ImGui::PopStyleColor();

			// y value
			ImGui::PushStyleColor(ImGuiCol_Text, s_LightPurple);
			ImGui::SetCursorPos({ 260.0f, yPosition });
			ImGui::SetNextItemWidth(50.0f);
			if (ImGui::DragFloat(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.CurrentVec3.y), 0.5f,
				0.0f, 0.0f,
				"%.2f"))
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
			}
			ImGui::PopStyleColor();

			// z value
			ImGui::PushStyleColor(ImGuiCol_Text, s_LightGreen);
			ImGui::SetCursorPos({ 320.0f, yPosition });
			ImGui::SetNextItemWidth(50.0f);
			if (ImGui::DragFloat(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.CurrentVec3.z), 0.5f,
				0.0f, 0.0f,
				"%.2f"))
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
			}
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			
		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetCursorPos({ 197.5f, yPosition });
			ImGui::Text(Utility::Conversions::FloatToString(spec.CurrentVec3.x).c_str());
			ImGui::SetCursorPos({ 260.0f, yPosition });
			ImGui::Text(Utility::Conversions::FloatToString(spec.CurrentVec3.y).c_str());
			ImGui::SetCursorPos({ 320.0f, yPosition });
			ImGui::Text(Utility::Conversions::FloatToString(spec.CurrentVec3.z).c_str());
		}

		ImGui::SameLine();
		CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.Editing);
		},
		EditorUIService::s_SmallEditButton,
		spec.Editing);
	}

	void EditorUIService::RadioSelector(RadioSelectorSpec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };

		if (spec.Flags & RadioSelector_Indented)
		{
			ImGui::SetCursorPosX(30.5f);
		}

		// Display Item
		TruncateText(spec.Label, 23);
		ImGui::SameLine(197.5f);

		if (spec.Editing)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, s_PureEmpty);
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
				spec.SelectAction();
			}, s_SmallCheckboxButton, spec.SelectedOption == 0);
			ImGui::SameLine();
			TruncateText(spec.FirstOptionLabel, 7);

			ImGui::SameLine(300.0f);
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
				spec.SelectAction();
			}, s_SmallCheckboxButton, spec.SelectedOption == 1);
			ImGui::SameLine();
			TruncateText(spec.SecondOptionLabel, 7);
			ImGui::PopStyleColor(2);
		}
		else
		{
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), nullptr,
				s_CheckboxDisabledButton, spec.SelectedOption == 0);
			ImGui::SameLine();
			TruncateText(spec.FirstOptionLabel, 7);

			ImGui::SameLine(300.0f);
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), nullptr,
				s_CheckboxDisabledButton, spec.SelectedOption == 1);
			ImGui::SameLine();
			TruncateText(spec.SecondOptionLabel, 7);
		}

		ImGui::SameLine();
		CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.Editing);
		},
		EditorUIService::s_SmallEditButton,
		spec.Editing);
	}

	void EditorUIService::Table(TableSpec& spec)
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
			ImGui::PushFont(EditorUIService::s_AntaLarge);
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

	static void DrawEntries(TreeSpec& spec , std::vector<TreeEntry>& entries, uint32_t& widgetCount, uint32_t depth, ImVec2 rootPosition)
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition{};
		for (auto& treeEntry : entries)
		{
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (depth * 30.0f));
			screenPosition = ImGui::GetCursorScreenPos();

			if (spec.SelectedEntry == &treeEntry)
			{
				// Draw SelectedEntry background
				draw_list->AddRectFilled(screenPosition,
					ImVec2(screenPosition.x + ImGui::CalcTextSize(treeEntry.Label.c_str()).x + 34.0f, screenPosition.y + 21.0f),
					ImColor(EditorUI::EditorUIService::s_LightPurple_Thin), 4, ImDrawFlags_RoundCornersAll);
			}

			if (treeEntry.IconHandle)
			{
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorUIService::s_PureEmpty);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, EditorUIService::s_PureEmpty);
				ImGui::PushStyleColor(ImGuiCol_Button, EditorUIService::s_PureEmpty);
				if (ImGui::ImageButtonEx(spec.WidgetID + WidgetIterator(widgetCount),
					(ImTextureID)(uint64_t)treeEntry.IconHandle->GetRendererID(),
					ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
					EditorUIService::s_PureEmpty,
					EditorUI::EditorUIService::s_PureWhite, 0))
				{
					if (treeEntry.OnClick)
					{
						treeEntry.OnClick(treeEntry);
					}
					spec.SelectedEntry = &treeEntry;
				}
				ImGui::PopStyleColor(3);
				ImGui::SameLine();
			}

			ImGui::Text(treeEntry.Label.c_str());

			if (ImGui::IsItemClicked())
			{
				if (treeEntry.OnClick)
				{
					treeEntry.OnClick(treeEntry);
				}
				spec.SelectedEntry = &treeEntry;
			}

			if (treeEntry.SubEntries.size() > 0)
			{
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Button, EditorUIService::s_PureEmpty);
				const Ref<Rendering::Texture2D> icon = treeEntry.Expanded ? EditorUIService::s_IconDown : EditorUIService::s_IconRight;
				if (ImGui::ImageButtonEx(spec.WidgetID + WidgetIterator(widgetCount),
					(ImTextureID)(uint64_t)icon->GetRendererID(),
					ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
					EditorUIService::s_PureEmpty,
					EditorUI::EditorUIService::s_PureWhite, 0))
				{
					Utility::Operations::ToggleBoolean(treeEntry.Expanded);
				}
				ImGui::PopStyleColor();

				if (treeEntry.Expanded)
				{
					/*draw_list->AddLine(ImVec2(screenPosition.x + 10.0f, screenPosition.y + 21.0f),
						ImVec2(screenPosition.x + 10.0f, screenPosition.y + 36.0f + (24.0f * (treeEntry.SubEntries.size() - 1))),
						ImColor(EditorUIService::s_PureWhite));*/
					DrawEntries(spec, treeEntry.SubEntries, widgetCount, depth + 1, screenPosition);
				}
			}

			if (depth > 0)
			{
				draw_list->AddLine(ImVec2(rootPosition.x + 10.0f, screenPosition.y + 10.0f),
					ImVec2(screenPosition.x, screenPosition.y + 10.0f),
					ImColor(EditorUIService::s_PureWhite));
			}
		}

		if (depth > 0)
		{
			draw_list->AddLine(ImVec2(rootPosition.x + 10.0f, rootPosition.y + 21.0f),
				ImVec2(rootPosition.x + 10.0f, screenPosition.y + 10.0f),
				ImColor(EditorUIService::s_PureWhite));
		}
	}

	void EditorUIService::Tree(TreeSpec& spec)
	{
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };

		DrawEntries(spec, spec.TreeEntries, widgetCount, 0);
	}

	void EditorUIService::PanelHeader(PanelHeaderSpec& spec)
	{
		std::string id = "##" + std::to_string(spec.WidgetID);
		ImGui::PushFont(EditorUIService::s_AntaLarge);
		ImGui::TextColored(spec.EditColorActive ? EditorUIService::s_PearlBlue : EditorUIService::s_PureWhite , spec.Label.c_str());
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
		ImGui::Separator(1.0f, s_PearlBlue_Thin);
		Spacing(0.2f);
	}

	void EditorUIService::CollapsingHeader(CollapsingHeaderSpec& spec)
	{
		uint32_t widgetCount{ 0 };
		std::string id = "##" + std::to_string(spec.WidgetID);
		ImGui::PushFont(EditorUIService::s_AntaLarge);
		ImGui::Text(spec.Label.c_str());
		ImGui::PopFont();
		ImGui::SameLine();
		CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.Expanded);
		},
		s_SmallExpandButton, spec.Expanded);

		if (spec.Expanded && !spec.SelectionList.empty())
		{
			ImGui::SameLine();
			CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
				{
					ImGui::OpenPopup(spec.WidgetID - 1);
				}, s_SmallOptionsButton);

			if (ImGui::BeginPopupEx(spec.WidgetID - 1, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
			{
				for (auto& [label, func] : spec.SelectionList)
				{
					if (ImGui::Selectable((label + id).c_str()))
					{
						func();
					}
				}
				ImGui::EndPopup();
			}
		}

		if ((spec.Flags & CollapsingHeader_UnderlineTitle) && spec.Expanded)
		{
			ImGui::Separator();
		}

		if (spec.Expanded && spec.OnExpand)
		{
			spec.OnExpand();
		}
	}

	void EditorUIService::LabeledText(const std::string& label, const std::string& text)
	{
		// Display Menu Item
		TruncateText(label, 23);
		//ImGui::TextColored(s_PureWhite, label.c_str());
		ImGui::PushStyleColor(ImGuiCol_Text, s_PearlBlue);
		WriteMultilineText(text, 200.0f, 23);
		ImGui::PopStyleColor();
	}
	void EditorUIService::Text(const std::string& Text)
	{
		ImGui::Text(Text.c_str());
	}
	void EditorUIService::TextInputPopup(TextInputSpec& spec)
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
			EditorUIService::s_SmallEditButton);
		}

		ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f));
		if (ImGui::BeginPopupModal(id.c_str(), NULL, ImGuiWindowFlags_NoTitleBar))
		{
			EditorUI::EditorUIService::TitleText(popUpLabel);

			ImGui::PushFont(EditorUI::EditorUIService::s_AntaRegular);

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
					spec.ConfirmAction();
				}
				memset(stringBuffer, 0, sizeof(stringBuffer));
				ImGui::CloseCurrentPopup();
			}, s_LargeConfirmButton);

			ImGui::Separator();

			ImGui::SetNextItemWidth(583.0f);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, s_LightPurple_Thin);
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, s_LightPurple_Thin);
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, s_LightPurple_Thin);
			ImGui::InputText((id + "InputText").c_str(), stringBuffer, sizeof(stringBuffer));
			ImGui::PopStyleColor(3);
			ImGui::PopFont();
			ImGui::EndPopup();
		}
	}
	void EditorUIService::ChooseDirectory(ChooseDirectorySpec& spec)
	{
		// Local Variables
		uint32_t widgetCount{ 0 };
		std::string id = "##" + std::to_string(spec.WidgetID);
		std::string popUpLabel = spec.Label;

		ImGui::TextColored(s_PureWhite, spec.Label.c_str());
		ImGui::PushStyleColor(ImGuiCol_Text, s_PearlBlue);
		WriteMultilineText(spec.CurrentOption.string(), 200.0f, 21);
		ImGui::PopStyleColor();

		ImGui::SameLine();
		CreateInlineButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			const std::filesystem::path initialDirectory = spec.CurrentOption.empty() ? std::filesystem::current_path() : spec.CurrentOption;
			std::filesystem::path outputDirectory = Utility::FileDialogs::ChooseDirectory(initialDirectory);
			if (outputDirectory.empty())
			{
				KG_WARN("Empty path returned to ChooseDirectory");
				return;
			}
			spec.CurrentOption = outputDirectory;
		},
		EditorUIService::s_SmallEditButton);

	}
	void EditorUIService::BeginTabBar(const std::string& title)
	{
		ImGui::BeginTabBar(title.c_str());
	}
	void EditorUIService::EndTabBar()
	{
		ImGui::EndTabBar();
	}
	bool EditorUIService::BeginTabItem(const std::string& title)
	{
		return ImGui::BeginTabItem(title.c_str());
	}
	void EditorUIService::EndTabItem()
	{
		ImGui::EndTabItem();
	}
}
