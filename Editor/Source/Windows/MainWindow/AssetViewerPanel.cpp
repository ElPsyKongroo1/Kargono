#include "Windows/MainWindow/AssetViewerPanel.h"

#include "EditorApp.h"

static Kargono::EditorApp* s_EditorApp { nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{
	AssetViewerPanel::AssetViewerPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(AssetViewerPanel::OnKeyPressedEditor));
		InitializeAssetsTable();
		InitializeAssetPopup();
	}

	void AssetViewerPanel::ViewAssetInformation(EditorUI::ListEntry& entry, std::size_t iteration)
	{
		m_ActiveAsset = entry.m_Handle;
		m_ActiveAssetType = Utility::StringToAssetType(entry.m_Label);
		m_ViewAssetPopup.m_OpenPopup = true;
	}

	void AssetViewerPanel::RefreshPanelData()
	{
		m_AllAssetsTable.m_OnRefresh();
	}

	bool AssetViewerPanel::OnAssetEvent(Events::Event* event)
	{
		Events::ManageAsset& manageEvent = *(Events::ManageAsset*)event;

		// Handle deletion event
		if (manageEvent.GetAction() == Events::ManageAssetAction::PreDelete)
		{
			// Search table for deleted asset
			std::size_t assetLocation = m_AllAssetsTable.SearchEntries([&](const EditorUI::ListEntry& currentEntry) 
			{
				// Check if type inside tree is the same
				if (Utility::StringToAssetType(currentEntry.m_Label) != manageEvent.GetAssetType())
				{
					return false;
				}

				// Check if handle matches
				if (currentEntry.m_Handle != manageEvent.GetAssetID())
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
		m_AllAssetsTable.m_Label = "All Assets";
		m_AllAssetsTable.m_Column1Title = "Asset Type";
		m_AllAssetsTable.m_Column2Title = "Asset File Location";
		m_AllAssetsTable.m_Expanded = true;
		m_AllAssetsTable.m_OnRefresh = [&]()
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

			for (auto& [handle, asset] : Assets::AssetService::GetEmitterConfigRegistry())
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
		m_AllAssetsTable.m_OnRefresh();
	}
	void AssetViewerPanel::InitializeAssetPopup()
	{
		m_ViewAssetPopup.m_Label = "View Asset";
		m_ViewAssetPopup.m_PopupContents = [&]() 
		{
			Assets::AssetInfo asset = Assets::AssetService::GetAssetFromAllRegistries(m_ActiveAsset, m_ActiveAssetType);
			EditorUI::EditorUIService::LabeledText("Asset Handle", m_ActiveAsset);
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
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_MainWindow->m_ShowAssetViewer);

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
