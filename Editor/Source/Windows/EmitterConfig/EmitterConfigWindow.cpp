#include "Windows/EmitterConfig/EmitterConfigWindow.h"

#include "EditorApp.h"

#include "Kargono/Utility/OSCommands.h"


static Kargono::EditorApp* s_EditorApp { nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Windows
{
	void EmitterConfigWindow::OpenCreateDialog(std::filesystem::path& createLocation)
	{
		// Check if panel is already occupied by an asset
		if (!m_EditorEmitterConfig)
		{
			// Open dialog to create editor emitter config
			OnCreateEmitterConfigDialog();
			m_SelectEmitterConfigLocationSpec.m_CurrentOption = createLocation;
		}
		else
		{
			// Add warning to close active emitter config before creating a new emitter config
			s_MainWindow->OpenWarningMessage("An emitter config is already active inside the editor. Please close the current emitter config before creating a new one.");
		}
	}
	void EmitterConfigWindow::ResetWindowResources()
	{
		// Reset editor emitter config
		m_EditorEmitterConfigHandle = 0;
		m_EditorEmitterConfig = nullptr;

		Particles::ParticleService::ClearEmitters();

		// Reset properties panel data
		m_PropertiesPanel->ClearPanelData();
	}

	void EmitterConfigWindow::OnRefreshData()
	{
	}

	void EmitterConfigWindow::OnCreateEmitterConfigDialog()
	{
		// Set default values for new emitter config creation location
		KG_ASSERT(Projects::ProjectService::GetActive());
		m_SelectEmitterConfigLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();

		// Set dialog popup to open on next frame
		m_CreateEmitterConfigPopupSpec.m_OpenPopup = true;
	}

	void EmitterConfigWindow::OnOpenEmitterConfigDialog()
	{
		// Set dialog popup to open on next frame
		m_OpenEmitterConfigPopupSpec.m_OpenPopup = true;
	}

	void EmitterConfigWindow::OnOpenEmitterConfig(Assets::AssetHandle newHandle)
	{
		// Set new in editor user interface

		// Get emitter an ensure it is valid
		Particles::EmitterConfig* openedEmitter = Assets::AssetService::GetEmitterConfig(newHandle).get();
		KG_ASSERT(openedEmitter);

		// Create editor only variant of emitter
		m_EditorEmitterConfig = CreateRef<Particles::EmitterConfig>(*openedEmitter);
		m_EditorEmitterConfigHandle = newHandle;

		// Set default values for header
		m_MainHeader.m_EditColorActive = false;
		m_MainHeader.m_Label = Assets::AssetService::GetEmitterConfigRegistry().at(
			m_EditorEmitterConfigHandle).Data.FileLocation.filename().string();

		// Handle opening the EmitterConfig in viewport
		m_ViewportPanel->OnOpenEmitterConfig();

		// Refresh widget data in editor to use new user interface
		OnRefreshData();

		// Load emitter
		LoadEditorEmitterIntoParticleService();
		
		
	}

	void EmitterConfigWindow::LoadEditorEmitterIntoParticleService()
	{
		if (!m_EditorEmitterConfig)
		{
			// Just clear the emitters if switching
			Particles::ParticleService::ClearEmitters();
		}
		else
		{
			// Open single emitter in editor
			Particles::ParticleService::ClearEmitters();
			Particles::ParticleService::AddEmitter(m_EditorEmitterConfig.get(), { 0.0f, 0.0f, 0.0f });
		}
		
	}

	EmitterConfigWindow::EmitterConfigWindow()
	{
		// Set up static editor app reference and register panel to editor app
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
	}

	void EmitterConfigWindow::InitializeOpeningScreen()
	{
		// Initialize open existing Emitter Config popup data
		m_OpenEmitterConfigPopupSpec.m_Label = "Open Emitter Config";
		m_OpenEmitterConfigPopupSpec.m_LineCount = 2;
		m_OpenEmitterConfigPopupSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_OpenEmitterConfigPopupSpec.m_Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenEmitterConfigPopupSpec.m_PopupAction = [&](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();
			spec.m_CurrentOption = { "None", Assets::EmptyHandle };

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetEmitterConfigRegistry())
			{
				spec.AddToOptions("All Options", asset.Data.FileLocation.filename().string(), handle);
			}
		};
		m_OpenEmitterConfigPopupSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.m_Handle == Assets::EmptyHandle)
			{
				KG_WARN("No Emitter Config Selected");
				return;
			}
			if (!Assets::AssetService::GetEmitterConfigRegistry().contains(selection.m_Handle))
			{
				KG_WARN("Could not find the Emitter Config specified");
				return;
			}

			OnOpenEmitterConfig(selection.m_Handle);
		};

		// Initialize create new Emitter Config popup data
		m_CreateEmitterConfigPopupSpec.m_Label = "Create Emitter Config";
		m_CreateEmitterConfigPopupSpec.m_ConfirmAction = [&]()
		{
			if (m_SelectEmitterConfigNameSpec.m_CurrentOption == "")
			{
				return;
			}

			m_EditorEmitterConfigHandle = Assets::AssetService::CreateEmitterConfig(m_SelectEmitterConfigNameSpec.m_CurrentOption.c_str(), m_SelectEmitterConfigLocationSpec.m_CurrentOption);
			if (m_EditorEmitterConfigHandle == Assets::EmptyHandle)
			{
				KG_WARN("Emitter Config was not created");
				return;
			}

			// Get emitter an ensure it is valid
			Particles::EmitterConfig* openedEmitter = Assets::AssetService::GetEmitterConfig(m_EditorEmitterConfigHandle).get();
			KG_ASSERT(openedEmitter);

			// Create editor only variant of emitterConfig
			m_EditorEmitterConfig = CreateRef<Particles::EmitterConfig>(*openedEmitter);

			// Manage changing main header to new emitter
			m_MainHeader.m_EditColorActive = false;
			m_MainHeader.m_Label = Assets::AssetService::GetEmitterConfigRegistry().at(
				m_EditorEmitterConfigHandle).Data.FileLocation.filename().string();
			OnRefreshData();

			// Load emitter
			LoadEditorEmitterIntoParticleService();
		};
		m_CreateEmitterConfigPopupSpec.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_SelectEmitterConfigNameSpec);
			EditorUI::EditorUIService::ChooseDirectory(m_SelectEmitterConfigLocationSpec);
		};

		// Initialize widget for selecting Emitter Config name
		m_SelectEmitterConfigNameSpec.m_Label = "New Name";
		m_SelectEmitterConfigNameSpec.m_CurrentOption = "Empty";

		// Initialize widget for selecting Emitter Config location
		m_SelectEmitterConfigLocationSpec.m_Label = "Location";
		m_SelectEmitterConfigLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
		m_SelectEmitterConfigLocationSpec.m_ConfirmAction = [&](const std::string& path)
		{
			if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveAssetDirectory(), path))
			{
				KG_WARN("Cannot create an asset outside of the project's asset directory.");
				m_SelectEmitterConfigLocationSpec.m_CurrentOption = Projects::ProjectService::GetActiveAssetDirectory();
			}
		};
	}

	void EmitterConfigWindow::InitializeEmitterConfigHeader()
	{
		// Header (Game State Name and Options)
		m_DeleteEmitterConfigWarning.m_Label = "Delete Emitter Config";
		m_DeleteEmitterConfigWarning.m_ConfirmAction = [&]()
		{
			// TODO: Remove EmitterConfig from asset manager
			Assets::AssetService::DeleteEmitterConfig(m_EditorEmitterConfigHandle);
			ResetWindowResources();
		};
		m_DeleteEmitterConfigWarning.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this Emitter Config object?");
		};

		// Intialize widget data for closing the Emitter Config warning popup
		m_CloseEmitterConfigWarning.m_Label = "Close Emitter Config";
		m_CloseEmitterConfigWarning.m_ConfirmAction = [&]()
		{
			ResetWindowResources();
		};
		m_CloseEmitterConfigWarning.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to close this Emitter Config object without saving?");
		};

		// Set up main header for Emitter Config editor panel
		m_MainHeader.AddToSelectionList("Save", [&]()
		{
			// Modify the asset memory
			Ref<Particles::EmitterConfig> cachedRef = Assets::AssetService::GetEmitterConfig(m_EditorEmitterConfigHandle);
			KG_ASSERT(cachedRef);
			*cachedRef = *m_EditorEmitterConfig; // KINDA DANGEROUS

			// Save emitter asset in registry
			Assets::AssetService::SaveEmitterConfig(m_EditorEmitterConfigHandle, cachedRef);

			// Update editted status
			m_MainHeader.m_EditColorActive = false;
		});
		m_MainHeader.AddToSelectionList("Close", [&]()
		{
			if (m_MainHeader.m_EditColorActive)
			{
				m_CloseEmitterConfigWarning.m_OpenPopup = true;
			}
			else
			{
				ResetWindowResources();
			}
		});
		m_MainHeader.AddToSelectionList("Delete", [&]()
		{
			m_DeleteEmitterConfigWarning.m_OpenPopup = true;
		});
	}

	void EmitterConfigWindow::InitPanels()
	{
		// Initialize main window widgets
		InitializeOpeningScreen();
		InitializeEmitterConfigHeader();

		// Initialize panels
		m_ViewportPanel = CreateScope<Panels::EmitterConfigViewportPanel>();
		m_PropertiesPanel = CreateScope<Panels::EmitterConfigPropertiesPanel>();
	}

	bool EmitterConfigWindow::OnInputEvent(Events::Event* event)
	{
		if (m_EditorEmitterConfig)
		{
			m_ViewportPanel->OnInputEvent(event);
		}

		// Handle key press events for the emitter config editor panel
		if (event->GetEventType() == Events::EventType::KeyPressed)
		{
			return OnKeyPressedEditor(*(Events::KeyPressedEvent*)event);
		}

		// Default to not handling the event
		return false;
	}

	void EmitterConfigWindow::OnUpdate(Timestep ts)
	{
		// Update the viewport panel
		if (m_EditorEmitterConfig)
		{
			m_ViewportPanel->OnUpdate(ts);
		}
	}

	void EmitterConfigWindow::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();

		EditorUI::EditorUIService::StartRendering();
		EditorUI::EditorUIService::StartDockspaceWindow();

		// Render the UI Editor's menu bar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::MenuItem("Back to Main Window"))
			{
				EngineService::SubmitToMainThread([]()
				{
					s_EditorApp->SetActiveEditorWindow(ActiveEditorUIWindow::MainWindow);
					s_EditorApp->m_MainWindow->LoadSceneParticleEmitters();
				});
			}

			if (ImGui::BeginMenu("Panels"))
			{
				ImGui::MenuItem("Viewport", NULL, &m_ShowViewport);
				ImGui::MenuItem("Properties", NULL, &m_ShowProperties);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("Engine Docs"))
				{
					Utility::OSCommands::OpenWebURL("https://elpsykongroo1.github.io/Kargono/");
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}


		// Begin rendering the Emitter Config editor panel
		if (!m_EditorEmitterConfig)
		{
			EditorUI::EditorUIService::StartWindow(m_PanelName);

			// Display opening screen for Emitter Config editor
			EditorUI::EditorUIService::NewItemScreen("Open Existing Emitter Config", KG_BIND_CLASS_FN(OnOpenEmitterConfigDialog), "Create New Emitter Config", KG_BIND_CLASS_FN(OnCreateEmitterConfigDialog));
			EditorUI::EditorUIService::GenericPopup(m_CreateEmitterConfigPopupSpec);
			EditorUI::EditorUIService::SelectOption(m_OpenEmitterConfigPopupSpec);

			// End main window
			EditorUI::EditorUIService::EndWindow();
		}
		else
		{
			if (m_ShowViewport)
			{
				m_ViewportPanel->OnEditorUIRender();
			}

			if (m_ShowProperties)
			{
				m_PropertiesPanel->OnEditorUIRender();
			}
		}

		// Display Emitter Config editor panel main content
		EditorUI::EditorUIService::GenericPopup(m_DeleteEmitterConfigWarning);
		EditorUI::EditorUIService::GenericPopup(m_CloseEmitterConfigWarning);
		EditorUI::EditorUIService::Tooltip(m_SelectScriptTooltip);

		// Clean up dockspace window
		EditorUI::EditorUIService::EndDockspaceWindow();

		// Add highlighting around the focused window
		EditorUI::EditorUIService::HighlightFocusedWindow();

		// End Editor UI Rendering
		EditorUI::EditorUIService::EndRendering();
	}
	bool EmitterConfigWindow::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		bool handled{ false };
		FixedString32 focusedWindow = EditorUI::EditorUIService::GetFocusedWindowName();
		if (focusedWindow == m_ViewportPanel->m_PanelName)
		{
			handled = m_ViewportPanel->OnKeyPressedEditor(event);
		}

		// Return if the event was handled
		if (handled)
		{
			return false;
		}

		// Handle varios key presses for the emitter config editor panel
		switch (event.GetKeyCode())
		{
		// Clear selected entry if escape key is pressed
		case Key::Escape:
			m_PropertiesPanel->ClearPanelData();
			return true;
		default:
			return false;
		
		}
	}

	bool EmitterConfigWindow::OnAssetEvent(Events::Event* event)
	{
		Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;

		// Handle emitter config deletion
		if (manageAsset->GetAssetType() == Assets::AssetType::EmitterConfig &&
			manageAsset->GetAction() == Events::ManageAssetAction::PreDelete)
		{
			// Check if editor needs modification
			if (manageAsset->GetAssetID() != m_EditorEmitterConfigHandle)
			{
				return false;
			}

			// Handle deletion of asset
			ResetWindowResources();
			return true;
		}

		// Handle emitter config name change
		if (manageAsset->GetAssetType() == Assets::AssetType::EmitterConfig &&
			manageAsset->GetAction() == Events::ManageAssetAction::UpdateAssetInfo)
		{
			// Check if editor needs modification
			if (manageAsset->GetAssetID() != m_EditorEmitterConfigHandle)
			{
				return false;
			}

			// Update header name with new asset name
			m_MainHeader.m_Label = Assets::AssetService::GetEmitterConfigFileLocation(manageAsset->GetAssetID()).filename().string();
			return true;
		}

		return false;
	}

	void EmitterConfigWindow::OpenAssetInEditor(std::filesystem::path& assetLocation)
	{
		// Ensure provided path is within the active asset directory
		std::filesystem::path activeAssetDirectory = Projects::ProjectService::GetActiveAssetDirectory();
		if (!Utility::FileSystem::DoesPathContainSubPath(activeAssetDirectory, assetLocation))
		{
			KG_WARN("Could not open asset in editor. Provided path does not exist within active asset directory");
			return;
		}

		// Look for asset in registry using the file location
		std::filesystem::path relativePath{ Utility::FileSystem::GetRelativePath(activeAssetDirectory, assetLocation) };
		Assets::AssetHandle assetHandle = Assets::AssetService::GetEmitterConfigHandleFromFileLocation(relativePath);

		// Validate resulting handle
		if (!assetHandle)
		{
			KG_WARN("Could not open asset in editor. Provided path does not result in an asset inside the registry.");
			return;
		}

		// Early out if asset is already open
		if (m_EditorEmitterConfigHandle == assetHandle)
		{
			// Open the emitter config window in the editor
			EngineService::SubmitToMainThread([]()
			{
				s_EditorApp->SetActiveEditorWindow(ActiveEditorUIWindow::EmitterConfigWindow);
				s_EditorApp->m_EmitterConfigEditorWindow->LoadEditorEmitterIntoParticleService();
			});
			return;
		}

		// Check if panel is already occupied by an asset
		if (!m_EditorEmitterConfig)
		{
			// Open emitter config in editor
			OnOpenEmitterConfig(assetHandle);

			// Open the emitter config window in the editor
			EngineService::SubmitToMainThread([]()
			{
				s_EditorApp->SetActiveEditorWindow(ActiveEditorUIWindow::EmitterConfigWindow);
			});
		}
		else
		{
			// Add warning to close active AI state before opening a new AIState
			s_MainWindow->OpenWarningMessage("An emitter config is already active inside the editor. Please close the current emitter config before opening a new one.");
		}
	}
}
