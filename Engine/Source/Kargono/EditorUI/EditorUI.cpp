#include "kgpch.h"

#include "Kargono/EditorUI/EditorUI.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/Utility/Operations.h"
#include "Kargono/Input/InputService.h"
#include "Kargono/Input/InputMode.h"
#include "Kargono/Utility/FileDialogs.h"

#include "API/EditorUI/ImGuiBackendAPI.h"
#include "API/Platform/GlfwAPI.h"
#include "API/Platform/gladAPI.h"


namespace Kargono::EditorUI
{
	ImFont* EditorUIService::s_FontAntaLarge{ nullptr };
	ImFont* EditorUIService::s_FontAntaRegular{ nullptr };
	ImFont* EditorUIService::s_FontPlexBold{ nullptr };
	ImFont* EditorUIService::s_FontRobotoMono{ nullptr };

	Ref<Rendering::Texture2D> EditorUIService::s_IconCamera{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconSettings{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconDelete{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconEdit{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconCancel{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconCancel2{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconConfirm{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconSearch{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconCheckbox_Disabled{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconCheckbox_Enabled{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconOptions{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconDown{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconRight{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconDash{};

	Ref<Rendering::Texture2D> EditorUIService::s_IconPlay{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconPause{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconStop{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconGrid{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconStep{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconSimulate{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconDisplay{};

	Ref<Rendering::Texture2D> EditorUIService::s_IconEntity{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconBoxCollider{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconCircleCollider{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconClassInstance{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconRigidBody{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconTag{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconTransform{};

	Ref<Rendering::Texture2D> EditorUIService::s_IconDirectory{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconGenericFile{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconBack{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconForward{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconAudio{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconImage{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconBinary{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconScene{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconRegistry{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconScriptProject{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconUserInterface{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconFont{};
	Ref<Rendering::Texture2D> EditorUIService::s_IconInput{};


	void EditorUIService::SetColorDefaults()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = EditorUIService::s_BackgroundColor;
		colors[ImGuiCol_PopupBg] = EditorUIService::s_DarkBackgroundColor;

		// TextColor
		colors[ImGuiCol_Text] = EditorUIService::s_PrimaryTextColor;

		// Separator
		colors[ImGuiCol_Separator] = EditorUIService::s_AccentColor;
		colors[ImGuiCol_SeparatorActive] = EditorUIService::s_HighlightColor1_Thin;
		colors[ImGuiCol_SeparatorHovered] = EditorUIService::s_HighlightColor1_Thin;

		// Drag Drop
		colors[ImGuiCol_DragDropTarget] = EditorUIService::s_HighlightColor1_Thin;

		// Scroll Bar
		colors[ImGuiCol_ScrollbarBg] = EditorUIService::s_DarkBackgroundColor;
		colors[ImGuiCol_ScrollbarGrab] = EditorUIService::s_DisabledColor;
		colors[ImGuiCol_ScrollbarGrabActive] = EditorUIService::s_SelectedColor;
		colors[ImGuiCol_ScrollbarGrabHovered] = EditorUIService::s_SelectedColor;

		// Headers
		colors[ImGuiCol_Header] = EditorUIService::s_AccentColor;
		colors[ImGuiCol_HeaderHovered] = EditorUIService::s_HoveredColor;
		colors[ImGuiCol_HeaderActive] = EditorUIService::s_ActiveColor;

		// Buttons
		colors[ImGuiCol_Button] = EditorUIService::s_AccentColor;
		colors[ImGuiCol_ButtonHovered] = EditorUIService::s_HoveredColor;
		colors[ImGuiCol_ButtonActive] = EditorUIService::s_ActiveColor;

		// Border
		colors[ImGuiCol_Border] = EditorUIService::s_PureEmpty;
		colors[ImGuiCol_BorderShadow] = EditorUIService::s_PureEmpty;

		// Frame BG
		colors[ImGuiCol_FrameBg] = EditorUIService::s_PureEmpty;
		colors[ImGuiCol_FrameBgHovered] = EditorUIService::s_PureEmpty;
		colors[ImGuiCol_FrameBgActive] = EditorUIService::s_PureEmpty;

		// Tabs
		colors[ImGuiCol_Tab] = EditorUIService::s_DarkAccentColor;
		colors[ImGuiCol_TabActive] = EditorUIService::s_SelectedColor;
		colors[ImGuiCol_TabUnfocused] = EditorUIService::s_DarkAccentColor;
		colors[ImGuiCol_TabUnfocusedActive] = EditorUIService::s_ActiveColor;
		colors[ImGuiCol_TabHovered] = EditorUIService::s_HoveredColor;

		// Title
		colors[ImGuiCol_TitleBg] = EditorUIService::s_DarkBackgroundColor;
		colors[ImGuiCol_TitleBgActive] = EditorUIService::s_DarkBackgroundColor;
		colors[ImGuiCol_TitleBgCollapsed] = EditorUIService::s_DarkBackgroundColor;

		// Menu Bar
		colors[ImGuiCol_MenuBarBg] = EditorUIService::s_DarkBackgroundColor;
	}

	static InlineButtonSpec s_TableEditButton {};
	static InlineButtonSpec s_TableLinkButton {};
	static InlineButtonSpec s_TableExpandButton {};

	void EditorUIService::SetButtonDefaults()
	{
		s_SmallEditButton =
		{
				-s_SmallButtonRightOffset,
				0.0f,
				13.0f,
				EditorUI::EditorUIService::s_IconEdit,
				EditorUI::EditorUIService::s_IconEdit,
				"Cancel Editing",
				"Edit",
				PositionType::Relative
		};

		s_SmallExpandButton =
		{
				-1.2f,
				4.0f,
				14.0f,
				EditorUI::EditorUIService::s_IconDown,
				EditorUI::EditorUIService::s_IconRight,
				"Collapse",
				"Expand",
				PositionType::Inline
		};

		s_MediumOptionsButton =
		{
				-s_MediumButtonRightOffset,
				1.0f,
				19.0f,
				EditorUI::EditorUIService::s_IconOptions,
				EditorUI::EditorUIService::s_IconOptions,
				"Options",
				"Options",
				PositionType::Relative
		};

		s_SmallCheckboxButton = {
				0.0f,
				0.0f,
				14.0f,
				EditorUI::EditorUIService::s_IconCheckbox_Enabled,
				EditorUI::EditorUIService::s_IconCheckbox_Disabled,
				"Uncheck",
				"Check",
				PositionType::Inline
		};

		s_SmallCheckboxDisabledButton = {
				0.0f,
				0.0f,
				14.0f,
				EditorUI::EditorUIService::s_IconCheckbox_Enabled,
				EditorUI::EditorUIService::s_IconCheckbox_Disabled,
				"",
				"",
				PositionType::Inline,
				true
		};

		s_SmallLinkButton = {
				-s_SmallButtonRightOffset,
				0.0f,
				14.0f,
				EditorUI::EditorUIService::s_IconForward,
				EditorUI::EditorUIService::s_IconForward,
				"Open",
				"Open",
				PositionType::Relative
		};

		s_LargeDeleteButton = {
				-112.0f,
				-0.6f,
				28.0f,
				EditorUI::EditorUIService::s_IconDelete,
				EditorUI::EditorUIService::s_IconDelete,
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

		s_TableEditButton = EditorUIService::s_SmallEditButton;
		s_TableEditButton.YPosition = -5.5f;

		s_TableLinkButton = EditorUIService::s_SmallLinkButton;
		s_TableLinkButton.YPosition = -5.5f;

		s_TableExpandButton = EditorUIService::s_SmallExpandButton;

		
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

		s_FontAntaLarge = io.Fonts->AddFontFromFileTTF("Resources/Fonts/Anta-Regular.ttf", 23.0f);
		s_FontAntaRegular = io.Fonts->AddFontFromFileTTF("Resources/Fonts/Anta-Regular.ttf", 20.0f);
		s_FontPlexBold = io.Fonts->AddFontFromFileTTF("Resources/Fonts/IBMPlexMono-Bold.ttf", 29.0f);
		s_FontRobotoMono = io.Fonts->AddFontFromFileTTF("Resources/Fonts/RobotoMono-SemiBold.ttf", 17.0f);
		io.FontDefault = s_FontRobotoMono;

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
		style.WindowPadding = { 7.0f, 4.0f };
		SetColorDefaults();

		// Setup Platform/Renderer backends
		Engine& core = EngineService::GetActiveEngine();
		GLFWwindow* window = static_cast<GLFWwindow*>(core.GetWindow().GetNativeWindow());
		KG_ASSERT(window, "No window active when initializing EditorUI");
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		// Set Up Editor Resources
		s_IconCamera = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Camera.png").string());
		s_IconSettings = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Settings.png").string());
		s_IconDelete = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Delete.png").string());
		s_IconEdit = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Edit.png").string());
		s_IconCancel = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Cancel.png").string());
		s_IconCancel2 = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Cancel2.png").string());
		s_IconConfirm = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Confirm.png").string());
		s_IconSearch = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Search.png").string());
		s_IconOptions = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Options.png").string());
		s_IconDown = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Down.png").string());
		s_IconRight = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Right.png").string());
		s_IconCheckbox_Enabled = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/CheckboxEnabled.png").string());
		s_IconCheckbox_Disabled = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/CheckboxDisabled.png").string());
		s_IconDash = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Dash.png").string());

		s_IconGrid = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Viewport/Grid.png").string());
		s_IconDisplay = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Viewport/Display.png").string());
		s_IconPlay = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Viewport/Play.png").string());
		s_IconStop = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Viewport/Stop.png").string());
		s_IconPause = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Viewport/Pause.png").string());
		s_IconStep = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Viewport/Step.png").string());
		s_IconSimulate = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Viewport/Simulate.png").string());

		s_IconEntity = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/SceneEditor/Entity.png").string());
		s_IconBoxCollider = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/SceneEditor/BoxCollider.png").string());
		s_IconTag = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/SceneEditor/Tag.png").string());
		s_IconCircleCollider = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/SceneEditor/CircleCollider.png").string());
		s_IconClassInstance = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/SceneEditor/ClassInstance.png").string());
		s_IconRigidBody = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/SceneEditor/Rigidbody.png").string());
		s_IconTransform = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/SceneEditor/Transform.png").string());

		s_IconDirectory = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Directory.png");
		s_IconGenericFile = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/GenericFile.png");
		s_IconBack = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Back.png");
		s_IconForward = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Forward.png");
		s_IconAudio = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Audio.png");
		s_IconImage = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Image.png");
		s_IconBinary = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Binary.png");
		s_IconScene = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Scene.png");
		s_IconRegistry = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Registry.png");
		s_IconScriptProject = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/ScriptProject.png");
		s_IconFont = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Font.png");
		s_IconUserInterface = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/UserInterface.png");
		s_IconInput = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Input.png");

		SetButtonDefaults();

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
		s_IconDisplay.reset();
		s_IconCamera.reset();
		s_IconSettings.reset();
		s_IconDelete.reset();
		s_IconEdit.reset();
		s_IconCancel.reset();
		s_IconCancel2.reset();
		s_IconConfirm.reset();
		s_IconSearch.reset();
		s_IconCheckbox_Enabled.reset();
		s_IconCheckbox_Disabled.reset();
		s_IconOptions.reset();
		s_IconDown.reset();
		s_IconRight.reset();
		s_IconDash.reset();

		s_IconDirectory.reset();
		s_IconGenericFile.reset();
		s_IconBack.reset();
		s_IconForward.reset();
		s_IconAudio.reset();
		s_IconImage.reset();
		s_IconBinary.reset();
		s_IconScene.reset();
		s_IconRegistry.reset();
		s_IconScriptProject.reset();
		s_IconUserInterface.reset();
		s_IconFont.reset();
		s_IconInput.reset();
		s_IconEntity.reset();
		s_IconBoxCollider.reset();
		s_IconCircleCollider.reset();
		s_IconClassInstance.reset();
		s_IconRigidBody.reset();
		s_IconTag.reset();
		s_IconTransform.reset();

		s_SmallEditButton = {};
		s_SmallExpandButton = {};
		s_MediumOptionsButton = {};
		s_SmallCheckboxButton = {};
		s_SmallCheckboxDisabledButton = {};
		s_SmallLinkButton = {};
		s_LargeDeleteButton = {};
		s_LargeCancelButton = {};
		s_LargeConfirmButton = {};
		s_LargeSearchButton = {};

		s_TableEditButton = {};
		s_TableLinkButton = {};
		s_TableExpandButton = {};

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
		// Start Window
		ImGui::Begin(label.c_str(), closeWindow, flags);

		// Calculate Widget Spacing Values
		s_PrimaryTextWidth = (s_SecondaryTextFirstPercentage * ImGui::GetContentRegionMax().x) - 20.0f;
		s_PrimaryTextIndentedWidth = (s_SecondaryTextFirstPercentage * ImGui::GetContentRegionMax().x) - 20.0f - s_TextLeftIndentOffset;
		s_SecondaryTextSmallWidth = ((s_SecondaryTextSecondPercentage - s_SecondaryTextFirstPercentage) * ImGui::GetContentRegionMax().x) - 10.0f;
		s_SecondaryTextMediumWidth = ((s_SecondaryTextMiddlePercentage - s_SecondaryTextFirstPercentage) * ImGui::GetContentRegionMax().x) - 30.0f;
		s_SecondaryTextLargeWidth = ((s_SecondaryTextFourthPercentage - s_SecondaryTextFirstPercentage) * ImGui::GetContentRegionMax().x) + s_SecondaryTextSmallWidth;

		s_SecondaryTextPosOne = ImGui::GetContentRegionMax().x * s_SecondaryTextFirstPercentage;
		s_SecondaryTextPosTwo = ImGui::GetContentRegionMax().x * s_SecondaryTextSecondPercentage;
		s_SecondaryTextPosThree = ImGui::GetContentRegionMax().x * s_SecondaryTextThirdPercentage;
		s_SecondaryTextPosFour = ImGui::GetContentRegionMax().x * s_SecondaryTextFourthPercentage;
		s_SecondaryTextPosMiddle = ImGui::GetContentRegionMax().x * s_SecondaryTextMiddlePercentage;
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

	bool EditorUIService::IsCurrentWindowVisible()
	{
		return ImGui::IsWindowVisible();
	}

	void EditorUIService::HighlightFocusedWindow()
	{
		ImGuiWindow* window = GImGui->NavWindow;
		if (window)
		{
			ImVec2 windowPos = window->Pos;
			ImVec2 windowSize = window->Size;
			ImGui::GetForegroundDrawList(window)->AddRect(windowPos,
				ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y), ImGui::ColorConvertFloat4ToU32(s_HighlightColor1_Thin));
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
		ImGui::PushFont(EditorUI::EditorUIService::s_FontPlexBold);
		ImGui::TextColored(s_HighlightColor1, text.c_str());
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

	static void CreateImage(Ref<Rendering::Texture2D> image, float size, ImVec4 tint = {1.0f ,1.0f, 1.0f, 1.0f})
	{
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 4.0f, ImGui::GetCursorPosY() + 3.2f));

		ImGui::Image((ImTextureID)(uint64_t)image->GetRendererID(), ImVec2(size, size),
			ImVec2{ 0, 1 }, ImVec2(1, 0), tint,
			EditorUIService::s_PureEmpty);
	}

	static float SmallButtonRelativeLocation(uint32_t slot)
	{
		return -EditorUIService::s_SmallButtonRightOffset - (EditorUIService::s_SmallButtonSpacing * slot);
	}

	static void CreateButton(ImGuiID widgetID, std::function<void()> onPress, 
		const InlineButtonSpec& spec, bool active = false, ImVec4 tintColor = {1.0f, 1.0f, 1.0f, 1.0f})
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
				ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x + spec.XPosition);
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
			tintColor, 0))
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
				ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, active ? spec.ActiveTooltip.c_str() : spec.InactiveTooltip.c_str());
				ImGui::EndTooltip();
			}
		}

	}

	static void WriteMultilineText(const std::string& text, float lineWidth, float xOffset = 0, float yOffset = 0)
	{
		std::string previewOutput{};
		std::string previewRemainder{ text };
		uint32_t iteration{ 0 };
		int32_t lineEndPosition;

		do 
		{
			lineEndPosition = ImGui::FindPositionAfterLength(previewRemainder.c_str(), lineWidth);
			ImGui::SameLine(xOffset);
			ImGui::SetCursorPosY({ ImGui::GetCursorPosY() + yOffset + (iteration * 20.0f) });
			if (lineEndPosition == -1)
			{
				ImGui::Text(previewRemainder.c_str());
			}
			else
			{
				previewOutput = previewRemainder.substr(0, lineEndPosition);
				previewRemainder = previewRemainder.substr(lineEndPosition, std::string::npos);
				ImGui::Text(previewOutput.c_str());
			}
			iteration++;
		} while (lineEndPosition != -1);
	}

	static void TruncateText(const std::string& text, uint32_t maxTextSize)
	{
		if (text.size() > maxTextSize)
		{
			std::string outputText = text.substr(0, maxTextSize - 2) + "..";
			ImGui::Text(outputText.c_str());
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, text.c_str());
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

			ImGui::PushFont(EditorUI::EditorUIService::s_FontAntaRegular);
			if (spec.DeleteAction)
			{
				// Optional Delete Tool Bar Button
				ImGui::SameLine();
				CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
				{
					if (spec.DeleteAction)
					{
						spec.DeleteAction();
					}
					ImGui::CloseCurrentPopup();
				}, s_LargeDeleteButton, false, s_PrimaryTextColor);
			}

			// Cancel Tool Bar Button
			ImGui::SameLine();
			CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				ImGui::CloseCurrentPopup();
			}, s_LargeCancelButton, false, s_PrimaryTextColor);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
				ImGui::CloseCurrentPopup();
			}, s_LargeConfirmButton, false, s_PrimaryTextColor);

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
		ImGui::PushFont(EditorUI::EditorUIService::s_FontAntaLarge);
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
				ImGui::SetCursorPosX(s_TextLeftIndentOffset);
			}
			ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
			int32_t labelPosition = ImGui::FindPositionAfterLength(spec.Label.c_str(),
				spec.Flags & SelectOption_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
			TruncateText(spec.Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
			WriteMultilineText(spec.CurrentOption.Label,s_SecondaryTextLargeWidth,  s_SecondaryTextPosOne);
			ImGui::PopStyleColor();

			ImGui::SameLine();
			CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				ImGui::OpenPopup(id.c_str());
				if (spec.PopupAction)
				{
					spec.PopupAction();
				}
				spec.CachedSelection = spec.CurrentOption;
			},
			EditorUIService::s_SmallEditButton, false, s_DisabledColor);

		}
		
		// Display Popup
		ImGui::SetNextWindowSize(ImVec2(700.0f, 0.0f));
		if (ImGui::BeginPopupModal(id.c_str(), NULL, ImGuiWindowFlags_NoTitleBar))
		{
			static char searchBuffer[256];

			EditorUI::EditorUIService::TitleText(spec.Label);

			ImGui::PushFont(EditorUI::EditorUIService::s_FontAntaRegular);
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

				ImGui::PushStyleColor(ImGuiCol_FrameBg, s_ActiveColor);
				ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, s_ActiveColor);
				ImGui::PushStyleColor(ImGuiCol_FrameBgActive, s_ActiveColor);

				ImGui::InputText((id + "InputText").c_str(), searchBuffer, sizeof(searchBuffer), ImGuiInputTextFlags_CallbackEdit, callback, (void*)&spec);
				ImGui::PopStyleColor(3);
			}

			// Search Tool Bar Button
			ImGui::SameLine();
			CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
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
			}, s_LargeSearchButton, spec.Searching, s_PrimaryTextColor);

			// Cancel Tool Bar Button
			ImGui::SameLine();
			CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				spec.Searching = false;
				memset(searchBuffer, 0, sizeof(searchBuffer));
				ImGui::CloseCurrentPopup();
			}, s_LargeCancelButton, false, s_PrimaryTextColor);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				spec.CurrentOption = spec.CachedSelection;
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction(spec.CurrentOption);
				}

				spec.Searching = false;
				memset(searchBuffer, 0, sizeof(searchBuffer));
				ImGui::CloseCurrentPopup();
			}, s_LargeConfirmButton, false, s_PrimaryTextColor);

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
						ImGui::PushStyleColor(ImGuiCol_Button, s_SelectedColor);
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

		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.Label.c_str(), s_PrimaryTextWidth);
		TruncateText(spec.Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
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
				ImGui::SameLine(s_SecondaryTextPosOne);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, s_PureEmpty);
				ImGui::PushStyleColor(ImGuiCol_Button, s_PureEmpty);
				TruncateText("True", 12);
				ImGui::SameLine();
				CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
				{
					if (spec.FieldBuffer.As<char>() == "True")
					{
						spec.FieldBuffer.SetString("False");
					}
					else
					{
						spec.FieldBuffer.SetString("True");
					}
				}, s_SmallCheckboxButton, std::string(spec.FieldBuffer.As<char>()) == "True", s_HighlightColor1);

				ImGui::SameLine(300.0f);
				TruncateText("False", 12);
				ImGui::SameLine();
				CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
				{
					if (spec.FieldBuffer.As<char>() == "False")
					{
						spec.FieldBuffer.SetString("True");
					}
					else
					{
						spec.FieldBuffer.SetString("False");
					}
				}, s_SmallCheckboxButton, std::string(spec.FieldBuffer.As<char>()) == "False", s_HighlightColor1);
				ImGui::PopStyleColor(2);
				break;
			}
			case WrappedVarType::String:
			{
				inputFlags = ImGuiInputTextFlags_CallbackEdit;
				ImGui::SameLine();
				ImGui::SetCursorPosX(s_SecondaryTextPosOne);
				ImGui::SetNextItemWidth(170.0f);
				ImGuiInputTextCallback typeCallback = [](ImGuiInputTextCallbackData* data)
				{
					return 0;
				};
				ImGui::InputText(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(),
					spec.FieldBuffer.As<char>(), spec.FieldBuffer.Size, inputFlags, typeCallback);
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
				ImGui::SameLine();
				ImGui::SetCursorPosX(s_SecondaryTextPosOne);
				ImGui::SetNextItemWidth(170.0f);
				ImGuiInputTextCallback typeCallback = [](ImGuiInputTextCallbackData* data)
				{
					return 0;
				};
				ImGui::InputText(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(),
					spec.FieldBuffer.As<char>(), spec.FieldBuffer.Size, inputFlags, typeCallback);
				break;
			}
		}
		ImGui::PopStyleColor();
		
	}

	void EditorUIService::Checkbox(CheckboxSpec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };

		if (spec.Flags & Checkbox_Indented)
		{
			ImGui::SetCursorPosX(s_TextLeftIndentOffset);
		}
		// Display Primary Label
		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.Label.c_str(), 
			spec.Flags & Checkbox_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
		TruncateText(spec.Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		if (spec.Flags & Checkbox_LeftLean)
		{
			ImGui::SameLine(s_SecondaryTextPosOne - 2.5f);
		}
		else
		{
			ImGui::SameLine(360.0f);
		}

		if (spec.Editing)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorUIService::s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, EditorUIService::s_PureEmpty);
			CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
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
			}, s_SmallCheckboxButton, spec.ToggleBoolean, s_HighlightColor1);
			ImGui::PopStyleColor(2);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorUIService::s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, EditorUIService::s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, EditorUIService::s_PureEmpty);
			CreateButton(spec.WidgetID + WidgetIterator(widgetCount), nullptr,
			s_SmallCheckboxDisabledButton,
			spec.ToggleBoolean, s_SecondaryTextColor);
			ImGui::PopStyleColor(3);
		}

		ImGui::SameLine();
		CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.Editing);
		},
		EditorUIService::s_SmallEditButton,
		spec.Editing, spec.Editing ? s_PrimaryTextColor : s_DisabledColor);
	}

	void EditorUIService::EditFloat(EditFloatSpec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };

		// Display Item
		if (spec.Flags & EditFloat_Indented)
		{
			ImGui::SetCursorPosX(s_TextLeftIndentOffset);
		}

		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.Label.c_str(), 
			spec.Flags & EditFloat_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
		TruncateText(spec.Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::SameLine(s_SecondaryTextPosOne);

		if (spec.Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, s_HighlightColor1);
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.CurrentFloat), 0.01f,
				0.0f, 0.0f,
				"%.2f"))
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(s_HighlightColor1, "X-Value");
				ImGui::EndTooltip();
			}
			ImGui::PopStyleVar();

		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetCursorPos({ s_SecondaryTextPosOne, yPosition });
			ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
			int32_t floatPosition = ImGui::FindPositionAfterLength(Utility::Conversions::FloatToString(spec.CurrentFloat).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.CurrentFloat), 
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.Editing);
		},
		EditorUIService::s_SmallEditButton,
		spec.Editing, spec.Editing ? s_PrimaryTextColor : s_DisabledColor);
	}

	void EditorUIService::EditVec2(EditVec2Spec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };

		// Display Item
		if (spec.Flags & EditVec2_Indented)
		{
			ImGui::SetCursorPosX(s_TextLeftIndentOffset);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);

		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.Label.c_str(), 
			spec.Flags & EditVec2_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
		TruncateText(spec.Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);

		ImGui::PopStyleColor();
		ImGui::SameLine(s_SecondaryTextPosOne);

		if (spec.Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, s_HighlightColor1);
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.CurrentVec2.x), 0.01f,
				0.0f, 0.0f,
				"%.2f"))
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(s_HighlightColor1, "X-Value");
				ImGui::EndTooltip();
			}

			// y value
			ImGui::PushStyleColor(ImGuiCol_Text, s_HighlightColor2);
			ImGui::SetCursorPos({ s_SecondaryTextPosTwo, yPosition });
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.CurrentVec2.y), 0.01f,
				0.0f, 0.0f,
				"%.2f"))
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(s_HighlightColor2, "Y-Value");
				ImGui::EndTooltip();
			}
			ImGui::PopStyleVar();

		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetCursorPos({ s_SecondaryTextPosOne, yPosition });
			ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
			int32_t floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(spec.CurrentVec2.x).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.CurrentVec2.x),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ s_SecondaryTextPosTwo, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(spec.CurrentVec2.y).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.CurrentVec2.y),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.Editing);
		},
		EditorUIService::s_SmallEditButton,
		spec.Editing, spec.Editing ? s_PrimaryTextColor : s_DisabledColor);
	}

	void EditorUIService::EditVec3(EditVec3Spec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };
		// Display Item
		if (spec.Flags & EditVec3_Indented)
		{
			ImGui::SetCursorPosX(s_TextLeftIndentOffset);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.Label.c_str(), 
			spec.Flags & EditVec3_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
		TruncateText(spec.Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::SameLine(s_SecondaryTextPosOne);
		
		if (spec.Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, s_HighlightColor1);
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.CurrentVec3.x), 0.01f,
				0.0f, 0.0f,
				"%.2f"))
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(s_HighlightColor1, "X-Value");
				ImGui::EndTooltip();
			}

			// y value
			ImGui::PushStyleColor(ImGuiCol_Text, s_HighlightColor2);
			ImGui::SetCursorPos({ s_SecondaryTextPosTwo, yPosition });
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.CurrentVec3.y), 0.01f,
				0.0f, 0.0f,
				"%.2f"))
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(s_HighlightColor2, "Y-Value");
				ImGui::EndTooltip();
			}

			// z value
			ImGui::PushStyleColor(ImGuiCol_Text, s_HighlightColor3);
			ImGui::SetCursorPos({ s_SecondaryTextPosThree, yPosition });
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.CurrentVec3.z), 0.01f,
				0.0f, 0.0f,
				"%.2f"))
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(s_HighlightColor3, "Z-Value");
				ImGui::EndTooltip();
			}
			ImGui::PopStyleVar();
			
		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
			ImGui::SetCursorPos({ s_SecondaryTextPosOne, yPosition });
			int32_t floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(spec.CurrentVec3.x).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.CurrentVec3.x),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ s_SecondaryTextPosTwo, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(spec.CurrentVec3.y).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.CurrentVec3.y),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ s_SecondaryTextPosThree, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(spec.CurrentVec3.z).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.CurrentVec3.z),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.Editing);
		},
		EditorUIService::s_SmallEditButton,
		spec.Editing, spec.Editing ? s_PrimaryTextColor : s_DisabledColor);
	}

	void EditorUIService::EditVec4(EditVec4Spec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };
		// Display Item
		if (spec.Flags & EditVec4_Indented)
		{
			ImGui::SetCursorPosX(s_TextLeftIndentOffset);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.Label.c_str(),
			spec.Flags & EditVec4_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
		TruncateText(spec.Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::SameLine(s_SecondaryTextPosOne);

		if (spec.Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, (spec.Flags & EditVec4_RGBA) ? s_Red : s_HighlightColor1);
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.CurrentVec4.x), 0.01f,
				0.0f, 0.0f,
				"%.2f"))
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(spec.Flags & EditVec4_RGBA ? s_Red : s_HighlightColor1, 
					spec.Flags & EditVec4_RGBA ? "Red Channel" : "X-Value");
				ImGui::EndTooltip();
			}
			
			// y value
			ImGui::PushStyleColor(ImGuiCol_Text, (spec.Flags & EditVec4_RGBA) ? s_Green : s_HighlightColor2);
			ImGui::SetCursorPos({ s_SecondaryTextPosTwo, yPosition });
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.CurrentVec4.y), 0.01f,
				0.0f, 0.0f,
				"%.2f"))
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(spec.Flags & EditVec4_RGBA ? s_Green : s_HighlightColor2, 
					spec.Flags & EditVec4_RGBA ? "Green Channel" : "Y-Value");
				ImGui::EndTooltip();
			}

			// z value
			ImGui::PushStyleColor(ImGuiCol_Text, (spec.Flags & EditVec4_RGBA) ? s_Blue : s_HighlightColor3);
			ImGui::SetCursorPos({ s_SecondaryTextPosThree, yPosition });
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.CurrentVec4.z), 0.01f,
				0.0f, 0.0f,
				"%.2f"))
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(spec.Flags & EditVec4_RGBA ? s_Blue : s_HighlightColor3,
					spec.Flags & EditVec4_RGBA ? "Blue Channel" : "Z-Value");
				ImGui::EndTooltip();
			}

			// w value
			ImGui::PushStyleColor(ImGuiCol_Text, (spec.Flags & EditVec4_RGBA) ? s_Alpha : s_HighlightColor4);
			ImGui::SetCursorPos({ s_SecondaryTextPosFour, yPosition });
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.CurrentVec4.w), 0.01f,
				0.0f, 0.0f,
				"%.2f"))
			{
				if (spec.ConfirmAction)
				{
					spec.ConfirmAction();
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(spec.Flags & EditVec4_RGBA ? s_Alpha : s_HighlightColor4, 
					spec.Flags & EditVec4_RGBA ? "Alpha Channel" : "W-Value");
				ImGui::EndTooltip();
			}

			ImGui::PopStyleVar();

		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
			ImGui::SetCursorPos({ s_SecondaryTextPosOne, yPosition });
			int32_t floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(spec.CurrentVec4.x).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.CurrentVec4.x),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ s_SecondaryTextPosTwo, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(spec.CurrentVec4.y).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.CurrentVec4.y),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ s_SecondaryTextPosThree, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(spec.CurrentVec4.z).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.CurrentVec4.z),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ s_SecondaryTextPosFour, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(spec.CurrentVec4.w).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.CurrentVec4.w),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.Editing);
		},
		EditorUIService::s_SmallEditButton,
		spec.Editing, spec.Editing ? s_PrimaryTextColor : s_DisabledColor);
	}

	void EditorUIService::RadioSelector(RadioSelectorSpec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };

		if (spec.Flags & RadioSelector_Indented)
		{
			ImGui::SetCursorPosX(s_TextLeftIndentOffset);
		}

		// Display Item
		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.Label.c_str(), 
			spec.Flags & RadioSelector_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
		TruncateText(spec.Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);

		ImGui::PopStyleColor();
		ImGui::SameLine(s_SecondaryTextPosOne - 2.5f);

		if (spec.Editing)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
			CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
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
			}, s_SmallCheckboxButton, spec.SelectedOption == 0, s_HighlightColor1);
			ImGui::SameLine();

			int32_t position = ImGui::FindPositionAfterLength(spec.FirstOptionLabel.c_str(), s_SecondaryTextMediumWidth);
			TruncateText(spec.FirstOptionLabel, position == -1 ? std::numeric_limits<int32_t>::max() : position);
			
			ImGui::SameLine(s_SecondaryTextPosMiddle - 2.5f);
			CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
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
			}, s_SmallCheckboxButton, spec.SelectedOption == 1, s_HighlightColor2);
			ImGui::SameLine();
			position = ImGui::FindPositionAfterLength(spec.SecondOptionLabel.c_str(), s_SecondaryTextMediumWidth);
			TruncateText(spec.SecondOptionLabel, position == -1 ? std::numeric_limits<int32_t>::max() : position);

			ImGui::PopStyleColor(3);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
			CreateButton(spec.WidgetID + WidgetIterator(widgetCount), nullptr,
				s_SmallCheckboxDisabledButton, spec.SelectedOption == 0, s_SecondaryTextColor);
			ImGui::SameLine();

			int32_t position = ImGui::FindPositionAfterLength(spec.FirstOptionLabel.c_str(), s_SecondaryTextMediumWidth);
			TruncateText(spec.FirstOptionLabel, position == -1 ? std::numeric_limits<int32_t>::max() : position);

			ImGui::SameLine(s_SecondaryTextPosMiddle - 2.5f);
			CreateButton(spec.WidgetID + WidgetIterator(widgetCount), nullptr,
				s_SmallCheckboxDisabledButton, spec.SelectedOption == 1, s_SecondaryTextColor);
			ImGui::SameLine();
			position = ImGui::FindPositionAfterLength(spec.SecondOptionLabel.c_str(), s_SecondaryTextMediumWidth);
			TruncateText(spec.SecondOptionLabel, position == -1 ? std::numeric_limits<int32_t>::max() : position);
			ImGui::PopStyleColor(4);
		}

		ImGui::SameLine();
		CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.Editing);
		},
		EditorUIService::s_SmallEditButton,
		spec.Editing, spec.Editing ? s_PrimaryTextColor : s_DisabledColor);
	}

	void EditorUIService::Table(TableSpec& spec)
	{
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };
		uint32_t smallButtonCount{ 0 };

		if (spec.Flags & Table_Indented)
		{
			ImGui::SetCursorPosX(s_TextLeftIndentOffset);
		}
		else
		{
			ImGui::PushFont(EditorUIService::s_FontAntaLarge);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.Label.c_str(), 
			spec.Flags & Table_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
		TruncateText(spec.Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

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
		CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.Expanded);
		}, 
		s_TableExpandButton ,spec.Expanded, spec.Expanded ? s_HighlightColor1 : s_DisabledColor);
		
		if (spec.Expanded)
		{
			if (!spec.EditTableSelectionList.empty())
			{
				ImGui::SameLine();
				CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
				{
					ImGui::OpenPopup(spec.WidgetID - 1);
				}, s_MediumOptionsButton, false, s_DisabledColor);

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
				ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
				ImGui::SetCursorPosX(spec.Flags & Table_Indented ? 61.0f: s_TextLeftIndentOffset);
				TruncateText(spec.Column1Title, 12);
				ImGui::SameLine();
				ImGui::SetCursorPosX(s_SecondaryTextPosOne);
				TruncateText(spec.Column2Title, 12);
				ImGui::PopStyleColor();
				Spacing(SpacingAmount::Small);
			}

			for (auto& tableEntry : spec.TableValues)
			{
				smallButtonCount = 0;
				ImGui::SetCursorPosX(spec.Flags & Table_Indented ? 42.5f : 12.0f);
				CreateImage(s_IconDash, 8, s_DisabledColor);
				ImGui::SameLine();
				ImGui::SetCursorPosX(spec.Flags & Table_Indented ? 61.0f : s_TextLeftIndentOffset);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.2f);
				TruncateText(tableEntry.Label, 16);
				ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
				WriteMultilineText(tableEntry.Value, s_SecondaryTextLargeWidth, s_SecondaryTextPosOne, -5.2f);
				ImGui::PopStyleColor();
				
				if (tableEntry.OnLink)
				{
					s_TableLinkButton.XPosition = SmallButtonRelativeLocation(smallButtonCount++);
					ImGui::SameLine();
					CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
					{
						if (tableEntry.OnLink)
						{
							tableEntry.OnLink(tableEntry);
						}
					}, s_TableLinkButton, false, s_DisabledColor);
				}

				if (tableEntry.OnEdit)
				{
					s_TableEditButton.XPosition = SmallButtonRelativeLocation(smallButtonCount++);
					ImGui::SameLine();
					CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
					{
						if (tableEntry.OnEdit)
						{
							tableEntry.OnEdit(tableEntry);
						}
					}, s_TableEditButton, false, s_DisabledColor);
				}
				
			}
		}
	}

	static void DrawEntries(TreeSpec& spec , std::vector<TreeEntry>& entries, uint32_t& widgetCount, TreePath& currentPath, ImVec2 rootPosition)
	{
		// Get initial positions and common resources
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition{};
		uint64_t depth = currentPath.GetDepth();
		uint32_t iteration = 0;
		for (auto& treeEntry : entries)
		{
			currentPath.AddNode(iteration);
			// Set x-position based on current tree depth
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (depth * 30.0f));
			screenPosition = ImGui::GetCursorScreenPos();

			// Display Selected Background
			if (spec.SelectedEntry == currentPath)
			{
				// Draw SelectedEntry background
				draw_list->AddRectFilled(screenPosition,
					ImVec2(screenPosition.x + ImGui::CalcTextSize(treeEntry.Label.c_str()).x + 34.0f, screenPosition.y + 21.0f),
					ImColor(EditorUI::EditorUIService::s_ActiveColor), 4, ImDrawFlags_RoundCornersAll);
			}

			// Display entry icon
			if (treeEntry.IconHandle)
			{
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorUIService::s_PureEmpty);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, EditorUIService::s_PureEmpty);
				ImGui::PushStyleColor(ImGuiCol_Button, EditorUIService::s_PureEmpty);
				if (ImGui::ImageButtonEx(spec.WidgetID + WidgetIterator(widgetCount),
					(ImTextureID)(uint64_t)treeEntry.IconHandle->GetRendererID(),
					ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
					EditorUIService::s_PureEmpty,
					EditorUI::EditorUIService::s_HighlightColor1, 0))
				{
					if (treeEntry.OnLeftClick)
					{
						treeEntry.OnLeftClick(treeEntry);
					}
					spec.SelectedEntry = currentPath;
				}

				if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				{
					if (treeEntry.OnRightClickSelection.size() > 0)
					{
						ImGui::OpenPopup(("##" + std::to_string(spec.WidgetID)).c_str());
						spec.CurrentRightClick = &treeEntry;
					}
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (treeEntry.OnDoubleLeftClick)
					{
						treeEntry.OnDoubleLeftClick(treeEntry);
					}
				}
				ImGui::PopStyleColor(3);
				ImGui::SameLine();
			}

			// Display entry text
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_PrimaryTextColor);
			ImGui::Text(treeEntry.Label.c_str());
			ImGui::PopStyleColor();

			if (ImGui::IsItemClicked())
			{
				if (treeEntry.OnLeftClick)
				{
					treeEntry.OnLeftClick(treeEntry);
				}
				spec.SelectedEntry = currentPath;
			}

			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			{
				if (treeEntry.OnRightClickSelection.size() > 0)
				{
					ImGui::OpenPopup(("##" + std::to_string(spec.WidgetID)).c_str());
					spec.CurrentRightClick = &treeEntry;
				}
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (treeEntry.OnDoubleLeftClick)
				{
					treeEntry.OnDoubleLeftClick(treeEntry);
				}
			}

			// Handle all sub-entries
			if (treeEntry.SubEntries.size() > 0)
			{
				// Draw expand icon
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Button, EditorUIService::s_PureEmpty);
				const Ref<Rendering::Texture2D> icon = spec.ExpandedNodes.contains(currentPath) ? EditorUIService::s_IconDown : EditorUIService::s_IconRight;
				if (ImGui::ImageButtonEx(spec.WidgetID + WidgetIterator(widgetCount),
					(ImTextureID)(uint64_t)icon->GetRendererID(),
					ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
					EditorUIService::s_PureEmpty,
					spec.ExpandedNodes.contains(currentPath) ? EditorUIService::s_HighlightColor1 : EditorUIService::s_DisabledColor, 0))
				{
					if (spec.ExpandedNodes.contains(currentPath))
					{
						spec.ExpandedNodes.erase(currentPath);
					}
					else
					{
						spec.ExpandedNodes.insert(currentPath);
					}
				}
				ImGui::PopStyleColor();

				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, spec.ExpandedNodes.contains(currentPath) ? "Collapse" : "Expand");
					ImGui::EndTooltip();
				}

				// Draw all sub-entries
				if (spec.ExpandedNodes.contains(currentPath))
				{
					DrawEntries(spec, treeEntry.SubEntries, widgetCount, currentPath, screenPosition);
				}
			}

			// Add horizontal line for each child node
			if (depth > 0)
			{
				draw_list->AddLine(ImVec2(rootPosition.x + 10.0f, screenPosition.y + 10.0f),
					ImVec2(screenPosition.x, screenPosition.y + 10.0f),
					ImColor(EditorUIService::s_PrimaryTextColor));
			}
			currentPath.PopNode();
			iteration++;
		}

		// Add vertical line for parent node
		if (depth > 0)
		{
			draw_list->AddLine(ImVec2(rootPosition.x + 10.0f, rootPosition.y + 21.0f),
				ImVec2(rootPosition.x + 10.0f, screenPosition.y + 10.0f),
				ImColor(EditorUIService::s_PrimaryTextColor));
		}
	}

	void EditorUIService::Tree(TreeSpec& spec)
	{
		std::string id = "##" + std::to_string(spec.WidgetID);
		uint32_t widgetCount{ 0 };
		TreePath treePath{};
		DrawEntries(spec, spec.TreeEntries, widgetCount, treePath, {});

		if (ImGui::BeginPopup(("##" + std::to_string(spec.WidgetID)).c_str()))
		{
			if (spec.CurrentRightClick)
			{
				for (auto& [label, func] : spec.CurrentRightClick->OnRightClickSelection)
				{
					if (ImGui::Selectable((label + "##" + std::to_string(spec.WidgetID)).c_str()))
					{
						func(*spec.CurrentRightClick);
					}
				}
			}
			ImGui::EndPopup();
		}
	}

	void EditorUIService::PanelHeader(PanelHeaderSpec& spec)
	{
		std::string id = "##" + std::to_string(spec.WidgetID);
		ImGui::PushFont(EditorUIService::s_FontAntaLarge);
		ImGui::TextColored(spec.EditColorActive ? EditorUIService::s_HighlightColor2 : EditorUIService::s_PrimaryTextColor , spec.Label.c_str());
		ImGui::PopFont();

		ImGui::SameLine();
		CreateButton(spec.WidgetID, [&]()
		{
			ImGui::OpenPopup(id.c_str());
		}, s_MediumOptionsButton, false, s_DisabledColor);
		
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
		ImGui::Separator(1.0f, s_HighlightColor1_Thin);
		Spacing(0.2f);
	}

	void EditorUIService::CollapsingHeader(CollapsingHeaderSpec& spec)
	{
		uint32_t widgetCount{ 0 };
		std::string id = "##" + std::to_string(spec.WidgetID);
		ImGui::PushFont(EditorUIService::s_FontAntaLarge);
		ImGui::TextColored(s_PrimaryTextColor , spec.Label.c_str());
		ImGui::PopFont();
		ImGui::SameLine();
		CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.Expanded);
		},
		s_SmallExpandButton, spec.Expanded, spec.Expanded ? s_HighlightColor1 : s_DisabledColor);

		if (spec.Expanded && !spec.SelectionList.empty())
		{
			ImGui::SameLine();
			CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				ImGui::OpenPopup(spec.WidgetID - 1);
			}, s_MediumOptionsButton, false, s_DisabledColor);

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
		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(label.c_str(), s_PrimaryTextWidth);
		TruncateText(label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();
		ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
		WriteMultilineText(text, s_SecondaryTextLargeWidth, s_SecondaryTextPosOne);
		ImGui::PopStyleColor();
	}
	void EditorUIService::Text(const std::string& Text)
	{
		ImGui::Text(Text.c_str());
	}
	void EditorUIService::EditText(EditTextSpec& spec)
	{
		// Local Variables
		static char stringBuffer[256];
		uint32_t widgetCount{ 0 };
		std::string id = "##" + std::to_string(spec.WidgetID);
		std::string popUpLabel = spec.Label;

		if (spec.Flags & EditText_PopupOnly)
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
			if (spec.Flags & EditText_Indented)
			{
				ImGui::SetCursorPosX(s_TextLeftIndentOffset);
			}
			ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
			int32_t labelPosition = ImGui::FindPositionAfterLength(spec.Label.c_str(), 
				spec.Flags & EditText_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
			TruncateText(spec.Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
			WriteMultilineText(spec.CurrentOption, s_SecondaryTextLargeWidth, s_SecondaryTextPosOne);
			ImGui::PopStyleColor();

			ImGui::SameLine();
			CreateButton(spec.WidgetID + WidgetIterator(widgetCount),[&]()
			{
				ImGui::OpenPopup(id.c_str());
				memset(stringBuffer, 0, sizeof(stringBuffer));
				memcpy_s(stringBuffer, sizeof(stringBuffer), spec.CurrentOption.data(), spec.CurrentOption.size());
			},
			EditorUIService::s_SmallEditButton, false, s_DisabledColor);
		}

		ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f));
		if (ImGui::BeginPopupModal(id.c_str(), NULL, ImGuiWindowFlags_NoTitleBar))
		{
			EditorUI::EditorUIService::TitleText(popUpLabel);

			ImGui::PushFont(EditorUI::EditorUIService::s_FontAntaRegular);

			// Cancel Tool Bar Button
			ImGui::SameLine();
			CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
			{
				memset(stringBuffer, 0, sizeof(stringBuffer));
				ImGui::CloseCurrentPopup();
			}, s_LargeCancelButton);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
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
			ImGui::InputText((id + "InputText").c_str(), stringBuffer, sizeof(stringBuffer));
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

		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.Label.c_str(), s_PrimaryTextWidth);
		TruncateText(spec.Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
		WriteMultilineText(spec.CurrentOption.string(), s_SecondaryTextLargeWidth, s_SecondaryTextPosOne);
		ImGui::PopStyleColor();

		ImGui::SameLine();
		CreateButton(spec.WidgetID + WidgetIterator(widgetCount), [&]()
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
		EditorUIService::s_SmallEditButton, false, s_DisabledColor);

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

	static bool RecursiveGetPathFromEntry(TreePath& outputPath, TreeEntry* entryQuery, const std::vector<TreeEntry>& entries)
	{
		uint32_t iteration{ 0 };
		for (auto& treeEntry : entries)
		{
			outputPath.AddNode(iteration);
			if (entryQuery == &treeEntry)
			{
				return true;
			}

			if (treeEntry.SubEntries.size() > 0)
			{
				bool success = RecursiveGetPathFromEntry(outputPath, entryQuery, treeEntry.SubEntries);
				if (success)
				{
					return true;
				}
			}

			outputPath.PopNode();
			iteration++;
		}
		return false;
	}

	TreePath TreeSpec::GetPathFromEntryReference(TreeEntry* entryQuery)
	{
		TreePath newPath{};
		if (RecursiveGetPathFromEntry(newPath, entryQuery, GetTreeEntries()))
		{
			return newPath;
		}
		
		KG_WARN("Could not locate provided entry query");
		return {};
		
	}
}
