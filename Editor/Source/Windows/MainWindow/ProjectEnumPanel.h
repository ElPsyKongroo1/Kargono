#pragma once
#include "Modules/Assets/Asset.h"
#include "Modules/Events/KeyEvent.h"
#include "Kargono/ProjectData/ProjectEnum.h"
#include "Modules/EditorUI/EditorUIInclude.h"

#include <string>

namespace Kargono::Panels
{
	constexpr size_t k_InvalidProjectEnumIndex{ std::numeric_limits<size_t>().max() };

	class ProjectEnumPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		ProjectEnumPanel();
	private:
		//=========================
		// Internal Initialization Functions
		//=========================
		void InitializeOpeningScreen();
		void InitializeDisplayProjectEnumScreen();
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
		//=========================
		// Internal Functionality
		//=========================
		void OnOpenProjectEnumDialog();
		void OnCreateProjectEnumDialog();
		void OnRefreshData();

		void OnOpenProjectEnum(Assets::AssetHandle newHandle);
	public:
		//=========================
		// Core Panel Data
		//=========================
		Ref<ProjectData::ProjectEnum> m_EditorProjectEnum{ nullptr };
		Assets::AssetHandle m_EditorProjectEnumHandle{ 0 };
	private:
		FixedString32 m_PanelName{ "Enum Editor" };
		size_t m_CurrentEnumeration{ k_InvalidProjectEnumIndex };

		//=========================
		// Widgets
		//=========================
		// Opening menu
		EditorUI::GenericPopupWidget m_CreateProjectEnumPopupSpec{};
		EditorUI::SelectOptionWidget m_OpenProjectEnumPopupSpec{};
		EditorUI::ChooseDirectoryWidget m_SelectProjectEnumLocationSpec{};
		// Header
		EditorUI::EditTextSpec m_SelectProjectEnumNameSpec{};
		EditorUI::PanelHeaderWidget m_MainHeader{};
		EditorUI::GenericPopupWidget m_DeleteProjectEnumWarning{};
		EditorUI::GenericPopupWidget m_CloseProjectEnumWarning{};
		// Identifiers Panel
		EditorUI::ListWidget m_EnumDataTable{};
		EditorUI::EditTextSpec m_AddIdentifierSpec{};
		EditorUI::EditTextSpec m_EditIdentifierSpec{};
		EditorUI::GenericPopupWidget m_DeleteIdentifierWarning{};
		EditorUI::TooltipWidget m_EnumTooltip{};

	};
}
