#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/EditorUI/EditorUI.h"
#include "Kargono/RuntimeUI/RuntimeUI.h"

#include <string>

namespace Kargono { class EditorApp; }

namespace Kargono::Panels
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
	class UIEditorPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		UIEditorPanel();

	private:
		//=========================
		// Internal Initialization Functions
		//=========================
		void InitializeOpeningScreen();
		void InitializeUIHeader();
		void InitializeMainContent();
		void InitializeWindowOptions();
		void InitializeWidgetOptions();

	public:
		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
		bool OnAssetEvent(Events::Event* event);
		void OpenAssetInEditor(std::filesystem::path& assetLocation);

	public:
		//=========================
		// External Functionality
		//=========================
		void OpenCreateDialog(std::filesystem::path& createLocation);
		void ResetPanelResources();
	private:
		//=========================
		// Manage Panel State
		//=========================
		void RecalculateTreeIndexData();
		void ClearPropertiesPanelData();
		void OnRefreshData();
		void OnRefreshUITree();

		//=========================
		// On Action Functions
		//=========================
		void OnOpenUIDialog();
		void OnCreateUIDialog();
		void OnOpenUI(Assets::AssetHandle newHandle);

		//=========================
		// Draw Properties Panel Functions
		//=========================
		void DrawWindowOptions();
		void DrawWidgetOptions();

		//=========================
		// Manage Windows
		//=========================
		void AddWindow();
		void SelectWindow(EditorUI::TreeEntry& entry);
		void DeleteWindow(EditorUI::TreeEntry& entry);

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
		// Manage Widgets
		//=========================
		void AddTextWidget(EditorUI::TreeEntry& entry);
		void SelectTextWidget(EditorUI::TreeEntry& entry);
		void DeleteWidget(EditorUI::TreeEntry& entry);

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
	private:
		FixedString32 m_PanelName{ "User Interface Editor" };
		UIPropertiesDisplay m_CurrentDisplay{ UIPropertiesDisplay::None };
		RuntimeUI::Widget* m_ActiveWidget {nullptr};
		RuntimeUI::Window* m_ActiveWindow {nullptr};

	private:
		//=========================
		// Widgets
		//=========================
		// Main Content
		EditorUI::TreeSpec m_UITree {};

		// Opening Panel w/ Popups
		EditorUI::SelectOptionSpec m_OpenUIPopupSpec {};
		EditorUI::GenericPopupSpec m_CreateUIPopupSpec {};
		EditorUI::EditTextSpec m_SelectUINameSpec {};
		EditorUI::ChooseDirectorySpec m_SelectUILocationSpec{};
		EditorUI::TooltipSpec m_SelectScriptTooltip{};

		// User Interface Header
		EditorUI::PanelHeaderSpec m_MainHeader {};
		EditorUI::GenericPopupSpec m_DeleteUIWarning {};
		EditorUI::GenericPopupSpec m_CloseUIWarning {};

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
