#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/EditorUI/EditorUI.h"
#include "Kargono/RuntimeUI/RuntimeUI.h"

#include "Windows/UserInterface/UIEditorViewportPanel.h"
#include "Windows/UserInterface/UIEditorPropertiesPanel.h"
#include "Windows/UserInterface/UIEditorTablePanel.h"

#include <string>

namespace Kargono { class EditorApp; }

namespace Kargono::Windows
{
	//=========================
	// UI Properties Display Enum
	//=========================
	enum class UIPropertiesDisplay : uint16_t
	{
		None = 0,
		Window,
		Widget
	};

	//=========================
	// UI Editor Panel Class
	//=========================
	class UIEditorWindow
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		UIEditorWindow();

		//=========================
		// Lifecycle Functions
		//=========================
		void InitPanels();

	private:
		//=========================
		// Internal Initialization Functions
		//=========================
		void InitializeWindowOptions();
		void InitializeWidgetOptions();

	public:
		//=========================
		// On Event Functions
		//=========================
		bool OnInputEvent(Events::Event* event);

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
		bool OnAssetEvent(Events::Event* event);
		void OpenAssetInEditor(std::filesystem::path& assetLocation);

	public:
		//=========================
		// External Functionality
		//=========================
		void OpenCreateDialog(std::filesystem::path& createLocation);
		void ResetWindowResources();
		void OnRefreshData();
		void ClearPropertiesPanelData();
	private:
		//=========================
		// Draw Properties Panel Functions
		//=========================
		void DrawWindowOptions();
		void DrawWidgetOptions();

		//=========================
		// Change Window Data
		//=========================
		// Modify tag
		void OnModifyWindowTag(EditorUI::EditTextSpec& spec);
		// Modify default widget
		void OnModifyWindowDefaultWidget(const EditorUI::OptionEntry& entry);
		void OnOpenWindowDefaultWidgetPopup();
		// Modify if window is displayed
		void OnModifyWindowDisplay(EditorUI::CheckboxSpec& spec);
		// Modify window location
		void OnModifyWindowLocation(EditorUI::EditVec3Spec& spec);
		// Modify window size
		void OnModifyWindowSize(EditorUI::EditVec2Spec& spec);
		// Modify window background color
		void OnModifyWindowBackgroundColor(EditorUI::EditVec4Spec& spec);

		//=========================
		// Change Widget Base Data
		//=========================
		// Modify widget tag
		void OnModifyWidgetTag(EditorUI::EditTextSpec& spec);
		// Modify widget location
		void OnModifyWidgetLocation(EditorUI::EditVec2Spec& spec);
		// Modify widget size
		void OnModifyWidgetSize(EditorUI::EditVec2Spec& spec);
		// Modify widget background color
		void OnModifyWidgetBackgroundColor(EditorUI::EditVec4Spec& spec);
		// Modify widget on press
		void OnModifyWidgetOnPress(const EditorUI::OptionEntry& entry);
		void OnOpenWidgetOnPressPopup();
		void OnOpenTooltipForWidgetOnPress();

		//=========================
		// Change Text Widget Data
		//=========================
		// Modify widget text
		void OnModifyTextWidgetText(EditorUI::EditTextSpec& spec);
		// Modify widget text size
		void OnModifyTextWidgetTextSize(EditorUI::EditFloatSpec& spec);
		// Modify widget text color
		void OnModifyTextWidgetTextColor(EditorUI::EditVec4Spec& spec);
		// Modify if widget text is centered
		void OnModifyTextWidgetCentered(EditorUI::CheckboxSpec& spec);

	public:
		//=========================
		// Core Panel Data
		//=========================
		Ref<RuntimeUI::UserInterface> m_EditorUI{ nullptr };
		Assets::AssetHandle m_EditorUIHandle { Assets::EmptyHandle };
		UIPropertiesDisplay m_CurrentDisplay{ UIPropertiesDisplay::None };
		RuntimeUI::Widget* m_ActiveWidget{ nullptr };
		RuntimeUI::Window* m_ActiveWindow{ nullptr };

		//=========================
		// Panels
		//=========================
		Scope<Panels::UIEditorViewportPanel> m_ViewportPanel{ nullptr };
		Scope<Panels::UIEditorPropertiesPanel> m_PropertiesPanel{ nullptr };
		Scope<Panels::UIEditorTablePanel> m_TablePanel{ nullptr };

		//=========================
		// Display Panel Booleans
		//=========================
		bool m_ShowViewport{ true };
		bool m_ShowProperties{ true };
		bool m_ShowTable{ true };
		
	private:
		//=========================
		// Widgets
		//=========================

		// Edit Window Options
		EditorUI::CollapsingHeaderSpec m_WindowHeader{};
		EditorUI::EditTextSpec m_WindowTag{};
		EditorUI::SelectOptionSpec m_WindowDefaultWidget{};
		EditorUI::CheckboxSpec m_WindowDisplay{};
		EditorUI::EditVec3Spec m_WindowLocation{};
		EditorUI::EditVec2Spec m_WindowSize{};
		EditorUI::EditVec4Spec m_WindowBackgroundColor{};

		// Edit Widget Options
		EditorUI::CollapsingHeaderSpec m_WidgetHeader{};
		EditorUI::EditTextSpec m_WidgetTag{};
		EditorUI::EditVec2Spec m_WidgetLocation{};
		EditorUI::EditVec2Spec m_WidgetSize{};
		EditorUI::EditVec4Spec m_WidgetBackgroundColor{};
		EditorUI::SelectOptionSpec m_WidgetOnPress{};
		EditorUI::EditTextSpec m_WidgetText{};
		EditorUI::EditFloatSpec m_WidgetTextSize{};
		EditorUI::EditVec4Spec m_WidgetTextColor{};
		EditorUI::CheckboxSpec m_WidgetCentered{};
	private:
		friend class PropertiesPanel;
		friend Kargono::EditorApp;

	};
}
