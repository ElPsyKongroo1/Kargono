#include "kgpch.h"

#include "Modules/EditorUI/EditorUIResources.h"

#include "Modules/Rendering/Assets/Texture2D.h"

#include "Modules/Core/Engine.h"

namespace Kargono::EditorUI
{
	void GeneralIcons::LoadIcons(const std::filesystem::path& path)
	{
		// Set Up Editor Resources
		m_Camera->RegisterTexture((path / "Resources/Icons/Camera.png").string());
		m_Settings->RegisterTexture((path / "Resources/Icons/Settings.png").string());
		m_Delete->RegisterTexture((path / "Resources/Icons/Delete.png").string());
		m_Edit->RegisterTexture((path / "Resources/Icons/Edit.png").string());
		m_Cancel->RegisterTexture((path / "Resources/Icons/Cancel.png").string());
		m_Cancel2->RegisterTexture((path / "Resources/Icons/Cancel2.png").string());
		m_Confirm->RegisterTexture((path / "Resources/Icons/Confirm.png").string());
		m_Search->RegisterTexture((path / "Resources/Icons/Search.png").string());
		m_Options->RegisterTexture((path / "Resources/Icons/Options.png").string());
		m_Down->RegisterTexture((path / "Resources/Icons/Down.png").string());
		m_Right->RegisterTexture((path / "Resources/Icons/Right.png").string());
		m_Checkbox_Enabled->RegisterTexture((path / "Resources/Icons/CheckboxEnabled.png").string());
		m_Checkbox_Disabled->RegisterTexture((path / "Resources/Icons/CheckboxDisabled.png").string());
		m_Dash->RegisterTexture((path / "Resources/Icons/Dash.png").string());
		m_AI->RegisterTexture((path / "Resources/Icons/AI.png").string());
		m_Notification->RegisterTexture((path / "Resources/Icons/Notification.png").string());
	}
	void SceneGraphIcons::LoadIcons(const std::filesystem::path& path)
	{
		// Scene icons
		m_Entity->RegisterTexture((path / "Resources/Icons/SceneEditor/Entity.png").string());
		m_BoxCollider->RegisterTexture((path / "Resources/Icons/SceneEditor/BoxCollider.png").string());
		m_Tag->RegisterTexture((path / "Resources/Icons/SceneEditor/Tag.png").string());
		m_CircleCollider->RegisterTexture((path / "Resources/Icons/SceneEditor/CircleCollider.png").string());
		m_ClassInstance->RegisterTexture((path / "Resources/Icons/SceneEditor/ClassInstance.png").string());
		m_RigidBody->RegisterTexture((path / "Resources/Icons/SceneEditor/Rigidbody.png").string());
		m_Transform->RegisterTexture((path / "Resources/Icons/SceneEditor/Transform.png").string());
		m_Particles->RegisterTexture((path / "Resources/Icons/SceneEditor/Particles.png").string());

	}
	void ViewportIcons::LoadIcons(const std::filesystem::path& path)
	{

		// Viewport icons
		m_Grid->RegisterTexture((path / "Resources/Icons/Viewport/Grid.png").string());
		m_Display->RegisterTexture((path / "Resources/Icons/Viewport/Display.png").string());
		m_Play->RegisterTexture((path / "Resources/Icons/Viewport/Play.png").string());
		m_Stop->RegisterTexture((path / "Resources/Icons/Viewport/Stop.png").string());
		m_Pause->RegisterTexture((path / "Resources/Icons/Viewport/Pause.png").string());
		m_Step->RegisterTexture((path / "Resources/Icons/Viewport/Step.png").string());
		m_Simulate->RegisterTexture((path / "Resources/Icons/Viewport/Simulate.png").string());
	}
	void RuntimeUIIcons::LoadIcons(const std::filesystem::path& path)
	{
		// Runtime UI icons
		m_Window->RegisterTexture((path / "Resources/Icons/UIEditor/Window.png").string());
		m_TextWidget->RegisterTexture((path / "Resources/Icons/UIEditor/TextWidget.png").string());
		m_ButtonWidget->RegisterTexture((path / "Resources/Icons/UIEditor/ButtonWidget.png").string());
		m_InputTextWidget->RegisterTexture((path / "Resources/Icons/UIEditor/InputText.png").string());
		m_UserInterface2->RegisterTexture((path / "Resources/Icons/UIEditor/UserInterface2.png").string());
		m_ImageButtonWidget->RegisterTexture((path / "Resources/Icons/UIEditor/ImageButtonWidget.png").string());
		m_SliderWidget->RegisterTexture((path / "Resources/Icons/UIEditor/SliderWidget.png").string());
		m_DropDownWidget->RegisterTexture((path / "Resources/Icons/UIEditor/DropDownWidget.png").string());
		m_HorizontalContainer->RegisterTexture((path / "Resources/Icons/UIEditor/HorizontalContainer.png").string());
		m_VerticalContainer->RegisterTexture((path / "Resources/Icons/UIEditor/VerticalContainer.png").string());
	}
	void ScriptingIcons::LoadIcons(const std::filesystem::path& path)
	{
		// Scripting UI icons
		m_Number->RegisterTexture((path / "Resources/Icons/TextEditor/Number.png").string());
		m_Variable->RegisterTexture((path / "Resources/Icons/TextEditor/Variable.png").string());
		m_Function->RegisterTexture((path / "Resources/Icons/TextEditor/Function.png").string());
		m_Boolean->RegisterTexture((path / "Resources/Icons/TextEditor/Boolean.png").string());
		m_Decimal->RegisterTexture((path / "Resources/Icons/TextEditor/Decimal.png").string());
	}
	void ContentBrowserIcons::LoadIcons(const std::filesystem::path& path)
	{
		// Content Browser icons
		m_Directory->RegisterTexture(path / "Resources/Icons/ContentBrowser/Directory.png");
		m_GenericFile->RegisterTexture(path / "Resources/Icons/ContentBrowser/GenericFile.png");
		m_Back->RegisterTexture(path / "Resources/Icons/ContentBrowser/Back.png");
		m_Forward->RegisterTexture(path / "Resources/Icons/ContentBrowser/Forward.png");
		m_Audio->RegisterTexture(path / "Resources/Icons/ContentBrowser/Audio.png");
		m_Texture->RegisterTexture(path / "Resources/Icons/ContentBrowser/Texture.png");
		m_Binary->RegisterTexture(path / "Resources/Icons/ContentBrowser/Binary.png");
		m_Scene->RegisterTexture(path / "Resources/Icons/ContentBrowser/Scene.png");
		m_Scene_KG->RegisterTexture(path / "Resources/Icons/ContentBrowser/Scene_KG.png");
		m_Registry->RegisterTexture(path / "Resources/Icons/ContentBrowser/Registry.png");
		m_Font->RegisterTexture(path / "Resources/Icons/ContentBrowser/Font.png");
		m_UserInterface->RegisterTexture(path / "Resources/Icons/ContentBrowser/UserInterface.png");
		m_Input->RegisterTexture(path / "Resources/Icons/ContentBrowser/Input.png");
		m_AI_KG->RegisterTexture(path / "Resources/Icons/ContentBrowser/AI_KG.png");
		m_Audio_KG->RegisterTexture(path / "Resources/Icons/ContentBrowser/Audio_KG.png");
		m_Font_KG->RegisterTexture(path / "Resources/Icons/ContentBrowser/Font_KG.png");
		m_GlobalState->RegisterTexture(path / "Resources/Icons/ContentBrowser/GameState.png");
		m_CustomComponent->RegisterTexture(path / "Resources/Icons/ContentBrowser/CustomComponent.png");
		m_Script->RegisterTexture(path / "Resources/Icons/ContentBrowser/Script.png");
		m_Texture_KG->RegisterTexture(path / "Resources/Icons/ContentBrowser/Texture_KG.png");
		m_EmitterConfig->RegisterTexture(path / "Resources/Icons/ContentBrowser/EmitterConfig.png");
		m_Enum->RegisterTexture(path / "Resources/Icons/ContentBrowser/Enum.png");
		m_ColorPalette->RegisterTexture(path / "Resources/Icons/ContentBrowser/ColorPalette.png");

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
	float ConfigSpacing::SmallButtonRelativeLocation(size_t slot)
	{
		return -m_SmallButtonRightOffset - (m_SmallButtonSpacing * slot);
	}
}