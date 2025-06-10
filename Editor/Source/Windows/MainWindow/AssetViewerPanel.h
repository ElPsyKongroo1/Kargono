#pragma once
#include "Modules/Events/KeyEvent.h"
#include "Modules/EditorUI/EditorUIInclude.h"

#include <string>

namespace Kargono::Panels
{
	class AssetViewerPanel
	{
	public:
		AssetViewerPanel();

	public:
		bool OnAssetEvent(Events::Event* event);
	private:
		void InitializeAssetsTable();
		void InitializeAssetPopup();
	public:
		void ViewAssetInformation(EditorUI::ListEntry& entry, std::size_t iteration);
		void RefreshPanelData();
	public:

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	private:
		FixedString32 m_PanelName{ "Asset Viewer" };
		EditorUI::ListWidget m_AllAssetsTable {};
		EditorUI::GenericPopupWidget m_ViewAssetPopup {};

		Assets::AssetHandle m_ActiveAsset {Assets::EmptyHandle};
		Assets::AssetType m_ActiveAssetType {Assets::AssetType::None};
	};
}
