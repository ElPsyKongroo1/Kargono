#pragma once
#include "AssetModule/Asset.h"
#include "EventModule/KeyEvent.h"
#include "Kargono/ProjectData/ProjectEnum.h"
#include "EditorUIModule/EditorUI.h"

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
		EditorUI::GenericPopupSpec m_CreateProjectEnumPopupSpec{};
		EditorUI::SelectOptionSpec m_OpenProjectEnumPopupSpec{};
		EditorUI::ChooseDirectorySpec m_SelectProjectEnumLocationSpec{};
		// Header
		EditorUI::EditTextSpec m_SelectProjectEnumNameSpec{};
		EditorUI::PanelHeaderSpec m_MainHeader{};
		EditorUI::GenericPopupSpec m_DeleteProjectEnumWarning{};
		EditorUI::GenericPopupSpec m_CloseProjectEnumWarning{};
		// Identifiers Panel
		EditorUI::ListSpec m_EnumDataTable{};
		EditorUI::EditTextSpec m_AddIdentifierSpec{};
		EditorUI::EditTextSpec m_EditIdentifierSpec{};
		EditorUI::GenericPopupSpec m_DeleteIdentifierWarning{};
		EditorUI::TooltipSpec m_EnumTooltip{};

	};
}
