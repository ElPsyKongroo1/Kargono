#include "kgpch.h"

#include "Kargono/EditorUI/EditorUI.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/Utility/Operations.h"
#include "Kargono/Input/InputService.h"
#include "Kargono/Input/InputMap.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Kargono/Rendering/Texture.h"
#include "Kargono/Projects/Project.h"

#include "API/EditorUI/ImGuiBackendAPI.h"
#include "API/Platform/GlfwAPI.h"
#include "API/Platform/gladAPI.h"
#include "API/EditorUI/ImGuiNotifyAPI.h"


namespace Kargono::EditorUI
{
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
	static InlineButtonSpec s_ListExpandButton {};

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
		s_TableEditButton.m_YPosition = -5.5f;

		s_TableLinkButton = EditorUIService::s_SmallLinkButton;
		s_TableLinkButton.m_YPosition = -5.5f;

		s_ListExpandButton = EditorUIService::s_SmallExpandButton;

		
	}

	const char* EditorUIService::GetHoveredWindowName()
	{
		ImGuiContext* context = ImGui::GetCurrentContext();
		if (context != nullptr)
		{
			// Get window pointer and ensure it is valid
			ImGuiWindow* hoveredWindow = context->HoveredWindow;
			if (hoveredWindow != nullptr) 
			{
				// Get the pointer to the hovered window
				ImGuiWindow* windowPtr = hoveredWindow;

				// Get the name of the hovered window
				return hoveredWindow->Name;
			}
		}
		return nullptr;
	}
	void EditorUIService::CreateInfoNotification(const char* text, int delayMS)
	{
		ImGuiToast toast{ ImGuiToastType::Info, delayMS, text };
		toast.setTitle("Kargono Editor Information");
		ImGui::InsertNotification(toast);
	}
	void EditorUIService::CreateWarningNotification(const char* text, int delayMS)
	{
		ImGuiToast toast{ ImGuiToastType::Warning, delayMS, text };
		toast.setTitle("Kargono Editor Warning");
		ImGui::InsertNotification(toast);
	}

	void EditorUIService::CreateCriticalNotification(const char* text, int delayMS)
	{
		ImGuiToast toast{ ImGuiToastType::Error, delayMS, text };
		toast.setTitle("Kargono Editor Critical");
		ImGui::InsertNotification(toast);
	}

	void EditorUIService::AutoCalcViewportSize(Math::vec2 screenViewportBounds[2], ViewportData& viewportData, bool& viewportFocused, bool& viewportHovered, const Math::uvec2& aspectRatio)
	{
		// Get current cursor position and GLFW viewport size
		ImVec2 windowScreenOffset = ImGui::GetWindowPos();
		Math::vec2 localViewportBounds[2];
		viewportFocused = ImGui::IsWindowFocused();
		viewportHovered = ImGui::IsWindowHovered();

		// Adjust viewport size based on current aspect ratio
		ImVec2 windowSize = ImGui::GetContentRegionAvail();
		ImVec2 cursorPosition = ImGui::GetCursorPos();
		if (aspectRatio.x > aspectRatio.y && ((windowSize.x / aspectRatio.x) * aspectRatio.y) < windowSize.y)
		{
			viewportData.m_Width = (uint32_t)windowSize.x;
			viewportData.m_Height = (uint32_t)(windowSize.x / aspectRatio.x) * aspectRatio.y;
		}
		else
		{
			viewportData.m_Width = (uint32_t)(windowSize.y / aspectRatio.y) * aspectRatio.x;
			viewportData.m_Height = (uint32_t)windowSize.y;
		}

		localViewportBounds[0] = { cursorPosition.x + (windowSize.x - (float)viewportData.m_Width) * 0.5f, cursorPosition.y + (windowSize.y - (float)viewportData.m_Height) * 0.5f };
		localViewportBounds[1] = { localViewportBounds[0].x + (float)viewportData.m_Width,  localViewportBounds[0].y + (float)viewportData.m_Height };
		screenViewportBounds[0] = { localViewportBounds[0].x + windowScreenOffset.x, localViewportBounds[0].y + windowScreenOffset.y };
		screenViewportBounds[1] = { screenViewportBounds[0].x + (float)viewportData.m_Width, screenViewportBounds[0].y + viewportData.m_Height };
		ImGui::SetCursorPos(ImVec2(localViewportBounds[0].x, localViewportBounds[0].y));
	}

	void EditorUIService::RenderImGuiNotify()
	{
		// Notifications style setup
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f); // Disable round borders
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f); // Disable borders

		// Notifications color setup
		ImGui::PushStyleColor(ImGuiCol_WindowBg, s_DarkBackgroundColor); // Background color

		// Main rendering function
		ImGui::RenderNotifications();

		//������������������������������� WARNING �������������������������������
		// Argument MUST match the amount of ImGui::PushStyleVar() calls 
		ImGui::PopStyleVar(2);
		// Argument MUST match the amount of ImGui::PushStyleColor() calls 
		ImGui::PopStyleColor(1);
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
		style.ItemSpacing = { 8.0f, 6.0f };
		style.WindowMinSize.x = 420.0f;

		ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | 
			ImGuiColorEditFlags_DisplayHSV | 
			ImGuiColorEditFlags_PickerHueBar | 
			ImGuiColorEditFlags_NoInputs |
			ImGuiColorEditFlags_NoSidePreview |
			ImGuiColorEditFlags_AlphaBar);

		SetColorDefaults();

		// Setup Platform/Renderer backends
		Engine& core = EngineService::GetActiveEngine();
		GLFWwindow* window = static_cast<GLFWwindow*>(core.GetWindow().GetNativeWindow());
		KG_ASSERT(window, "No window active when initializing EditorUI");
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		// Initialize toast notification API
		io.Fonts->AddFontDefault();
		float baseFontSize = 16.0f;
		float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
		static constexpr ImWchar iconsRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
		ImFontConfig iconsConfig;
		iconsConfig.MergeMode = true;
		iconsConfig.PixelSnapH = true;
		iconsConfig.GlyphMinAdvanceX = iconFontSize;
		io.Fonts->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data, fa_solid_900_compressed_size, iconFontSize, &iconsConfig, iconsRanges);
		ImGui::InitNotificationSystem();

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
		s_IconAI = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/AI.png").string());
		s_IconNotification = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Notification.png").string());
		

		// Viewport icons
		s_IconGrid = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Viewport/Grid.png").string());
		s_IconDisplay = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Viewport/Display.png").string());
		s_IconPlay = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Viewport/Play.png").string());
		s_IconStop = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Viewport/Stop.png").string());
		s_IconPause = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Viewport/Pause.png").string());
		s_IconStep = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Viewport/Step.png").string());
		s_IconSimulate = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/Viewport/Simulate.png").string());

		// Scene icons
		s_IconEntity = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/SceneEditor/Entity.png").string());
		s_IconBoxCollider = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/SceneEditor/BoxCollider.png").string());
		s_IconTag = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/SceneEditor/Tag.png").string());
		s_IconCircleCollider = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/SceneEditor/CircleCollider.png").string());
		s_IconClassInstance = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/SceneEditor/ClassInstance.png").string());
		s_IconRigidBody = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/SceneEditor/Rigidbody.png").string());
		s_IconTransform = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/SceneEditor/Transform.png").string());
		s_IconParticles = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/SceneEditor/Particles.png").string());

		// Runtime UI icons
		s_IconWindow = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/UIEditor/Window.png").string());
		s_IconTextWidget = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/UIEditor/TextWidget.png").string());
		s_IconButtonWidget = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/UIEditor/ButtonWidget.png").string());

		// Scripting UI icons
		s_IconNumber = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/TextEditor/Number.png").string());
		s_IconVariable = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/TextEditor/Variable.png").string());
		s_IconFunction = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/TextEditor/Function.png").string());
		s_IconBoolean = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/TextEditor/Boolean.png").string());
		s_IconDecimal = Rendering::Texture2D::CreateEditorTexture((EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/TextEditor/Decimal.png").string());

		// Content Browser icons
		s_IconDirectory = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Directory.png");
		s_IconGenericFile = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/GenericFile.png");
		s_IconBack = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Back.png");
		s_IconForward = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Forward.png");
		s_IconAudio = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Audio.png");
		s_IconTexture = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Texture.png");
		s_IconBinary = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Binary.png");
		s_IconScene = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Scene.png");
		s_IconScene_KG = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Scene_KG.png");
		s_IconRegistry = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Registry.png");
		s_IconFont = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Font.png");
		s_IconUserInterface = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/UserInterface.png");
		s_IconInput = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Input.png");
		s_IconAI_KG = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/AI_KG.png");
		s_IconAudio_KG = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Audio_KG.png");
		s_IconFont_KG = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Font_KG.png");
		s_IconGameState = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/GameState.png");
		s_IconProjectComponent = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/ProjectComponent.png");
		s_IconScript = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Script.png");
		s_IconTexture_KG = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/Texture_KG.png");
		s_IconEmitterConfig = Rendering::Texture2D::CreateEditorTexture(EngineService::GetActiveEngine().GetWorkingDirectory() / "Resources/Icons/ContentBrowser/EmitterConfig.png");
		

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
		s_IconAI.reset();
		s_IconNotification.reset();

		s_IconWindow.reset();
		s_IconTextWidget.reset();
		s_IconButtonWidget.reset();

		s_IconNumber.reset();
		s_IconVariable.reset();
		s_IconFunction.reset();
		s_IconBoolean.reset();
		s_IconDecimal.reset();

		s_IconDirectory.reset();
		s_IconGenericFile.reset();
		s_IconBack.reset();
		s_IconForward.reset();
		s_IconAudio.reset();
		s_IconTexture.reset();
		s_IconBinary.reset();
		s_IconScene.reset();
		s_IconScene_KG.reset();
		s_IconRegistry.reset();
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
		s_IconAI_KG.reset();
		s_IconAudio_KG.reset();
		s_IconFont_KG.reset();
		s_IconGameState.reset();
		s_IconProjectComponent.reset();
		s_IconScript.reset();
		s_IconTexture_KG.reset();
		s_IconParticles.reset();
		s_IconEmitterConfig.reset();

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
		s_ListExpandButton = {};

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
		RenderImGuiNotify();

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

	static void RecalculateWindowDimensions()
	{
		// Calculate Widget Spacing Values
		EditorUIService::s_WindowPosition = ImGui::GetWindowPos();
		EditorUIService::s_PrimaryTextWidth = (EditorUIService::s_SecondaryTextFirstPercentage * ImGui::GetContentRegionMax().x) - 20.0f;
		EditorUIService::s_PrimaryTextIndentedWidth = (EditorUIService::s_SecondaryTextFirstPercentage * ImGui::GetContentRegionMax().x) - 20.0f - EditorUIService::s_TextLeftIndentOffset;
		EditorUIService::s_SecondaryTextSmallWidth = ((EditorUIService::s_SecondaryTextSecondPercentage - EditorUIService::s_SecondaryTextFirstPercentage) * ImGui::GetContentRegionMax().x) - 10.0f;
		EditorUIService::s_SecondaryTextMediumWidth = ((EditorUIService::s_SecondaryTextMiddlePercentage - EditorUIService::s_SecondaryTextFirstPercentage) * ImGui::GetContentRegionMax().x) - 30.0f;
		EditorUIService::s_SecondaryTextLargeWidth = ((EditorUIService::s_SecondaryTextFourthPercentage - EditorUIService::s_SecondaryTextFirstPercentage) * ImGui::GetContentRegionMax().x) + EditorUIService::s_SecondaryTextSmallWidth;

		EditorUIService::s_SecondaryTextPosOne = ImGui::GetContentRegionMax().x * EditorUIService::s_SecondaryTextFirstPercentage;
		EditorUIService::s_SecondaryTextPosTwo = ImGui::GetContentRegionMax().x * EditorUIService::s_SecondaryTextSecondPercentage;
		EditorUIService::s_SecondaryTextPosThree = ImGui::GetContentRegionMax().x * EditorUIService::s_SecondaryTextThirdPercentage;
		EditorUIService::s_SecondaryTextPosFour = ImGui::GetContentRegionMax().x * EditorUIService::s_SecondaryTextFourthPercentage;
		EditorUIService::s_SecondaryTextPosMiddle = ImGui::GetContentRegionMax().x * EditorUIService::s_SecondaryTextMiddlePercentage;
	}

	void EditorUIService::StartWindow(const char* label, bool* closeWindow, int32_t flags)
	{
		// Start Window
		ImGui::Begin(label, closeWindow, flags);
		RecalculateWindowDimensions();
	}

	void EditorUIService::EndWindow()
	{
		ImGui::End();
	}

	void EditorUIService::StartDockspaceWindow()
	{
		// Set dockspace as main viewport window
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		// Apply window padding and rounding settings
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		// Set window flags
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		// Create the dockspace window
		EditorUI::EditorUIService::StartWindow("DockSpace", nullptr, window_flags);

		// Clear the window padding and rounding settings
		ImGui::PopStyleVar(3);

		// Submit the DockSpace
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_NoCloseButton;
		ImGui::DockSpace(ImGui::GetID("MyDockSpace"), ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	void EditorUIService::EndDockspaceWindow()
	{
		ImGui::End();
	}

	uint32_t EditorUIService::GetActiveWidgetID()
	{
		return GImGui->ActiveId;
	}

	const char* EditorUIService::GetFocusedWindowName()
	{
		if (GImGui->NavWindow)
		{
			return GImGui->NavWindow->Name;
		}
		return nullptr;
	}

	void EditorUIService::SetFocusedWindow(const char* windowName)
	{
		ImGui::SetWindowFocus(windowName);
	}

	void EditorUIService::BringWindowToFront(const char* windowName)
	{
		ImGui::BringWindowToFront(windowName);
	}

	void EditorUIService::BringCurrentWindowToFront()
	{
		if (ImGui::IsWindowAppearing())
			ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
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

	bool EditorUIService::IsAnyItemHovered()
	{
		return ImGui::IsAnyItemHovered();
	}

	void EditorUIService::SetDisableLeftClick(bool option)
	{
		s_DisableLeftClick = option;
	}

	void EditorUIService::BlockMouseEvents(bool block)
	{
		s_BlockMouseEvents = block;
	}

	bool EditorUIService::OnInputEvent(Events::Event* e)
	{
		KG_PROFILE_FUNCTION();
		ImGuiIO& io = ImGui::GetIO();
		if (!s_BlockMouseEvents)
		{
			return e->IsInCategory(Events::Mouse) && io.WantCaptureMouse;
		}
		return e->IsInCategory(Events::Keyboard) && io.WantCaptureKeyboard;
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
		ImGui::PushStyleColor(ImGuiCol_Text, s_HighlightColor1);
		ImGui::TextUnformatted(text.c_str());
		ImGui::PopStyleColor();
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
				if (!Utility::Regex::GetMatchSuccess(option.m_Label, searchQuery, false))
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
		switch (spec.m_XPositionType)
		{
			case PositionType::Inline:
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + spec.m_XPosition);
				break;
			}
			case PositionType::Absolute:
			{
				ImGui::SetCursorPosX(spec.m_XPosition);
				break;
			}
			case PositionType::Relative:
			{
				ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x + spec.m_XPosition);
				break;
			}
			default:
			{
				break;	
			}
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spec.m_YPosition);
		ImGui::PushStyleColor(ImGuiCol_Button, EditorUIService::s_PureEmpty);
		if (spec.m_Disabled)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorUIService::s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, EditorUIService::s_PureEmpty);
		}
		Ref<Rendering::Texture2D> iconChoice = active ? spec.m_ActiveIcon : spec.m_InactiveIcon;
		if (ImGui::ImageButtonEx(widgetID,
			(ImTextureID)(uint64_t)iconChoice->GetRendererID(),
			ImVec2(spec.m_IconSize, spec.m_IconSize), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
			EditorUIService::s_PureEmpty,
			tintColor, 0))
		{
			if (onPress)
			{
				onPress();
			}
		}
		ImGui::PopStyleColor(spec.m_Disabled ? 3 : 1);

		if (!spec.m_Disabled)
		{
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_HighlightColor1);
				ImGui::TextUnformatted(active ? spec.m_ActiveTooltip.c_str() : spec.m_InactiveTooltip.c_str());
				ImGui::PopStyleColor();
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
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		do 
		{
			lineEndPosition = ImGui::FindPositionAfterLength(previewRemainder.c_str(), lineWidth);
			ImGui::SameLine(xOffset);
			ImGui::SetCursorPosY({ ImGui::GetCursorPosY() + yOffset + (iteration * 20.0f) });
			if (lineEndPosition == -1)
			{
				// Draw backgrounds
				ImVec2 screenPosition = ImGui::GetCursorScreenPos();
				draw_list->AddRectFilled(ImVec2(EditorUI::EditorUIService::s_WindowPosition.x + EditorUI::EditorUIService::s_SecondaryTextPosOne - 5.0f, screenPosition.y),
					ImVec2(EditorUI::EditorUIService::s_WindowPosition.x + EditorUI::EditorUIService::s_SecondaryTextPosOne + EditorUI::EditorUIService::s_SecondaryTextLargeWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
					ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, iteration == 0 ? ImDrawFlags_RoundCornersAll: ImDrawFlags_RoundCornersBottom);
				
				// Draw the text
				ImGui::TextUnformatted(previewRemainder.c_str());
			}
			else
			{
				// Draw backgrounds
				ImVec2 screenPosition = ImGui::GetCursorScreenPos();
				draw_list->AddRectFilled(ImVec2(EditorUI::EditorUIService::s_WindowPosition.x + EditorUI::EditorUIService::s_SecondaryTextPosOne - 5.0f, screenPosition.y),
					ImVec2(EditorUI::EditorUIService::s_WindowPosition.x + EditorUI::EditorUIService::s_SecondaryTextPosOne + EditorUI::EditorUIService::s_SecondaryTextLargeWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
					ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, iteration == 0 ? ImDrawFlags_RoundCornersTop: ImDrawFlags_RoundCornersNone);


				previewOutput = previewRemainder.substr(0, lineEndPosition);
				previewRemainder = previewRemainder.substr(lineEndPosition, std::string::npos);
				ImGui::TextUnformatted(previewOutput.c_str());
			}
			iteration++;
		} while (lineEndPosition != -1);
	}

	static void TruncateText(const std::string& text, uint32_t maxTextSize)
	{
		if (text.size() > maxTextSize)
		{
			std::string outputText = text.substr(0, maxTextSize - 2) + "..";
			ImGui::TextUnformatted(outputText.c_str());
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_HighlightColor1);
				ImGui::TextUnformatted(text.c_str());
				ImGui::PopStyleColor();
				ImGui::EndTooltip();
			}
		}
		else
		{
			ImGui::TextUnformatted(text.c_str());
		}
	}

	void EditorUIService::GenericPopup(GenericPopupSpec& spec)
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		uint32_t widgetCount{ 0 };

		if (spec.m_OpenPopup)
		{
			ImGui::OpenPopup(id);
			spec.m_OpenPopup = false;
			spec.m_CloseActivePopup = false;

			if (spec.m_PopupAction)
			{
				spec.m_PopupAction();
			}
		}

		// Display Popup
		ImGui::SetNextWindowSize(ImVec2(spec.m_PopupWidth, 0.0f));
		if (ImGui::BeginPopupModal(id, NULL, ImGuiWindowFlags_NoTitleBar))
		{
			// Close popup externally
			if (spec.m_CloseActivePopup)
			{
				ImGui::CloseCurrentPopup();
			}

			RecalculateWindowDimensions();
			EditorUI::EditorUIService::TitleText(spec.m_Label);

			ImGui::PushFont(EditorUI::EditorUIService::s_FontAntaRegular);
			if (spec.m_DeleteAction)
			{
				// Optional Delete Tool Bar Button
				ImGui::SameLine();
				CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
				{
					if (spec.m_DeleteAction)
					{
						spec.m_DeleteAction();
					}
					ImGui::CloseCurrentPopup();
				}, s_LargeDeleteButton, false, s_PrimaryTextColor);
			}

			// Cancel Tool Bar Button
			ImGui::SameLine();
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{
				if (spec.m_CancelAction)
				{
					spec.m_CancelAction();
				}
				ImGui::CloseCurrentPopup();
			}, s_LargeCancelButton, false, s_PrimaryTextColor);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction();
				}
				ImGui::CloseCurrentPopup();
			}, s_LargeConfirmButton, false, s_PrimaryTextColor);

			ImGui::Separator();

			if (spec.m_PopupContents)
			{
				spec.m_PopupContents();
			}

			ImGui::PopFont();
			ImGui::EndPopup();
			RecalculateWindowDimensions();
		}
	}

	void EditorUIService::WarningPopup(WarningPopupSpec& spec)
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		uint32_t widgetCount{ 0 };

		if (spec.m_OpenPopup)
		{
			ImGui::OpenPopup(id);
			spec.m_OpenPopup = false;
		}

		// Display Popup
		ImGui::SetNextWindowSize(ImVec2(spec.m_PopupWidth, 0.0f));
		if (ImGui::BeginPopupModal(id, NULL, ImGuiWindowFlags_NoTitleBar))
		{
			RecalculateWindowDimensions();
			EditorUI::EditorUIService::TitleText(spec.m_Label);

			ImGui::PushFont(EditorUI::EditorUIService::s_FontAntaRegular);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{
				ImGui::CloseCurrentPopup();
			}, s_LargeConfirmButton, false, s_PrimaryTextColor);

			ImGui::Separator();

			if (spec.m_PopupContents)
			{
				spec.m_PopupContents();
			}

			ImGui::PopFont();
			ImGui::EndPopup();
			RecalculateWindowDimensions();
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
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		uint32_t widgetCount{ 0 };

		if (spec.m_Flags & (SelectOption_PopupOnly | SelectOption_HandleEditButtonExternally))
		{
			if (spec.m_OpenPopup)
			{
				ImGui::OpenPopup(id);
				spec.m_OpenPopup = false;
				if (spec.m_PopupAction)
				{
					spec.m_PopupAction();
				}
				spec.m_CachedSelection = spec.m_CurrentOption;
			}
		}
		
		if ((spec.m_Flags & SelectOption_PopupOnly) == 0)
		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 screenPosition = ImGui::GetCursorScreenPos();
			draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne - 5.0f, screenPosition.y),
				ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne + s_SecondaryTextLargeWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
				ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

			// Display Menu Item
			if (spec.m_Flags & SelectOption_Indented)
			{
				ImGui::SetCursorPosX(s_TextLeftIndentOffset);
			}
			ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
			int32_t labelPosition = ImGui::FindPositionAfterLength(spec.m_Label.c_str(),
				spec.m_Flags & SelectOption_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
			TruncateText(spec.m_Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
			WriteMultilineText(spec.m_CurrentOption.m_Label,s_SecondaryTextLargeWidth,  s_SecondaryTextPosOne);
			ImGui::PopStyleColor();

			ImGui::SameLine();
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{
				// Handle custom edit functionality
				if (spec.m_Flags & SelectOption_HandleEditButtonExternally)
				{
					if (spec.m_OnEdit)
					{
						spec.m_OnEdit();
					}
				}
				// Open the button normally
				else
				{
					ImGui::OpenPopup(id);
					if (spec.m_PopupAction)
					{
						spec.m_PopupAction();
					}
					spec.m_CachedSelection = spec.m_CurrentOption;
				}
			},
			EditorUIService::s_SmallEditButton, false, s_DisabledColor);
		}
		
		// Display Popup
		ImGui::SetNextWindowSize(ImVec2(700.0f, 500.0f));
		if (ImGui::BeginPopupModal(id, NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar))
		{
			static char searchBuffer[256];

			// Set up the header for the popup
			EditorUI::EditorUIService::TitleText(spec.m_Label);
			ImGui::PushFont(EditorUI::EditorUIService::s_FontAntaRegular);
			if (spec.m_Searching)
			{
				ImGui::SameLine(ImGui::GetWindowWidth() - 124.0f - 200.0f);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.6f);
				ImGui::SetNextItemWidth(200.0f);

				ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData* data)
				{
					std::string currentData = std::string(data->Buf);
					SelectOptionSpec* providedSpec = (SelectOptionSpec*)data->UserData;
					providedSpec->m_CachedSearchResults = GenerateSearchCache(providedSpec->GetAllOptions(), currentData);
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
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{
				if (spec.m_Searching)
				{
					spec.m_Searching = false;
				}
				else
				{
					spec.m_Searching = true;
					spec.m_CachedSearchResults = GenerateSearchCache(spec.m_ActiveOptions, searchBuffer);
				}
			}, s_LargeSearchButton, spec.m_Searching, s_PrimaryTextColor);

			// Cancel Tool Bar Button
			ImGui::SameLine();
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{
				spec.m_Searching = false;
				memset(searchBuffer, 0, sizeof(searchBuffer));
				ImGui::CloseCurrentPopup();
			}, s_LargeCancelButton, false, s_PrimaryTextColor);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{
				spec.m_CurrentOption = spec.m_CachedSelection;
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction(spec.m_CurrentOption);
				}

				spec.m_Searching = false;
				memset(searchBuffer, 0, sizeof(searchBuffer));
				ImGui::CloseCurrentPopup();
			}, s_LargeConfirmButton, false, s_PrimaryTextColor);

			ImGui::Separator();

			ImGui::BeginChildEx("##", spec.m_WidgetID + WidgetIterator(widgetCount), 
				{ 0.0f, 0.0f }, false, 0);
			// Start the window body
			for (auto& [title, options] :
				spec.m_Searching ? spec.m_CachedSearchResults : spec.GetAllOptions())
			{
				ImGui::TextUnformatted(title.c_str());
				uint32_t iteration{ 1 };
				bool selectedButton = false;
				for (auto& option : options)
				{
					if (spec.m_CachedSelection == option)
					{
						selectedButton = true;
					}

					if (selectedButton)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, s_SelectedColor);
					}

					if (ImGui::Button((option.m_Label.c_str() + id + std::string(option.m_Handle)).c_str()))
					{
						spec.m_CachedSelection = option;
					}
					if (iteration % spec.m_LineCount != 0 && iteration != 0 && iteration != options.size())
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

			ImGui::EndChild();

			ImGui::PopFont();
			ImGui::EndPopup();
		}
	}

	void EditorUIService::EditVariable(EditVariableSpec& spec)
	{
		// Local Variables
		uint32_t widgetCount{ 0 };
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		static ImGuiInputTextFlags inputFlags {};

		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.m_Label.c_str(), s_PrimaryTextWidth);
		TruncateText(spec.m_Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
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
				CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
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
				CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
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
				ImGui::InputText(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(),
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
				ImGui::InputText(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(),
					spec.FieldBuffer.As<char>(), spec.FieldBuffer.Size, inputFlags, typeCallback);
				break;
			}
		}
		ImGui::PopStyleColor();
		
	}

	void EditorUIService::Checkbox(CheckboxSpec& spec)
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		uint32_t widgetCount{ 0 };

		// Draw background
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne + 21.0f, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		if (spec.m_Flags & Checkbox_Indented)
		{
			ImGui::SetCursorPosX(s_TextLeftIndentOffset);
		}
		// Display Primary Label
		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.m_Label.c_str(), 
			spec.m_Flags & Checkbox_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
		TruncateText(spec.m_Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		if (spec.m_Flags & Checkbox_LeftLean)
		{
			ImGui::SameLine(s_SecondaryTextPosOne - 2.5f);
		}
		else
		{
			ImGui::SameLine(360.0f);
		}

		if (spec.m_Editing)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorUIService::s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, EditorUIService::s_PureEmpty);
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{

				if (spec.m_CurrentBoolean)
				{
					spec.m_CurrentBoolean = false;
					if (!spec.m_ConfirmAction)
					{
						return;
					}
					spec.m_ConfirmAction(spec);
				}
				else
				{
					spec.m_CurrentBoolean = true;
					if (!spec.m_ConfirmAction)
					{
						return;
					}
					spec.m_ConfirmAction(spec);
				}
			}, s_SmallCheckboxButton, spec.m_CurrentBoolean, s_HighlightColor1);
			ImGui::PopStyleColor(2);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorUIService::s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, EditorUIService::s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, EditorUIService::s_PureEmpty);
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), nullptr,
			s_SmallCheckboxDisabledButton,
			spec.m_CurrentBoolean, s_SecondaryTextColor);
			ImGui::PopStyleColor(3);
		}

		ImGui::SameLine();
		CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.m_Editing);
		},
		EditorUIService::s_SmallEditButton,
		spec.m_Editing, spec.m_Editing ? s_PrimaryTextColor : s_DisabledColor);
	}

	void EditorUIService::EditInteger(EditIntegerSpec& spec)
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		uint32_t widgetCount{ 0 };

		// Draw background
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne + s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);


		// Display Item
		if (spec.m_Flags & EditInteger_Indented)
		{
			ImGui::SetCursorPosX(s_TextLeftIndentOffset);
		}


		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.m_Label.c_str(),
			spec.m_Flags & EditInteger_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
		TruncateText(spec.m_Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::SameLine(s_SecondaryTextPosOne);
		
		if (spec.m_Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, s_HighlightColor1);
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragInt(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.m_CurrentInteger), (float)spec.m_ScrollSpeed,
				spec.m_Bounds[0], spec.m_Bounds[1],
				"%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction(spec);
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
			int32_t integerPosition = ImGui::FindPositionAfterLength(std::to_string(spec.m_CurrentInteger).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(std::to_string(spec.m_CurrentInteger),
				integerPosition == -1 ? std::numeric_limits<int32_t>::max() : integerPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{
				Utility::Operations::ToggleBoolean(spec.m_Editing);
			},
			EditorUIService::s_SmallEditButton,
			spec.m_Editing, spec.m_Editing ? s_PrimaryTextColor : s_DisabledColor);
	}

	void EditorUIService::EditIVec2(EditIVec2Spec& spec)
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		uint32_t widgetCount{ 0 };

		// Draw backgrounds
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne + s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosTwo - 5.0f, screenPosition.y),
			ImVec2(s_WindowPosition.x + s_SecondaryTextPosTwo + s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		// Display Item
		if (spec.m_Flags & EditIVec2_Indented)
		{
			ImGui::SetCursorPosX(s_TextLeftIndentOffset);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.m_Label.c_str(),
			spec.m_Flags & EditIVec2_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
		TruncateText(spec.m_Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);

		ImGui::PopStyleColor();
		ImGui::SameLine(s_SecondaryTextPosOne);

		if (spec.m_Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, s_HighlightColor1);
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragInt(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.m_CurrentIVec2.x), (float)spec.m_ScrollSpeed,
				spec.m_Bounds[0], spec.m_Bounds[1],
				"%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction(spec);
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
			if (ImGui::DragInt(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.m_CurrentIVec2.y), (float)spec.m_ScrollSpeed,
				spec.m_Bounds[0], spec.m_Bounds[1],
				"%d", ImGuiSliderFlags_AlwaysClamp))
			{
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction(spec);
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
			int32_t integerPosition = ImGui::FindPositionAfterLength(
				std::to_string(spec.m_CurrentIVec2.x).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(std::to_string(spec.m_CurrentIVec2.x),
				integerPosition == -1 ? std::numeric_limits<int32_t>::max() : integerPosition);
			ImGui::SetCursorPos({ s_SecondaryTextPosTwo, yPosition });
			integerPosition = ImGui::FindPositionAfterLength(
				std::to_string(spec.m_CurrentIVec2.y).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(std::to_string(spec.m_CurrentIVec2.y),
				integerPosition == -1 ? std::numeric_limits<int32_t>::max() : integerPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.m_Editing);
		},
		EditorUIService::s_SmallEditButton,
		spec.m_Editing, spec.m_Editing ? s_PrimaryTextColor : s_DisabledColor);
	}

	void EditorUIService::EditFloat(EditFloatSpec& spec)
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		uint32_t widgetCount{ 0 };

		// Draw background
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne + s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);


		// Display Item
		if (spec.m_Flags & EditFloat_Indented)
		{
			ImGui::SetCursorPosX(s_TextLeftIndentOffset);
		}


		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.m_Label.c_str(), 
			spec.m_Flags & EditFloat_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
		TruncateText(spec.m_Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::SameLine(s_SecondaryTextPosOne);

		if (spec.m_Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, s_HighlightColor1);
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.m_CurrentFloat), spec.m_ScrollSpeed,
				spec.m_Bounds[0], spec.m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction(spec);
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
			int32_t floatPosition = ImGui::FindPositionAfterLength(Utility::Conversions::FloatToString(spec.m_CurrentFloat).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.m_CurrentFloat), 
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.m_Editing);
		},
		EditorUIService::s_SmallEditButton,
		spec.m_Editing, spec.m_Editing ? s_PrimaryTextColor : s_DisabledColor);
	}

	void EditorUIService::EditVec2(EditVec2Spec& spec)
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		uint32_t widgetCount{ 0 };

		// Draw backgrounds
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne + s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosTwo - 5.0f, screenPosition.y),
			ImVec2(s_WindowPosition.x + s_SecondaryTextPosTwo + s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		// Display Item
		if (spec.m_Flags & EditVec2_Indented)
		{
			ImGui::SetCursorPosX(s_TextLeftIndentOffset);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);

		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.m_Label.c_str(), 
			spec.m_Flags & EditVec2_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
		TruncateText(spec.m_Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);

		ImGui::PopStyleColor();
		ImGui::SameLine(s_SecondaryTextPosOne);

		if (spec.m_Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, s_HighlightColor1);
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.m_CurrentVec2.x), spec.m_ScrollSpeed,
				spec.m_Bounds[0], spec.m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction(spec);
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
			if (ImGui::DragFloat(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.m_CurrentVec2.y), spec.m_ScrollSpeed,
				spec.m_Bounds[0], spec.m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction(spec);
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
				Utility::Conversions::FloatToString(spec.m_CurrentVec2.x).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.m_CurrentVec2.x),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ s_SecondaryTextPosTwo, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(spec.m_CurrentVec2.y).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.m_CurrentVec2.y),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.m_Editing);
		},
		EditorUIService::s_SmallEditButton,
		spec.m_Editing, spec.m_Editing ? s_PrimaryTextColor : s_DisabledColor);
	}

	void EditorUIService::EditVec3(EditVec3Spec& spec)
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		uint32_t widgetCount{ 0 };

		// Draw backgrounds
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne + s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosTwo - 5.0f, screenPosition.y),
			ImVec2(s_WindowPosition.x + s_SecondaryTextPosTwo + s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosThree - 5.0f, screenPosition.y),
			ImVec2(s_WindowPosition.x + s_SecondaryTextPosThree + s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		// Display Item
		if (spec.m_Flags & EditVec3_Indented)
		{
			ImGui::SetCursorPosX(s_TextLeftIndentOffset);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.m_Label.c_str(), 
			spec.m_Flags & EditVec3_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
		TruncateText(spec.m_Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::SameLine(s_SecondaryTextPosOne);
		
		if (spec.m_Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, s_HighlightColor1);
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.m_CurrentVec3.x), spec.m_ScrollSpeed,
				spec.m_Bounds[0], spec.m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction(spec);
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
			if (ImGui::DragFloat(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.m_CurrentVec3.y), spec.m_ScrollSpeed,
				spec.m_Bounds[0], spec.m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction(spec);
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
			if (ImGui::DragFloat(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.m_CurrentVec3.z), spec.m_ScrollSpeed,
				spec.m_Bounds[0], spec.m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction(spec);
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
				Utility::Conversions::FloatToString(spec.m_CurrentVec3.x).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.m_CurrentVec3.x),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ s_SecondaryTextPosTwo, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(spec.m_CurrentVec3.y).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.m_CurrentVec3.y),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ s_SecondaryTextPosThree, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(spec.m_CurrentVec3.z).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.m_CurrentVec3.z),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.m_Editing);
		},
		EditorUIService::s_SmallEditButton,
		spec.m_Editing, spec.m_Editing ? s_PrimaryTextColor : s_DisabledColor);
	}

	void EditorUIService::EditVec4(EditVec4Spec& spec)
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		uint32_t widgetCount{ 0 };

		// Draw backgrounds
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne + s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosTwo - 5.0f, screenPosition.y),
			ImVec2(s_WindowPosition.x + s_SecondaryTextPosTwo + s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosThree - 5.0f, screenPosition.y),
			ImVec2(s_WindowPosition.x + s_SecondaryTextPosThree + s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosFour - 5.0f, screenPosition.y),
			ImVec2(s_WindowPosition.x + s_SecondaryTextPosFour + ((spec.m_Flags & EditVec4_RGBA) ?
				s_SecondaryTextSmallWidth - 25.0f : s_SecondaryTextSmallWidth), screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		if (spec.m_Flags & EditVec4_RGBA)
		{
			draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosFour + s_SecondaryTextSmallWidth - 23.0f, screenPosition.y),
				ImVec2(s_WindowPosition.x + s_SecondaryTextPosFour + s_SecondaryTextSmallWidth, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
				ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		}
		

		// Display Item
		if (spec.m_Flags & EditVec4_Indented)
		{
			ImGui::SetCursorPosX(s_TextLeftIndentOffset);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.m_Label.c_str(),
			spec.m_Flags & EditVec4_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
		TruncateText(spec.m_Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::SameLine(s_SecondaryTextPosOne);

		if (spec.m_Editing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
			// x value
			ImGui::PushStyleColor(ImGuiCol_Text, (spec.m_Flags & EditVec4_RGBA) ? s_Red : s_HighlightColor1);
			float yPosition = ImGui::GetCursorPosY();
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.m_CurrentVec4.x), spec.m_ScrollSpeed,
				spec.m_Bounds[0], spec.m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction(spec);
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(spec.m_Flags & EditVec4_RGBA ? s_Red : s_HighlightColor1, 
					spec.m_Flags & EditVec4_RGBA ? "Red Channel" : "X-Value");
				ImGui::EndTooltip();
			}
			
			// y value
			ImGui::PushStyleColor(ImGuiCol_Text, (spec.m_Flags & EditVec4_RGBA) ? s_Green : s_HighlightColor2);
			ImGui::SetCursorPos({ s_SecondaryTextPosTwo, yPosition });
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.m_CurrentVec4.y), spec.m_ScrollSpeed,
				spec.m_Bounds[0], spec.m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction(spec);
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(spec.m_Flags & EditVec4_RGBA ? s_Green : s_HighlightColor2, 
					spec.m_Flags & EditVec4_RGBA ? "Green Channel" : "Y-Value");
				ImGui::EndTooltip();
			}

			// z value
			ImGui::PushStyleColor(ImGuiCol_Text, (spec.m_Flags & EditVec4_RGBA) ? s_Blue : s_HighlightColor3);
			ImGui::SetCursorPos({ s_SecondaryTextPosThree, yPosition });
			ImGui::SetNextItemWidth(s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.m_CurrentVec4.z), spec.m_ScrollSpeed,
				spec.m_Bounds[0], spec.m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction(spec);
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(spec.m_Flags & EditVec4_RGBA ? s_Blue : s_HighlightColor3,
					spec.m_Flags & EditVec4_RGBA ? "Blue Channel" : "Z-Value");
				ImGui::EndTooltip();
			}

			// w value
			ImGui::PushStyleColor(ImGuiCol_Text, (spec.m_Flags & EditVec4_RGBA) ? s_Alpha : s_HighlightColor4);
			ImGui::SetCursorPos({ s_SecondaryTextPosFour, yPosition });
			ImGui::SetNextItemWidth((spec.m_Flags & EditVec4_RGBA) ? s_SecondaryTextSmallWidth - 28.0f : s_SecondaryTextSmallWidth);
			if (ImGui::DragFloat(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(), &(spec.m_CurrentVec4.w), spec.m_ScrollSpeed,
				spec.m_Bounds[0], spec.m_Bounds[1],
				"%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction(spec);
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(spec.m_Flags & EditVec4_RGBA ? s_Alpha : s_HighlightColor4, 
					spec.m_Flags & EditVec4_RGBA ? "Alpha Channel" : "W-Value");
				ImGui::EndTooltip();
			}

			ImGui::PopStyleVar();

			if (spec.m_Flags & EditVec4_RGBA)
			{
				ImVec4 colorPickerValue{ Utility::MathVec4ToImVec4(spec.m_CurrentVec4) };
				ImGuiColorEditFlags misc_flags = ImGuiColorEditFlags_NoInputs |
					ImGuiColorEditFlags_NoLabel |
					ImGuiColorEditFlags_AlphaPreviewHalf |
					ImGuiColorEditFlags_NoSidePreview;
				ImGui::SetCursorPos({ s_SecondaryTextPosFour + s_SecondaryTextSmallWidth - 21.0f, yPosition + 1.0f });
				if (ImGui::ColorEdit4(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(),
					(float*)&colorPickerValue, 
					misc_flags))
				{
					spec.m_CurrentVec4 = Utility::ImVec4ToMathVec4(colorPickerValue);
					if (spec.m_ConfirmAction)
					{
						spec.m_ConfirmAction(spec);
					}
				}
			}
		}
		else
		{
			float yPosition = ImGui::GetCursorPosY();
			ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
			ImGui::SetCursorPos({ s_SecondaryTextPosOne, yPosition });
			int32_t floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(spec.m_CurrentVec4.x).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.m_CurrentVec4.x),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ s_SecondaryTextPosTwo, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(spec.m_CurrentVec4.y).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.m_CurrentVec4.y),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ s_SecondaryTextPosThree, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(spec.m_CurrentVec4.z).c_str(), s_SecondaryTextSmallWidth);
			TruncateText(Utility::Conversions::FloatToString(spec.m_CurrentVec4.z),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::SetCursorPos({ s_SecondaryTextPosFour, yPosition });
			floatPosition = ImGui::FindPositionAfterLength(
				Utility::Conversions::FloatToString(spec.m_CurrentVec4.w).c_str(), s_SecondaryTextSmallWidth - 
			((spec.m_Flags & EditVec4_RGBA) ? 24.0f : 0.0f));
			TruncateText(Utility::Conversions::FloatToString(spec.m_CurrentVec4.w),
				floatPosition == -1 ? std::numeric_limits<int32_t>::max() : floatPosition);
			ImGui::PopStyleColor();

			if (spec.m_Flags & EditVec4_RGBA)
			{
				ImVec4 colorPickerValue{ Utility::MathVec4ToImVec4(spec.m_CurrentVec4) };
				ImGuiColorEditFlags misc_flags = ImGuiColorEditFlags_NoInputs |
					ImGuiColorEditFlags_NoLabel |
					ImGuiColorEditFlags_AlphaPreviewHalf |
					ImGuiColorEditFlags_NoSidePreview |
					ImGuiColorEditFlags_NoPicker;
				ImGui::SetCursorPos({ s_SecondaryTextPosFour + s_SecondaryTextSmallWidth - 21.0f, yPosition + 1.0f });
				ImGui::ColorButton(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(),
					colorPickerValue,
					misc_flags, ImVec2(18.0f, 18.0f));
			}
		}

		ImGui::SameLine();
		CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.m_Editing);
		},
		EditorUIService::s_SmallEditButton,
		spec.m_Editing, spec.m_Editing ? s_PrimaryTextColor : s_DisabledColor);
	}

	void EditorUIService::RadioSelector(RadioSelectorSpec& spec)
	{
		// Local Variables
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		uint32_t widgetCount{ 0 };

		// Draw backgrounds
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 screenPosition = ImGui::GetCursorScreenPos();
		draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne - 5.0f, screenPosition.y),
			ImVec2(s_WindowPosition.x + s_SecondaryTextPosOne + s_SecondaryTextMediumWidth + 19.0f, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRectFilled(ImVec2(s_WindowPosition.x + s_SecondaryTextPosMiddle - 5.0f, screenPosition.y),
			ImVec2(s_WindowPosition.x + s_SecondaryTextPosMiddle + s_SecondaryTextMediumWidth + 19.0f, screenPosition.y + EditorUI::EditorUIService::s_TextBackgroundHeight),
			ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 4.0f, ImDrawFlags_RoundCornersAll);

		if (spec.m_Flags & RadioSelector_Indented)
		{
			ImGui::SetCursorPosX(s_TextLeftIndentOffset);
		}

		// Display Item
		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.m_Label.c_str(), 
			spec.m_Flags & RadioSelector_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
		TruncateText(spec.m_Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);

		ImGui::PopStyleColor();
		ImGui::SameLine(s_SecondaryTextPosOne - 2.5f);

		if (spec.m_Editing)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{
				if (spec.m_SelectedOption == 0)
				{
					spec.m_SelectedOption = 1;
				}
				else
				{
					spec.m_SelectedOption = 0;
				}
				spec.m_SelectAction();
			}, s_SmallCheckboxButton, spec.m_SelectedOption == 0, s_HighlightColor1);
			ImGui::SameLine();

			int32_t position = ImGui::FindPositionAfterLength(spec.m_FirstOptionLabel.c_str(), s_SecondaryTextMediumWidth - 18.0f);
			TruncateText(spec.m_FirstOptionLabel, position == -1 ? std::numeric_limits<int32_t>::max() : position);
			
			ImGui::SameLine(s_SecondaryTextPosMiddle - 2.5f);
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{
				if (spec.m_SelectedOption == 1)
				{
					spec.m_SelectedOption = 0;
				}
				else
				{
					spec.m_SelectedOption = 1;
				}
				spec.m_SelectAction();
			}, s_SmallCheckboxButton, spec.m_SelectedOption == 1, s_HighlightColor2);
			ImGui::SameLine();
			position = ImGui::FindPositionAfterLength(spec.m_SecondOptionLabel.c_str(), s_SecondaryTextMediumWidth - 18.0f);
			TruncateText(spec.m_SecondOptionLabel, position == -1 ? std::numeric_limits<int32_t>::max() : position);

			ImGui::PopStyleColor(3);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Button, s_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), nullptr,
				s_SmallCheckboxDisabledButton, spec.m_SelectedOption == 0, s_SecondaryTextColor);
			ImGui::SameLine();

			int32_t position = ImGui::FindPositionAfterLength(spec.m_FirstOptionLabel.c_str(), s_SecondaryTextMediumWidth - 18.0f);
			TruncateText(spec.m_FirstOptionLabel, position == -1 ? std::numeric_limits<int32_t>::max() : position);

			ImGui::SameLine(s_SecondaryTextPosMiddle - 2.5f);
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), nullptr,
				s_SmallCheckboxDisabledButton, spec.m_SelectedOption == 1, s_SecondaryTextColor);
			ImGui::SameLine();
			position = ImGui::FindPositionAfterLength(spec.m_SecondOptionLabel.c_str(), s_SecondaryTextMediumWidth - 18.0f);
			TruncateText(spec.m_SecondOptionLabel, position == -1 ? std::numeric_limits<int32_t>::max() : position);
			ImGui::PopStyleColor(4);
		}

		ImGui::SameLine();
		CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.m_Editing);
		},
		EditorUIService::s_SmallEditButton,
		spec.m_Editing, spec.m_Editing ? s_PrimaryTextColor : s_DisabledColor);
	}

	void EditorUIService::List(ListSpec& spec)
	{
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		uint32_t widgetCount{ 0 };
		uint32_t smallButtonCount{ 0 };

		if (spec.m_Flags & List_Indented)
		{
			ImGui::SetCursorPosX(s_TextLeftIndentOffset);
		}
		if (!(spec.m_Flags & (List_RegularSizeTitle | List_Indented)))
		{
			ImGui::PushFont(EditorUIService::s_FontAntaLarge);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.m_Label.c_str(), 
			spec.m_Flags & List_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
		TruncateText(spec.m_Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		if (!(spec.m_Flags & (List_RegularSizeTitle | List_Indented)))
		{
			ImGui::PopFont();
		}
		s_ListExpandButton.m_IconSize = 14.0f;
		s_ListExpandButton.m_YPosition = spec.m_Flags & List_Indented ? 0.0f : 3.0f;
		ImGui::SameLine();
		CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.m_Expanded);
		}, 
		s_ListExpandButton ,spec.m_Expanded, spec.m_Expanded ? s_HighlightColor1 : s_DisabledColor);

		if (spec.m_Expanded && !spec.m_EditListSelectionList.empty())
		{
			ImGui::SameLine();
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
				{
					ImGui::OpenPopup(spec.m_WidgetID - 1);
				}, s_MediumOptionsButton, false, s_DisabledColor);

			if (ImGui::BeginPopupEx(spec.m_WidgetID - 1, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
			{
				for (auto& [label, func] : spec.m_EditListSelectionList)
				{
					if (ImGui::Selectable((label.c_str() + id).c_str()))
					{
						func();
					}
				}
				ImGui::EndPopup();
			}
		}

		if (spec.m_Flags & List_UnderlineTitle)
		{
			ImGui::Separator();
		}
		
		if (spec.m_Expanded)
		{
			if (!spec.m_ListEntries.empty())
			{
				// Column Titles
				ImGui::PushStyleColor(ImGuiCol_Text, s_HighlightColor1);
				ImGui::SetCursorPosX(spec.m_Flags & List_Indented ? 61.0f: s_TextLeftIndentOffset);
				if (spec.m_Flags & (List_Indented | List_RegularSizeTitle))
				{
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 7.0f);
				}
				labelPosition = ImGui::FindPositionAfterLength(spec.m_Column1Title.c_str(), s_SecondaryTextLargeWidth);
				TruncateText(spec.m_Column1Title, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
				ImGui::SameLine();
				ImGui::SetCursorPosX(s_SecondaryTextPosOne);
				labelPosition = ImGui::FindPositionAfterLength(spec.m_Column2Title.c_str(), s_SecondaryTextLargeWidth);
				TruncateText(spec.m_Column2Title, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
				ImGui::PopStyleColor();
				if (!(spec.m_Flags & (List_Indented | List_RegularSizeTitle)))
				{
					Spacing(SpacingAmount::Small);
				}
				
			}
			std::size_t iteration{ 0 };
			for (ListEntry& listEntry : spec.m_ListEntries)
			{
				smallButtonCount = 0;
				if (!(spec.m_Flags & (List_Indented | List_RegularSizeTitle)))
				{
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
				}
				ImGui::SetCursorPosX(spec.m_Flags & List_Indented ? 42.5f : 12.0f);
				CreateImage(s_IconDash, 8, s_DisabledColor);
				ImGui::SameLine();
				ImGui::SetCursorPosX(spec.m_Flags & List_Indented  ? 61.0f : s_TextLeftIndentOffset);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.2f);
				TruncateText(listEntry.m_Label, 16);
				ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
				if (!listEntry.m_Value.empty())
				{
					WriteMultilineText(listEntry.m_Value, s_SecondaryTextLargeWidth, s_SecondaryTextPosOne, -5.2f);
				}
				ImGui::PopStyleColor();

				if (listEntry.m_OnEdit)
				{
					s_TableEditButton.m_XPosition = SmallButtonRelativeLocation(smallButtonCount++);
					ImGui::SameLine();
					CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
					{
						if (listEntry.m_OnEdit)
						{
							listEntry.m_OnEdit(listEntry, iteration);
						}
					}, s_TableEditButton, false, s_DisabledColor);
				}
				iteration++;
			}
		}
	}

	void DrawEntries(TreeSpec& spec , std::vector<TreeEntry>& entries, uint32_t& widgetCount, TreePath& currentPath, ImVec2 rootPosition)
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
			ImVec2 buttonDimensions{ ImGui::CalcTextSize(treeEntry.m_Label.c_str()).x + 34.0f, EditorUI::EditorUIService::s_TextBackgroundHeight };

			// Create Invisible Button for Interation with current node
			if (ImGui::InvisibleButton(("##" + std::to_string(spec.m_WidgetID + WidgetIterator(widgetCount))).c_str(), buttonDimensions))
			{
				if (treeEntry.m_OnLeftClick)
				{
					treeEntry.m_OnLeftClick(treeEntry);
				}
				spec.m_SelectedEntry = currentPath;
			}

			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			{
				if (treeEntry.m_OnRightClickSelection.size() > 0)
				{
					ImGui::OpenPopup(("##" + std::to_string(spec.m_WidgetID)).c_str());
					spec.m_CurrentRightClick = &treeEntry;
				}
			}

			if (ImGui::IsItemHovered())
			{
				// Draw SelectedEntry background
				draw_list->AddRectFilled(screenPosition,
					ImVec2(screenPosition.x + buttonDimensions.x, screenPosition.y + buttonDimensions.y),
					ImColor(EditorUI::EditorUIService::s_HoveredColor), 4, ImDrawFlags_RoundCornersAll);

				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && treeEntry.m_OnDoubleLeftClick)
				{
					treeEntry.m_OnDoubleLeftClick(treeEntry);
				}
			}

			// Display Selected Background
			ImGui::SetCursorScreenPos(screenPosition);
			if (spec.m_SelectedEntry == currentPath)
			{
				// Draw SelectedEntry background
				draw_list->AddRectFilled(screenPosition,
					ImVec2(screenPosition.x + buttonDimensions.x, screenPosition.y + buttonDimensions.y),
					ImColor(EditorUI::EditorUIService::s_ActiveColor), 4, ImDrawFlags_RoundCornersAll);
				if (spec.m_SelectionChanged)
				{
					ImGui::SetScrollHereY();
					spec.m_SelectionChanged = false;
				}
			}

			// Display entry icon
			if (treeEntry.m_IconHandle)
			{
				CreateImage(treeEntry.m_IconHandle, 14, EditorUI::EditorUIService::s_HighlightColor1);
				ImGui::SameLine();
			}

			// Display entry text
			ImGui::PushStyleColor(ImGuiCol_Text, EditorUIService::s_PrimaryTextColor);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 1.5f);
			ImGui::TextUnformatted(treeEntry.m_Label.c_str());
			ImGui::PopStyleColor();


			// Handle all sub-entries
			if (treeEntry.m_SubEntries.size() > 0)
			{
				// Draw expand icon
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.5f);
				ImGui::PushStyleColor(ImGuiCol_Button, EditorUIService::s_PureEmpty);
				const Ref<Rendering::Texture2D> icon = spec.m_ExpandedNodes.contains(currentPath) ? EditorUIService::s_IconDown : EditorUIService::s_IconRight;
				if (ImGui::ImageButtonEx(spec.m_WidgetID + WidgetIterator(widgetCount),
					(ImTextureID)(uint64_t)icon->GetRendererID(),
					ImVec2(13, 13), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
					EditorUIService::s_PureEmpty,
					spec.m_ExpandedNodes.contains(currentPath) ? EditorUIService::s_HighlightColor1 : EditorUIService::s_DisabledColor, 0))
				{
					if (spec.m_ExpandedNodes.contains(currentPath))
					{
						spec.m_ExpandedNodes.erase(currentPath);
					}
					else
					{
						spec.m_ExpandedNodes.insert(currentPath);
					}
				}
				ImGui::PopStyleColor();

				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, spec.m_ExpandedNodes.contains(currentPath) ? "Collapse" : "Expand");
					ImGui::EndTooltip();
				}

				// Draw all sub-entries
				if (spec.m_ExpandedNodes.contains(currentPath))
				{
					DrawEntries(spec, treeEntry.m_SubEntries, widgetCount, currentPath, screenPosition);
				}
			}

			// Add horizontal line for each child node
			if (depth > 0)
			{
				draw_list->AddLine(ImVec2(rootPosition.x + 10.0f, screenPosition.y + 10.0f),
					ImVec2(screenPosition.x, screenPosition.y + 10.0f),
					ImColor(EditorUIService::s_PrimaryTextColor));
			}
			currentPath.PopBack();
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
		uint32_t widgetCount{ 0 };
		TreePath treePath{};
		DrawEntries(spec, spec.m_TreeEntries, widgetCount, treePath, {});

		if (ImGui::BeginPopup(("##" + std::to_string(spec.m_WidgetID)).c_str()))
		{
			if (spec.m_CurrentRightClick)
			{
				for (auto& [label, func] : spec.m_CurrentRightClick->m_OnRightClickSelection)
				{
					if (!spec.m_CurrentRightClick) { break; }
					if (ImGui::Selectable((label + "##" + std::to_string(spec.m_WidgetID)).c_str()))
					{
						func(*spec.m_CurrentRightClick);
					}
				}
			}
			ImGui::EndPopup();
		}
	}

	void EditorUIService::PanelHeader(PanelHeaderSpec& spec)
	{
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		ImGui::PushFont(EditorUIService::s_FontAntaLarge);
		ImGui::PushStyleColor(ImGuiCol_Text, spec.m_EditColorActive ? EditorUIService::s_HighlightColor2 : EditorUIService::s_PrimaryTextColor);
		ImGui::TextUnformatted(spec.m_Label.c_str());
		ImGui::PopStyleColor();
		ImGui::PopFont();

		ImGui::SameLine();
		CreateButton(spec.m_WidgetID, [&]()
		{
			ImGui::OpenPopup(id);
		}, s_MediumOptionsButton, false, s_DisabledColor);
		
		if (ImGui::BeginPopup(id))
		{
			for (auto& [label, func] : spec.GetSelectionList())
			{
				if (ImGui::Selectable((label.c_str() + id).c_str()))
				{
					func();
				}
			}
			ImGui::EndPopup();
		}
		ImGui::Separator(1.0f, s_HighlightColor1_Thin);
		Spacing(0.2f);
	}

	void EditorUIService::NavigationHeader(NavigationHeaderSpec& spec)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		if (!spec.m_IsBackActive)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		}

		// Draw icon for moving a directory back
		if (ImGui::ImageButton((ImTextureID)(uint64_t)EditorUI::EditorUIService::s_IconBack->GetRendererID(),
			{ 24.0f, 24.0f }, { 0, 1 }, { 1, 0 },
			-1, ImVec4(0, 0, 0, 0),
			spec.m_IsBackActive ? EditorUI::EditorUIService::s_PrimaryTextColor : EditorUI::EditorUIService::s_DisabledColor))
		{
			if (spec.m_IsBackActive && spec.m_OnNavigateBack)
			{
				spec.m_OnNavigateBack();
			}
		}

		if (!spec.m_IsBackActive)
		{
			ImGui::PopStyleColor(2);
		}
		// Handle back navigation's payload
		if (spec.m_Flags & NavigationHeaderFlags::NavigationHeader_AllowDragDrop && 
			spec.m_IsBackActive && 
			spec.m_OnReceivePayloadBack &&
			ImGui::BeginDragDropTarget())
		{
			for (const char* payloadName : spec.m_AcceptableOnReceivePayloads)
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName))
				{
					spec.m_OnReceivePayloadBack(payloadName , payload->Data, payload->DataSize);
					break;
				}
			}
			ImGui::EndDragDropTarget();
		}

		// Draw forward navigation icon
		ImGui::SameLine();
		if (!spec.m_IsForwardActive)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		}
		if (ImGui::ImageButton((ImTextureID)(uint64_t)EditorUI::EditorUIService::s_IconForward->GetRendererID(),
			{ 24.0f, 24.0f }, { 0, 1 }, { 1, 0 },
			-1, ImVec4(0, 0, 0, 0),
			spec.m_IsForwardActive ? EditorUI::EditorUIService::s_PrimaryTextColor : EditorUI::EditorUIService::s_DisabledColor))
		{
			if (spec.m_IsForwardActive && spec.m_OnNavigateForward)
			{
				spec.m_OnNavigateForward();
			}
		}
		if (!spec.m_IsForwardActive)
		{
			ImGui::PopStyleColor(2);
		}
		if (spec.m_Flags & NavigationHeaderFlags::NavigationHeader_AllowDragDrop &&
			spec.m_IsForwardActive &&
			spec.m_OnReceivePayloadForward &&
			ImGui::BeginDragDropTarget())
		{
			for (const char* payloadName : spec.m_AcceptableOnReceivePayloads)
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName))
				{
					spec.m_OnReceivePayloadForward(payloadName, payload->Data, payload->DataSize);
					break;
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopStyleColor();

		ImGui::PushFont(EditorUI::EditorUIService::s_FontPlexBold);
		ImGui::SameLine();
		ImGui::TextUnformatted(spec.m_Label);
		ImGui::PopFont();

		ImGui::Separator();
		Spacing(0.2f);
	}

	void EditorUIService::Grid(GridSpec& spec)
	{
		uint32_t widgetCount{ 0 };
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);

		// Calculate grid cell count using provided spec sizes
		float cellSize = spec.m_CellIconSize + spec.m_CellPadding;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int32_t columnCount = (int32_t)(panelWidth / cellSize);
		columnCount = columnCount > 0 ? columnCount : 1;

		// Start drawing columns
		ImGui::Columns(columnCount, id.CString(), false);
		ImGui::PushStyleColor(ImGuiCol_Button, s_PureEmpty);
		for (GridEntry& currentEntry : spec.m_Entries)
		{
			// Check if entry is selected
			bool entryIsSelected = currentEntry.m_EntryID == spec.m_SelectedEntry;

			// Get entry archetype and grid element ID
			FixedString<16> entryID{ id };
			GridEntryArchetype* entryArchetype = &(spec.m_EntryArchetypes.at(currentEntry.m_ArchetypeID));
			entryID.AppendInteger(WidgetIterator(widgetCount));
			KG_ASSERT(entryArchetype);

			// Display grid icon
			ImGui::PushID(entryID.CString());
			if (ImGui::ImageButton((ImTextureID)(uint64_t)entryArchetype->m_Icon->GetRendererID(), { spec.m_CellIconSize, spec.m_CellIconSize },
				{ 0, 1 }, { 1, 0 },
				-1, entryIsSelected ? s_ActiveColor : s_PureEmpty,
				entryArchetype->m_IconColor))
			{
				// Handle on left-click
				if (entryArchetype->m_OnLeftClick)
				{
					entryArchetype->m_OnLeftClick(currentEntry);
				}
				spec.m_SelectedEntry = currentEntry.m_EntryID;
			}

			// Handle double left clicks
			if (entryArchetype->m_OnDoubleLeftClick && 
				ImGui::IsItemHovered() &&
				ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				entryArchetype->m_OnDoubleLeftClick(currentEntry);
			}

			// Handle right clicks
			if (entryArchetype->m_OnRightClick && 
				ImGui::IsItemHovered() && 
				ImGui::IsMouseClicked(ImGuiMouseButton_Right)
				)
			{
				entryArchetype->m_OnRightClick(currentEntry);
			}

			// Handle payloads
			if (spec.m_Flags & GridFlags::Grid_AllowDragDrop)
			{
				// Handle create payload
				if (entryArchetype->m_OnCreatePayload && ImGui::BeginDragDropSource())
				{
					DragDropPayload newPayload;
					entryArchetype->m_OnCreatePayload(currentEntry, newPayload);
					ImGui::SetDragDropPayload(newPayload.m_Label, newPayload.m_DataPointer, newPayload.m_DataSize, ImGuiCond_Once);
					ImGui::EndDragDropSource();
				}

				// Handle receive payload
				if (entryArchetype->m_OnReceivePayload && ImGui::BeginDragDropTarget())
				{
					for (const char* payloadName : entryArchetype->m_AcceptableOnReceivePayloads)
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName))
						{
							entryArchetype->m_OnReceivePayload(currentEntry, payloadName, payload->Data, payload->DataSize);
							break;
						}
					}
					ImGui::EndDragDropTarget();
				}
			}

			// Draw label for each cell
			ImGui::TextWrapped(currentEntry.m_Label.CString());

			// Reset cell data for next call
			ImGui::NextColumn();
			ImGui::PopID();
		}

		// End drawing columns
		ImGui::PopStyleColor();
		ImGui::Columns(1);
	}

	void EditorUIService::CollapsingHeader(CollapsingHeaderSpec& spec)
	{
		uint32_t widgetCount{ 0 };
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		ImGui::PushFont(EditorUIService::s_FontAntaLarge);
		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		ImGui::TextUnformatted(spec.m_Label.c_str());
		ImGui::PopStyleColor();
		ImGui::PopFont();
		ImGui::SameLine();
		CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
		{
			Utility::Operations::ToggleBoolean(spec.m_Expanded);
		},
		s_SmallExpandButton, spec.m_Expanded, spec.m_Expanded ? s_HighlightColor1 : s_DisabledColor);

		if (spec.m_Expanded && !spec.m_SelectionList.empty())
		{
			ImGui::SameLine();
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{
				ImGui::OpenPopup(spec.m_WidgetID - 1);
			}, s_MediumOptionsButton, false, s_DisabledColor);

			if (ImGui::BeginPopupEx(spec.m_WidgetID - 1, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
			{
				for (auto& [label, func] : spec.m_SelectionList)
				{
					if (ImGui::Selectable((label.c_str() + id).c_str()))
					{
						func(spec);
					}
				}
				ImGui::EndPopup();
			}
		}

		if ((spec.m_Flags & CollapsingHeader_UnderlineTitle) && spec.m_Expanded)
		{
			ImGui::Separator();
		}

		if (spec.m_Expanded && spec.m_OnExpand)
		{
			spec.m_OnExpand();
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
	void EditorUIService::Text(const char* text)
	{
		ImGui::TextUnformatted(text);
	}

	void EditorUIService::EditText(EditTextSpec& spec)
	{
		// Local Variables
		static char stringBuffer[256];
		uint32_t widgetCount{ 0 };
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		std::string popUpLabel = spec.m_Label;

		if (spec.m_Flags & EditText_PopupOnly)
		{
			if (spec.m_StartPopup)
			{
				ImGui::OpenPopup(id);
				spec.m_StartPopup = false;
				memset(stringBuffer, 0, sizeof(stringBuffer));
				memcpy(stringBuffer, spec.m_CurrentOption.data(), sizeof(stringBuffer));
			}
		}
		else
		{
			if (spec.m_Flags & EditText_Indented)
			{
				ImGui::SetCursorPosX(s_TextLeftIndentOffset);
			}
			ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
			int32_t labelPosition = ImGui::FindPositionAfterLength(spec.m_Label.c_str(), 
				spec.m_Flags & EditText_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
			TruncateText(spec.m_Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
			WriteMultilineText(spec.m_CurrentOption, s_SecondaryTextLargeWidth, s_SecondaryTextPosOne);
			ImGui::PopStyleColor();

			ImGui::SameLine();
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount),[&]()
			{
				ImGui::OpenPopup(id);
				memset(stringBuffer, 0, sizeof(stringBuffer));
				memcpy(stringBuffer, spec.m_CurrentOption.data(), sizeof(stringBuffer));
			},
			EditorUIService::s_SmallEditButton, false, s_DisabledColor);
		}

		ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f));
		if (ImGui::BeginPopupModal(id, NULL, ImGuiWindowFlags_NoTitleBar))
		{
			EditorUI::EditorUIService::TitleText(popUpLabel);

			ImGui::PushFont(EditorUI::EditorUIService::s_FontAntaRegular);

			// Cancel Tool Bar Button
			ImGui::SameLine();
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{
				memset(stringBuffer, 0, sizeof(stringBuffer));
				ImGui::CloseCurrentPopup();
			}, s_LargeCancelButton);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{
				spec.m_CurrentOption = std::string(stringBuffer);
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction(spec);
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
	void EditorUIService::EditMultiLineText(EditMultiLineTextSpec& spec)
	{
		// Local Variables
		static char stringBuffer[2 * 1024];
		uint32_t widgetCount{ 0 };
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);
		std::string popUpLabel = spec.m_Label;

		if (spec.m_Flags & EditText_PopupOnly)
		{
			if (spec.m_StartPopup)
			{
				ImGui::OpenPopup(id);
				spec.m_StartPopup = false;
				memset(stringBuffer, 0, sizeof(stringBuffer));
				memcpy(stringBuffer, spec.m_CurrentOption.data(), sizeof(stringBuffer));
			}
		}
		else
		{
			if (spec.m_Flags & EditText_Indented)
			{
				ImGui::SetCursorPosX(s_TextLeftIndentOffset);
			}
			ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
			int32_t labelPosition = ImGui::FindPositionAfterLength(spec.m_Label.c_str(),
				spec.m_Flags & EditText_Indented ? s_PrimaryTextIndentedWidth : s_PrimaryTextWidth);
			TruncateText(spec.m_Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
			WriteMultilineText(spec.m_CurrentOption, s_SecondaryTextLargeWidth, s_SecondaryTextPosOne);
			ImGui::PopStyleColor();

			ImGui::SameLine();
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{
				ImGui::OpenPopup(id);
				memset(stringBuffer, 0, sizeof(stringBuffer));
				memcpy(stringBuffer, spec.m_CurrentOption.data(), sizeof(stringBuffer));
			},
			EditorUIService::s_SmallEditButton, false, s_DisabledColor);
		}

		ImGuiWindowFlags popupFlags
		{
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse
		};

		ImGui::SetNextWindowSize(ImVec2(700.0f, 500.0f));
		if (ImGui::BeginPopupModal(id, NULL, popupFlags))
		{
			EditorUI::EditorUIService::TitleText(popUpLabel);

			ImGui::PushFont(EditorUI::EditorUIService::s_FontAntaRegular);

			// Cancel Tool Bar Button
			ImGui::SameLine();
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{
				memset(stringBuffer, 0, sizeof(stringBuffer));
				ImGui::CloseCurrentPopup();
			}, s_LargeCancelButton);

			// Confirm Tool Bar Button
			ImGui::SameLine();
			CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
			{
				spec.m_CurrentOption = std::string(stringBuffer);
				if (spec.m_ConfirmAction)
				{
					spec.m_ConfirmAction(spec);
				}
				memset(stringBuffer, 0, sizeof(stringBuffer));
				ImGui::CloseCurrentPopup();
			}, s_LargeConfirmButton);

			ImGui::Separator();

			//ImGui::SetNextItemWidth(0.0f);
			
			ImGui::InputTextMultiline((id + "InputText").c_str(), stringBuffer, sizeof(stringBuffer), ImVec2(683.0f, 450.0f));
			ImGui::PopFont();
			ImGui::EndPopup();
		}
	}
	void EditorUIService::ChooseDirectory(ChooseDirectorySpec& spec)
	{
		// Local Variables
		uint32_t widgetCount{ 0 };
		std::string popUpLabel = spec.m_Label;

		ImGui::PushStyleColor(ImGuiCol_Text, s_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(spec.m_Label.c_str(), s_PrimaryTextWidth);
		TruncateText(spec.m_Label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Text, s_SecondaryTextColor);
		WriteMultilineText(spec.m_CurrentOption.string(), s_SecondaryTextLargeWidth, s_SecondaryTextPosOne);
		ImGui::PopStyleColor();

		ImGui::SameLine();
		CreateButton(spec.m_WidgetID + WidgetIterator(widgetCount), [&]()
		{
			const std::filesystem::path initialDirectory = spec.m_CurrentOption.empty() ? std::filesystem::current_path() : spec.m_CurrentOption;
			std::filesystem::path outputDirectory = Utility::FileDialogs::ChooseDirectory(initialDirectory);
			if (outputDirectory.empty())
			{
				KG_WARN("Empty path returned to ChooseDirectory");
				return;
			}
			spec.m_CurrentOption = outputDirectory;
			if (spec.m_ConfirmAction)
			{
				spec.m_ConfirmAction(outputDirectory.string());
			}
		},
		EditorUIService::s_SmallEditButton, false, s_DisabledColor);

	}


	void ProcessTooltipEntries(TooltipSpec& spec, std::vector<TooltipEntry>& entryList)
	{
		// Process each entry in current list
		for (TooltipEntry& currentEntry : entryList)
		{
			// Ignore entries that are not visible
			if (!currentEntry.m_IsVisible)
			{
				continue;
			}

			// Handle case where current entry acts as a menu
			if (std::vector<TooltipEntry>* subEntryList = std::get_if<std::vector<TooltipEntry>>(&currentEntry.m_EntryData))
			{
				// Create menu option
				if (ImGui::BeginMenu(currentEntry.m_Label))
				{
					// Handle sub entries using recursive call
					ProcessTooltipEntries(spec, *subEntryList);

					ImGui::EndMenu();
				}


			}
			// Handle case where current entry is a terminal node
			else if (std::function<void(TooltipEntry&)>* terminalNode = std::get_if<std::function<void(TooltipEntry&)>>(&currentEntry.m_EntryData))
			{
				// Display current node
				if (ImGui::MenuItem(currentEntry.m_Label))
				{
					// Call function pointer if valid
					std::function<void(TooltipEntry&)>& functionPointerRef = *terminalNode;
					if (functionPointerRef)
					{
						functionPointerRef(currentEntry);
					}
				}
			}
			else if (TooltipSeperatorData* tooltipSeperatorData = std::get_if<TooltipSeperatorData>(&currentEntry.m_EntryData))
			{
				ImGui::Separator(1.0f, tooltipSeperatorData->m_SeperatorColor);
			}
			else
			{
				KG_ERROR("Invalid variant type of entry data provided for tooltip entry");
			}

		}
	}



	void EditorUIService::Tooltip(TooltipSpec& spec)
	{
		uint32_t widgetCount{ 0 };
		FixedString<16> id{ "##" };
		id.AppendInteger(spec.m_WidgetID);

		// Handle turning on the tooltip
		if (spec.m_TooltipActive)
		{
			// Only open tooltip if menu items are present
			if (spec.m_Entries.size() != 0)
			{
				ImGui::OpenPopup(id);
			}
			spec.m_TooltipActive = false;
		}

		// Draw tooltip if active
		if (ImGui::BeginPopup(id))
		{
			// Recursively handle tooltip items and sub-menus
			ProcessTooltipEntries(spec, spec.m_Entries);
			ImGui::EndPopup();
		}
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

			if (treeEntry.m_SubEntries.size() > 0)
			{
				bool success = RecursiveGetPathFromEntry(outputPath, entryQuery, treeEntry.m_SubEntries);
				if (success)
				{
					return true;
				}
			}

			outputPath.PopBack();
			iteration++;
		}
		return false;
	}

	void TreeSpec::ExpandFirstLayer()
	{
		for (auto& entry : m_TreeEntries)
		{
			m_ExpandedNodes.insert(GetPathFromEntryReference(&entry));
		}
	}

	void TreeSpec::ExpandNodePath(TreePath& path)
	{
		if (GetEntryFromPath(path))
		{
			m_ExpandedNodes.insert(path);
		}
	}

	TreeEntry* TreeSpec::GetEntryFromPath(TreePath& path)
	{
		std::vector<TreeEntry>* currentEntryList = &m_TreeEntries;
		TreeEntry* currentEntry{nullptr};
		for (auto node : path.GetPath())
		{
			if (node >= currentEntryList->size())
			{
				return {};
			}

			currentEntry = &currentEntryList->at(node);
			currentEntryList = &currentEntry->m_SubEntries;
		}

		return currentEntry;
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

	void TreeSpec::MoveUp()
	{
		uint16_t currentSelectedBack = m_SelectedEntry.GetBack();
		TreePath newPath = m_SelectedEntry;

		// Check if we can move up within current parent node
		if (currentSelectedBack <= 0)
		{
			// If we are in the top-most depth, we cannot move up further
			if (newPath.GetDepth() <= 1)
			{
				return;
			}
			// If we cannot move up in current parent node, find node above current parent node
			newPath.PopBack();
			currentSelectedBack = newPath.GetBack();
			// Check if a node exists above current parent node
			if (currentSelectedBack <= 0)
			{
				return;
			}
			// Set new path to last entry of new parent node
			currentSelectedBack--;
			newPath.SetBack(currentSelectedBack);
			TreeEntry* entry = GetEntryFromPath(newPath);
			TreePath newParentPath = newPath;
			newPath.AddNode((uint16_t)entry->m_SubEntries.size() - 1);
			// Exit if no final node could be found
			if (!GetEntryFromPath(newPath))
			{
				return;
			}
			// Set new SelectedEntry
			m_ExpandedNodes.insert(newParentPath);
			m_SelectedEntry = newPath;
			m_SelectionChanged = true;
			return;

		}
		// Move up within current parent node
		currentSelectedBack--;
		newPath.SetBack(currentSelectedBack);
		m_SelectedEntry = newPath;
		m_SelectionChanged = true;
	}

	void TreeSpec::MoveDown()
	{
		uint16_t currentSelectedBack = m_SelectedEntry.GetBack();
		currentSelectedBack++;
		TreePath newPath = m_SelectedEntry;
		newPath.SetBack(currentSelectedBack);

		// Check if new path leads to valid entry
		if (!GetEntryFromPath(newPath))
		{
			// If we are in the top-most depth, we cannot move down further
			if (newPath.GetDepth() <= 1)
			{
				return;
			}
			// Attempt to move to the next node in the same depth of the tree
			newPath.PopBack();
			currentSelectedBack = newPath.GetBack();
			currentSelectedBack++;
			newPath.SetBack(currentSelectedBack);
			TreePath newParentPath = newPath;
			newPath.AddNode(0);
			// Exit if no node could be found
			if (!GetEntryFromPath(newPath))
			{
				return;
			}
			m_ExpandedNodes.insert(newParentPath);
		}
		m_SelectedEntry = newPath;
		m_SelectionChanged = true;
	}

	void TreeSpec::MoveLeft()
	{
		std::size_t currentDepth = m_SelectedEntry.GetDepth();

		// Exit if we are already at the top level of tree
		if (currentDepth <= 1)
		{
			if (m_ExpandedNodes.contains(m_SelectedEntry))
			{
				m_ExpandedNodes.erase(m_SelectedEntry);
			}
			return;
		}

		TreePath newPath = m_SelectedEntry;
		newPath.PopBack();
		m_SelectedEntry = newPath;
		m_SelectionChanged = true;
	}

	void TreeSpec::MoveRight()
	{
		TreeEntry* currentEntry = GetEntryFromPath(m_SelectedEntry);

		// Exit if current entry node does not contain any sub entries
		if (!currentEntry || currentEntry->m_SubEntries.size() == 0)
		{
			return;
		}
		if (!m_ExpandedNodes.contains(m_SelectedEntry))
		{
			m_ExpandedNodes.insert(m_SelectedEntry);
		}
		m_SelectedEntry.AddNode(0);
		m_SelectionChanged = true;
	}

	void TreeSpec::SelectFirstEntry()
	{
		if (m_TreeEntries.size() > 0)
		{
			m_SelectedEntry = GetPathFromEntryReference(&m_TreeEntries.at(0));
		}
	}

	bool TreeSpec::SelectEntry(TreePath& path)
	{
		// Check if entry exists
		if (TreeEntry* entry = GetEntryFromPath(path))
		{
			// Handle Left Click
			if (entry->m_OnLeftClick)
			{
				entry->m_OnLeftClick(*entry);
			}
			// Set SelectedEntry to provided path
			m_SelectedEntry = path;
			m_SelectionChanged = true;
			return true;
		}

		// Entry could not be found
		return false;

	}

	TreeEntry* TreeSpec::SearchFirstLayer(UUID handle)
	{
		for (TreeEntry& entry : m_TreeEntries)
		{
			// Found the entry
			if (entry.m_Handle == handle)
			{
				return &entry;
			}
		}
		return {};
	}

	TreeEntry* TreeSpec::SearchDepth(UUID queryHandle, size_t terminalDepth)
	{
		if (terminalDepth == 0)
		{
			for (TreeEntry& entry : m_TreeEntries)
			{
				if (entry.m_Handle == queryHandle)
				{
					return &entry;
				}
			}
			return nullptr;
		}

		TreeEntry* returnEntry{ nullptr };
		for (TreeEntry& entry : m_TreeEntries)
		{
			returnEntry = SearchDepthRecursive(entry, 1, terminalDepth, queryHandle);
			if (returnEntry)
			{
				return returnEntry;
			}
		}
		return nullptr;
	}

	TreeEntry* TreeSpec::SearchDepthRecursive(TreeEntry& currentEntry, size_t currentDepth, size_t terminalDepth, UUID queryHandle)
	{
		if (currentDepth == terminalDepth)
		{
			for (TreeEntry& subEntry : currentEntry.m_SubEntries)
			{
				if (subEntry.m_Handle == queryHandle)
				{
					return &subEntry;
				}
			}
			return nullptr;
		}

		if (currentDepth > terminalDepth)
		{
			return nullptr;
		}

		TreeEntry* returnedEntry{ nullptr };
		for (TreeEntry& subEntry : currentEntry.m_SubEntries)
		{
			returnedEntry = SearchDepthRecursive(subEntry, currentDepth + 1, terminalDepth, queryHandle);
			if (returnedEntry)
			{
				return returnedEntry;
			}
		}
		return nullptr;
	}

	void TreeSpec::SearchDepthRecursive(TreeEntry& currentEntry, size_t currentDepth, size_t terminalDepth,
		std::function<bool(TreeEntry& entry)> searchFunction, std::vector<TreePath>& allPaths)
	{
		if (currentDepth >= terminalDepth)
		{
			for (TreeEntry& subEntry : currentEntry.m_SubEntries)
			{
				if (searchFunction(subEntry))
				{
					allPaths.push_back(GetPathFromEntryReference(&subEntry));
				}
			}
			return;
		}

		for (TreeEntry& subEntry : currentEntry.m_SubEntries)
		{
			SearchDepthRecursive(subEntry, currentDepth + 1, terminalDepth, searchFunction, allPaths);
		}

	}

	std::vector<TreePath> TreeSpec::SearchDepth(std::function<bool(TreeEntry& entry)> searchFunction, size_t terminalDepth)
	{
		std::vector<TreePath> allPaths {};
		if (terminalDepth == 0)
		{
			for (TreeEntry& entry : m_TreeEntries)
			{
				if (searchFunction(entry))
				{
					allPaths.push_back(GetPathFromEntryReference(&entry));
				}
			}
			return allPaths;
		}

		for (TreeEntry& entry : m_TreeEntries)
		{
			SearchDepthRecursive(entry, 1, terminalDepth, searchFunction, allPaths);
		}
		return allPaths;
	}

	void TreeSpec::EditDepthRecursive(TreeEntry& currentEntry, size_t currentDepth, size_t terminalDepth, std::function<void(TreeEntry& entry)> editFunction)
	{
		if (currentDepth >= terminalDepth)
		{
			for (TreeEntry& subEntry : currentEntry.m_SubEntries)
			{
				editFunction(subEntry);
			}
			return;
		}

		for (auto& subEntry : currentEntry.m_SubEntries)
		{
			EditDepthRecursive(subEntry, currentDepth + 1, terminalDepth, editFunction);
		}
	}

	void TreeSpec::EditDepth(std::function<void(TreeEntry& entry)> editFunction, size_t terminalDepth)
	{
		std::vector<TreePath> allPaths {};
		if (terminalDepth == 0)
		{
			for (TreeEntry& entry : m_TreeEntries)
			{
				editFunction(entry);
			}
			return;
		}

		for (TreeEntry& entry : m_TreeEntries)
		{
			EditDepthRecursive(entry, 1, terminalDepth, editFunction);
		}
	}

	void TreeSpec::RemoveEntry(TreePath& path)
	{
		uint32_t iteration{ 0 };
		TreeEntry* parentEntry{ nullptr };
		TreeEntry* currentEntry{ nullptr };

		// Locate entry and its parent entry using the provided path
		for (uint16_t location : path.GetPath())
		{
			if (iteration == 0)
			{
				if (location >= m_TreeEntries.size())
				{
					KG_WARN("Failed to remove entry. Path provided is beyond TreeEntries vector bounds!");
					return;
				}
				currentEntry = &(m_TreeEntries.at(location));
			}
			else
			{
				if (location >= currentEntry->m_SubEntries.size())
				{
					KG_WARN("Failed to remove entry. Path provided is beyond SubEntries vector bounds!");
					return;
				}
				parentEntry = currentEntry;
				currentEntry = &(currentEntry->m_SubEntries.at(location));
			}
			iteration++;
		}

		if (!currentEntry)
		{
			KG_WARN("Failed to remove entry. Invalid entry acquired");
			return;
		}

		uint32_t locationCurrentList = path.GetPath().at(iteration - 1);

		// Handle Right Click
		if (m_CurrentRightClick == currentEntry)
		{
			m_CurrentRightClick = nullptr;
		}

		// Clear SelectedEntry field if path is the same
		if (m_SelectedEntry == path)
		{
			m_SelectedEntry = {};
		}

		// Decriment SelectedEntry if end of path is greater
		if (m_SelectedEntry.SameParentPath(path) && m_SelectedEntry.GetPath().at(iteration - 1) > locationCurrentList)
		{
			m_SelectedEntry.SetNode(m_SelectedEntry.GetPath().at(m_SelectedEntry.GetPath().size() - 1) - 1, m_SelectedEntry.GetPath().size() - 1);
		}

		// Remove Entry from Tree
		if (parentEntry == nullptr)
		{
			m_TreeEntries.erase(m_TreeEntries.begin() + locationCurrentList);
		}
		else
		{
			parentEntry->m_SubEntries.erase(parentEntry->m_SubEntries.begin() + locationCurrentList);
		}

		// Decriment elements inside ExpandedNodes that are higher than the provided path
		std::vector<TreePath> pathCache {};
		for (auto nodePath : m_ExpandedNodes)
		{
			if (nodePath.SameParentPath(path) && nodePath.GetPath().at(iteration - 1) > locationCurrentList)
			{
				pathCache.push_back(nodePath);
			}
		}
		// Remove current path from ExpandedNodes
		if (m_ExpandedNodes.contains(path))
		{
			m_ExpandedNodes.erase(path);
		}
		for (auto& nodePath : pathCache)
		{
			m_ExpandedNodes.erase(nodePath);
			nodePath.SetNode(nodePath.GetPath().at(nodePath.GetPath().size() - 1) - 1, nodePath.GetPath().size() - 1);
		}
		for (auto& nodePath : pathCache)
		{
			m_ExpandedNodes.insert(nodePath);
		}
	}

	bool TooltipSpec::ValidateEntryID(UUID newEntryID)
	{
		// Ensure empty id is not provided
		if (newEntryID == k_EmptyUUID)
		{
			return false;
		}

		return ValidateEntryIDRecursive(m_Entries, newEntryID);
	}

	bool TooltipSpec::ValidateEntryIDRecursive(std::vector<TooltipEntry>& entries, UUID queryID)
	{
		// Ensure that no match id is found in internal entries list
		for (TooltipEntry& currentEntry : entries)
		{
			// Check each entry
			if (currentEntry.m_EntryID == queryID)
			{
				return false;
			}

			// Check for recursive entries inside menu
			if (std::vector<TooltipEntry>* subEntriesListRef = std::get_if<std::vector<TooltipEntry>>(&currentEntry.m_EntryData))
			{
				std::vector<TooltipEntry>& subEntriesList = *subEntriesListRef;
				bool success = ValidateEntryIDRecursive(subEntriesList, queryID);
				if (!success)
				{
					return false;
				}
			}
		}
		
		// Return true, if no duplicates are found
		return true;
	}

	bool TooltipSpec::SetIsVisible(UUID entry, bool isVisible)
	{
		// Ensure empty id is not provided
		if (entry == k_EmptyUUID)
		{
			return false;
		}

		return SetIsVisibleRecursive(m_Entries, entry, isVisible);
	}

	bool TooltipSpec::SetIsVisibleRecursive(std::vector<TooltipEntry>& entries, UUID queryID, bool isVisible)
	{
		// Ensure that no match id is found in internal entries list
		for (TooltipEntry& currentEntry : entries)
		{
			// Check each entry
			if (currentEntry.m_EntryID == queryID)
			{
				// Handle setting visibility
				currentEntry.m_IsVisible = isVisible;
				return true;
			}

			// Check for recursive entries inside menu
			if (std::vector<TooltipEntry>* subEntriesListRef = std::get_if<std::vector<TooltipEntry>>(&currentEntry.m_EntryData))
			{
				std::vector<TooltipEntry>& subEntriesList = *subEntriesListRef;
				bool success = SetIsVisibleRecursive(subEntriesList, queryID, isVisible);
				if (success)
				{
					return true;
				}
			}
		}

		// Return true, if no duplicates are found
		return false;
	}

	bool TooltipSpec::SetAllChildrenIsVisible(UUID entry, bool isVisible)
	{
		// Ensure empty id is not provided
		if (entry == k_EmptyUUID)
		{
			return false;
		}

		return SetAllChildrenIsVisibleRecursive(m_Entries, entry, isVisible);
	}

	bool TooltipSpec::SetAllChildrenIsVisibleRecursive(std::vector<TooltipEntry>& entries, UUID queryID, bool isVisible)
	{
		// Ensure that no match id is found in internal entries list
		for (TooltipEntry& currentEntry : entries)
		{
			// Check each entry
			if (currentEntry.m_EntryID == queryID)
			{
				// Handle setting visibility
				if (std::vector<TooltipEntry>* subEntriesListRef = std::get_if<std::vector<TooltipEntry>>(&currentEntry.m_EntryData))
				{
					std::vector<TooltipEntry>& subEntriesList = *subEntriesListRef;
					for (TooltipEntry& subEntry : subEntriesList)
					{
						subEntry.m_IsVisible = isVisible;
					}
					return true;
				}
				// Found entry, however, entry is not a menu.
				else
				{
					return false;
				}
			}

			// Check for recursive entries inside menu
			if (std::vector<TooltipEntry>* subEntriesListRef = std::get_if<std::vector<TooltipEntry>>(&currentEntry.m_EntryData))
			{
				std::vector<TooltipEntry>& subEntriesList = *subEntriesListRef;
				bool success = SetAllChildrenIsVisibleRecursive(subEntriesList, queryID, isVisible);
				if (success)
				{
					return true;
				}
			}
		}

		// Return true, if no duplicates are found
		return false;
	}
}
