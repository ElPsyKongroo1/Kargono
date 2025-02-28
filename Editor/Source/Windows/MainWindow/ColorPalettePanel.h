#pragma once
#include "Kargono/Assets/Asset.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/ProjectData/ColorPalette.h"
#include "Kargono/EditorUI/EditorUI.h"

#include <string>

namespace Kargono::Panels
{

	struct ColorWidgets
	{
		std::vector<EditorUI::EditVec4Spec> m_ColorWidget;
	};


	class ColorPalettePanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		ColorPalettePanel();
	private:
		//=========================
		// Initialization Functions (Internal)
		//=========================
		void InitializeOpeningScreen();
		void InitializeDisplayColorPaletteScreen();
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
		void DrawColorPaletteColors();

	private:
		//=========================
		// Manage Global State Asset (Internal)
		//=========================
		void OnOpenColorPaletteDialog();
		void OnCreateColorPaletteDialog();
		void OnOpenColorPalette(Assets::AssetHandle newHandle);

		//=========================
		// Manage Global State Fields (Internal)
		//=========================
		// Modify field callback functions
		void OnModifyColor(EditorUI::EditVec4Spec& spec);

		//=========================
		// Other Internal Functionality
		//=========================
		void OnRefreshData();

	public:
		//=========================
		// Core Panel Data
		//=========================
		Ref<ProjectData::ColorPalette> m_EditorColorPalette{ nullptr };
		Assets::AssetHandle m_EditorColorPaletteHandle{ 0 };
	private:
		FixedString32 m_PanelName{ "Color Palette Editor" };

		//=========================
		// Widgets
		//=========================
		// Opening menu
		EditorUI::GenericPopupSpec m_CreateColorPalettePopup{};
		EditorUI::SelectOptionSpec m_OpenColorPalettePopup{};
		EditorUI::ChooseDirectorySpec m_SelectColorPaletteLocationSpec{};
		// Header
		EditorUI::EditTextSpec m_SelectColorPaletteNameSpec{};
		EditorUI::PanelHeaderSpec m_MainHeader{};
		EditorUI::GenericPopupSpec m_DeleteColorPaletteWarning{};
		EditorUI::GenericPopupSpec m_CloseColorPaletteWarning{};
		// Modify fields widgets
		EditorUI::CollapsingHeaderSpec m_ColorsHeader{};
		ColorWidgets m_ColorWidgets{};
	};
}
