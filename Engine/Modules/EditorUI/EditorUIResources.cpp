#include "kgpch.h"

#include "Modules/EditorUI/EditorUIResources.h"

#include "Modules/Rendering/Texture.h"

#include "Modules/Core/Engine.h"

namespace Kargono::EditorUI
{
	void GeneralIcons::LoadIcons(const std::filesystem::path& path)
	{
		// Set Up Editor Resources
		m_Camera = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Camera.png").string());
		m_Settings = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Settings.png").string());
		m_Delete = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Delete.png").string());
		m_Edit = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Edit.png").string());
		m_Cancel = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Cancel.png").string());
		m_Cancel2 = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Cancel2.png").string());
		m_Confirm = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Confirm.png").string());
		m_Search = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Search.png").string());
		m_Options = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Options.png").string());
		m_Down = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Down.png").string());
		m_Right = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Right.png").string());
		m_Checkbox_Enabled = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/CheckboxEnabled.png").string());
		m_Checkbox_Disabled = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/CheckboxDisabled.png").string());
		m_Dash = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Dash.png").string());
		m_AI = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/AI.png").string());
		m_Notification = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Notification.png").string());
	}
	void SceneGraphIcons::LoadIcons(const std::filesystem::path& path)
	{
		// Scene icons
		m_Entity = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/SceneEditor/Entity.png").string());
		m_BoxCollider = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/SceneEditor/BoxCollider.png").string());
		m_Tag = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/SceneEditor/Tag.png").string());
		m_CircleCollider = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/SceneEditor/CircleCollider.png").string());
		m_ClassInstance = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/SceneEditor/ClassInstance.png").string());
		m_RigidBody = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/SceneEditor/Rigidbody.png").string());
		m_Transform = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/SceneEditor/Transform.png").string());
		m_Particles = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/SceneEditor/Particles.png").string());

	}
	void ViewportIcons::LoadIcons(const std::filesystem::path& path)
	{

		// Viewport icons
		m_Grid = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Viewport/Grid.png").string());
		m_Display = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Viewport/Display.png").string());
		m_Play = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Viewport/Play.png").string());
		m_Stop = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Viewport/Stop.png").string());
		m_Pause = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Viewport/Pause.png").string());
		m_Step = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Viewport/Step.png").string());
		m_Simulate = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/Viewport/Simulate.png").string());
	}
	void RuntimeUIIcons::LoadIcons(const std::filesystem::path& path)
	{
		// Runtime UI icons
		m_Window = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/UIEditor/Window.png").string());
		m_TextWidget = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/UIEditor/TextWidget.png").string());
		m_ButtonWidget = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/UIEditor/ButtonWidget.png").string());
		m_InputTextWidget = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/UIEditor/InputText.png").string());
		m_UserInterface2 = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/UIEditor/UserInterface2.png").string());
		m_ImageButtonWidget = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/UIEditor/ImageButtonWidget.png").string());
		m_SliderWidget = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/UIEditor/SliderWidget.png").string());
		m_DropDownWidget = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/UIEditor/DropDownWidget.png").string());
		m_HorizontalContainer = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/UIEditor/HorizontalContainer.png").string());
		m_VerticalContainer = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/UIEditor/VerticalContainer.png").string());
	}
	void ScriptingIcons::LoadIcons(const std::filesystem::path& path)
	{
		// Scripting UI icons
		m_Number = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/TextEditor/Number.png").string());
		m_Variable = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/TextEditor/Variable.png").string());
		m_Function = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/TextEditor/Function.png").string());
		m_Boolean = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/TextEditor/Boolean.png").string());
		m_Decimal = Rendering::Texture2D::CreateEditorTexture((path / "Resources/Icons/TextEditor/Decimal.png").string());
	}
	void ContentBrowserIcons::LoadIcons(const std::filesystem::path& path)
	{
		// Content Browser icons
		m_Directory = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/Directory.png");
		m_GenericFile = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/GenericFile.png");
		m_Back = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/Back.png");
		m_Forward = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/Forward.png");
		m_Audio = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/Audio.png");
		m_Texture = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/Texture.png");
		m_Binary = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/Binary.png");
		m_Scene = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/Scene.png");
		m_Scene_KG = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/Scene_KG.png");
		m_Registry = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/Registry.png");
		m_Font = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/Font.png");
		m_UserInterface = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/UserInterface.png");
		m_Input = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/Input.png");
		m_AI_KG = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/AI_KG.png");
		m_Audio_KG = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/Audio_KG.png");
		m_Font_KG = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/Font_KG.png");
		m_GlobalState = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/GameState.png");
		m_ProjectComponent = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/ProjectComponent.png");
		m_Script = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/Script.png");
		m_Texture_KG = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/Texture_KG.png");
		m_EmitterConfig = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/EmitterConfig.png");
		m_Enum = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/Enum.png");
		m_ColorPalette = Rendering::Texture2D::CreateEditorTexture(path / "Resources/Icons/ContentBrowser/ColorPalette.png");

	}
	void ConfigFonts::LoadFonts(ImGuiIO& io)
	{
		m_HeaderLarge = io.Fonts->AddFontFromFileTTF("Resources/Fonts/Anta-Regular.ttf", 23.0f);
		m_HeaderRegular = io.Fonts->AddFontFromFileTTF("Resources/Fonts/Anta-Regular.ttf", 20.0f);
		m_Title = io.Fonts->AddFontFromFileTTF("Resources/Fonts/IBMPlexMono-Bold.ttf", 29.0f);
		m_Default = io.Fonts->AddFontFromFileTTF("Resources/Fonts/RobotoMono-SemiBold.ttf", 17.0f);
		io.FontDefault = m_Default;
	}
	void ConfigColors::SetColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = m_BackgroundColor;
		colors[ImGuiCol_PopupBg] = m_DarkBackgroundColor;

		// TextColor
		colors[ImGuiCol_Text] = m_PrimaryTextColor;

		// Separator
		colors[ImGuiCol_Separator] = m_AccentColor;
		colors[ImGuiCol_SeparatorActive] = m_HighlightColor1_Thin;
		colors[ImGuiCol_SeparatorHovered] = m_HighlightColor1_Thin;

		// Drag Drop
		colors[ImGuiCol_DragDropTarget] = m_HighlightColor1_Thin;

		// Scroll Bar
		colors[ImGuiCol_ScrollbarBg] = m_DarkBackgroundColor;
		colors[ImGuiCol_ScrollbarGrab] = m_DisabledColor;
		colors[ImGuiCol_ScrollbarGrabActive] = m_SelectedColor;
		colors[ImGuiCol_ScrollbarGrabHovered] = m_SelectedColor;

		// Headers
		colors[ImGuiCol_Header] = m_AccentColor;
		colors[ImGuiCol_HeaderHovered] = m_HoveredColor;
		colors[ImGuiCol_HeaderActive] = m_ActiveColor;

		// Buttons
		colors[ImGuiCol_Button] = m_AccentColor;
		colors[ImGuiCol_ButtonHovered] = m_HoveredColor;
		colors[ImGuiCol_ButtonActive] = m_ActiveColor;

		// Border
		colors[ImGuiCol_Border] = k_PureEmpty;
		colors[ImGuiCol_BorderShadow] = k_PureEmpty;

		// Frame BG
		colors[ImGuiCol_FrameBg] = k_PureEmpty;
		colors[ImGuiCol_FrameBgHovered] = k_PureEmpty;
		colors[ImGuiCol_FrameBgActive] = k_PureEmpty;

		// Tabs
		colors[ImGuiCol_Tab] = m_DarkAccentColor;
		colors[ImGuiCol_TabActive] = m_SelectedColor;
		colors[ImGuiCol_TabUnfocused] = m_DarkAccentColor;
		colors[ImGuiCol_TabUnfocusedActive] = m_ActiveColor;
		colors[ImGuiCol_TabHovered] = m_HoveredColor;

		// Title
		colors[ImGuiCol_TitleBg] = m_DarkBackgroundColor;
		colors[ImGuiCol_TitleBgActive] = m_DarkBackgroundColor;
		colors[ImGuiCol_TitleBgCollapsed] = m_DarkBackgroundColor;

		// Menu Bar
		colors[ImGuiCol_MenuBarBg] = m_DarkBackgroundColor;
	}
}