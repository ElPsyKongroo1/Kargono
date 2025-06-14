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
		UNREFERENCED_PARAMETER(iteration);
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

			// Access the asset registry for every asset type
			for (Assets::AssetType type : Assets::s_AllAssetTypes)
			{
				if (type == Assets::AssetType::Shader)
				{
					continue;
				}

				// Get the relevant asset registry and ensure it is valid
				Assets::AssetRegistry* currentRegistryRef = Assets::AssetService::GetAssetRegistry(type);
				KG_ASSERT(currentRegistryRef);

				// Add each asset from the found registry to the asset list
				for (auto& [handle, asset] : *currentRegistryRef)
				{
					if (type == Assets::AssetType::Script && 
						asset.Data.GetSpecificMetaData<Assets::ScriptMetaData>()->m_ScriptType == Scripting::ScriptType::Engine)
					{
						continue;
					}

					EditorUI::ListEntry newEntry
					{
						Utility::AssetTypeToString(type),
							asset.Data.FileLocation.filename().string(),
							handle,
							KG_BIND_CLASS_FN(AssetViewerPanel::ViewAssetInformation)
					};
					m_AllAssetsTable.InsertListEntry(newEntry);
				}
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
			EditorUI::EditorUIContext::LabeledText("Asset Handle", static_cast<std::string>(m_ActiveAsset).c_str());
			EditorUI::EditorUIContext::LabeledText("Asset Type", Utility::AssetTypeToString(asset.Data.Type));
			if (!asset.Data.FileLocation.empty())
			{
				EditorUI::EditorUIContext::LabeledText("File Location", asset.Data.FileLocation.string().c_str());
			}
			if (!asset.Data.IntermediateLocation.empty())
			{
				EditorUI::EditorUIContext::LabeledText("Intermediate Location", asset.Data.IntermediateLocation.string().c_str());
			}
			
			EditorUI::EditorUIContext::LabeledText("Checksum", asset.Data.CheckSum.c_str());
		};
	}
	void AssetViewerPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIContext::StartRenderWindow(m_PanelName, &s_MainWindow->m_ShowAssetViewer);

		if (!EditorUI::EditorUIContext::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIContext::EndRenderWindow();
			return;
		}

		m_AllAssetsTable.RenderList();
		m_ViewAssetPopup.RenderPopup();


		EditorUI::EditorUIContext::EndRenderWindow();
	}
	bool AssetViewerPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
}
