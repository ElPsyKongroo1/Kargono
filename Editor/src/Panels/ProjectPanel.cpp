#include "Panels/ProjectPanel.h"

#include "Kargono.h"
#include "EditorLayer.h"

namespace Kargono
{
	static EditorLayer* s_EditorLayer { nullptr };

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
			Projects::Project::GetStartScenePath(false).string(),
			Projects::Project::GetStartSceneHandle()};
		s_SelectStartSceneSpec.PopupAction = [](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();
			for (auto& [handle, asset] : Assets::AssetManager::GetSceneRegistry())
			{
				spec.AddToOptions("All Options", asset.Data.IntermediateLocation.string(), handle);
			}
			spec.CurrentOption = {
				Projects::Project::GetStartScenePath(false).string(),
			Projects::Project::GetStartSceneHandle()};
		};
		s_SelectStartSceneSpec.ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{
			if (!Assets::AssetManager::GetSceneRegistry().contains(entry.Handle))
			{
				KG_WARN("Could not find scene using asset handle in Project Panel");
				return;
			}

			const Assets::Asset asset = Assets::AssetManager::GetSceneRegistry().at(entry.Handle);
			Projects::Project::SetStartingScene(entry.Handle, asset.Data.IntermediateLocation);
		};

		// Default Full Screen
		s_DefaultFullscreenSpec.Label = "Default Fullscreen";
		s_DefaultFullscreenSpec.ConfirmAction = [](bool value)
		{
			Projects::Project::SetIsFullscreen(value);
		};

		// Set Networking Specification
		s_ToggleNetworkSpec.Label = "Networking";
		s_ToggleNetworkSpec.ConfirmAction = [](bool value)
		{
			Projects::Project::SetAppIsNetworked(value);
		};

		// Resolution Specification
		s_SelectResolutionSpec.Label = "Target Resolution";
		s_SelectResolutionSpec.LineCount = 4;
		s_SelectResolutionSpec.CurrentOption = {
			Utility::ScreenResolutionToString(Projects::Project::GetTargetResolution()),
			Assets::EmptyHandle};
		s_SelectResolutionSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			Projects::Project::SetTargetResolution(Utility::StringToScreenResolution(selection.Label));
		};
		s_SelectResolutionSpec.PopupAction = [&](EditorUI::SelectOptionSpec& spec)
		{
			s_SelectResolutionSpec.CurrentOption = {
				Utility::ScreenResolutionToString(Projects::Project::GetTargetResolution()),
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
		if (Projects::Project::GetStartGameState() != 0)
		{
			s_SelectStartGameStateSpec.CurrentOption = { Assets::AssetManager::GetGameStateRegistry().at
			(Projects::Project::GetStartGameState()).Data.IntermediateLocation.string(),
			Projects::Project::GetStartGameState()};
		}
		else
		{
			s_SelectStartGameStateSpec.CurrentOption = { "None", Assets::EmptyHandle };
		}
		s_SelectStartGameStateSpec.PopupAction = [&](EditorUI::SelectOptionSpec& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetManager::GetGameStateRegistry())
			{
				spec.AddToOptions("All Options", asset.Data.IntermediateLocation.string(), handle);
			}
			
			if (Projects::Project::GetStartGameState() != Assets::EmptyHandle)
			{
				spec.CurrentOption = { Assets::AssetManager::GetGameStateRegistry().at
				(Projects::Project::GetStartGameState()).Data.IntermediateLocation.string(),
				Projects::Project::GetStartGameState()};
			}
			else
			{
				spec.CurrentOption = { "None", Assets::EmptyHandle };
			}

		};

		s_SelectStartGameStateSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetManager::GetGameState(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not locate starting game state in ProjectPanel");
				return;
			}
			Projects::Project::SetStartGameState(selection.Handle);
			
		};

		// Runtime Start Spec
		s_SelectRuntimeStartSpec.Label = "Runtime Start";
		s_SelectRuntimeStartSpec.LineCount = 3;
		s_SelectRuntimeStartSpec.CurrentOption = { Projects::Project::GetOnRuntimeStart() ?
			Assets::AssetManager::GetScript(Projects::Project::GetOnRuntimeStart())->m_ScriptName : "None",
			Projects::Project::GetOnRuntimeStart()};
		s_SelectRuntimeStartSpec.PopupAction = [](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_None)
				{
					spec.AddToOptions("All Options", script->m_ScriptName, handle);
				}
			}

			spec.CurrentOption = { Projects::Project::GetOnRuntimeStart() ?
				Assets::AssetManager::GetScript(Projects::Project::GetOnRuntimeStart())->m_ScriptName : "None",
				Projects::Project::GetOnRuntimeStart()};
		};
		s_SelectRuntimeStartSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetManager::GetScriptRegistryMap().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find runtime start function in Project Panel");
				return;
			}
			Projects::Project::SetOnRuntimeStart(selection.Handle);
		};

		// Update User Count Spec
		s_SelectUpdateUserCountSpec.Label = "Update User Count";
		s_SelectUpdateUserCountSpec.LineCount = 3;
		s_SelectUpdateUserCountSpec.CurrentOption = { Projects::Project::GetOnUpdateUserCount() ?
			Assets::AssetManager::GetScript(Projects::Project::GetOnUpdateUserCount())->m_ScriptName : "None",
		Projects::Project::GetOnUpdateUserCount()};
		s_SelectUpdateUserCountSpec.PopupAction = [](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_UInt32)
				{
					spec.AddToOptions("All Options", script->m_ScriptName, handle);
				}
			}

			spec.CurrentOption = { Projects::Project::GetOnUpdateUserCount() ?
				Assets::AssetManager::GetScript(Projects::Project::GetOnUpdateUserCount())->m_ScriptName : "None",
				Projects::Project::GetOnUpdateUserCount()};
		};
		s_SelectUpdateUserCountSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetManager::GetScriptRegistryMap().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find on update user count function in Project Panel");
				return;
			}
			Projects::Project::SetOnUpdateUserCount(selection.Handle);
		};

		// Update Approve Join Session
		s_SelectApproveJoinSessionSpec.Label = "Approve Join Session";
		s_SelectApproveJoinSessionSpec.LineCount = 3;
		s_SelectApproveJoinSessionSpec.CurrentOption = { Projects::Project::GetOnApproveJoinSession() ?
			Assets::AssetManager::GetScript(Projects::Project::GetOnApproveJoinSession())->m_ScriptName : "None",
			Projects::Project::GetOnApproveJoinSession() };
		s_SelectApproveJoinSessionSpec.PopupAction = [](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_UInt16)
				{
					spec.AddToOptions("All Options", script->m_ScriptName, handle);
				}
			}

			spec.CurrentOption = { Projects::Project::GetOnApproveJoinSession() ?
				Assets::AssetManager::GetScript(Projects::Project::GetOnApproveJoinSession())->m_ScriptName : "None",
				Projects::Project::GetOnApproveJoinSession()};
		};
		s_SelectApproveJoinSessionSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetManager::GetScriptRegistryMap().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find on approve join session function in Project Panel");
				return;
			}
			Projects::Project::SetOnApproveJoinSession(selection.Handle);
		};

		// Update User Left Session
		s_SelectUserLeftSessionSpec.Label = "User Left Session";
		s_SelectUserLeftSessionSpec.LineCount = 3;
		s_SelectUserLeftSessionSpec.CurrentOption = { Projects::Project::GetOnUserLeftSession() ?
			Assets::AssetManager::GetScript(Projects::Project::GetOnUserLeftSession())->m_ScriptName : "None",
			Projects::Project::GetOnUserLeftSession() };
		s_SelectUserLeftSessionSpec.PopupAction = [](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_UInt16)
				{
					spec.AddToOptions("All Options", script->m_ScriptName, handle);
				}
			}

			spec.CurrentOption = { Projects::Project::GetOnUserLeftSession() ?
				Assets::AssetManager::GetScript(Projects::Project::GetOnUserLeftSession())->m_ScriptName : "None",
				Projects::Project::GetOnUserLeftSession()};
		};
		s_SelectUserLeftSessionSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetManager::GetScriptRegistryMap().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find on user left session function in Project Panel");
				return;
			}
			Projects::Project::SetOnUserLeftSession(selection.Handle);
		};

		// Update Start Session
		s_SelectSessionInitSpec.Label = "Session Initialization";
		s_SelectSessionInitSpec.LineCount = 3;
		s_SelectSessionInitSpec.CurrentOption = { Projects::Project::GetOnCurrentSessionInit() ?
			Assets::AssetManager::GetScript(Projects::Project::GetOnCurrentSessionInit())->m_ScriptName : "None",
			Projects::Project::GetOnCurrentSessionInit()};
		s_SelectSessionInitSpec.PopupAction = [](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_None)
				{
					spec.AddToOptions("All Options", script->m_ScriptName, handle);
				}
			}

			spec.CurrentOption = { Projects::Project::GetOnCurrentSessionInit() ?
				Assets::AssetManager::GetScript(Projects::Project::GetOnCurrentSessionInit())->m_ScriptName : "None",
				Projects::Project::GetOnCurrentSessionInit()};
		};
		s_SelectSessionInitSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetManager::GetScriptRegistryMap().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find current session function in Project Panel");
				return;
			}
			Projects::Project::SetOnCurrentSessionInit(selection.Handle);
		};

		// Update Connection Terminated
		s_SelectConnectionTerminatedSpec.Label = "Connection Terminated";
		s_SelectConnectionTerminatedSpec.LineCount = 3;
		s_SelectConnectionTerminatedSpec.CurrentOption = { Projects::Project::GetOnConnectionTerminated() ?
			Assets::AssetManager::GetScript(Projects::Project::GetOnConnectionTerminated())->m_ScriptName : "None",
			Projects::Project::GetOnConnectionTerminated()};
		s_SelectConnectionTerminatedSpec.PopupAction = [](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_None)
				{
					spec.AddToOptions("All Options", script->m_ScriptName, handle);
				}
			}

			spec.CurrentOption = { Projects::Project::GetOnConnectionTerminated() ?
				Assets::AssetManager::GetScript(Projects::Project::GetOnConnectionTerminated())->m_ScriptName : "None",
			Projects::Project::GetOnConnectionTerminated()};
		};
		s_SelectConnectionTerminatedSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetManager::GetScriptRegistryMap().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find connection terminated function in Project Panel");
				return;
			}
			Projects::Project::SetOnConnectionTerminated(selection.Handle);
		};

		// Update Session User Slot
		s_SelectUpdateSessionSlotSpec.Label = "Update Session User Slot";
		s_SelectUpdateSessionSlotSpec.LineCount = 3;
		s_SelectUpdateSessionSlotSpec.CurrentOption = { Projects::Project::GetOnUpdateSessionUserSlot() ?
			Assets::AssetManager::GetScript(Projects::Project::GetOnUpdateSessionUserSlot())->m_ScriptName : "None",
			Projects::Project::GetOnUpdateSessionUserSlot()};
		s_SelectUpdateSessionSlotSpec.PopupAction = [](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_UInt16)
				{
					spec.AddToOptions("All Options", script->m_ScriptName, handle);
				}
			}

			spec.CurrentOption = { Projects::Project::GetOnUpdateSessionUserSlot() ?
				Assets::AssetManager::GetScript(Projects::Project::GetOnUpdateSessionUserSlot())->m_ScriptName : "None",
				Projects::Project::GetOnUpdateSessionUserSlot()};
		};
		s_SelectUpdateSessionSlotSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetManager::GetScriptRegistryMap().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find on update session user slot function in Project Panel");
				return;
			}
			Projects::Project::SetOnUpdateSessionUserSlot(selection.Handle);
		};

		// Update On Start Session
		s_SelectStartSessionSpec.Label = "Start Session";
		s_SelectStartSessionSpec.LineCount = 3;
		s_SelectStartSessionSpec.CurrentOption = { Projects::Project::GetOnStartSession() ?
			Assets::AssetManager::GetScript(Projects::Project::GetOnStartSession())->m_ScriptName : "None",
			Projects::Project::GetOnStartSession()};
		s_SelectStartSessionSpec.PopupAction = [](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_None)
				{
					spec.AddToOptions("All Options", script->m_ScriptName, handle);
				}
			}

			spec.CurrentOption = { Projects::Project::GetOnStartSession() ?
				Assets::AssetManager::GetScript(Projects::Project::GetOnStartSession())->m_ScriptName : "None",
				Projects::Project::GetOnStartSession() };
		};
		s_SelectStartSessionSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetManager::GetScriptRegistryMap().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find start session function in Project Panel");
				return;
			}
			Projects::Project::SetOnStartSession(selection.Handle);
		};

		// Update On Session Ready Check
		s_SelectSessionReadyCheckSpec.Label = "Session Ready Check";
		s_SelectSessionReadyCheckSpec.LineCount = 3;
		s_SelectSessionReadyCheckSpec.PopupAction = [](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				spec.AddToOptions("All Options", name, Assets::EmptyHandle);
			}
		};
		s_SelectSessionReadyCheckSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Label == "None")
			{
				Projects::Project::SetProjectOnSessionReadyCheckConfirm("None");
				return;
			}

			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				if (name == selection.Label)
				{
					Projects::Project::SetProjectOnSessionReadyCheckConfirm(name);
					return;
				}
			}
			KG_ERROR("Could not locate session ready check function in ProjectPanel");
		};

		// Update On Receive Signal
		s_SelectReceiveSignalSpec.Label = "Receive Signal";
		s_SelectReceiveSignalSpec.LineCount = 3;
		s_SelectReceiveSignalSpec.PopupAction = [](EditorUI::SelectOptionSpec& spec)
		{
			spec.GetAllOptions().clear();

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				spec.AddToOptions("All Options", name, Assets::EmptyHandle);
			}
		};
		s_SelectReceiveSignalSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (selection.Label == "None")
			{
				Projects::Project::SetProjectOnReceiveSignal("None");
				return;
			}

			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				if (name == selection.Label)
				{
					Projects::Project::SetProjectOnReceiveSignal(name);
					return;
				}
			}
			KG_ERROR("Could not locate receive signal function in ProjectPanel");
		};

	}

	ProjectPanel::ProjectPanel()
	{
		s_EditorLayer = EditorLayer::GetCurrentLayer();
		InitializeStaticResources();
	}
	void ProjectPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::Editor::StartWindow("Project", &s_EditorLayer->m_ShowProject);
		// Project Name
		EditorUI::Editor::LabeledText("Project Name", Projects::Project::GetProjectName());
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Project Directory
		EditorUI::Editor::LabeledText("Project Directory", Projects::Project::GetProjectDirectory().string());
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Select Starting Scene
		EditorUI::Editor::SelectOption(s_SelectStartSceneSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Default Fullscreen
		s_DefaultFullscreenSpec.ToggleBoolean = Projects::Project::GetIsFullscreen();
		EditorUI::Editor::Checkbox(s_DefaultFullscreenSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Networking Checkbox
		s_ToggleNetworkSpec.ToggleBoolean = Projects::Project::GetAppIsNetworked();
		EditorUI::Editor::Checkbox(s_ToggleNetworkSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Select Starting Game State
		EditorUI::Editor::SelectOption(s_SelectStartGameStateSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Resolution Select Option
		EditorUI::Editor::SelectOption(s_SelectResolutionSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Runtime Start
		EditorUI::Editor::SelectOption(s_SelectRuntimeStartSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Update User Count
		EditorUI::Editor::SelectOption(s_SelectUpdateUserCountSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Approve Join Session
		EditorUI::Editor::SelectOption(s_SelectApproveJoinSessionSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Select On User Left Session
		EditorUI::Editor::SelectOption(s_SelectUserLeftSessionSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Current Session Start
		EditorUI::Editor::SelectOption(s_SelectSessionInitSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Connection Terminated
		EditorUI::Editor::SelectOption(s_SelectConnectionTerminatedSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Update Session User Slot
		EditorUI::Editor::SelectOption(s_SelectUpdateSessionSlotSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Start Session
		EditorUI::Editor::SelectOption(s_SelectStartSessionSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Session Ready Check
		s_SelectSessionReadyCheckSpec.CurrentOption = { Projects::Project::GetProjectOnSessionReadyCheckConfirm(), Assets::EmptyHandle };
		EditorUI::Editor::SelectOption(s_SelectSessionReadyCheckSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);

		// Select On Receive Signal Function
		s_SelectReceiveSignalSpec.CurrentOption = { Projects::Project::GetProjectOnReceiveSignal(), Assets::EmptyHandle };
		EditorUI::Editor::SelectOption(s_SelectReceiveSignalSpec);
		EditorUI::Editor::Spacing(EditorUI::SpacingAmount::Small);


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
		if (ImGui::ImageButton((ImTextureID)(uint64_t)EditorUI::Editor::s_IconSettings->GetRendererID(), ImVec2(17, 17), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
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
				auto& generators = Projects::Project::GetAppTickGenerators();

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

			for (auto& generatorValue : Projects::Project::GetAppTickGenerators())
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
					if (ImGui::ImageButton(("Delete Generator##AppTickGeneratorDelete" + std::to_string(generatorValue)).c_str(), (ImTextureID)(uint64_t)EditorUI::Editor::s_IconDelete->GetRendererID(),
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
			auto& generators = Projects::Project::GetAppTickGenerators();
			generators.erase(generatorToDelete);
		}

		ImGui::NewLine();


		EditorUI::Editor::EndWindow();
	}
}
