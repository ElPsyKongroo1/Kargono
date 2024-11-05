#include "Panels/ProjectPanel.h"

#include "Kargono.h"
#include "EditorApp.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	void ProjectPanel::InitializeStaticResources()
	{
		// Resolution Specification
		m_SelectStartSceneSpec.Label = "Starting Scene";
		//m_SelectStartSceneSpec.WidgetID = 0x75ebbc8750034f81;
		m_SelectStartSceneSpec.LineCount = 2;
		
		m_SelectStartSceneSpec.CurrentOption = {
			 Assets::AssetService::GetSceneRegistry().at(Projects::ProjectService::GetActiveStartSceneHandle()).Data.FileLocation.string(),
			Projects::ProjectService::GetActiveStartSceneHandle()};
		m_SelectStartSceneSpec.PopupAction = [&]()
		{
			m_SelectStartSceneSpec.GetAllOptions().clear();
			for (auto& [handle, asset] : Assets::AssetService::GetSceneRegistry())
			{
				m_SelectStartSceneSpec.AddToOptions("All Options", asset.Data.FileLocation.string(), handle);
			}
			m_SelectStartSceneSpec.CurrentOption = {
				Assets::AssetService::GetSceneRegistry().at(Projects::ProjectService::GetActiveStartSceneHandle()).Data.FileLocation.string(),
			Projects::ProjectService::GetActiveStartSceneHandle()};
		};
		m_SelectStartSceneSpec.ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{
			if (!Assets::AssetService::GetSceneRegistry().contains(entry.Handle))
			{
				KG_WARN("Could not find scene using asset handle in Project Panel");
				return;
			}

			const Assets::Asset asset = Assets::AssetService::GetSceneRegistry().at(entry.Handle);
			Projects::ProjectService::SetActiveStartingScene(entry.Handle);
		};

		// Default Full Screen
		m_DefaultFullscreenSpec.Label = "Default Fullscreen";
		m_DefaultFullscreenSpec.ConfirmAction = [](EditorUI::CheckboxSpec& spec)
		{
			Projects::ProjectService::SetActiveIsFullscreen(spec.CurrentBoolean);
		};

		// Set Networking Specification
		m_ToggleNetworkSpec.Label = "Networking";
		m_ToggleNetworkSpec.ConfirmAction = [](EditorUI::CheckboxSpec& spec)
		{
			Projects::ProjectService::SetActiveAppIsNetworked(spec.CurrentBoolean);
		};

		// Resolution Specification
		m_SelectResolutionSpec.Label = "Target Resolution";
		m_SelectResolutionSpec.LineCount = 4;
		m_SelectResolutionSpec.CurrentOption = {
			Utility::ScreenResolutionToString(Projects::ProjectService::GetActiveTargetResolution()),
			Assets::EmptyHandle};
		m_SelectResolutionSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			Projects::ProjectService::SetActiveTargetResolution(Utility::StringToScreenResolution(selection.Label));
		};
		m_SelectResolutionSpec.PopupAction = [&]()
		{
			m_SelectResolutionSpec.CurrentOption = {
				Utility::ScreenResolutionToString(Projects::ProjectService::GetActiveTargetResolution()),
			Assets::EmptyHandle};
		};
		m_SelectResolutionSpec.AddToOptions("Aspect Ratio: 1:1 (Box)", "800x800", Assets::EmptyHandle);
		m_SelectResolutionSpec.AddToOptions("Aspect Ratio: 1:1 (Box)", "400x400", Assets::EmptyHandle);
		m_SelectResolutionSpec.AddToOptions("Aspect Ratio: 16:9 (Widescreen)", "1920x1080", Assets::EmptyHandle);
		m_SelectResolutionSpec.AddToOptions("Aspect Ratio: 16:9 (Widescreen)", "1600x900", Assets::EmptyHandle);
		m_SelectResolutionSpec.AddToOptions("Aspect Ratio: 16:9 (Widescreen)", "1366x768", Assets::EmptyHandle);
		m_SelectResolutionSpec.AddToOptions("Aspect Ratio: 16:9 (Widescreen)", "1280x720", Assets::EmptyHandle);
		m_SelectResolutionSpec.AddToOptions("Aspect Ratio: 4:3 (Fullscreen)", "1600x1200", Assets::EmptyHandle);
		m_SelectResolutionSpec.AddToOptions("Aspect Ratio: 4:3 (Fullscreen)", "1280x960", Assets::EmptyHandle);
		m_SelectResolutionSpec.AddToOptions("Aspect Ratio: 4:3 (Fullscreen)", "1152x864", Assets::EmptyHandle);
		m_SelectResolutionSpec.AddToOptions("Aspect Ratio: 4:3 (Fullscreen)", "1024x768", Assets::EmptyHandle);
		m_SelectResolutionSpec.AddToOptions("Aspect Ratio: Automatic (Based on Device Used)", "Match Device", Assets::EmptyHandle);

		// Select Start Game State
		m_SelectStartGameStateSpec.Label = "Start Game State";
		m_SelectStartGameStateSpec.LineCount = 3;
		if (Projects::ProjectService::GetActiveStartGameState() != 0)
		{
			m_SelectStartGameStateSpec.CurrentOption = { Assets::AssetService::GetGameStateRegistry().at
			(Projects::ProjectService::GetActiveStartGameState()).Data.FileLocation.string(),
			Projects::ProjectService::GetActiveStartGameState()};
		}
		else
		{
			m_SelectStartGameStateSpec.CurrentOption = { "None", Assets::EmptyHandle };
		}
		m_SelectStartGameStateSpec.PopupAction = [&]()
		{
			m_SelectStartGameStateSpec.ClearOptions();
			m_SelectStartGameStateSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetGameStateRegistry())
			{
				m_SelectStartGameStateSpec.AddToOptions("All Options", asset.Data.FileLocation.string(), handle);
			}

			if (Projects::ProjectService::GetActiveStartGameState() != Assets::EmptyHandle)
			{
				m_SelectStartGameStateSpec.CurrentOption = { Assets::AssetService::GetGameStateRegistry().at
				(Projects::ProjectService::GetActiveStartGameState()).Data.FileLocation.string(),
				Projects::ProjectService::GetActiveStartGameState() };
			}
			else
			{
				m_SelectStartGameStateSpec.CurrentOption = { "None", Assets::EmptyHandle };
			}

		};

		m_SelectStartGameStateSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetGameState(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not locate starting game state in ProjectPanel");
				return;
			}
			Projects::ProjectService::SetActiveStartGameState(selection.Handle);

		};

		// Runtime Start Spec
		m_SelectRuntimeStartSpec.Label = "Runtime Start";
		m_SelectRuntimeStartSpec.LineCount = 3;
		m_SelectRuntimeStartSpec.CurrentOption = { Projects::ProjectService::GetActiveOnRuntimeStart() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnRuntimeStart())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnRuntimeStart()};
		m_SelectRuntimeStartSpec.PopupAction = [&]()
		{
			m_SelectRuntimeStartSpec.GetAllOptions().clear();

			m_SelectRuntimeStartSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				m_SelectRuntimeStartSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			m_SelectRuntimeStartSpec.CurrentOption = { Projects::ProjectService::GetActiveOnRuntimeStart() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnRuntimeStart())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnRuntimeStart() };
		};
		m_SelectRuntimeStartSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find runtime start function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnRuntimeStart(selection.Handle);
		};

		// Update User Count Spec
		m_SelectUpdateUserCountSpec.Label = "Update User Count";
		m_SelectUpdateUserCountSpec.LineCount = 3;
		m_SelectUpdateUserCountSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUpdateUserCount() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUpdateUserCount())->m_ScriptName : "None",
		Projects::ProjectService::GetActiveOnUpdateUserCount()};
		m_SelectUpdateUserCountSpec.PopupAction = [&]()
		{
			m_SelectUpdateUserCountSpec.GetAllOptions().clear();

			m_SelectUpdateUserCountSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_FuncType != WrappedFuncType::Void_UInt32)
				{
					continue;
				}
				m_SelectUpdateUserCountSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			m_SelectUpdateUserCountSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUpdateUserCount() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUpdateUserCount())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnUpdateUserCount() };
		};
		m_SelectUpdateUserCountSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find on update user count function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnUpdateUserCount(selection.Handle);
		};

		// Update Approve Join Session
		m_SelectApproveJoinSessionSpec.Label = "Approve Join Session";
		m_SelectApproveJoinSessionSpec.LineCount = 3;
		m_SelectApproveJoinSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnApproveJoinSession() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnApproveJoinSession())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnApproveJoinSession() };
		m_SelectApproveJoinSessionSpec.PopupAction = [&]()
		{
			m_SelectApproveJoinSessionSpec.GetAllOptions().clear();

			m_SelectApproveJoinSessionSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_FuncType != WrappedFuncType::Void_UInt16)
				{
					continue;
				}
				m_SelectApproveJoinSessionSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			m_SelectApproveJoinSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnApproveJoinSession() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnApproveJoinSession())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnApproveJoinSession() };
		};
		m_SelectApproveJoinSessionSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find on approve join session function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnApproveJoinSession(selection.Handle);
		};

		// Update User Left Session
		m_SelectUserLeftSessionSpec.Label = "User Left Session";
		m_SelectUserLeftSessionSpec.LineCount = 3;
		m_SelectUserLeftSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUserLeftSession() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUserLeftSession())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnUserLeftSession() };
		m_SelectUserLeftSessionSpec.PopupAction = [&]()
		{
			m_SelectUserLeftSessionSpec.GetAllOptions().clear();

			m_SelectUserLeftSessionSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_FuncType != WrappedFuncType::Void_UInt16)
				{
					continue;
				}
				m_SelectUserLeftSessionSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			m_SelectUserLeftSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUserLeftSession() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUserLeftSession())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnUserLeftSession() };
		};
		m_SelectUserLeftSessionSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find on user left session function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnUserLeftSession(selection.Handle);
		};

		// Update Start Session
		m_SelectSessionInitSpec.Label = "Session Initialization";
		m_SelectSessionInitSpec.LineCount = 3;
		m_SelectSessionInitSpec.CurrentOption = { Projects::ProjectService::GetActiveOnCurrentSessionInit() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnCurrentSessionInit())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnCurrentSessionInit()};
		m_SelectSessionInitSpec.PopupAction = [&]()
		{
			m_SelectSessionInitSpec.GetAllOptions().clear();

			m_SelectSessionInitSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				m_SelectSessionInitSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			m_SelectSessionInitSpec.CurrentOption = { Projects::ProjectService::GetActiveOnCurrentSessionInit() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnCurrentSessionInit())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnCurrentSessionInit() };
		};
		m_SelectSessionInitSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find current session function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnCurrentSessionInit(selection.Handle);
		};

		// Update Connection Terminated
		m_SelectConnectionTerminatedSpec.Label = "Connection Terminated";
		m_SelectConnectionTerminatedSpec.LineCount = 3;
		m_SelectConnectionTerminatedSpec.CurrentOption = { Projects::ProjectService::GetActiveOnConnectionTerminated() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnConnectionTerminated())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnConnectionTerminated()};
		m_SelectConnectionTerminatedSpec.PopupAction = [&]()
		{
			m_SelectConnectionTerminatedSpec.GetAllOptions().clear();

			m_SelectConnectionTerminatedSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				m_SelectConnectionTerminatedSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			m_SelectConnectionTerminatedSpec.CurrentOption = { Projects::ProjectService::GetActiveOnConnectionTerminated() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnConnectionTerminated())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnConnectionTerminated() };
		};
		m_SelectConnectionTerminatedSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find connection terminated function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnConnectionTerminated(selection.Handle);
		};

		// Update Session User Slot
		m_SelectUpdateSessionSlotSpec.Label = "Update Session User Slot";
		m_SelectUpdateSessionSlotSpec.LineCount = 3;
		m_SelectUpdateSessionSlotSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUpdateSessionUserSlot() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUpdateSessionUserSlot())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnUpdateSessionUserSlot()};
		m_SelectUpdateSessionSlotSpec.PopupAction = [&]()
		{
			m_SelectUpdateSessionSlotSpec.GetAllOptions().clear();

			m_SelectUpdateSessionSlotSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_FuncType != WrappedFuncType::Void_UInt16)
				{
					continue;
				}
				m_SelectUpdateSessionSlotSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			m_SelectUpdateSessionSlotSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUpdateSessionUserSlot() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUpdateSessionUserSlot())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnUpdateSessionUserSlot() };
		};
		m_SelectUpdateSessionSlotSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find on update session user slot function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnUpdateSessionUserSlot(selection.Handle);
		};

		// Update On Start Session
		m_SelectStartSessionSpec.Label = "Start Session";
		m_SelectStartSessionSpec.LineCount = 3;
		m_SelectStartSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnStartSession() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnStartSession())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnStartSession() };
		m_SelectStartSessionSpec.PopupAction = [&]()
		{
			m_SelectStartSessionSpec.GetAllOptions().clear();

			m_SelectStartSessionSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				m_SelectStartSessionSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			m_SelectStartSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnStartSession() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnStartSession())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnStartSession() };
		};
		m_SelectStartSessionSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find start session function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnStartSession(selection.Handle);
		};

		// Update On Session Ready Check Confirm
		m_SelectSessionReadyCheckSpec.Label = "Session Ready Check Confirm";
		m_SelectSessionReadyCheckSpec.LineCount = 3;
		m_SelectSessionReadyCheckSpec.CurrentOption = { Projects::ProjectService::GetActiveOnSessionReadyCheckConfirm() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnSessionReadyCheckConfirm())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnSessionReadyCheckConfirm() };
		m_SelectSessionReadyCheckSpec.PopupAction = [&]()
		{
			m_SelectSessionReadyCheckSpec.GetAllOptions().clear();

			m_SelectSessionReadyCheckSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				m_SelectSessionReadyCheckSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			m_SelectSessionReadyCheckSpec.CurrentOption = { Projects::ProjectService::GetActiveOnSessionReadyCheckConfirm() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnSessionReadyCheckConfirm())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnSessionReadyCheckConfirm() };
		};
		m_SelectSessionReadyCheckSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find session ready check function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnSessionReadyCheckConfirm(selection.Handle);
		};

		// Update On Receive Signal
		m_SelectReceiveSignalSpec.Label = "On Receive Signal";
		m_SelectReceiveSignalSpec.LineCount = 3;
		m_SelectReceiveSignalSpec.CurrentOption = { Projects::ProjectService::GetActiveOnReceiveSignal() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnReceiveSignal())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnReceiveSignal() };
		m_SelectReceiveSignalSpec.PopupAction = [&]()
		{
			m_SelectReceiveSignalSpec.GetAllOptions().clear();

			m_SelectReceiveSignalSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_FuncType != WrappedFuncType::Void_UInt16)
				{
					continue;
				}
				m_SelectReceiveSignalSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			m_SelectReceiveSignalSpec.CurrentOption = { Projects::ProjectService::GetActiveOnReceiveSignal() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnReceiveSignal())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnReceiveSignal() };
		};
		m_SelectReceiveSignalSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find on receive signal function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnReceiveSignal(selection.Handle);
		};

	}

	void ProjectPanel::InitializeMessageTypeResources()
	{
		// Manage AI Message Types
		m_MessageTypeTable.Label = "All Message Types";
		m_MessageTypeTable.Column1Title = "Message Type";
		m_MessageTypeTable.Column2Title = "";
		m_MessageTypeTable.Expanded = false;
		m_MessageTypeTable.AddToSelectionList("Create New Message Type", [&]()
		{
			m_CreateMessageTypePopup.StartPopup = true;
		});
		m_MessageTypeTable.OnRefresh = [&]()
		{
			m_MessageTypeTable.ClearTable();
			for (std::string& label : Projects::ProjectService::GetAllMessageTypes())
			{
				m_MessageTypeTable.InsertTableEntry(label, "", [&](EditorUI::TableEntry& entry)
				{
					m_ActiveAIMessageType = entry.Label;
					m_EditMessageTypePopup.PopupActive = true;
				});
			}
		};
		m_MessageTypeTable.OnRefresh();

		m_CreateMessageTypePopup.Label = "Create New Message Type";
		m_CreateMessageTypePopup.Flags |= EditorUI::EditText_PopupOnly;
		m_CreateMessageTypePopup.ConfirmAction = [&](EditorUI::EditTextSpec& spec)
		{
			// Ensure input string does not use whitespace
			Utility::Operations::RemoveWhitespaceFromString(m_CreateMessageTypePopup.CurrentOption);
			// Create new AI message
			bool success = Projects::ProjectService::AddAIMessageType(m_CreateMessageTypePopup.CurrentOption);
			if (!success)
			{
				KG_WARN("Failed to create message type");
				return;
			}
			m_MessageTypeTable.OnRefresh();
			s_EditorApp->m_TextEditorPanel->RefreshKGScriptEditor();
		};

		m_EditMessageTypePopup.Label = "Edit Message Type";
		m_EditMessageTypePopup.PopupAction = [&]()
		{
			m_EditMessageTypeText.CurrentOption = m_ActiveAIMessageType;
		};
		m_EditMessageTypePopup.ConfirmAction = [&]()
		{
			// Ensure input string does not use whitespace
			Utility::Operations::RemoveWhitespaceFromString(m_CreateMessageTypePopup.CurrentOption);

			bool success = Projects::ProjectService::EditAIMessageType(m_ActiveAIMessageType, 
				m_EditMessageTypeText.CurrentOption);

			// Create new AI message type
			if (!success)
			{
				KG_WARN("Failed to edit message type");
				return;
			}
			OnRefresh();
			s_EditorApp->m_TextEditorPanel->RefreshKGScriptEditor();
		};
		m_EditMessageTypePopup.DeleteAction = [&]()
		{
			bool success = Projects::ProjectService::DeleteAIMessageType(m_ActiveAIMessageType);
			if (!success)
			{
				KG_WARN("Failed to delete section label");
				return;
			}

			OnRefresh();
			s_EditorApp->m_TextEditorPanel->RefreshKGScriptEditor();
		};
		m_EditMessageTypePopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_EditMessageTypeText);
		};

		m_EditMessageTypeText.Label = "Message Type";
		m_EditMessageTypeText.CurrentOption = "Empty";

	}

	ProjectPanel::ProjectPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(ProjectPanel::OnKeyPressedEditor));
		InitializeStaticResources();
		InitializeMessageTypeResources();
	}
	void ProjectPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowProject);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}
		// Project Name
		EditorUI::EditorUIService::LabeledText("Project Name", Projects::ProjectService::GetActiveProjectName());
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Project Directory
		EditorUI::EditorUIService::LabeledText("Project Directory", Projects::ProjectService::GetActiveProjectDirectory().string());
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select Starting Scene
		EditorUI::EditorUIService::SelectOption(m_SelectStartSceneSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Default Fullscreen
		m_DefaultFullscreenSpec.CurrentBoolean = Projects::ProjectService::GetActiveIsFullscreen();
		EditorUI::EditorUIService::Checkbox(m_DefaultFullscreenSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Networking Checkbox
		m_ToggleNetworkSpec.CurrentBoolean = Projects::ProjectService::GetActiveAppIsNetworked();
		EditorUI::EditorUIService::Checkbox(m_ToggleNetworkSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select Starting Game State
		EditorUI::EditorUIService::SelectOption(m_SelectStartGameStateSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Resolution Select Option
		EditorUI::EditorUIService::SelectOption(m_SelectResolutionSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Runtime Start
		EditorUI::EditorUIService::SelectOption(m_SelectRuntimeStartSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Update User Count
		EditorUI::EditorUIService::SelectOption(m_SelectUpdateUserCountSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Approve Join Session
		EditorUI::EditorUIService::SelectOption(m_SelectApproveJoinSessionSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On User Left Session
		EditorUI::EditorUIService::SelectOption(m_SelectUserLeftSessionSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Current Session Start
		EditorUI::EditorUIService::SelectOption(m_SelectSessionInitSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Connection Terminated
		EditorUI::EditorUIService::SelectOption(m_SelectConnectionTerminatedSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Update Session User Slot
		EditorUI::EditorUIService::SelectOption(m_SelectUpdateSessionSlotSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Start Session
		EditorUI::EditorUIService::SelectOption(m_SelectStartSessionSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Session Ready Check
		EditorUI::EditorUIService::SelectOption(m_SelectSessionReadyCheckSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Receive Signal Function
		EditorUI::EditorUIService::SelectOption(m_SelectReceiveSignalSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);


		ImGui::TextUnformatted("App Tick Generators:");
		ImGui::Separator();
		// Enable Delete Option
		static bool deleteMenuToggle = false;
		bool deleteGenerator = false;
		bool openGeneratorPopup = false;
		uint64_t generatorToDelete{};
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 126.0f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 20.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		if (ImGui::ImageButton((ImTextureID)(uint64_t)EditorUI::EditorUIService::s_IconSettings->GetRendererID(), ImVec2(17, 17), 
			ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, 
			ImVec4(0.0f, 0.0f, 0.0f, 0.0f), EditorUI::EditorUIService::s_DisabledColor))
		{
			ImGui::OpenPopup("AppTickGeneratorSettings");
		}
		ImGui::PopStyleColor();

		if (ImGui::BeginPopup("AppTickGeneratorSettings"))
		{
			if (ImGui::Selectable("Add New Generator"))
			{
				openGeneratorPopup = true;
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Toggle Delete Option", deleteMenuToggle))
			{
				deleteMenuToggle = deleteMenuToggle ? false : true; // Conditional Toggles Boolean
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		static int32_t newHours {0};
		static int32_t newMinutes {0};
		static int32_t newSeconds {0};
		static int32_t newMilliseconds {0};
		if (openGeneratorPopup)
		{
			ImGui::OpenPopup("New App Tick Generator");
			newHours = 0;
			newMinutes = 0;
			newSeconds = 0;
			newMilliseconds = 0;
		}

		if (ImGui::BeginPopup("New App Tick Generator"))
		{
			ImGui::DragInt("Hours", &newHours, 1, 0, 2'000'000'000);
			ImGui::DragInt("Minutes", &newMinutes, 1, 0, 59);
			ImGui::DragInt("Seconds", &newSeconds, 1, 0, 59);
			ImGui::DragInt("Milliseconds", &newMilliseconds, 1, 0, 999);
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Submit"))
			{
				auto& generators = Projects::ProjectService::GetActiveAppTickGenerators();

				if (newSeconds < 0) { newSeconds = 0; }
				if (newMinutes < 0) { newMinutes = 0; }
				if (newHours < 0) { newHours = 0; }
				if (newMilliseconds < 0) { newMilliseconds = 0; }

				uint64_t finalMilliseconds = newMilliseconds +
					static_cast<uint64_t>(newSeconds * 1000) +
					static_cast<uint64_t>(newMinutes * 60'000) +
					static_cast<uint64_t>(newHours * 3'600'000);

				generators.insert(finalMilliseconds);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
		if (ImGui::BeginTable("AppTickGeneratorTable", deleteMenuToggle ? 2 : 1, flags))
		{
			ImGui::TableSetupColumn("All Active Generators", ImGuiTableColumnFlags_WidthStretch);
			if (deleteMenuToggle) { ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, 20.0f); }
			ImGui::TableHeadersRow();

			for (auto& generatorValue : Projects::ProjectService::GetActiveAppTickGenerators())
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 6.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);

				std::string textOutput = "Every " + Utility::Time::GetStringFromMilliseconds(generatorValue);

				ImGui::Text(textOutput.c_str());
				if (deleteMenuToggle)
				{
					ImGui::SameLine();
					ImGui::TableSetColumnIndex(1);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
					if (ImGui::ImageButton(("Delete Generator##AppTickGeneratorDelete" + std::to_string(generatorValue)).c_str(), (ImTextureID)(uint64_t)EditorUI::EditorUIService::s_IconDelete->GetRendererID(),
						ImVec2(17.0f, 17.0f), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
					{
						deleteGenerator = true;
						generatorToDelete = generatorValue;
					}
					ImGui::PopStyleColor();
				}

			}
			ImGui::EndTable();
		}
		if (deleteGenerator)
		{
			auto& generators = Projects::ProjectService::GetActiveAppTickGenerators();
			generators.erase(generatorToDelete);
		}

		ImGui::NewLine();

		ImGui::Text("Physics Settings:");
		if (ImGui::DragFloat2("Gravity", glm::value_ptr(s_EditorApp->m_EditorScene->GetPhysicsSpecification().Gravity), 0.05f))
		{
			if (Physics::Physics2DService::GetActivePhysics2DWorld())
			{
				Scenes::SceneService::GetActiveScene()->GetPhysicsSpecification().Gravity = s_EditorApp->m_EditorScene->GetPhysicsSpecification().Gravity;
				Physics::Physics2DService::SetActiveGravity(s_EditorApp->m_EditorScene->GetPhysicsSpecification().Gravity);
			}
		}


		EditorUI::EditorUIService::Table(m_MessageTypeTable);
		EditorUI::EditorUIService::EditText(m_CreateMessageTypePopup);
		EditorUI::EditorUIService::GenericPopup(m_EditMessageTypePopup);

		EditorUI::EditorUIService::EndWindow();
	}
	bool ProjectPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	void ProjectPanel::ResetPanelResources()
	{
		InitializeStaticResources();
	}
	void ProjectPanel::OnRefresh()
	{
		m_MessageTypeTable.OnRefresh();
	}
}
