#include "GameStatePanel.h"

#include "EditorLayer.h"
#include "Kargono.h"

namespace Kargono
{
	static EditorLayer* s_EditorLayer { nullptr };
	static GameStatePanel* s_GameStatePanel { nullptr };

	static std::string s_CurrentField {};

	static std::function<void()> s_OnOpenGameState { nullptr };
	static std::function<void()> s_OnCreateGameState { nullptr };
	static UI::GenericPopupSpec s_CreateGameStatePopupSpec {};
	static UI::SelectOptionSpec s_OpenGameStatePopupSpec {};
	static UI::TextInputSpec s_SelectGameStateNameSpec {};
	static UI::SelectorHeaderSpec s_MainHeader {};
	static UI::GenericPopupSpec s_DeleteGameStateWarning {};
	static UI::GenericPopupSpec s_CloseGameStateWarning {};
	static UI::TableSpec s_FieldsTable {};
	static UI::SelectOptionSpec s_AddFieldPopup {};
	static UI::GenericPopupSpec s_EditFieldPopup {};
	static UI::TextInputSpec s_EditFieldName {};
	static UI::SelectOptionSpec s_EditFieldType {};
	static UI::EditVariableSpec s_EditFieldValue {};

	void InitializeOpeningScreen()
	{
		s_OnOpenGameState = [&]()
		{
			s_OpenGameStatePopupSpec.StartPopup = true;
		};

		s_OnCreateGameState = [&]()
		{
			s_CreateGameStatePopupSpec.PopupActive = true;
		};

		s_OpenGameStatePopupSpec.Label = "Open Game State";
		s_OpenGameStatePopupSpec.WidgetID = 0x698b28531a4b4ed9;
		s_OpenGameStatePopupSpec.CurrentOption = "None";
		s_OpenGameStatePopupSpec.PopupOnly = true;
		s_OpenGameStatePopupSpec.PopupAction = [&](UI::SelectOptionSpec& spec)
		{
			spec.GetOptionsList().clear();
			spec.CurrentOption = "None";

			spec.AddToOptionsList("Clear", "None");
			for (auto& [handle, asset] : Assets::AssetManager::GetGameStateRegistry())
			{
				spec.AddToOptionsList("All Options", asset.Data.IntermediateLocation.string());
			}
		};

		s_OpenGameStatePopupSpec.ConfirmAction = [&](const std::string& selection)
		{
			if (selection == "None")
			{
				return;
			}

			for (auto& [id, asset] : Assets::AssetManager::GetGameStateRegistry())
			{
				if (selection == asset.Data.IntermediateLocation)
				{
					s_EditorLayer->m_GameStatePanel->m_EditorGameState = Assets::AssetManager::GetGameState(asset.Handle);
					s_EditorLayer->m_GameStatePanel->m_EditorGameStateHandle = asset.Handle;
					s_MainHeader.EditColorActive = false;
					s_FieldsTable.OnRefresh();
					return;
				}
			}
		};

		s_SelectGameStateNameSpec.Label = "New Name";
		s_SelectGameStateNameSpec.WidgetID = 0x1938fc315cf34a63;
		s_SelectGameStateNameSpec.CurrentOption = "Empty";
		s_SelectGameStateNameSpec.ConfirmAction = [&](const std::string& option)
		{
			s_SelectGameStateNameSpec.CurrentOption = option;
		};

		s_CreateGameStatePopupSpec.Label = "Create Game State";
		s_CreateGameStatePopupSpec.WidgetID = 0xd24ee715292c;
		s_CreateGameStatePopupSpec.PopupWidth = 420.0f;
		s_CreateGameStatePopupSpec.ConfirmAction = [&]()
		{
			if (s_SelectGameStateNameSpec.CurrentOption == "")
			{
				return;
			}

			for (auto& [id, asset] : Assets::AssetManager::GetGameStateRegistry())
			{
				if (asset.Data.GetSpecificFileData<Assets::GameStateMetaData>()->Name == s_SelectGameStateNameSpec.CurrentOption)
				{
					return;
				}
			}
			m_EditorGameStateHandle = Assets::AssetManager::CreateNewGameState(s_SelectGameStateNameSpec.CurrentOption);
			m_EditorGameState = Assets::AssetManager::GetGameState(m_EditorGameStateHandle);
			s_MainHeader.EditColorActive = false;
			s_FieldsTable.OnRefresh();
		};
		s_CreateGameStatePopupSpec.PopupContents = [&]()
		{
			UI::Editor::TextInputModal(s_SelectGameStateNameSpec);
		};
	}

	void InitializeDisplayGameStateScreen()
	{
		// Header (Game State Name and Options)
		s_DeleteGameStateWarning.Label = "Delete Game State";
		s_DeleteGameStateWarning.WidgetID = 0x50315ffeaeb74340;
		s_DeleteGameStateWarning.ConfirmAction = [&]()
		{
			Assets::AssetManager::DeleteGameState(m_EditorGameStateHandle);
			m_EditorGameStateHandle = 0;
			m_EditorGameState = nullptr;
		};
		s_DeleteGameStateWarning.PopupContents = [&]()
		{
			UI::Editor::Text("Are you sure you want to delete this game state object?");
		};

		s_CloseGameStateWarning.Label = "Close Game State";
		s_CloseGameStateWarning.WidgetID = 0x7bf12767ea9848c5;
		s_CloseGameStateWarning.ConfirmAction = [&]()
		{
			m_EditorGameStateHandle = 0;
			m_EditorGameState = nullptr;
		};
		s_CloseGameStateWarning.PopupContents = [&]()
		{
			UI::Editor::Text("Are you sure you want to close this game state object without saving?");
		};

		s_MainHeader.WidgetID = 0x982a395861d943e2;
		s_MainHeader.AddToSelectionList("Save", [&]()
			{
				Assets::AssetManager::SaveGameState(m_EditorGameStateHandle, m_EditorGameState);
				s_MainHeader.EditColorActive = false;
			});
		s_MainHeader.AddToSelectionList("Close", [&]()
			{
			if (s_MainHeader.EditColorActive)
			{
				s_CloseGameStateWarning.PopupActive = true;
			}
			else
			{
				m_EditorGameStateHandle = 0;
				m_EditorGameState = nullptr;
			}
			});
		s_MainHeader.AddToSelectionList("Delete", [&]()
			{
				s_DeleteGameStateWarning.PopupActive = true;
			});
		

		// Fields Table
		s_FieldsTable.Label = "Fields";
		s_FieldsTable.WidgetID = 0x2729865b71ab4ee4;
		s_FieldsTable.OnRefresh = [&]()
		{
			s_FieldsTable.ClearTable();
			if (m_EditorGameState)
			{
				for (auto& [name, field] : m_EditorGameState->GetAllFields())
				{
					switch (field->Type())
					{
					case WrappedVarType::UInteger16:
						{
							s_FieldsTable.InsertTableEntry(name,
								std::to_string(field->GetWrappedValue<uint16_t>()),
								[&](UI::TableEntry& entry)
								{
									s_EditFieldPopup.PopupActive = true;
									s_CurrentField = name;
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
		s_FieldsTable.Column1Title = "Field Name";
		s_FieldsTable.Column2Title = "Field Value";
		s_FieldsTable.AddToSelectionList("Add New Field", [&]()
			{
				s_AddFieldPopup.StartPopup = true;
			});

		s_AddFieldPopup.Label = "Add New Field";
		s_AddFieldPopup.WidgetID = 0x00e5c69ec3f44d44;
		s_AddFieldPopup.PopupOnly = true;
		s_AddFieldPopup.CurrentOption = "None";
		s_AddFieldPopup.LineCount = 2;
		s_AddFieldPopup.PopupAction = [&](UI::SelectOptionSpec& spec)
		{
			spec.ClearOptionsList();
			spec.AddToOptionsList("Clear", "None");
			spec.AddToOptionsList("All Options", "UInteger16");
		};
		s_AddFieldPopup.ConfirmAction = [&](const std::string& selection)
		{
			if (selection == "None")
			{
				return;
			}

			if (selection == "UInteger16")
			{
				uint32_t iteration{ 1 };
				while (!m_EditorGameState->AddField("New Field " + std::to_string(iteration), WrappedVarType::UInteger16))
				{
					iteration++;
				}
				s_MainHeader.EditColorActive = true;
			}
			s_FieldsTable.OnRefresh();
		};

		s_EditFieldName.Label = "Field Name";
		s_EditFieldName.WidgetID = 0x6d282dd1f6fc476a;
		s_EditFieldName.CurrentOption = "Empty";
		s_EditFieldName.ConfirmAction = [&](const std::string& option)
		{
			s_EditFieldName.CurrentOption = option;
		};

		s_EditFieldType.Label = "Field Type";
		s_EditFieldType.WidgetID = 0xf41839b571044a43;
		s_EditFieldType.PopupOnly = false;
		s_EditFieldType.CurrentOption = "None";
		s_EditFieldType.LineCount = 2;
		s_EditFieldType.PopupAction = [&](UI::SelectOptionSpec& spec)
		{
			spec.ClearOptionsList();
			spec.AddToOptionsList("All Options", "UInteger16");
		};
		s_EditFieldType.ConfirmAction = [&](const std::string& selection)
		{
			if (selection == "UInteger16")
			{
				// Set Field
			}
			s_FieldsTable.OnRefresh();
		};
		s_EditFieldValue.Label = "Field Value";
		s_EditFieldValue.WidgetID = 0xcfc46ea99d774fa0;
		s_EditFieldValue.AllocateBuffer();

		s_EditFieldPopup.Label = "Edit Field";
		s_EditFieldPopup.WidgetID = 0xcada361be94b47c5;
		s_EditFieldPopup.DeleteActive = true;
		s_EditFieldPopup.DeleteAction = [&]()
		{
			if (!m_EditorGameState->DeleteField(s_CurrentField))
			{
				KG_ERROR("Unable to delete field inside game state!");
				return;
			}
			s_MainHeader.EditColorActive = true;
			s_FieldsTable.OnRefresh();
		};
		s_EditFieldPopup.PopupWidth = 420.0f;
		s_EditFieldPopup.PopupAction = [&](UI::GenericPopupSpec& spec)
		{
			Ref<WrappedVariable> field = m_EditorGameState->GetField(s_CurrentField);

			if (!field)
			{
				KG_ERROR("Unable to retreive field from current game state object");
				return;
			}
			s_EditFieldName.CurrentOption = s_CurrentField;
			s_EditFieldType.CurrentOption = Utility::WrappedVarTypeToString(field->Type());
			std::string convertCache { std::to_string(field->GetWrappedValue<uint16_t>()) };
			s_EditFieldValue.FieldBuffer.SetDataToByte(0);
			memcpy(s_EditFieldValue.FieldBuffer.Data, convertCache.data(), convertCache.size());
		};
		s_EditFieldPopup.ConfirmAction = [&]()
		{
			auto& fieldMap = m_EditorGameState->GetAllFields();
			Ref<WrappedVariable> oldField = fieldMap.at(s_CurrentField);
			Ref<WrappedVariable> newField { nullptr };
			if (s_CurrentField != s_EditFieldName.CurrentOption)
			{
				if (!oldField)
				{
					KG_ERROR("Could not obtain field pointer from game state field map");
					return;
				}
				fieldMap.erase(s_CurrentField);
				s_FieldsTable.OnRefresh();
			}

			switch (Utility::StringToWrappedVarType(s_EditFieldType.CurrentOption))
			{
				case WrappedVarType::UInteger16:
				{
					newField = CreateRef<WrappedUInteger16>();

					bool success = Utility::Conversions::BufferToVariable<uint16_t>(s_EditFieldValue.FieldBuffer,
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

			fieldMap.insert_or_assign(s_EditFieldName.CurrentOption, newField);
			s_MainHeader.EditColorActive = true;
			s_FieldsTable.OnRefresh();
		};
		s_EditFieldPopup.PopupContents = [&]()
		{
			UI::Editor::TextInputModal(s_EditFieldName);
			UI::Editor::SelectOption(s_EditFieldType);
			UI::Editor::EditVariable(s_EditFieldValue);
		};
		
	}

	GameStatePanel::GameStatePanel()
	{
		s_EditorLayer = EditorLayer::GetCurrentLayer();
		s_GameStatePanel = this;

		InitializeOpeningScreen();

		InitializeDisplayGameStateScreen();

	}
	void GameStatePanel::OnEditorUIRender()
	{
		UI::Editor::StartWindow("Game State Editor", &s_EditorLayer->m_ShowGameStateEditor);

		if (!m_EditorGameState)
		{
			UI::Editor::NewItemScreen("Open Existing Game State", s_OnOpenGameState, "Create New Game State", s_OnCreateGameState);
			UI::Editor::GenericPopup(s_CreateGameStatePopupSpec);
			UI::Editor::SelectOption(s_OpenGameStatePopupSpec);
		}
		else
		{
			s_MainHeader.Label = m_EditorGameState->GetName();
			UI::Editor::SelectorHeader(s_MainHeader);
			UI::Editor::GenericPopup(s_DeleteGameStateWarning);
			UI::Editor::GenericPopup(s_CloseGameStateWarning);
			UI::Editor::Table(s_FieldsTable);
			UI::Editor::SelectOption(s_AddFieldPopup);
			UI::Editor::GenericPopup(s_EditFieldPopup);
		}

		UI::Editor::EndWindow();
	}
}
