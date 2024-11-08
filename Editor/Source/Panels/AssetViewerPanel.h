#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/EditorUI/EditorUI.h"

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
		void ViewAssetInformation(EditorUI::TableEntry& entry);
		void RefreshPanelData();
	public:

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	private:
		FixedString32 m_PanelName{ "Asset Viewer" };
		EditorUI::TableSpec m_AllAssetsTable {};
		EditorUI::GenericPopupSpec m_ViewAssetPopup {};

		Assets::AssetHandle m_ActiveAsset {Assets::EmptyHandle};
		Assets::AssetType m_ActiveAssetType {Assets::AssetType::None};
	};
}
