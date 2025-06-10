#include "Windows/MainWindow/GlobalStatePanel.h"

#include "EditorApp.h"

static Kargono::EditorApp* s_EditorApp{ nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{
	void GlobalStatePanel::InitializeOpeningScreen()
	{
		m_OpenGlobalStatePopup.m_Label = "Open Global State";
		m_OpenGlobalStatePopup.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_OpenGlobalStatePopup.m_Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenGlobalStatePopup.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
			{
				spec.GetAllOptions().clear();
				spec.m_CurrentOption = { "None", Assets::EmptyHandle };

				spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, asset] : Assets::AssetService::GetGlobalStateRegistry())
				{
					spec.AddToOptions("All Options", asset.Data.FileLocation.filename().string(), handle);
				}
			};

		m_OpenGlobalStatePopup.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (selection.m_Handle == Assets::EmptyHandle)
				{
					KG_WARN("No global state Selected");
					return;
				}
				if (!Assets::AssetService::GetGlobalStateRegistry().contains(selection.m_Handle))
				{
					KG_WARN("Could not find on global state in global state editor");
					return;
				}

				OnOpenGlobalState(selection.m_Handle);
			};

		m_SelectGlobalStateNameSpec.m_Label = "New Name";
		m_SelectGlobalStateNameSpec.m_CurrentOption = "Empty";

		m_SelectGlobalStateLocationSpec.m_Label = "Location";
		m_SelectGlobalStateLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveContext().GetProjectPaths().GetAssetDirectory();
		m_SelectGlobalStateLocationSpec.m_ConfirmAction = [&](std::string_view path)
			{
				if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveContext().GetProjectPaths().GetAssetDirectory(), path))
				{
					KG_WARN("Cannot create an asset outside of the project's asset directory.");
					m_SelectGlobalStateLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveContext().GetProjectPaths().GetAssetDirectory();
				}
			};

		m_CreateGlobalStatePopup.m_Label = "Create Global State";
		m_CreateGlobalStatePopup.m_ConfirmAction = [&]()
			{
				if (m_SelectGlobalStateNameSpec.m_CurrentOption == "")
				{
					return;
				}

				for (auto& [id, asset] : Assets::AssetService::GetGlobalStateRegistry())
				{
					if (asset.Data.GetSpecificMetaData<Assets::GlobalStateMetaData>()->Name == m_SelectGlobalStateNameSpec.m_CurrentOption)
					{
						return;
					}
				}
				m_EditorGlobalStateHandle = Assets::AssetService::CreateGlobalState(m_SelectGlobalStateNameSpec.m_CurrentOption.c_str(), m_SelectGlobalStateLocationSpec.m_CurrentOption);
				m_EditorGlobalState = Assets::AssetService::GetGlobalState(m_EditorGlobalStateHandle);
				m_MainHeader.m_EditColorActive = false;
				m_MainHeader.m_Label = Assets::AssetService::GetGlobalStateRegistry().at(
					m_EditorGlobalStateHandle).Data.FileLocation.filename().string();
			};
		m_CreateGlobalStatePopup.m_PopupContents = [&]()
			{
				m_SelectGlobalStateNameSpec.RenderText();
				m_SelectGlobalStateLocationSpec.RenderChooseDir();
			};
	}


	void GlobalStatePanel::InitializeDisplayGlobalStateScreen()
	{
		// Header (global state Name and Options)
		m_DeleteGlobalStateWarning.m_Label = "Delete Global State";
		m_DeleteGlobalStateWarning.m_ConfirmAction = [&]()
		{
			Assets::AssetService::DeleteGlobalState(m_EditorGlobalStateHandle);
				
			ResetPanelResources();
		};
		m_DeleteGlobalStateWarning.m_PopupContents = [&]()
			{
				EditorUI::EditorUIService::Text("Are you sure you want to delete this global state object?");
			};

		m_CloseGlobalStateWarning.m_Label = "Close Global State";
		m_CloseGlobalStateWarning.m_ConfirmAction = [&]()
		{
			ResetPanelResources();
		};
		m_CloseGlobalStateWarning.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to close this Global State object without saving?");
		};

		m_MainHeader.AddToSelectionList("Save", [&]()
		{
			Assets::AssetService::SaveGlobalState(m_EditorGlobalStateHandle, m_EditorGlobalState);
			m_MainHeader.m_EditColorActive = false;
		});
		m_MainHeader.AddToSelectionList("Close", [&]()
		{
			if (m_MainHeader.m_EditColorActive)
			{
				m_CloseGlobalStateWarning.m_OpenPopup = true;
			}
			else
			{
				ResetPanelResources();
			}
		});
		m_MainHeader.AddToSelectionList("Delete", [&]()
		{
			m_DeleteGlobalStateWarning.m_OpenPopup = true;
		});
	}

	void GlobalStatePanel::InitializeDisplayGlobalStateFields()
	{
		m_FieldsHeader.m_Label = "Fields";
		m_FieldsHeader.m_Expanded = true;
		m_FieldsHeader.AddToSelectionList("Create Field", KG_BIND_CLASS_FN(OnOpenCreateFieldDialog));

		m_CreateFieldPopup.m_Label = "Create Field";
		m_CreateFieldPopup.m_PopupContents = [&]() 
		{
			m_CreateFieldName.RenderText();
			m_CreateFieldType.RenderOptions();
		};
		m_CreateFieldPopup.m_ConfirmAction = KG_BIND_CLASS_FN(OnConfirmCreateField);

		m_CreateFieldName.m_Label = "Field Name";
		m_CreateFieldName.m_CurrentOption = "NewField";

		m_CreateFieldType.m_Label = "Field Type";
		m_CreateFieldType.m_PopupAction = KG_BIND_CLASS_FN(OnOpenCreateFieldType);
		
	}

	GlobalStatePanel::GlobalStatePanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(GlobalStatePanel::OnKeyPressedEditor));

		InitializeOpeningScreen();
		InitializeDisplayGlobalStateScreen();
		InitializeDisplayGlobalStateFields();
	}
	void GlobalStatePanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_MainWindow->m_ShowGlobalStateEditor);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (!m_EditorGlobalState)
		{

			EditorUI::EditorUIService::NewItemScreen("Open Existing Global State", KG_BIND_CLASS_FN(OnOpenGlobalStateDialog), "Create New Global State", KG_BIND_CLASS_FN(OnCreateGlobalStateDialog));
			m_CreateGlobalStatePopup.RenderPopup();
			m_OpenGlobalStatePopup.RenderOptions();
		}
		else
		{
			m_MainHeader.RenderHeader();
			DrawGlobalStateFields();
			m_CreateFieldPopup.RenderPopup();
			m_DeleteGlobalStateWarning.RenderPopup();
			m_CloseGlobalStateWarning.RenderPopup();
		}

		EditorUI::EditorUIService::EndWindow();
	}
	bool GlobalStatePanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	bool GlobalStatePanel::OnAssetEvent(Events::Event* event)
	{
		// Validate event type and asset type
		if (event->GetEventType() != Events::EventType::ManageAsset)
		{
			return false;
		}
		Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;
		if (manageAsset->GetAssetType() != Assets::AssetType::GlobalState)
		{
			return false;
		}

		if (manageAsset->GetAssetID() != m_EditorGlobalStateHandle)
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
			// Update global state header if necessary
			m_MainHeader.m_Label = Assets::AssetService::GetGlobalStateFileLocation(manageAsset->GetAssetID()).filename().string();

			return true;
		}
		return false;
	}
	void GlobalStatePanel::ResetPanelResources()
	{
		EngineService::GetActiveEngine().GetThread().SubmitFunction([&]()
		{
			m_EditorGlobalState = nullptr;
			m_EditorGlobalStateHandle = Assets::EmptyHandle;
		});
	}
	void GlobalStatePanel::OpenCreateDialog(std::filesystem::path& createLocation)
	{
		// Open global state Window
		s_MainWindow->m_ShowGlobalStateEditor = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		if (!m_EditorGlobalState)
		{
			// Open dialog to create editor global state
			OnCreateGlobalStateDialog();
			m_SelectGlobalStateLocationSpec.m_CurrentOption = createLocation;
		}
		else
		{
			// Add warning to close active global state before creating a new global state
			s_MainWindow->OpenWarningMessage("A Global State is already active inside the editor. Please close the current global state before creating a new one.");
		}
	}

	void GlobalStatePanel::OpenAssetInEditor(std::filesystem::path& assetLocation)
	{
		// Ensure provided path is within the active asset directory
		std::filesystem::path activeAssetDirectory = Projects::ProjectService::GetActiveContext().GetProjectPaths().GetAssetDirectory();
		if (!Utility::FileSystem::DoesPathContainSubPath(activeAssetDirectory, assetLocation))
		{
			KG_WARN("Could not open asset in editor. Provided path does not exist within active asset directory");
			return;
		}

		// Look for asset in registry using the file location
		std::filesystem::path relativePath{ Utility::FileSystem::GetRelativePath(activeAssetDirectory, assetLocation) };
		Assets::AssetHandle assetHandle = Assets::AssetService::GetGlobalStateHandleFromFileLocation(relativePath);

		// Validate resulting handle
		if (!assetHandle)
		{
			KG_WARN("Could not open asset in editor. Provided path does not result in an asset inside the registry.");
			return;
		}

		// Open the editor panel to be visible
		s_MainWindow->m_ShowGlobalStateEditor = true;
		EditorUI::EditorUIService::BringWindowToFront(m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_PanelName);

		// Early out if asset is already open
		if (m_EditorGlobalStateHandle == assetHandle)
		{
			return;
		}

		// Check if panel is already occupied by an asset
		if (!m_EditorGlobalState)
		{
			OnOpenGlobalState(assetHandle);
		}
		else
		{
			// Add warning to close active AI state before opening a new AIState
			s_MainWindow->OpenWarningMessage("An Global State is already active inside the editor. Please close the current Global State before opening a new one.");
		}
	}

	struct DrawGlobalStateFieldsVisitor
	{
		void operator()(EditorUI::CheckboxWidget& spec)
		{
			// Get iteration/type information from widget
			auto [iteration, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();
			Ref<ProjectData::GlobalState> editorState = s_EditorApp->m_MainWindow->m_GlobalStatePanel->m_EditorGlobalState;

			// Set the field data
			spec.m_CurrentBoolean = *ProjectData::GlobalStateService::GetGlobalStateField<bool>
				(
					editorState.get(),
					iteration
				);

			// Draw the field
			spec.RenderCheckbox();
		}
		void operator()(EditorUI::EditTextSpec& spec)
		{
			// Get iteration/type information from widget
			auto [iteration, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();
			Ref<ProjectData::GlobalState> editorState = s_EditorApp->m_MainWindow->m_GlobalStatePanel->m_EditorGlobalState;

			// Set the field data
			spec.m_CurrentOption = *ProjectData::GlobalStateService::GetGlobalStateField<std::string>
			(
				editorState.get(),
				iteration
			);

			// Draw the field
			spec.RenderText();
		}
		void operator()(EditorUI::EditIntegerWidget& spec)
		{
			// Get iteration/type information from widget
			auto [iteration, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();
			Ref<ProjectData::GlobalState> editorState = s_EditorApp->m_MainWindow->m_GlobalStatePanel->m_EditorGlobalState;

			// Set the field data
			switch (type)
			{
			case WrappedVarType::Integer16:
				spec.m_CurrentInteger = (int32_t)*ProjectData::GlobalStateService::GetGlobalStateField<int16_t>(editorState.get(), iteration);
				break;
			case WrappedVarType::Integer32:
				spec.m_CurrentInteger = (int32_t)*ProjectData::GlobalStateService::GetGlobalStateField<int32_t>(editorState.get(), iteration);
				break;
			case WrappedVarType::Integer64:
				spec.m_CurrentInteger = (int32_t)*ProjectData::GlobalStateService::GetGlobalStateField<int64_t>(editorState.get(), iteration);
				break;
			case WrappedVarType::UInteger16:
				spec.m_CurrentInteger = (int32_t)*ProjectData::GlobalStateService::GetGlobalStateField<uint16_t>(editorState.get(), iteration);
				break;
			case WrappedVarType::UInteger32:
				spec.m_CurrentInteger = (int32_t)*ProjectData::GlobalStateService::GetGlobalStateField<uint32_t>(editorState.get(), iteration);
				break;
			case WrappedVarType::UInteger64:
				spec.m_CurrentInteger = (int32_t)*ProjectData::GlobalStateService::GetGlobalStateField<uint64_t>(editorState.get(), iteration);
				break;
			default:
				KG_ERROR("Unhandled wrapped var type provided when drawing global state widgets");
				break;
			}

			// Draw the field
			spec.RenderInteger();
		}
		void operator()(EditorUI::EditFloatWidget& spec)
		{
			// Get iteration/type information from widget
			auto [iteration, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();
			Ref<ProjectData::GlobalState> editorState = s_EditorApp->m_MainWindow->m_GlobalStatePanel->m_EditorGlobalState;

			// Set the field data
			spec.m_CurrentFloat = *ProjectData::GlobalStateService::GetGlobalStateField<float>(editorState.get(), iteration);

			// Draw the field
			spec.RenderFloat();
		}
		void operator()(EditorUI::EditVec2Widget& spec)
		{
			// Get iteration/type information from widget
			auto [iteration, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();
			Ref<ProjectData::GlobalState> editorState = s_EditorApp->m_MainWindow->m_GlobalStatePanel->m_EditorGlobalState;

			// Set the field data
			spec.m_CurrentVec2 = *ProjectData::GlobalStateService::GetGlobalStateField<Math::vec2>(editorState.get(), iteration);

			// Draw the field
			spec.RenderVec2();
		}
		void operator()(EditorUI::EditVec3Widget& spec)
		{
			// Get iteration/type information from widget
			auto [iteration, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();
			Ref<ProjectData::GlobalState> editorState = s_EditorApp->m_MainWindow->m_GlobalStatePanel->m_EditorGlobalState;

			// Set the field data
			spec.m_CurrentVec3 = *ProjectData::GlobalStateService::GetGlobalStateField<Math::vec3>(editorState.get(), iteration);

			// Draw the field
			spec.RenderVec3();
		}
		void operator()(EditorUI::EditVec4Widget& spec)
		{
			// Get iteration/type information from widget
			auto [iteration, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();
			Ref<ProjectData::GlobalState> editorState = s_EditorApp->m_MainWindow->m_GlobalStatePanel->m_EditorGlobalState;

			// Set the field data
			spec.m_CurrentVec4 = *ProjectData::GlobalStateService::GetGlobalStateField<Math::vec4>(editorState.get(), iteration);

			// Draw the field
			spec.RenderVec4();
		}
		void operator()(EditorUI::EditIVec2Widget& spec)
		{
			// Get iteration/type information from widget
			auto [iteration, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();
			Ref<ProjectData::GlobalState> editorState = s_EditorApp->m_MainWindow->m_GlobalStatePanel->m_EditorGlobalState;

			// Set the field data
			spec.m_CurrentIVec2 = *ProjectData::GlobalStateService::GetGlobalStateField<Math::ivec2>(editorState.get(), iteration);

			// Draw the field
			spec.RenderIVec2();
		}
		void operator()(EditorUI::EditIVec3Widget& spec)
		{
			// Get iteration/type information from widget
			auto [iteration, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();
			Ref<ProjectData::GlobalState> editorState = s_EditorApp->m_MainWindow->m_GlobalStatePanel->m_EditorGlobalState;

			// Set the field data
			spec.m_CurrentIVec3 = *ProjectData::GlobalStateService::GetGlobalStateField<Math::ivec3>(editorState.get(), iteration);

			// Draw the field
			spec.RenderIVec3();
		}
		void operator()(EditorUI::EditIVec4Widget& spec)
		{
			// Get iteration/type information from widget
			auto [iteration, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();
			Ref<ProjectData::GlobalState> editorState = s_EditorApp->m_MainWindow->m_GlobalStatePanel->m_EditorGlobalState;

			// Set the field data
			spec.m_CurrentIVec4 = *ProjectData::GlobalStateService::GetGlobalStateField<Math::ivec4>(editorState.get(), iteration);

			// Draw the field
			spec.RenderIVec4();
		}
	};

	void GlobalStatePanel::DrawGlobalStateFields()
	{
		// Draw header for fields
		m_FieldsHeader.RenderHeader();

		if (m_FieldsHeader.m_Expanded)
		{
			// Call the draw functions (on the visitor struct) for each field
			for (size_t iteration{ 0 }; iteration < m_Fields.m_Fields.size(); iteration++)
			{
				std::visit(DrawGlobalStateFieldsVisitor{}, m_Fields.m_Fields.at(iteration));
			}
		}
		
	}

	void GlobalStatePanel::OnOpenGlobalStateDialog()
	{
		m_OpenGlobalStatePopup.m_OpenPopup = true;
	}
	void GlobalStatePanel::OnCreateGlobalStateDialog()
	{
		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

		m_SelectGlobalStateLocationSpec.m_CurrentOption = projectPaths.GetAssetDirectory();
		m_CreateGlobalStatePopup.m_OpenPopup = true;
	}
	void GlobalStatePanel::OnRefreshData()
	{
		// Revalidate the widgets that modify the global state's fields
		m_Fields.m_Fields.clear();
		size_t iteration{ 0 };
		for (WrappedVarType type : m_EditorGlobalState->m_DataTypes)
		{
			switch (type)
			{
			case WrappedVarType::Bool:
			{
				EditorWidget& widget = m_Fields.m_Fields.emplace_back(EditorUI::CheckboxWidget());
				EditorUI::CheckboxWidget& checkboxWidget = *std::get_if<EditorUI::CheckboxWidget>(&widget);
				checkboxWidget.m_Label = m_EditorGlobalState->m_DataNames.at(iteration);
				checkboxWidget.m_Flags |= EditorUI::Checkbox_Indented;
				checkboxWidget.m_ProvidedData = CreateRef<std::pair<size_t, WrappedVarType>>(iteration, type);
				checkboxWidget.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyFieldCheckbox);
				break;
			}
			case WrappedVarType::Float:
			{
				EditorWidget& widget = m_Fields.m_Fields.emplace_back(EditorUI::EditFloatWidget());
				EditorUI::EditFloatWidget& editFloatSpec = *std::get_if<EditorUI::EditFloatWidget>(&widget);
				editFloatSpec.m_Label = m_EditorGlobalState->m_DataNames.at(iteration);
				editFloatSpec.m_Flags |= EditorUI::EditFloat_Indented;
				editFloatSpec.m_ProvidedData = CreateRef<std::pair<size_t, WrappedVarType>>(iteration, type);
				editFloatSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyFieldEditFloat);
				break;
			}
			case WrappedVarType::Integer16:
			case WrappedVarType::Integer32:
			case WrappedVarType::Integer64:
			{
				EditorWidget& widget = m_Fields.m_Fields.emplace_back(EditorUI::EditIntegerWidget());
				EditorUI::EditIntegerWidget& editIntSpec = *std::get_if<EditorUI::EditIntegerWidget>(&widget);
				editIntSpec.m_Label = m_EditorGlobalState->m_DataNames.at(iteration);
				editIntSpec.m_Flags |= EditorUI::EditInteger_Indented;
				editIntSpec.m_Bounds = { -10'000, 10'000 };
				editIntSpec.m_ProvidedData = CreateRef<std::pair<size_t, WrappedVarType>>(iteration, type);
				editIntSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyFieldEditInteger);
				break;
			}
			case WrappedVarType::UInteger16:
			case WrappedVarType::UInteger32:
			case WrappedVarType::UInteger64:
			{
				EditorWidget& widget = m_Fields.m_Fields.emplace_back(EditorUI::EditIntegerWidget());
				EditorUI::EditIntegerWidget& editIntSpec = *std::get_if<EditorUI::EditIntegerWidget>(&widget);
				editIntSpec.m_Label = m_EditorGlobalState->m_DataNames.at(iteration);
				editIntSpec.m_Flags |= EditorUI::EditInteger_Indented;
				editIntSpec.m_Bounds = { 0, 10'000 };
				editIntSpec.m_ProvidedData = CreateRef<std::pair<size_t, WrappedVarType>>(iteration, type);
				editIntSpec.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyFieldEditInteger);
				break;
			}

			case WrappedVarType::Vector2:
			{
				EditorWidget& widget = m_Fields.m_Fields.emplace_back(EditorUI::EditVec2Widget());
				EditorUI::EditVec2Widget& editVec2Widget = *std::get_if<EditorUI::EditVec2Widget>(&widget);
				editVec2Widget.m_Label = m_EditorGlobalState->m_DataNames.at(iteration);
				editVec2Widget.m_Flags |= EditorUI::EditVec2_Indented;
				editVec2Widget.m_ProvidedData = CreateRef<std::pair<size_t, WrappedVarType>>(iteration, type);
				editVec2Widget.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyFieldEditVec2);
				break;
			}
			case WrappedVarType::Vector3:
			{
				EditorWidget& widget = m_Fields.m_Fields.emplace_back(EditorUI::EditVec3Widget());
				EditorUI::EditVec3Widget& editVec3Widget = *std::get_if<EditorUI::EditVec3Widget>(&widget);
				editVec3Widget.m_Label = m_EditorGlobalState->m_DataNames.at(iteration);
				editVec3Widget.m_Flags |= EditorUI::EditVec3_Indented;
				editVec3Widget.m_ProvidedData = CreateRef<std::pair<size_t, WrappedVarType>>(iteration, type);
				editVec3Widget.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyFieldEditVec3);
				break;
			}
			case WrappedVarType::Vector4:
			{
				EditorWidget& widget = m_Fields.m_Fields.emplace_back(EditorUI::EditVec4Widget());
				EditorUI::EditVec4Widget& editVec4Widget = *std::get_if<EditorUI::EditVec4Widget>(&widget);
				editVec4Widget.m_Label = m_EditorGlobalState->m_DataNames.at(iteration);
				editVec4Widget.m_Flags |= EditorUI::EditVec4_Indented;
				editVec4Widget.m_ProvidedData = CreateRef<std::pair<size_t, WrappedVarType>>(iteration, type);
				editVec4Widget.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyFieldEditVec4);
				break;
			}
			case WrappedVarType::IVector2:
			{
				EditorWidget& widget = m_Fields.m_Fields.emplace_back(EditorUI::EditIVec2Widget());
				EditorUI::EditIVec2Widget& editVec2Widget = *std::get_if<EditorUI::EditIVec2Widget>(&widget);
				editVec2Widget.m_Label = m_EditorGlobalState->m_DataNames.at(iteration);
				editVec2Widget.m_Flags |= EditorUI::EditIVec2_Indented;
				editVec2Widget.m_ProvidedData = CreateRef<std::pair<size_t, WrappedVarType>>(iteration, type);
				editVec2Widget.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyFieldEditIVec2);
				break;
			}
			case WrappedVarType::IVector3:
			{
				EditorWidget& widget = m_Fields.m_Fields.emplace_back(EditorUI::EditIVec3Widget());
				EditorUI::EditIVec3Widget& editVec3Widget = *std::get_if<EditorUI::EditIVec3Widget>(&widget);
				editVec3Widget.m_Label = m_EditorGlobalState->m_DataNames.at(iteration);
				editVec3Widget.m_Flags |= EditorUI::EditIVec3_Indented;
				editVec3Widget.m_ProvidedData = CreateRef<std::pair<size_t, WrappedVarType>>(iteration, type);
				editVec3Widget.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyFieldEditIVec3);
				break;
			}
			case WrappedVarType::IVector4:
			{
				EditorWidget& widget = m_Fields.m_Fields.emplace_back(EditorUI::EditIVec4Widget());
				EditorUI::EditIVec4Widget& editVec4Widget = *std::get_if<EditorUI::EditIVec4Widget>(&widget);
				editVec4Widget.m_Label = m_EditorGlobalState->m_DataNames.at(iteration);
				editVec4Widget.m_Flags |= EditorUI::EditIVec4_Indented;
				editVec4Widget.m_ProvidedData = CreateRef<std::pair<size_t, WrappedVarType>>(iteration, type);
				editVec4Widget.m_ConfirmAction = KG_BIND_CLASS_FN(OnModifyFieldEditIVec4);
				break;
			}
			}
			iteration++;
		}
	}
	void GlobalStatePanel::OnOpenGlobalState(Assets::AssetHandle newHandle)
	{
		m_EditorGlobalState = Assets::AssetService::GetGlobalState(newHandle);
		m_EditorGlobalStateHandle = newHandle;
		m_MainHeader.m_EditColorActive = false;
		m_MainHeader.m_Label = Assets::AssetService::GetGlobalStateRegistry().at(
			m_EditorGlobalStateHandle).Data.FileLocation.filename().string();
		OnRefreshData();
	}
	void GlobalStatePanel::OnOpenCreateFieldDialog(EditorUI::CollapsingHeaderWidget& spec)
	{
		m_CreateFieldPopup.m_OpenPopup = true;
		m_CreateFieldName.m_CurrentOption = "NewField";
		m_CreateFieldType.m_CurrentOption = { Utility::WrappedVarTypeToString(WrappedVarType::Bool),
			(uint64_t)WrappedVarType::Bool};
	}
	void GlobalStatePanel::OnConfirmCreateField()
	{
		KG_ASSERT(m_EditorGlobalState);

		// Validate the provided values inside the editor widgets
		if (m_CreateFieldType.m_CurrentOption.m_Handle == (uint64_t)WrappedVarType::None)
		{
			KG_WARN("Failed to create new global state field. None type specified for the field type");
			return;
		}

		if (ProjectData::GlobalStateService::DoesGlobalStateContainName(m_EditorGlobalState.get(), m_CreateFieldName.m_CurrentOption.c_str()))
		{
			KG_WARN("Failed to create new global state field. Duplicate field name found");
			return;
		}

		// Add a new field
		if (!ProjectData::GlobalStateService::AddFieldToGlobalState(m_EditorGlobalState.get(),
			m_CreateFieldName.m_CurrentOption.c_str(), (WrappedVarType)(uint64_t)m_CreateFieldType.m_CurrentOption.m_Handle))
		{
			KG_WARN("Failed to create new global state field. Error occured while adding the global state field");
			return;
		}

		// Refresh table data and set the editor global state as modified
		OnRefreshData();
		m_MainHeader.m_EditColorActive = true;


	}
	void GlobalStatePanel::OnOpenCreateFieldType(EditorUI::SelectOptionWidget& spec)
	{
		// Clear options widget
		m_CreateFieldType.ClearOptions();

		// Fill options widget
		for (WrappedVarType type : s_AllWrappedVarTypes)
		{
			if (type == WrappedVarType::Entity || type == WrappedVarType::None || type == WrappedVarType::Void)
			{
				continue;
			}
			m_CreateFieldType.AddToOptions(Utility::WrappedVarTypeToCategory(type), 
				Utility::WrappedVarTypeToString(type), (uint64_t)type);
		}
	}
	void GlobalStatePanel::OnModifyFieldCheckbox(EditorUI::CheckboxWidget& spec)
	{
		// Get index/type information from widget
		auto [index, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();

		// Modify the field
		bool success = ProjectData::GlobalStateService::SetGlobalStateField<bool>(m_EditorGlobalState.get(), index,
			type, &spec.m_CurrentBoolean);

		if (!success)
		{
			KG_WARN("Failed to modify field");
			return;
		}
	}
	void GlobalStatePanel::OnModifyFieldEditFloat(EditorUI::EditFloatWidget& spec)
	{
		// Get index/type information from widget
		auto [index, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();

		// Modify the field
		bool success = ProjectData::GlobalStateService::SetGlobalStateField<float>(m_EditorGlobalState.get(), index,
			type, &spec.m_CurrentFloat);

		if (!success)
		{
			KG_WARN("Failed to modify field");
			return;
		}
	}
	void GlobalStatePanel::OnModifyFieldEditInteger(EditorUI::EditIntegerWidget& spec)
	{
		// Get index/type information from widget
		auto [index, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();

		bool success{ false };

		// Set the field data
		switch (type)
		{
		case WrappedVarType::Integer16:
		{
			int16_t widgetValue = (int16_t)spec.m_CurrentInteger;
			success = ProjectData::GlobalStateService::SetGlobalStateField<int16_t>(m_EditorGlobalState.get(), index,
				type, &widgetValue);
			break;
		}
		case WrappedVarType::Integer32:
		{
			int32_t widgetValue = (int32_t)spec.m_CurrentInteger;
			success = ProjectData::GlobalStateService::SetGlobalStateField<int32_t>(m_EditorGlobalState.get(), index,
				type, &widgetValue);
			break;
		}
		case WrappedVarType::Integer64:
		{
			int64_t widgetValue = (int64_t)spec.m_CurrentInteger;
			success = ProjectData::GlobalStateService::SetGlobalStateField<int64_t>(m_EditorGlobalState.get(), index,
				type, &widgetValue);
			break;
		}
		case WrappedVarType::UInteger16:
		{
			uint16_t widgetValue = (uint16_t)spec.m_CurrentInteger;
			success = ProjectData::GlobalStateService::SetGlobalStateField<uint16_t>(m_EditorGlobalState.get(), index,
				type, &widgetValue);
			break;
		}
		case WrappedVarType::UInteger32:
		{
			uint32_t widgetValue = (uint32_t)spec.m_CurrentInteger;
			success = ProjectData::GlobalStateService::SetGlobalStateField<uint32_t>(m_EditorGlobalState.get(), index,
				type, &widgetValue);
			break;
		}
		case WrappedVarType::UInteger64:
		{
			uint64_t widgetValue = (uint64_t)spec.m_CurrentInteger;
			success = ProjectData::GlobalStateService::SetGlobalStateField<uint64_t>(m_EditorGlobalState.get(), index,
				type, &widgetValue);
			break;
		}
		default:
			KG_ERROR("Unhandled wrapped var type provided when modifying global state widgets");
			break;
		}

		if (!success)
		{
			KG_WARN("Failed to modify field");
			return;
		}
	}
	void GlobalStatePanel::OnModifyFieldEditVec2(EditorUI::EditVec2Widget& spec)
	{
		// Get index/type information from widget
		auto [index, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();

		// Modify the field
		bool success = ProjectData::GlobalStateService::SetGlobalStateField<Math::vec2>(m_EditorGlobalState.get(), index,
			type, &spec.m_CurrentVec2);

		if (!success)
		{
			KG_WARN("Failed to modify field");
			return;
		}
	}
	void GlobalStatePanel::OnModifyFieldEditVec3(EditorUI::EditVec3Widget& spec)
	{
		// Get index/type information from widget
		auto [index, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();

		// Modify the field
		bool success = ProjectData::GlobalStateService::SetGlobalStateField<Math::vec3>(m_EditorGlobalState.get(), index,
			type, &spec.m_CurrentVec3);

		if (!success)
		{
			KG_WARN("Failed to modify field");
			return;
		}
	}
	void GlobalStatePanel::OnModifyFieldEditVec4(EditorUI::EditVec4Widget& spec)
	{
		// Get index/type information from widget
		auto [index, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();

		// Modify the field
		bool success = ProjectData::GlobalStateService::SetGlobalStateField<Math::vec4>(m_EditorGlobalState.get(), index,
			type, &spec.m_CurrentVec4);

		if (!success)
		{
			KG_WARN("Failed to modify field");
			return;
		}
	}
	void GlobalStatePanel::OnModifyFieldEditIVec2(EditorUI::EditIVec2Widget& spec)
	{
		// Get index/type information from widget
		auto [index, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();

		// Modify the field
		bool success = ProjectData::GlobalStateService::SetGlobalStateField<Math::ivec2>(m_EditorGlobalState.get(), index,
			type, &spec.m_CurrentIVec2);

		if (!success)
		{
			KG_WARN("Failed to modify field");
			return;
		}
	}
	void GlobalStatePanel::OnModifyFieldEditIVec3(EditorUI::EditIVec3Widget& spec)
	{
		// Get index/type information from widget
		auto [index, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();

		// Modify the field
		bool success = ProjectData::GlobalStateService::SetGlobalStateField<Math::ivec3>(m_EditorGlobalState.get(), index,
			type, &spec.m_CurrentIVec3);

		if (!success)
		{
			KG_WARN("Failed to modify field");
			return;
		}
	}
	void GlobalStatePanel::OnModifyFieldEditIVec4(EditorUI::EditIVec4Widget& spec)
	{
		// Get index/type information from widget
		auto [index, type] = *(std::pair<size_t, WrappedVarType>*)spec.m_ProvidedData.get();

		// Modify the field
		bool success = ProjectData::GlobalStateService::SetGlobalStateField<Math::ivec4>(m_EditorGlobalState.get(), index,
			type, &spec.m_CurrentIVec4);

		if (!success)
		{
			KG_WARN("Failed to modify field");
			return;
		}
	}
}
