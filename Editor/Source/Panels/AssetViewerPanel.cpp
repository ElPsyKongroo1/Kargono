#include "Panels/AssetViewerPanel.h"

#include "EditorApp.h"
#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	AssetViewerPanel::AssetViewerPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(AssetViewerPanel::OnKeyPressedEditor));
		InitializeAssetsTable();
		InitializeAssetPopup();
	}

	void AssetViewerPanel::ViewAssetInformation(EditorUI::ListEntry& entry, std::size_t iteration)
	{
		m_ActiveAsset = entry.Handle;
		m_ActiveAssetType = Utility::StringToAssetType(entry.Label);
		m_ViewAssetPopup.OpenPopup = true;
	}

	void AssetViewerPanel::RefreshPanelData()
	{
		m_AllAssetsTable.OnRefresh();
	}

	bool AssetViewerPanel::OnAssetEvent(Events::Event* event)
	{
		Events::ManageAsset& manageEvent = *(Events::ManageAsset*)event;

		// Handle deletion event
		if (manageEvent.GetAction() == Events::ManageAssetAction::Delete)
		{
			// Search table for deleted asset
			std::size_t assetLocation = m_AllAssetsTable.SearchEntries([&](const EditorUI::ListEntry& currentEntry) 
			{
				// Check if type inside tree is the same
				if (Utility::StringToAssetType(currentEntry.Label) != manageEvent.GetAssetType())
				{
					return false;
				}

				// Check if handle matches
				if (currentEntry.Handle != manageEvent.GetAssetID())
				{
					return false;
				}

				// Index has been found
				return true;

			});

			// Validate that table search was successful
			KG_ASSERT(assetLocation != EditorUI::k_ListSearchIndex, "Asset being deleted was not found in asset table");

			// Delete entry and validate deletion
			bool deletionSuccess = m_AllAssetsTable.RemoveEntry(assetLocation);
			KG_ASSERT(deletionSuccess, "Unable to delete asset inside AssetViewer");
		}

		// Handle all othe event types
		else
		{
			RefreshPanelData();
		}

		return true;
	}

	void AssetViewerPanel::InitializeAssetsTable()
	{
		m_AllAssetsTable.Label = "All Assets";
		m_AllAssetsTable.Column1Title = "Asset Type";
		m_AllAssetsTable.Column2Title = "Asset File Location";
		m_AllAssetsTable.Expanded = true;
		m_AllAssetsTable.OnRefresh = [&]()
		{
			m_AllAssetsTable.ClearList();
			for (auto& [handle, asset] : Assets::AssetService::GetAIStateRegistry())
			{
				EditorUI::ListEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.filename().string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation)
				};
				m_AllAssetsTable.InsertListEntry(newEntry);
			}
			for (auto& [handle, asset] : Assets::AssetService::GetAudioBufferRegistry())
			{
				EditorUI::ListEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.filename().string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation)
				};
				m_AllAssetsTable.InsertListEntry(newEntry);
			}
			for (auto& [handle, asset] : Assets::AssetService::GetFontRegistry())
			{
				EditorUI::ListEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.filename().string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation)
				};
				m_AllAssetsTable.InsertListEntry(newEntry);
			}

			for (auto& [handle, asset] : Assets::AssetService::GetGameStateRegistry())
			{
				EditorUI::ListEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.filename().string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation)
				};
				m_AllAssetsTable.InsertListEntry(newEntry);
			}

			for (auto& [handle, asset] : Assets::AssetService::GetInputMapRegistry())
			{
				EditorUI::ListEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.filename().string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation)
				};
				m_AllAssetsTable.InsertListEntry(newEntry);
			}

			for (auto& [handle, asset] : Assets::AssetService::GetProjectComponentRegistry())
			{
				EditorUI::ListEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.filename().string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation)
				};
				m_AllAssetsTable.InsertListEntry(newEntry);
			}

			for (auto& [handle, asset] : Assets::AssetService::GetSceneRegistry())
			{
				EditorUI::ListEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.filename().string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation)
				};
				m_AllAssetsTable.InsertListEntry(newEntry);
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

				EditorUI::ListEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.filename().string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation)
				};
				m_AllAssetsTable.InsertListEntry(newEntry);
			}

			for (auto& [handle, asset] : Assets::AssetService::GetTexture2DRegistry())
			{
				EditorUI::ListEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.filename().string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation)
				};
				m_AllAssetsTable.InsertListEntry(newEntry);
			}

			for (auto& [handle, asset] : Assets::AssetService::GetUserInterfaceRegistry())
			{
				EditorUI::ListEntry newEntry
				{
					Utility::AssetTypeToString(asset.Data.Type),
						asset.Data.FileLocation.filename().string(),
						handle,
						KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation)
				};
				m_AllAssetsTable.InsertListEntry(newEntry);
			}

		};
		m_AllAssetsTable.OnRefresh();
	}
	void AssetViewerPanel::InitializeAssetPopup()
	{
		m_ViewAssetPopup.Label = "View Asset";
		m_ViewAssetPopup.PopupContents = [&]() 
		{
			Assets::AssetInfo asset = Assets::AssetService::GetAssetFromAllRegistries(m_ActiveAsset, m_ActiveAssetType);
			EditorUI::EditorUIService::LabeledText("Asset Type", Utility::AssetTypeToString(asset.Data.Type));
			if (!asset.Data.FileLocation.empty())
			{
				EditorUI::EditorUIService::LabeledText("File Location", asset.Data.FileLocation.string());
			}
			if (!asset.Data.IntermediateLocation.empty())
			{
				EditorUI::EditorUIService::LabeledText("Intermediate Location", asset.Data.IntermediateLocation.string());
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

		EditorUI::EditorUIService::List(m_AllAssetsTable);
		EditorUI::EditorUIService::GenericPopup(m_ViewAssetPopup);


		EditorUI::EditorUIService::EndWindow();
	}
	bool AssetViewerPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
}
