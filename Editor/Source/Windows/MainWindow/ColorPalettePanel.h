#pragma once
#include "Modules/Assets/Asset.h"
#include "Modules/Events/KeyEvent.h"
#include "Kargono/ProjectData/ColorPalette.h"
#include "Modules/EditorUI/EditorUIInclude.h"

#include <string>

namespace Kargono::Panels
{

	using ColorEditorWidgets = std::vector<EditorUI::EditVec4Widget>;

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
		// Manage Color Palette Asset (Internal)
		//=========================
		void OnOpenColorPaletteDialog();
		void OnCreateColorPaletteDialog();
		void OnOpenColorPalette(Assets::AssetHandle newHandle);

		//=========================
		// Manage Color Palette Fields (Internal)
		//=========================
		// Modify field callback functions
		void OnModifyColor(EditorUI::EditVec4Widget& spec);
		void OnModifyColorName(EditorUI::EditTextSpec& spec);
		void OnOpenEditTooltip(EditorUI::EditVec4Widget& spec);
		void OnAddWhite();

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
		EditorUI::GenericPopupWidget m_CreateColorPalettePopup{};
		EditorUI::SelectOptionWidget m_OpenColorPalettePopup{};
		EditorUI::ChooseDirectoryWidget m_SelectColorPaletteLocationSpec{};
		// Header
		EditorUI::EditTextSpec m_SelectColorPaletteNameSpec{};
		EditorUI::PanelHeaderWidget m_MainHeader{};
		EditorUI::GenericPopupWidget m_DeleteColorPaletteWarning{};
		EditorUI::GenericPopupWidget m_CloseColorPaletteWarning{};
		// Modify fields widgets
		EditorUI::CollapsingHeaderWidget m_ColorsHeader{};
		EditorUI::EditTextSpec m_EditColorName{};
		ColorEditorWidgets m_ColorEditorWidgets{};
		// General widgets
		EditorUI::TooltipWidget m_LocalTooltip{};
	};
}
