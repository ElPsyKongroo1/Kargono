#include "Windows/MainWindow/GameStatePanel.h"

#include "EditorApp.h"

static Kargono::EditorApp* s_EditorApp { nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{
	void GameStatePanel::InitializeOpeningScreen()
	{
		m_OpenGameStatePopupSpec.m_Label = "Open Game State";
		m_OpenGameStatePopupSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_OpenGameStatePopupSpec.m_Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenGameStatePopupSpec.m_PopupAction = [&](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();
			spec.m_CurrentOption = { "None", Assets::EmptyHandle };

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetGameStateRegistry())
			{
				spec.AddToOptions("All Options", asset.Data.FileLocation.filename().string(), handle);
			}
		};

		m_OpenGameStatePopupSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.m_Handle == Assets::EmptyHandle)
			{
				KG_WARN("No Game State Selected");
				return;
			}
			if (!Assets::AssetService::GetGameStateRegistry().contains(selection.m_Handle))
			{
				KG_WARN("Could not find on game state in game state editor");
				return;
			}

			OnOpenGameState(selection.m_Handle);
		};

		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

		m_SelectGameStateNameSpec.m_Label = "New Name";
		m_SelectGameStateNameSpec.m_CurrentOption = "Empty";

		m_SelectGameStateLocationSpec.m_Label = "Location";
		m_SelectGameStateLocationSpec.m_CurrentOption = projectPaths.GetAssetDirectory();
		m_SelectGameStateLocationSpec.m_ConfirmAction = [&](std::string_view path)
		{
			if (!Utility::FileSystem::DoesPathContainSubPath(projectPaths.GetAssetDirectory(), path))
			{
				KG_WARN("Cannot create an asset outside of the project's asset directory.");
				m_SelectGameStateLocationSpec.m_CurrentOption = projectPaths.GetAssetDirectory();
			}
		};

		m_CreateGameStatePopupSpec.m_Label = "Create Game State";
		m_CreateGameStatePopupSpec.m_ConfirmAction = [&]()
		{
			if (m_SelectGameStateNameSpec.m_CurrentOption == "")
			{
				return;
			}

			for (auto& [id, asset] : Assets::AssetService::GetGameStateRegistry())
			{
				if (asset.Data.GetSpecificMetaData<Assets::GameStateMetaData>()->Name == m_SelectGameStateNameSpec.m_CurrentOption)
				{
					return;
				}
			}
			m_EditorGameStateHandle = Assets::AssetService::CreateGameState(m_SelectGameStateNameSpec.m_CurrentOption.c_str(), m_SelectGameStateLocationSpec.m_CurrentOption);
			m_EditorGameState = Assets::AssetService::GetGameState(m_EditorGameStateHandle);
			m_MainHeader.m_EditColorActive = false;
			m_MainHeader.m_Label = Assets::AssetService::GetGameStateRegistry().at(
				m_EditorGameStateHandle).Data.FileLocation.filename().string();
			m_FieldsTable.m_OnRefresh();
		};
		m_CreateGameStatePopupSpec.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_SelectGameStateNameSpec);
			EditorUI::EditorUIService::ChooseDirectory(m_SelectGameStateLocationSpec);
		};
	}


	void GameStatePanel::InitializeDisplayGameStateScreen()
	{
		// Header (Game State Name and Options)
		m_DeleteGameStateWarning.m_Label = "Delete Game State";
		m_DeleteGameStateWarning.m_ConfirmAction = [&]()
		{
			Assets::AssetService::DeleteGameState(m_EditorGameStateHandle);
			m_EditorGameStateHandle = 0;
			m_EditorGameState = nullptr;
		};
		m_DeleteGameStateWarning.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this game state object?");
		};

		m_CloseGameStateWarning.m_Label = "Close Game State";
		m_CloseGameStateWarning.m_ConfirmAction = [&]()
		{
			m_EditorGameStateHandle = 0;
			m_EditorGameState = nullptr;
		};
		m_CloseGameStateWarning.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to close this game state object without saving?");
		};

		m_MainHeader.AddToSelectionList("Save", [&]()
			{
				Assets::AssetService::SaveGameState(m_EditorGameStateHandle, m_EditorGameState);
				m_MainHeader.m_EditColorActive = false;
			});
		m_MainHeader.AddToSelectionList("Close", [&]()
			{
				if (m_MainHeader.m_EditColorActive)
				{
					m_CloseGameStateWarning.m_OpenPopup = true;
				}
				else
				{
					m_EditorGameStateHandle = 0;
					m_EditorGameState = nullptr;
				}
			});
		m_MainHeader.AddToSelectionList("Delete", [&]()
			{
				m_DeleteGameStateWarning.m_OpenPopup = true;
			});


		// Fields List
		m_FieldsTable.m_Label = "Fields";
		m_FieldsTable.m_Expanded = true;
		m_FieldsTable.m_OnRefresh = [&]()
		{
			m_FieldsTable.ClearList();
			if (m_EditorGameState)
			{
				for (auto& [name, field] : m_EditorGameState->GetAllFields())
				{
					switch (field->Type())
					{
					case WrappedVarType::UInteger16:
					{
						m_FieldsTable.InsertListEntry(name,
							std::to_string(field->GetWrappedValue<uint16_t>()),
							[&](EditorUI::ListEntry& entry, std::size_t iteration)
							{
								UNREFERENCED_PARAMETER(iteration);
								UNREFERENCED_PARAMETER(entry);
								m_EditFieldPopup.m_OpenPopup = true;
								m_CurrentField = name;
							});
						break;
					}
					default:
					{
						KG_WARN("Attempt to add invalid/unsupported type to game state fields UI");
						break;
					}
					}
				}
			}
		};
		m_FieldsTable.m_Column1Title = "Field Name";
		m_FieldsTable.m_Column2Title = "Field Value";
		m_FieldsTable.AddToSelectionList("Add New Field", [&]()
			{
				m_AddFieldPopup.m_OpenPopup = true;
			});

		m_AddFieldPopup.m_Label = "Add New Field";
		m_AddFieldPopup.m_Flags |= EditorUI::SelectOption_PopupOnly;
		m_AddFieldPopup.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_AddFieldPopup.m_LineCount = 2;
		m_AddFieldPopup.m_PopupAction = [&](EditorUI::SelectOptionSpec& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			spec.AddToOptions("All Options", "UInteger16", Assets::EmptyHandle);
		};
		m_AddFieldPopup.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.m_Label == "None")
			{
				return;
			}

			if (selection.m_Label == "UInteger16")
			{
				uint32_t iteration{ 1 };
				while (!m_EditorGameState->AddField("New Field " + std::to_string(iteration), WrappedVarType::UInteger16))
				{
					iteration++;
				}
				m_MainHeader.m_EditColorActive = true;
			}
			m_FieldsTable.m_OnRefresh();
		};

		m_EditFieldName.m_Label = "Field Name";
		m_EditFieldName.m_CurrentOption = "Empty";

		m_EditFieldType.m_Label = "Field Type";
		m_EditFieldType.m_Flags |= EditorUI::SelectOption_PopupOnly;
		m_EditFieldType.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_EditFieldType.m_LineCount = 2;
		m_EditFieldType.m_PopupAction = [&](EditorUI::SelectOptionSpec& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("All Options", "UInteger16", Assets::EmptyHandle);
		};
		m_EditFieldType.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			UNREFERENCED_PARAMETER(selection);
			m_FieldsTable.m_OnRefresh();
		};
		m_EditFieldValue.m_Label = "Field Value";
		m_EditFieldValue.AllocateBuffer();

		m_EditFieldPopup.m_Label = "Edit Field";
		m_EditFieldPopup.m_DeleteAction = [&]()
		{
			if (!m_EditorGameState->DeleteField(m_CurrentField))
			{
				KG_ERROR("Unable to delete field inside game state!");
				return;
			}
			m_MainHeader.m_EditColorActive = true;
			m_FieldsTable.m_OnRefresh();
		};
		m_EditFieldPopup.m_PopupWidth = 420.0f;
		m_EditFieldPopup.m_PopupAction = [&]()
		{
			Ref<WrappedVariable> field = m_EditorGameState->GetField(m_CurrentField);

			if (!field)
			{
				KG_ERROR("Unable to retreive field from current game state object");
				return;
			}
			m_EditFieldName.m_CurrentOption = m_CurrentField;
			m_EditFieldType.m_CurrentOption.m_Label = Utility::WrappedVarTypeToString(field->Type());
			std::string convertCache { std::to_string(field->GetWrappedValue<uint16_t>()) };
			m_EditFieldValue.FieldBuffer.SetDataToByte(0);
			memcpy(m_EditFieldValue.FieldBuffer.Data, convertCache.data(), convertCache.size());
		};
		m_EditFieldPopup.m_ConfirmAction = [&]()
		{
			auto& fieldMap = m_EditorGameState->GetAllFields();
			Ref<WrappedVariable> oldField = fieldMap.at(m_CurrentField);
			Ref<WrappedVariable> newField { nullptr };
			if (m_CurrentField != m_EditFieldName.m_CurrentOption)
			{
				if (!oldField)
				{
					KG_ERROR("Could not obtain field pointer from game state field map");
					return;
				}
				fieldMap.erase(m_CurrentField);
				m_FieldsTable.m_OnRefresh();
			}

			switch (Utility::StringToWrappedVarType(m_EditFieldType.m_CurrentOption.m_Label.CString()))
			{
				case WrappedVarType::UInteger16:
				{
					newField = CreateRef<WrappedUInteger16>();

					bool success = Utility::Conversions::CharBufferToVariable<uint16_t>(m_EditFieldValue.FieldBuffer,
						((WrappedUInteger16*)newField.get())->m_Value);

					if (!success && oldField->Type() == WrappedVarType::UInteger16)
					{
						((WrappedUInteger16*)newField.get())->m_Value = oldField->GetWrappedValue<uint16_t>();
					}

					break;
				}
				default:
				{
					KG_ERROR("Invalid wrapped type provided to editfieldpopup.confirm function");
					return;
				}
			}

			fieldMap.insert_or_assign(m_EditFieldName.m_CurrentOption, newField);
			m_MainHeader.m_EditColorActive = true;
			m_FieldsTable.m_OnRefresh();
		};
		m_EditFieldPopup.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_EditFieldName);
			EditorUI::EditorUIService::SelectOption(m_EditFieldType);
			EditorUI::EditorUIService::EditVariable(m_EditFieldValue);
		};
		
	}

	GameStatePanel::GameStatePanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(GameStatePanel::OnKeyPressedEditor));

		InitializeOpeningScreen();
		InitializeDisplayGameStateScreen();

	}
	void GameStatePanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_MainWindow->m_ShowGameStateEditor);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (!m_EditorGameState)
		{
			
			EditorUI::EditorUIService::NewItemScreen("Open Existing Game State", KG_BIND_CLASS_FN(OnOpenGameStateDialog), "Create New Game State", KG_BIND_CLASS_FN(OnCreateGameStateDialog));
			EditorUI::EditorUIService::GenericPopup(m_CreateGameStatePopupSpec);
			EditorUI::EditorUIService::SelectOption(m_OpenGameStatePopupSpec);
		}
		else
		{
			EditorUI::EditorUIService::PanelHeader(m_MainHeader);
			EditorUI::EditorUIService::GenericPopup(m_DeleteGameStateWarning);
			EditorUI::EditorUIService::GenericPopup(m_CloseGameStateWarning);
			EditorUI::EditorUIService::List(m_FieldsTable);
			EditorUI::EditorUIService::SelectOption(m_AddFieldPopup);
			EditorUI::EditorUIService::GenericPopup(m_EditFieldPopup);
		}

		EditorUI::EditorUIService::EndWindow();
	}
	bool GameStatePanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	bool GameStatePanel::OnAssetEvent(Events::Event* event)
	{
		// Validate event type and asset type
		if (event->GetEventType() != Events::EventType::ManageAsset)
		{
			return false;
		}
		Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;
		if (manageAsset->GetAssetType() != Assets::AssetType::GameState)
		{
			return false;
		}

		if (manageAsset->GetAssetID() != m_EditorGameStateHandle)
		{
			return false;
		}

		// Handle deletion of asset
		if (manageAsset->GetAction() == Events::ManageAssetAction::PreDelete)
		{
			ResetPanelResources();
			return true;
		}

		// Handle updating of asset
		if (manageAsset->GetAction() == Events::ManageAssetAction::UpdateAssetInfo)
		{
			// Update game state header if necessary
			m_MainHeader.m_Label = Assets::AssetService::GetGameStateFileLocation(manageAsset->GetAssetID()).filename().string();
			
			return true;
		}
		return false;
	}
	void GameStatePanel::ResetPanelResources()
	{
		m_EditorGameState = nullptr;
		m_EditorGameStateHandle = Assets::EmptyHandle;
	}
	void GameStatePanel::OpenCreateDialog(std::filesystem::path& createLocation)
	{
		
		// Open game state Window
		s_MainWindow->m_ShowGameStateEditor = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		if (!m_EditorGameState)
		{
			// Open dialog to create editor game state
			OnCreateGameStateDialog();
			m_SelectGameStateLocationSpec.m_CurrentOption = createLocation;
		}
		else
		{
			// Add warning to close active game state before creating a new game state
			s_MainWindow->OpenWarningMessage("A game state is already active inside the editor. Please close the current game state before creating a new one.");
		}
	}

	void GameStatePanel::OpenAssetInEditor(std::filesystem::path& assetLocation)
	{
		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

		// Ensure provided path is within the active asset directory
		std::filesystem::path activeAssetDirectory = projectPaths.GetAssetDirectory();
		if (!Utility::FileSystem::DoesPathContainSubPath(activeAssetDirectory, assetLocation))
		{
			KG_WARN("Could not open asset in editor. Provided path does not exist within active asset directory");
			return;
		}

		// Look for asset in registry using the file location
		std::filesystem::path relativePath{ Utility::FileSystem::GetRelativePath(activeAssetDirectory, assetLocation) };
		Assets::AssetHandle assetHandle = Assets::AssetService::GetGameStateHandleFromFileLocation(relativePath);

		// Validate resulting handle
		if (!assetHandle)
		{
			KG_WARN("Could not open asset in editor. Provided path does not result in an asset inside the registry.");
			return;
		}

		// Open the editor panel to be visible
		s_MainWindow->m_ShowGameStateEditor = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		// Early out if asset is already open
		if (m_EditorGameStateHandle == assetHandle)
		{
			return;
		}

		// Check if panel is already occupied by an asset
		if (!m_EditorGameState)
		{
			OnOpenGameState(assetHandle);
		}
		else
		{
			// Add warning to close active AI state before opening a new AIState
			s_MainWindow->OpenWarningMessage("An game state is already active inside the editor. Please close the current game state before opening a new one.");
		}
	}

	void GameStatePanel::OnOpenGameStateDialog()
	{
		m_OpenGameStatePopupSpec.m_OpenPopup = true;
	}
	void GameStatePanel::OnCreateGameStateDialog()
	{
		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

		m_SelectGameStateLocationSpec.m_CurrentOption = projectPaths.GetAssetDirectory();
		m_CreateGameStatePopupSpec.m_OpenPopup = true;
	}
	void GameStatePanel::OnRefreshData()
	{
		m_FieldsTable.m_OnRefresh();
	}
	void GameStatePanel::OnOpenGameState(Assets::AssetHandle newHandle)
	{
		m_EditorGameState = Assets::AssetService::GetGameState(newHandle);
		m_EditorGameStateHandle = newHandle;
		m_MainHeader.m_EditColorActive = false;
		m_MainHeader.m_Label = Assets::AssetService::GetGameStateRegistry().at(
			m_EditorGameStateHandle).Data.FileLocation.filename().string();
		OnRefreshData();
	}
}
