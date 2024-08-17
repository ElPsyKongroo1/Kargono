#include "Panels/UIEditorPanel.h"

#include "EditorApp.h"

#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{

	// Opening Panel w/ Popups
	static EditorUI::SelectOptionSpec s_OpenUIPopupSpec {};
	static EditorUI::GenericPopupSpec s_CreateUIPopupSpec {};
	static EditorUI::EditTextSpec s_SelectUINameSpec {};

	// User Interface Header
	static EditorUI::PanelHeaderSpec s_MainHeader {};
	static EditorUI::GenericPopupSpec s_DeleteUIWarning {};
	static EditorUI::GenericPopupSpec s_CloseUIWarning {};

	// Reusable Functions
	static void OnOpenUI()
	{
		s_OpenUIPopupSpec.PopupActive = true;
	}
	static void OnCreateUI()
	{
		s_CreateUIPopupSpec.PopupActive = true;
	}

	UIEditorPanel::UIEditorPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(UIEditorPanel::OnKeyPressedEditor));
		InitializeOpeningScreen();
		InitializeUIHeader();
	}

	void UIEditorPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();

		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowUserInterfaceEditor);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (!m_EditorUI)
		{
			// Opening/Null State Screen
			EditorUI::EditorUIService::NewItemScreen("Open Existing User Interface", OnOpenUI, "Create New User Interface", OnCreateUI);
			EditorUI::EditorUIService::GenericPopup(s_CreateUIPopupSpec);
			EditorUI::EditorUIService::SelectOption(s_OpenUIPopupSpec);
		}
		else
		{
			// Header
			EditorUI::EditorUIService::PanelHeader(s_MainHeader);
			EditorUI::EditorUIService::GenericPopup(s_DeleteUIWarning);
			EditorUI::EditorUIService::GenericPopup(s_CloseUIWarning);
		}

		

		EditorUI::EditorUIService::EndWindow();
	}
	bool UIEditorPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}

	void UIEditorPanel::InitializeOpeningScreen()
	{
		s_OpenUIPopupSpec.Label = "Open User Interface";
		s_OpenUIPopupSpec.LineCount = 2;
		s_OpenUIPopupSpec.CurrentOption = { "None", Assets::EmptyHandle };
		s_OpenUIPopupSpec.Flags |= EditorUI::SelectOption_PopupOnly;
		s_OpenUIPopupSpec.PopupAction = [&]()
		{
			s_OpenUIPopupSpec.GetAllOptions().clear();
			s_OpenUIPopupSpec.CurrentOption = { "None", Assets::EmptyHandle };

			s_OpenUIPopupSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetManager::GetUserInterfaceRegistry())
			{
				s_OpenUIPopupSpec.AddToOptions("All Options", asset.Data.IntermediateLocation.string(), handle);
			}
		};

		s_OpenUIPopupSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Handle == Assets::EmptyHandle)
			{
				KG_WARN("No User Interface Selected");
				return;
			}
			if (!Assets::AssetManager::GetUserInterfaceRegistry().contains(selection.Handle))
			{
				KG_WARN("Could not find the user interface specified");
				return;
			}

			m_EditorUI = Assets::AssetManager::GetUserInterface(selection.Handle);
			m_EditorUIHandle = selection.Handle;
			s_MainHeader.EditColorActive = false;
			s_MainHeader.Label = Assets::AssetManager::GetUserInterfaceRegistry().at(
				m_EditorUIHandle).Data.IntermediateLocation.string();
			// TODO: Refresh data
			//OnRefreshData();
		};

		s_SelectUINameSpec.Label = "New Name";
		s_SelectUINameSpec.CurrentOption = "Empty";

		s_CreateUIPopupSpec.Label = "Create User Interface";
		s_CreateUIPopupSpec.PopupWidth = 420.0f;
		s_CreateUIPopupSpec.ConfirmAction = [&]()
		{
			if (s_SelectUINameSpec.CurrentOption == "")
			{
				return;
			}

			m_EditorUIHandle = Assets::AssetManager::CreateNewUserInterface(s_SelectUINameSpec.CurrentOption);
			if (m_EditorUIHandle == Assets::EmptyHandle)
			{
				KG_WARN("User Interface was not created");
				return;
			}
			m_EditorUI = Assets::AssetManager::GetUserInterface(m_EditorUIHandle);
			s_MainHeader.EditColorActive = false;
			s_MainHeader.Label = Assets::AssetManager::GetUserInterfaceRegistry().at(
				m_EditorUIHandle).Data.IntermediateLocation.string();
			// TODO: Refresh data
			//OnRefreshData();
		};
		s_CreateUIPopupSpec.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(s_SelectUINameSpec);
		};
	}
	void UIEditorPanel::InitializeUIHeader()
	{
		// Header (Game State Name and Options)
		s_DeleteUIWarning.Label = "Delete User Interface";
		s_DeleteUIWarning.ConfirmAction = [&]()
		{
			// TODO: Remove UI from asset manager
			Assets::AssetManager::DeleteUserInterface(m_EditorUIHandle);
			m_EditorUIHandle = 0;
			m_EditorUI = nullptr;
		};
		s_DeleteUIWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to delete this user interface object?");
		};

		s_CloseUIWarning.Label = "Close User Interface";
		s_CloseUIWarning.ConfirmAction = [&]()
		{
			m_EditorUIHandle = 0;
			m_EditorUI = nullptr;
		};
		s_CloseUIWarning.PopupContents = [&]()
		{
			EditorUI::EditorUIService::Text("Are you sure you want to close this user interface object without saving?");
		};

		s_MainHeader.AddToSelectionList("Save", [&]()
		{
			Assets::AssetManager::SaveUserInterface(m_EditorUIHandle, m_EditorUI);
			s_MainHeader.EditColorActive = false;
		});
		s_MainHeader.AddToSelectionList("Close", [&]()
		{
			if (s_MainHeader.EditColorActive)
			{
				s_CloseUIWarning.PopupActive = true;
			}
			else
			{
				m_EditorUIHandle = 0;
				m_EditorUI = nullptr;
			}
		});
		s_MainHeader.AddToSelectionList("Delete", [&]()
		{
			s_DeleteUIWarning.PopupActive = true;
		});
	}
}
