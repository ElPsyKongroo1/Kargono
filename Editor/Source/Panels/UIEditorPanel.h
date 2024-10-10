#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/EditorUI/EditorUI.h"
#include "Kargono/RuntimeUI/RuntimeUI.h"

#include <string>

namespace Kargono { class EditorApp; }

namespace Kargono::Panels
{
	enum class UIPropertiesDisplay : uint16_t
	{
		None = 0,
		Window,
		Widget
	};

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

	private:
		//=========================
		// Internal Functionality
		//=========================
		void OnOpenUI();
		void OnCreateUI();
		void OnRefreshData();
		void RecalculateTreeIterators();
		void DrawWindowOptions();
		void DrawWidgetOptions();
		void AddWindow();
		void AddTextWidget(EditorUI::TreeEntry& entry);
		void DeleteWindow(EditorUI::TreeEntry& entry);
		void DeleteWidget(EditorUI::TreeEntry& entry);
	public:
		//=========================
		// Core Panel Data
		//=========================
		Ref<RuntimeUI::UserInterface> m_EditorUI{ nullptr };
		Assets::AssetHandle m_EditorUIHandle { Assets::EmptyHandle };
	private:
		std::string m_PanelName{ "User Interface Editor" };
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
