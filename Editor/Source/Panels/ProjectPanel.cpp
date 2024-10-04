#include "Panels/ProjectPanel.h"

#include "Kargono.h"
#include "EditorApp.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	static EditorUI::SelectOptionSpec s_SelectStartSceneSpec {};
	static EditorUI::CheckboxSpec s_DefaultFullscreenSpec {};
	static EditorUI::CheckboxSpec s_ToggleNetworkSpec {};
	static EditorUI::SelectOptionSpec s_SelectResolutionSpec {};
	static EditorUI::SelectOptionSpec s_SelectStartGameStateSpec {};
	static EditorUI::SelectOptionSpec s_SelectRuntimeStartSpec {};
	static EditorUI::SelectOptionSpec s_SelectUpdateUserCountSpec {};
	static EditorUI::SelectOptionSpec s_SelectApproveJoinSessionSpec {};
	static EditorUI::SelectOptionSpec s_SelectUserLeftSessionSpec {};
	static EditorUI::SelectOptionSpec s_SelectSessionInitSpec {};
	static EditorUI::SelectOptionSpec s_SelectConnectionTerminatedSpec {};
	static EditorUI::SelectOptionSpec s_SelectUpdateSessionSlotSpec {};
	static EditorUI::SelectOptionSpec s_SelectStartSessionSpec {};
	static EditorUI::SelectOptionSpec s_SelectSessionReadyCheckSpec {};
	static EditorUI::SelectOptionSpec s_SelectReceiveSignalSpec {};

	void InitializeStaticResources()
	{
		// Resolution Specification
		s_SelectStartSceneSpec.Label = "Starting Scene";
		//s_SelectStartSceneSpec.WidgetID = 0x75ebbc8750034f81;
		s_SelectStartSceneSpec.LineCount = 2;
		
		s_SelectStartSceneSpec.CurrentOption = {
			 Assets::AssetService::GetSceneRegistry().at(Projects::ProjectService::GetActiveStartSceneHandle()).Data.FileLocation.string(),
			Projects::ProjectService::GetActiveStartSceneHandle()};
		s_SelectStartSceneSpec.PopupAction = []()
		{
			s_SelectStartSceneSpec.GetAllOptions().clear();
			for (auto& [handle, asset] : Assets::AssetService::GetSceneRegistry())
			{
				s_SelectStartSceneSpec.AddToOptions("All Options", asset.Data.FileLocation.string(), handle);
			}
			s_SelectStartSceneSpec.CurrentOption = {
				Assets::AssetService::GetSceneRegistry().at(Projects::ProjectService::GetActiveStartSceneHandle()).Data.FileLocation.string(),
			Projects::ProjectService::GetActiveStartSceneHandle()};
		};
		s_SelectStartSceneSpec.ConfirmAction = [&](const EditorUI::OptionEntry& entry)
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
		s_DefaultFullscreenSpec.Label = "Default Fullscreen";
		s_DefaultFullscreenSpec.ConfirmAction = [](bool value)
		{
			Projects::ProjectService::SetActiveIsFullscreen(value);
		};

		// Set Networking Specification
		s_ToggleNetworkSpec.Label = "Networking";
		s_ToggleNetworkSpec.ConfirmAction = [](bool value)
		{
			Projects::ProjectService::SetActiveAppIsNetworked(value);
		};

		// Resolution Specification
		s_SelectResolutionSpec.Label = "Target Resolution";
		s_SelectResolutionSpec.LineCount = 4;
		s_SelectResolutionSpec.CurrentOption = {
			Utility::ScreenResolutionToString(Projects::ProjectService::GetActiveTargetResolution()),
			Assets::EmptyHandle};
		s_SelectResolutionSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			Projects::ProjectService::SetActiveTargetResolution(Utility::StringToScreenResolution(selection.Label));
		};
		s_SelectResolutionSpec.PopupAction = [&]()
		{
			s_SelectResolutionSpec.CurrentOption = {
				Utility::ScreenResolutionToString(Projects::ProjectService::GetActiveTargetResolution()),
			Assets::EmptyHandle};
		};
		s_SelectResolutionSpec.AddToOptions("Aspect Ratio: 1:1 (Box)", "800x800", Assets::EmptyHandle);
		s_SelectResolutionSpec.AddToOptions("Aspect Ratio: 1:1 (Box)", "400x400", Assets::EmptyHandle);
		s_SelectResolutionSpec.AddToOptions("Aspect Ratio: 16:9 (Widescreen)", "1920x1080", Assets::EmptyHandle);
		s_SelectResolutionSpec.AddToOptions("Aspect Ratio: 16:9 (Widescreen)", "1600x900", Assets::EmptyHandle);
		s_SelectResolutionSpec.AddToOptions("Aspect Ratio: 16:9 (Widescreen)", "1366x768", Assets::EmptyHandle);
		s_SelectResolutionSpec.AddToOptions("Aspect Ratio: 16:9 (Widescreen)", "1280x720", Assets::EmptyHandle);
		s_SelectResolutionSpec.AddToOptions("Aspect Ratio: 4:3 (Fullscreen)", "1600x1200", Assets::EmptyHandle);
		s_SelectResolutionSpec.AddToOptions("Aspect Ratio: 4:3 (Fullscreen)", "1280x960", Assets::EmptyHandle);
		s_SelectResolutionSpec.AddToOptions("Aspect Ratio: 4:3 (Fullscreen)", "1152x864", Assets::EmptyHandle);
		s_SelectResolutionSpec.AddToOptions("Aspect Ratio: 4:3 (Fullscreen)", "1024x768", Assets::EmptyHandle);
		s_SelectResolutionSpec.AddToOptions("Aspect Ratio: Automatic (Based on Device Used)", "Match Device", Assets::EmptyHandle);

		// Select Start Game State
		s_SelectStartGameStateSpec.Label = "Start Game State";
		s_SelectStartGameStateSpec.LineCount = 3;
		if (Projects::ProjectService::GetActiveStartGameState() != 0)
		{
			s_SelectStartGameStateSpec.CurrentOption = { Assets::AssetService::GetGameStateRegistry().at
			(Projects::ProjectService::GetActiveStartGameState()).Data.FileLocation.string(),
			Projects::ProjectService::GetActiveStartGameState()};
		}
		else
		{
			s_SelectStartGameStateSpec.CurrentOption = { "None", Assets::EmptyHandle };
		}
		s_SelectStartGameStateSpec.PopupAction = [&]()
		{
			s_SelectStartGameStateSpec.ClearOptions();
			s_SelectStartGameStateSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetGameStateRegistry())
			{
				s_SelectStartGameStateSpec.AddToOptions("All Options", asset.Data.FileLocation.string(), handle);
			}

			if (Projects::ProjectService::GetActiveStartGameState() != Assets::EmptyHandle)
			{
				s_SelectStartGameStateSpec.CurrentOption = { Assets::AssetService::GetGameStateRegistry().at
				(Projects::ProjectService::GetActiveStartGameState()).Data.FileLocation.string(),
				Projects::ProjectService::GetActiveStartGameState() };
			}
			else
			{
				s_SelectStartGameStateSpec.CurrentOption = { "None", Assets::EmptyHandle };
			}

		};

		s_SelectStartGameStateSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetGameState(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not locate starting game state in ProjectPanel");
				return;
			}
			Projects::ProjectService::SetActiveStartGameState(selection.Handle);

		};

		// Runtime Start Spec
		s_SelectRuntimeStartSpec.Label = "Runtime Start";
		s_SelectRuntimeStartSpec.LineCount = 3;
		s_SelectRuntimeStartSpec.CurrentOption = { Projects::ProjectService::GetActiveOnRuntimeStart() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnRuntimeStart())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnRuntimeStart()};
		s_SelectRuntimeStartSpec.PopupAction = []()
		{
			s_SelectRuntimeStartSpec.GetAllOptions().clear();

			s_SelectRuntimeStartSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Class || script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				s_SelectRuntimeStartSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			s_SelectRuntimeStartSpec.CurrentOption = { Projects::ProjectService::GetActiveOnRuntimeStart() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnRuntimeStart())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnRuntimeStart() };
		};
		s_SelectRuntimeStartSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find runtime start function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnRuntimeStart(selection.Handle);
		};

		// Update User Count Spec
		s_SelectUpdateUserCountSpec.Label = "Update User Count";
		s_SelectUpdateUserCountSpec.LineCount = 3;
		s_SelectUpdateUserCountSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUpdateUserCount() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUpdateUserCount())->m_ScriptName : "None",
		Projects::ProjectService::GetActiveOnUpdateUserCount()};
		s_SelectUpdateUserCountSpec.PopupAction = []()
		{
			s_SelectUpdateUserCountSpec.GetAllOptions().clear();

			s_SelectUpdateUserCountSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Class || script->m_FuncType != WrappedFuncType::Void_UInt32)
				{
					continue;
				}
				s_SelectUpdateUserCountSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			s_SelectUpdateUserCountSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUpdateUserCount() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUpdateUserCount())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnUpdateUserCount() };
		};
		s_SelectUpdateUserCountSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find on update user count function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnUpdateUserCount(selection.Handle);
		};

		// Update Approve Join Session
		s_SelectApproveJoinSessionSpec.Label = "Approve Join Session";
		s_SelectApproveJoinSessionSpec.LineCount = 3;
		s_SelectApproveJoinSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnApproveJoinSession() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnApproveJoinSession())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnApproveJoinSession() };
		s_SelectApproveJoinSessionSpec.PopupAction = []()
		{
			s_SelectApproveJoinSessionSpec.GetAllOptions().clear();

			s_SelectApproveJoinSessionSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Class || script->m_FuncType != WrappedFuncType::Void_UInt16)
				{
					continue;
				}
				s_SelectApproveJoinSessionSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			s_SelectApproveJoinSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnApproveJoinSession() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnApproveJoinSession())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnApproveJoinSession() };
		};
		s_SelectApproveJoinSessionSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find on approve join session function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnApproveJoinSession(selection.Handle);
		};

		// Update User Left Session
		s_SelectUserLeftSessionSpec.Label = "User Left Session";
		s_SelectUserLeftSessionSpec.LineCount = 3;
		s_SelectUserLeftSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUserLeftSession() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUserLeftSession())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnUserLeftSession() };
		s_SelectUserLeftSessionSpec.PopupAction = [&]()
		{
			s_SelectUserLeftSessionSpec.GetAllOptions().clear();

			s_SelectUserLeftSessionSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Class || script->m_FuncType != WrappedFuncType::Void_UInt16)
				{
					continue;
				}
				s_SelectUserLeftSessionSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			s_SelectUserLeftSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUserLeftSession() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUserLeftSession())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnUserLeftSession() };
		};
		s_SelectUserLeftSessionSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find on user left session function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnUserLeftSession(selection.Handle);
		};

		// Update Start Session
		s_SelectSessionInitSpec.Label = "Session Initialization";
		s_SelectSessionInitSpec.LineCount = 3;
		s_SelectSessionInitSpec.CurrentOption = { Projects::ProjectService::GetActiveOnCurrentSessionInit() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnCurrentSessionInit())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnCurrentSessionInit()};
		s_SelectSessionInitSpec.PopupAction = []()
		{
			s_SelectSessionInitSpec.GetAllOptions().clear();

			s_SelectSessionInitSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Class || script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				s_SelectSessionInitSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			s_SelectSessionInitSpec.CurrentOption = { Projects::ProjectService::GetActiveOnCurrentSessionInit() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnCurrentSessionInit())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnCurrentSessionInit() };
		};
		s_SelectSessionInitSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find current session function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnCurrentSessionInit(selection.Handle);
		};

		// Update Connection Terminated
		s_SelectConnectionTerminatedSpec.Label = "Connection Terminated";
		s_SelectConnectionTerminatedSpec.LineCount = 3;
		s_SelectConnectionTerminatedSpec.CurrentOption = { Projects::ProjectService::GetActiveOnConnectionTerminated() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnConnectionTerminated())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnConnectionTerminated()};
		s_SelectConnectionTerminatedSpec.PopupAction = []()
		{
			s_SelectConnectionTerminatedSpec.GetAllOptions().clear();

			s_SelectConnectionTerminatedSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Class || script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				s_SelectConnectionTerminatedSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			s_SelectConnectionTerminatedSpec.CurrentOption = { Projects::ProjectService::GetActiveOnConnectionTerminated() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnConnectionTerminated())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnConnectionTerminated() };
		};
		s_SelectConnectionTerminatedSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find connection terminated function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnConnectionTerminated(selection.Handle);
		};

		// Update Session User Slot
		s_SelectUpdateSessionSlotSpec.Label = "Update Session User Slot";
		s_SelectUpdateSessionSlotSpec.LineCount = 3;
		s_SelectUpdateSessionSlotSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUpdateSessionUserSlot() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUpdateSessionUserSlot())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnUpdateSessionUserSlot()};
		s_SelectUpdateSessionSlotSpec.PopupAction = []()
		{
			s_SelectUpdateSessionSlotSpec.GetAllOptions().clear();

			s_SelectUpdateSessionSlotSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Class || script->m_FuncType != WrappedFuncType::Void_UInt16)
				{
					continue;
				}
				s_SelectUpdateSessionSlotSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			s_SelectUpdateSessionSlotSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUpdateSessionUserSlot() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUpdateSessionUserSlot())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnUpdateSessionUserSlot() };
		};
		s_SelectUpdateSessionSlotSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find on update session user slot function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnUpdateSessionUserSlot(selection.Handle);
		};

		// Update On Start Session
		s_SelectStartSessionSpec.Label = "Start Session";
		s_SelectStartSessionSpec.LineCount = 3;
		s_SelectStartSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnStartSession() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnStartSession())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnStartSession() };
		s_SelectStartSessionSpec.PopupAction = []()
		{
			s_SelectStartSessionSpec.GetAllOptions().clear();

			s_SelectStartSessionSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Class || script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				s_SelectStartSessionSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			s_SelectStartSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnStartSession() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnStartSession())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnStartSession() };
		};
		s_SelectStartSessionSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find start session function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnStartSession(selection.Handle);
		};

		// Update On Session Ready Check Confirm
		s_SelectSessionReadyCheckSpec.Label = "Session Ready Check Confirm";
		s_SelectSessionReadyCheckSpec.LineCount = 3;
		s_SelectSessionReadyCheckSpec.CurrentOption = { Projects::ProjectService::GetActiveOnSessionReadyCheckConfirm() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnSessionReadyCheckConfirm())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnSessionReadyCheckConfirm() };
		s_SelectSessionReadyCheckSpec.PopupAction = []()
		{
			s_SelectSessionReadyCheckSpec.GetAllOptions().clear();

			s_SelectSessionReadyCheckSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Class || script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}
				s_SelectSessionReadyCheckSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			s_SelectSessionReadyCheckSpec.CurrentOption = { Projects::ProjectService::GetActiveOnSessionReadyCheckConfirm() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnSessionReadyCheckConfirm())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnSessionReadyCheckConfirm() };
		};
		s_SelectSessionReadyCheckSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find session ready check function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnSessionReadyCheckConfirm(selection.Handle);
		};

		// Update On Receive Signal
		s_SelectReceiveSignalSpec.Label = "On Receive Signal";
		s_SelectReceiveSignalSpec.LineCount = 3;
		s_SelectReceiveSignalSpec.CurrentOption = { Projects::ProjectService::GetActiveOnReceiveSignal() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnReceiveSignal())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnReceiveSignal() };
		s_SelectReceiveSignalSpec.PopupAction = []()
		{
			s_SelectReceiveSignalSpec.GetAllOptions().clear();

			s_SelectReceiveSignalSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetService::GetScriptCache())
			{
				if (script->m_ScriptType == Scripting::ScriptType::Class || script->m_FuncType != WrappedFuncType::Void_UInt16)
				{
					continue;
				}
				s_SelectReceiveSignalSpec.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType)
					+ "::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}

			s_SelectReceiveSignalSpec.CurrentOption = { Projects::ProjectService::GetActiveOnReceiveSignal() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnReceiveSignal())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnReceiveSignal() };
		};
		s_SelectReceiveSignalSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find on receive signal function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnReceiveSignal(selection.Handle);
		};

	}

	ProjectPanel::ProjectPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(ProjectPanel::OnKeyPressedEditor));
		InitializeStaticResources();
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
		EditorUI::EditorUIService::SelectOption(s_SelectStartSceneSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Default Fullscreen
		s_DefaultFullscreenSpec.ToggleBoolean = Projects::ProjectService::GetActiveIsFullscreen();
		EditorUI::EditorUIService::Checkbox(s_DefaultFullscreenSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Networking Checkbox
		s_ToggleNetworkSpec.ToggleBoolean = Projects::ProjectService::GetActiveAppIsNetworked();
		EditorUI::EditorUIService::Checkbox(s_ToggleNetworkSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select Starting Game State
		EditorUI::EditorUIService::SelectOption(s_SelectStartGameStateSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Resolution Select Option
		EditorUI::EditorUIService::SelectOption(s_SelectResolutionSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Runtime Start
		EditorUI::EditorUIService::SelectOption(s_SelectRuntimeStartSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Update User Count
		EditorUI::EditorUIService::SelectOption(s_SelectUpdateUserCountSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Approve Join Session
		EditorUI::EditorUIService::SelectOption(s_SelectApproveJoinSessionSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On User Left Session
		EditorUI::EditorUIService::SelectOption(s_SelectUserLeftSessionSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Current Session Start
		EditorUI::EditorUIService::SelectOption(s_SelectSessionInitSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Connection Terminated
		EditorUI::EditorUIService::SelectOption(s_SelectConnectionTerminatedSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Update Session User Slot
		EditorUI::EditorUIService::SelectOption(s_SelectUpdateSessionSlotSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Start Session
		EditorUI::EditorUIService::SelectOption(s_SelectStartSessionSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Session Ready Check
		EditorUI::EditorUIService::SelectOption(s_SelectSessionReadyCheckSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Receive Signal Function
		EditorUI::EditorUIService::SelectOption(s_SelectReceiveSignalSpec);
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
}
