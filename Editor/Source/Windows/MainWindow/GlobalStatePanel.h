#pragma once
#include "Modules/Assets/Asset.h"
#include "Modules/Events/KeyEvent.h"
#include "Kargono/ProjectData/GlobalState.h"
#include "Modules/EditorUI/EditorUIInclude.h"

#include <string>

namespace Kargono::Panels
{
	using EditorWidget = std::variant<EditorUI::EditFloatWidget, EditorUI::EditTextSpec,
		EditorUI::CheckboxWidget, EditorUI::EditIntegerWidget,
		EditorUI::EditVec2Widget, EditorUI::EditVec3Widget, EditorUI::EditVec4Widget,
		EditorUI::EditIVec2Widget, EditorUI::EditIVec3Widget, EditorUI::EditIVec4Widget>;

	struct GlobalStateWidgetData
	{
		std::vector<EditorWidget> m_Fields;
	};


	class GlobalStatePanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		GlobalStatePanel();
	private:
		//=========================
		// Initialization Functions (Internal)
		//=========================
		void InitializeOpeningScreen();
		void InitializeDisplayGlobalStateScreen();
		void InitializeDisplayGlobalStateFields();
	public:
		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
		bool OnAssetEvent(Events::Event* event);

		//=========================
		// External API
		//=========================
		void ResetPanelResources();
		void OpenCreateDialog(std::filesystem::path& createLocation);
		void OpenAssetInEditor(std::filesystem::path& assetLocation);
	private:
		void DrawGlobalStateFields();

	private:
		//=========================
		// Manage Global State Asset (Internal)
		//=========================
		void OnOpenGlobalStateDialog();
		void OnCreateGlobalStateDialog();
		void OnOpenGlobalState(Assets::AssetHandle newHandle);

		//=========================
		// Manage Global State Fields (Internal)
		//=========================
		// Manage create-field popup
		void OnOpenCreateFieldDialog(EditorUI::CollapsingHeaderWidget& spec);
		void OnConfirmCreateField();
		// Manage create-field popup's type widget
		void OnOpenCreateFieldType(EditorUI::SelectOptionWidget& spec);
		// Modify field callback functions
		void OnModifyFieldCheckbox(EditorUI::CheckboxWidget& spec);
		void OnModifyFieldEditFloat(EditorUI::EditFloatWidget& spec);
		void OnModifyFieldEditInteger(EditorUI::EditIntegerWidget& spec);
		void OnModifyFieldEditVec2(EditorUI::EditVec2Widget& spec);
		void OnModifyFieldEditVec3(EditorUI::EditVec3Widget& spec);
		void OnModifyFieldEditVec4(EditorUI::EditVec4Widget& spec);
		void OnModifyFieldEditIVec2(EditorUI::EditIVec2Widget& spec);
		void OnModifyFieldEditIVec3(EditorUI::EditIVec3Widget& spec);
		void OnModifyFieldEditIVec4(EditorUI::EditIVec4Widget& spec);

		//=========================
		// Other Internal Functionality
		//=========================
		void OnRefreshData();

	public:
		//=========================
		// Core Panel Data
		//=========================
		Ref<ProjectData::GlobalState> m_EditorGlobalState{ nullptr };
		Assets::AssetHandle m_EditorGlobalStateHandle{ 0 };
	private:
		FixedString32 m_PanelName{ "Global State Editor" };

		//=========================
		// Widgets
		//=========================
		// Opening menu
		EditorUI::GenericPopupWidget m_CreateGlobalStatePopup{};
		EditorUI::SelectOptionWidget m_OpenGlobalStatePopup{};
		EditorUI::ChooseDirectoryWidget m_SelectGlobalStateLocationSpec{};
		// Header
		EditorUI::EditTextSpec m_SelectGlobalStateNameSpec{};
		EditorUI::PanelHeaderWidget m_MainHeader{};
		EditorUI::GenericPopupWidget m_DeleteGlobalStateWarning{};
		EditorUI::GenericPopupWidget m_CloseGlobalStateWarning{};
		// Modify fields widgets
		EditorUI::CollapsingHeaderWidget m_FieldsHeader{};
		EditorUI::GenericPopupWidget m_CreateFieldPopup{};
		EditorUI::EditTextSpec m_CreateFieldName{};
		EditorUI::SelectOptionWidget m_CreateFieldType{};
		GlobalStateWidgetData m_Fields{};


	};
}
