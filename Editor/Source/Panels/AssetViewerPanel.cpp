#include "Panels/AssetViewerPanel.h"

#include "EditorApp.h"
#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	AssetViewerPanel::AssetViewerPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(AssetViewerPanel::OnKeyPressedEditor));
		InitializeAssetsTable();
		InitializeAssetPopup();
	}

	void AssetViewerPanel::ViewAssetInformation(EditorUI::TableEntry& entry)
	{
		m_ActiveAsset = entry.Handle;
		m_ActiveAssetType = Utility::StringToAssetType(entry.Label);
		m_ViewAssetPopup.PopupActive = true;
	}

	void AssetViewerPanel::RefreshPanelData()
	{
		m_AllAssetsTable.OnRefresh();
	}

	bool AssetViewerPanel::OnAssetEvent(Events::Event* event)
	{
		if (event->GetEventType() == Events::EventType::ManageAsset)
		{
			RefreshPanelData();
			return true;
		}
		return false;
	}

	void AssetViewerPanel::InitializeAssetsTable()
	{
		m_AllAssetsTable.Label = "All Assets";
		m_AllAssetsTable.Column1Title = "Asset Type";
		m_AllAssetsTable.Column2Title = "Asset File Location";
		m_AllAssetsTable.Expanded = true;
		m_AllAssetsTable.OnRefresh = [&]()
		{
			m_AllAssetsTable.ClearTable();
			for (auto& [handle, asset] : Assets::AssetService::GetAudioBufferRegistry())
			{
				EditorUI::TableEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation),
						nullptr
				};
				m_AllAssetsTable.InsertTableEntry(newEntry);
			}
			for (auto& [handle, asset] : Assets::AssetService::GetFontRegistry())
			{
				EditorUI::TableEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation),
						nullptr
				};
				m_AllAssetsTable.InsertTableEntry(newEntry);
			}

			for (auto& [handle, asset] : Assets::AssetService::GetGameStateRegistry())
			{
				EditorUI::TableEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation),
						nullptr
				};
				m_AllAssetsTable.InsertTableEntry(newEntry);
			}

			for (auto& [handle, asset] : Assets::AssetService::GetInputModeRegistry())
			{
				EditorUI::TableEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation),
						nullptr
				};
				m_AllAssetsTable.InsertTableEntry(newEntry);
			}

			for (auto& [handle, asset] : Assets::AssetService::GetProjectComponentRegistry())
			{
				EditorUI::TableEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation),
						nullptr
				};
				m_AllAssetsTable.InsertTableEntry(newEntry);
			}

			for (auto& [handle, asset] : Assets::AssetService::GetSceneRegistry())
			{
				EditorUI::TableEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation),
						nullptr
				};
				m_AllAssetsTable.InsertTableEntry(newEntry);
			}

			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (!script)
				{
					KG_WARN("Obtained invalid script when reading all assets");
					continue;
				}

				if (script->m_ScriptType == Scripting::ScriptType::Engine)
				{
					continue;
				}

				EditorUI::TableEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation),
						nullptr
				};
				m_AllAssetsTable.InsertTableEntry(newEntry);
			}

			for (auto& [handle, asset] : Assets::AssetService::GetTexture2DRegistry())
			{
				EditorUI::TableEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation),
						nullptr
				};
				m_AllAssetsTable.InsertTableEntry(newEntry);
			}

			for (auto& [handle, asset] : Assets::AssetService::GetUserInterfaceRegistry())
			{
				EditorUI::TableEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation),
						nullptr
				};
				m_AllAssetsTable.InsertTableEntry(newEntry);
			}

		};
		m_AllAssetsTable.OnRefresh();
	}
	void AssetViewerPanel::InitializeAssetPopup()
	{
		m_ViewAssetPopup.Label = "View Asset";
		m_ViewAssetPopup.PopupContents = [&]() 
		{
			Assets::Asset asset = Assets::AssetService::GetAssetFromAllRegistries(m_ActiveAsset, m_ActiveAssetType);
			EditorUI::EditorUIService::LabeledText("Asset Type", Utility::AssetTypeToString(asset.Data.Type));
			if (!asset.Data.FileLocation.empty())
			{
				EditorUI::EditorUIService::LabeledText("File Location", (Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.FileLocation).string());
			}
			if (!asset.Data.IntermediateLocation.empty())
			{
				EditorUI::EditorUIService::LabeledText("Intermediate Location", (Projects::ProjectService::GetActiveIntermediateDirectory() /asset.Data.IntermediateLocation).string());
			}
			
			EditorUI::EditorUIService::LabeledText("Checksum", asset.Data.CheckSum);
		};
	}
	void AssetViewerPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowAssetViewer);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		EditorUI::EditorUIService::Table(m_AllAssetsTable);
		EditorUI::EditorUIService::GenericPopup(m_ViewAssetPopup);


		EditorUI::EditorUIService::EndWindow();
	}
	bool AssetViewerPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
}
