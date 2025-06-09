#pragma once
#include "Modules/Assets/Asset.h"
#include "Modules/Events/KeyEvent.h"
#include "Kargono/ProjectData/GlobalState.h"
#include "Modules/EditorUI/EditorUI.h"

#include <string>

namespace Kargono::Panels
{
	using EditorWidget = std::variant<EditorUI::EditFloatSpec, EditorUI::EditTextSpec,
		EditorUI::CheckboxWidget, EditorUI::EditIntegerSpec,
		EditorUI::EditVec2Spec, EditorUI::EditVec3Spec, EditorUI::EditVec4Spec,
		EditorUI::EditIVec2Spec, EditorUI::EditIVec3Spec, EditorUI::EditIVec4Spec>;

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
		void OnOpenCreateFieldDialog(EditorUI::CollapsingHeaderSpec& spec);
		void OnConfirmCreateField();
		// Manage create-field popup's type widget
		void OnOpenCreateFieldType(EditorUI::SelectOptionSpec& spec);
		// Modify field callback functions
		void OnModifyFieldCheckbox(EditorUI::CheckboxWidget& spec);
		void OnModifyFieldEditFloat(EditorUI::EditFloatSpec& spec);
		void OnModifyFieldEditInteger(EditorUI::EditIntegerSpec& spec);
		void OnModifyFieldEditVec2(EditorUI::EditVec2Spec& spec);
		void OnModifyFieldEditVec3(EditorUI::EditVec3Spec& spec);
		void OnModifyFieldEditVec4(EditorUI::EditVec4Spec& spec);
		void OnModifyFieldEditIVec2(EditorUI::EditIVec2Spec& spec);
		void OnModifyFieldEditIVec3(EditorUI::EditIVec3Spec& spec);
		void OnModifyFieldEditIVec4(EditorUI::EditIVec4Spec& spec);

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
		EditorUI::GenericPopupSpec m_CreateGlobalStatePopup{};
		EditorUI::SelectOptionSpec m_OpenGlobalStatePopup{};
		EditorUI::ChooseDirectorySpec m_SelectGlobalStateLocationSpec{};
		// Header
		EditorUI::EditTextSpec m_SelectGlobalStateNameSpec{};
		EditorUI::PanelHeaderSpec m_MainHeader{};
		EditorUI::GenericPopupSpec m_DeleteGlobalStateWarning{};
		EditorUI::GenericPopupSpec m_CloseGlobalStateWarning{};
		// Modify fields widgets
		EditorUI::CollapsingHeaderSpec m_FieldsHeader{};
		EditorUI::GenericPopupSpec m_CreateFieldPopup{};
		EditorUI::EditTextSpec m_CreateFieldName{};
		EditorUI::SelectOptionSpec m_CreateFieldType{};
		GlobalStateWidgetData m_Fields{};


	};
}
