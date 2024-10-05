#include "Panels/ProjectComponentPanel.h"

#include "EditorApp.h"
#include "Kargono.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	ProjectComponentPanel::ProjectComponentPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(ProjectComponentPanel::OnKeyPressedEditor));
		InitializeOpeningPanel();
	}
	void ProjectComponentPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowProjectComponent);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (!m_EditorProjectComponent)
		{
			EditorUI::EditorUIService::NewItemScreen("Open Existing Component", KG_BIND_CLASS_FN(OpenComponent), "Create New Component", KG_BIND_CLASS_FN(CreateComponent));
			EditorUI::EditorUIService::GenericPopup(m_CreateComponentPopup);
			EditorUI::EditorUIService::SelectOption(m_OpenComponentPopup);
		}
		else
		{
			/*EditorUI::EditorUIService::PanelHeader(m_TagHeader);
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);
			EditorUI::EditorUIService::GenericPopup(s_DeleteEntityClassWarning);
			EditorUI::EditorUIService::GenericPopup(s_CloseEntityClassWarning);
			EditorUI::EditorUIService::Table(s_FieldsTable);
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

			EditorUI::EditorUIService::CollapsingHeader(s_StaticFunctionHeaderSpec);
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);
			if (s_StaticFunctionHeaderSpec.Expanded)
			{
				EditorUI::EditorUIService::SelectOption(s_SelectOnPhysicsCollisionStartSpec);
				EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

				EditorUI::EditorUIService::SelectOption(s_SelectOnPhysicsCollisionEndSpec);
				EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

				EditorUI::EditorUIService::SelectOption(s_SelectOnCreateSpec);
				EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

				EditorUI::EditorUIService::SelectOption(s_SelectOnUpdateSpec);
				EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);
			}

			EditorUI::EditorUIService::Table(s_AllScriptsTableSpec);
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

			EditorUI::EditorUIService::SelectOption(s_AddFieldPopup);
			EditorUI::EditorUIService::GenericPopup(s_EditFieldPopup);*/
		}


		EditorUI::EditorUIService::EndWindow();
	}
	void ProjectComponentPanel::InitializeOpeningPanel()
	{
		m_OpenComponentPopup.Label = "Open Component";
		m_OpenComponentPopup.Flags |= EditorUI::SelectOption_PopupOnly;
		m_OpenComponentPopup.CurrentOption = { "None", Assets::EmptyHandle };
		m_OpenComponentPopup.PopupAction = [&]()
		{
			m_OpenComponentPopup.GetAllOptions().clear();
			m_OpenComponentPopup.CurrentOption = { "None", Assets::EmptyHandle };

			m_OpenComponentPopup.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetProjectComponentRegistry())
			{
				m_OpenComponentPopup.AddToOptions("All Options", asset.Data.FileLocation.string(), handle);
			}
		};

		m_OpenComponentPopup.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Handle == Assets::EmptyHandle)
			{
				KG_WARN("No  Selected");
				return;
			}
			if (!Assets::AssetService::GetProjectComponentRegistry().contains(selection.Handle))
			{
				KG_WARN("Could not find component in component editor");
				return;
			}

			m_EditorProjectComponent = Assets::AssetService::GetProjectComponent(selection.Handle);
			m_EditorProjectComponentHandle = selection.Handle;
			m_TagHeader.Label = Assets::AssetService::GetProjectComponentRegistry().at(
				selection.Handle).Data.FileLocation.string();
			m_TagHeader.EditColorActive = false;
			RefreshData();
		};

		m_SelectComponentName.Label = "New Name";
		m_SelectComponentName.CurrentOption = "Empty";

		m_CreateComponentPopup.Label = "Create  Component";
		m_CreateComponentPopup.PopupWidth = 420.0f;
		m_CreateComponentPopup.ConfirmAction = [&]()
		{
			if (m_SelectComponentName.CurrentOption == "")
			{
				return;
			}

			for (auto& [id, asset] : Assets::AssetService::GetProjectComponentRegistry())
			{
				if (asset.Data.GetSpecificMetaData<Assets::ProjectComponentMetaData>()->Name == m_SelectComponentName.CurrentOption)
				{
					return;
				}
			}
			m_EditorProjectComponentHandle = Assets::AssetService::CreateProjectComponent(m_SelectComponentName.CurrentOption);
			m_EditorProjectComponent = Assets::AssetService::GetProjectComponent(m_EditorProjectComponentHandle);
			m_TagHeader.EditColorActive = false;
			m_TagHeader.Label = Assets::AssetService::GetProjectComponentRegistry().at(
				m_EditorProjectComponentHandle).Data.FileLocation.string();
			RefreshData();
		};
		m_CreateComponentPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_SelectComponentName);
		};
	}
	bool ProjectComponentPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	void ProjectComponentPanel::OpenComponent()
	{
		m_OpenComponentPopup.PopupActive = true;
	}
	void ProjectComponentPanel::CreateComponent()
	{
		m_CreateComponentPopup.PopupActive = true;
	}
	void ProjectComponentPanel::RefreshData()
	{
	}
}
			
