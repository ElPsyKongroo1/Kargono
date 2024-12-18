#include "Windows/UserInterface/UIEditorWindow.h"

#include "EditorApp.h"

#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Windows
{
	void UIEditorWindow::OpenCreateDialog(std::filesystem::path& createLocation)
	{
		// TODO: Open UIEditorWindow
		EditorUI::EditorUIService::BringWindowToFront(m_TreePanel->m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_TreePanel->m_PanelName);

		// Check if panel is already occupied by an asset
		if (!m_EditorUI)
		{
			// Open dialog to create editor user interface
			m_TreePanel->OnCreateUIDialog();
			m_TreePanel->m_SelectUILocationSpec.m_CurrentOption = createLocation;
		}
		else
		{
			// Add warning to close active user interface before creating a new user interface
			s_MainWindow->OpenWarningMessage("A user interface is already active inside the editor. Please close the current user interface before creating a new one.");
		}
	}
	void UIEditorWindow::ResetWindowResources()
	{
		// Reset editor user interface
		m_EditorUIHandle = 0;
		m_EditorUI = nullptr;

		// Reset properties panel data
		m_PropertiesPanel->ClearPanelData();
	}

	void UIEditorWindow::OnRefreshData()
	{
		// Revalidate data with current user interface
		m_TreePanel->OnRefreshData();
	}

	UIEditorWindow::UIEditorWindow()
	{
		// Set up static editor app reference and register panel to editor app
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
	}

	void UIEditorWindow::InitPanels()
	{
		// Initialize panels
		m_ViewportPanel = CreateScope<Panels::UIEditorViewportPanel>();
		m_PropertiesPanel = CreateScope<Panels::UIEditorPropertiesPanel>();
		m_TreePanel = CreateScope<Panels::UIEditorTreePanel>();
	}

	bool UIEditorWindow::OnInputEvent(Events::Event* event)
	{
		if (m_EditorUI)
		{
			m_ViewportPanel->OnInputEvent(event);
		}

		// Handle key press events for the user interface editor panel
		if (event->GetEventType() == Events::EventType::KeyPressed)
		{
			return OnKeyPressedEditor(*(Events::KeyPressedEvent*)event);
		}

		// Default to not handling the event
		return false;
	}

	void UIEditorWindow::OnUpdate(Timestep ts)
	{
		// Update the viewport panel
		if (m_EditorUI)
		{
			m_ViewportPanel->OnUpdate(ts);
		}
	}

	void UIEditorWindow::OnEditorUIRender()
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
				});
			}

			if (ImGui::BeginMenu("Panels"))
			{
				ImGui::MenuItem("Tree", NULL, &m_ShowTree);
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

		// Render the viewport and properties panels
		if (m_EditorUI)
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

		// Render the table panel
		if (m_ShowTree)
		{
			m_TreePanel->OnEditorUIRender();
		}

		// Clean up dockspace window
		EditorUI::EditorUIService::EndDockspaceWindow();

		// Add highlighting around the focused window
		EditorUI::EditorUIService::HighlightFocusedWindow();

		// End Editor UI Rendering
		EditorUI::EditorUIService::EndRendering();
	}
	bool UIEditorWindow::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		// Handle varios key presses for the user interface editor panel
		switch (event.GetKeyCode())
		{
		// Clear selected entry if escape key is pressed
		case Key::Escape:
			m_TreePanel->m_UITree.m_SelectedEntry = {};
			m_PropertiesPanel->ClearPanelData();
			return true;
		default:
			return false;
		
		}
	}

	bool UIEditorWindow::OnAssetEvent(Events::Event* event)
	{
		Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;

		// Clear script references from user interface widgets
		if (manageAsset->GetAssetType() == Assets::AssetType::Script &&
			manageAsset->GetAction() == Events::ManageAssetAction::Delete)
		{
			if (m_PropertiesPanel->m_WidgetOnPress.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_PropertiesPanel->m_WidgetOnPress.m_CurrentOption = { "None", Assets::EmptyHandle };
			}

			if (m_EditorUI)
			{
				Assets::AssetService::RemoveScriptFromUserInterface(m_EditorUI, manageAsset->GetAssetID());
			}
		}

		// Handle user interface deletion
		if (manageAsset->GetAssetType() == Assets::AssetType::UserInterface &&
			manageAsset->GetAction() == Events::ManageAssetAction::Delete)
		{
			// Check if editor needs modification
			if (manageAsset->GetAssetID() != m_EditorUIHandle)
			{
				return false;
			}

			// Handle deletion of asset
			ResetWindowResources();
			return true;
		}

		// Handle user interface name change
		if (manageAsset->GetAssetType() == Assets::AssetType::UserInterface &&
			manageAsset->GetAction() == Events::ManageAssetAction::UpdateAssetInfo)
		{
			// Check if editor needs modification
			if (manageAsset->GetAssetID() != m_EditorUIHandle)
			{
				return false;
			}

			// Update header name with new asset name
			m_TreePanel->m_MainHeader.m_Label = Assets::AssetService::GetUserInterfaceFileLocation(manageAsset->GetAssetID()).filename().string();
			return true;
		}

		return false;
	}

	void UIEditorWindow::OpenAssetInEditor(std::filesystem::path& assetLocation)
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
		Assets::AssetHandle assetHandle = Assets::AssetService::GetUserInterfaceHandleFromFileLocation(relativePath);

		// Validate resulting handle
		if (!assetHandle)
		{
			KG_WARN("Could not open asset in editor. Provided path does not result in an asset inside the registry.");
			return;
		}

		// TODO: Ensure the UIEditorWindow is open
		EditorUI::EditorUIService::BringWindowToFront(m_TreePanel->m_PanelName);
		EditorUI::EditorUIService::SetFocusedWindow(m_TreePanel->m_PanelName);

		// Early out if asset is already open
		if (m_EditorUIHandle == assetHandle)
		{
			return;
		}

		// Check if panel is already occupied by an asset
		if (!m_EditorUI)
		{
			m_TreePanel->OnOpenUI(assetHandle);
		}
		else
		{
			// Add warning to close active AI state before opening a new AIState
			s_MainWindow->OpenWarningMessage("An user interface is already active inside the editor. Please close the current user interface before opening a new one.");
		}
	}
}
