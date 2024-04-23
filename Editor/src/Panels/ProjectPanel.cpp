#include "Panels/ProjectPanel.h"

#include "Kargono.h"
#include "EditorLayer.h"

namespace Kargono
{
	static EditorLayer* s_EditorLayer { nullptr };

	static UI::SelectOptionSpec s_SelectStartSceneSpec {};
	static UI::CheckboxSpec s_DefaultFullscreenSpec {};
	static UI::CheckboxSpec s_ToggleNetworkSpec {};
	static UI::SelectOptionSpec s_SelectResolutionSpec {};
	static UI::SelectOptionSpec s_SelectRuntimeStartSpec {};
	static UI::SelectOptionSpec s_SelectUpdateUserCountSpec {};
	static UI::SelectOptionSpec s_SelectApproveJoinSessionSpec {};
	static UI::SelectOptionSpec s_SelectUserLeftSessionSpec {};
	static UI::SelectOptionSpec s_SelectSessionInitSpec {};
	static UI::SelectOptionSpec s_SelectConnectionTerminatedSpec {};
	static UI::SelectOptionSpec s_SelectUpdateSessionSlotSpec {};
	static UI::SelectOptionSpec s_SelectStartSessionSpec {};
	static UI::SelectOptionSpec s_SelectSessionReadyCheckSpec {};
	static UI::SelectOptionSpec s_SelectReceiveSignalSpec {};

	void InitializeStaticResources()
	{
		// Resolution Specification
		s_SelectStartSceneSpec.Label = "Starting Scene";
		s_SelectStartSceneSpec.WidgetID = 0x75ebbc8750034f81;
		s_SelectStartSceneSpec.LineCount = 2;
		s_SelectStartSceneSpec.PopupAction = [](UI::SelectOptionSpec& spec)
		{
			spec.GetOptionsList().clear();
			for (auto& [handle, asset] : Assets::AssetManager::GetSceneRegistry())
			{
				spec.AddToOptionsList("All Options", asset.Data.IntermediateLocation.string());
			}
		};
		s_SelectStartSceneSpec.ConfirmAction = [&](const std::string& selection)
		{
			for (auto& [handle, asset] : Assets::AssetManager::GetSceneRegistry())
			{
				if (asset.Data.IntermediateLocation == std::filesystem::path(selection))
				{
					Projects::Project::SetStartingScene(handle, asset.Data.IntermediateLocation);
					return;
				}
			}
			KG_ERROR("Could not locate starting scene in ProjectPanel");
			
		};

		// Default Full Screen
		s_DefaultFullscreenSpec.Label = "Default Fullscreen";
		s_DefaultFullscreenSpec.WidgetID = 0x3c6130198495483f;
		s_DefaultFullscreenSpec.ConfirmAction = [](bool value)
		{
			Projects::Project::SetIsFullscreen(value);
		};

		// Set Networking Specification
		s_ToggleNetworkSpec.Label = "Networking";
		s_ToggleNetworkSpec.WidgetID = 0xef051e3d9bce4b41;
		s_ToggleNetworkSpec.ConfirmAction = [](bool value)
		{
			Projects::Project::SetAppIsNetworked(value);
		};

		// Resolution Specification
		s_SelectResolutionSpec.Label = "Target Resolution";
		s_SelectResolutionSpec.WidgetID = 0x46a8cd8fbde44223;
		s_SelectResolutionSpec.LineCount = 4;
		s_SelectResolutionSpec.ConfirmAction = [&](const std::string& selection)
		{
			Projects::Project::SetTargetResolution(Utility::StringToScreenResolution(selection));
		};
		s_SelectResolutionSpec.AddToOptionsList("Aspect Ratio: 1:1 (Box)", "800x800");
		s_SelectResolutionSpec.AddToOptionsList("Aspect Ratio: 1:1 (Box)", "400x400");
		s_SelectResolutionSpec.AddToOptionsList("Aspect Ratio: 16:9 (Widescreen)", "1920x1080");
		s_SelectResolutionSpec.AddToOptionsList("Aspect Ratio: 16:9 (Widescreen)", "1600x900");
		s_SelectResolutionSpec.AddToOptionsList("Aspect Ratio: 16:9 (Widescreen)", "1366x768");
		s_SelectResolutionSpec.AddToOptionsList("Aspect Ratio: 16:9 (Widescreen)", "1280x720");
		s_SelectResolutionSpec.AddToOptionsList("Aspect Ratio: 4:3 (Fullscreen)", "1600x1200");
		s_SelectResolutionSpec.AddToOptionsList("Aspect Ratio: 4:3 (Fullscreen)", "1280x960");
		s_SelectResolutionSpec.AddToOptionsList("Aspect Ratio: 4:3 (Fullscreen)", "1152x864");
		s_SelectResolutionSpec.AddToOptionsList("Aspect Ratio: 4:3 (Fullscreen)", "1024x768");
		s_SelectResolutionSpec.AddToOptionsList("Aspect Ratio: Automatic (Based on Device Used)", "Match Device");

		// Runtime Start Spec
		s_SelectRuntimeStartSpec.Label = "Runtime Start";
		s_SelectRuntimeStartSpec.WidgetID = 0xe8527560bb194a06;
		s_SelectRuntimeStartSpec.LineCount = 3;
		s_SelectRuntimeStartSpec.PopupAction = [](UI::SelectOptionSpec& spec)
		{
			spec.GetOptionsList().clear();

			spec.AddToOptionsList("Clear", "None");
			for (auto& [id, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_None)
				{
					spec.AddToOptionsList("All Options", script->m_ScriptName);
				}
			}
		};
		s_SelectRuntimeStartSpec.ConfirmAction = [&](const std::string& selection)
		{
			if (selection == "None")
			{
				Projects::Project::SetOnRuntimeStart(0);
				return;
			}

			for (auto& [id, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_ScriptName == selection)
				{
					Projects::Project::SetOnRuntimeStart(id);
					return;
				}
			}
			KG_ERROR("Could not locate runtime start function in ProjectPanel");
		};

		// Update User Count Spec
		s_SelectUpdateUserCountSpec.Label = "Update User Count";
		s_SelectUpdateUserCountSpec.WidgetID = 0x329248441b45489b;
		s_SelectUpdateUserCountSpec.LineCount = 3;
		s_SelectUpdateUserCountSpec.PopupAction = [](UI::SelectOptionSpec& spec)
		{
			spec.GetOptionsList().clear();

			spec.AddToOptionsList("Clear", "None");
			for (auto& [id, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_UInt32)
				{
					spec.AddToOptionsList("All Options", script->m_ScriptName);
				}
			}
		};
		s_SelectUpdateUserCountSpec.ConfirmAction = [&](const std::string& selection)
		{
			if (selection == "None")
			{
				Projects::Project::SetOnUpdateUserCount(0);
				return;
			}

			for (auto& [id, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_ScriptName == selection)
				{
					Projects::Project::SetOnUpdateUserCount(id);
					return;
				}
			}
			KG_ERROR("Could not locate update user count function in ProjectPanel");

		};

		// Update Approve Join Session
		s_SelectApproveJoinSessionSpec.Label = "Approve Join Session";
		s_SelectApproveJoinSessionSpec.WidgetID = 0xb1c13beef2044d2b;
		s_SelectApproveJoinSessionSpec.LineCount = 3;
		s_SelectApproveJoinSessionSpec.PopupAction = [](UI::SelectOptionSpec& spec)
		{
			spec.GetOptionsList().clear();

			spec.AddToOptionsList("Clear", "None");
			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				spec.AddToOptionsList("All Options", name);
			}
		};
		s_SelectApproveJoinSessionSpec.ConfirmAction = [&](const std::string& selection)
		{
			if (selection == "None")
			{
				Projects::Project::SetProjectOnApproveJoinSession("None");
				return;
			}

			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				if (name == selection)
				{
					Projects::Project::SetProjectOnApproveJoinSession(name);
					return;
				}
			}
			KG_ERROR("Could not locate approve join session function in ProjectPanel");
		};

		// Update User Left Session
		s_SelectUserLeftSessionSpec.Label = "User Left Session";
		s_SelectUserLeftSessionSpec.WidgetID = 0x98f9288e1cd04962;
		s_SelectUserLeftSessionSpec.LineCount = 3;
		s_SelectUserLeftSessionSpec.PopupAction = [](UI::SelectOptionSpec& spec)
		{
			spec.GetOptionsList().clear();

			spec.AddToOptionsList("Clear", "None");
			for (auto& [id, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_UInt16)
				{
					spec.AddToOptionsList("All Options", script->m_ScriptName);
				}
			}
		};
		s_SelectUserLeftSessionSpec.ConfirmAction = [&](const std::string& selection)
		{
			if (selection == "None")
			{
				Projects::Project::SetOnUserLeftSession(0);
				return;
			}

			for (auto& [id, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_ScriptName == selection)
				{
					Projects::Project::SetOnUserLeftSession(id);
					return;
				}
			}
			KG_ERROR("Could not locate user left session function in ProjectPanel");
		};

		// Update Start Session
		s_SelectSessionInitSpec.Label = "Session Initialization";
		s_SelectSessionInitSpec.WidgetID = 0xe8dbc36fd6774a8f;
		s_SelectSessionInitSpec.LineCount = 3;
		s_SelectSessionInitSpec.PopupAction = [](UI::SelectOptionSpec& spec)
		{
			spec.GetOptionsList().clear();

			spec.AddToOptionsList("Clear", "None");
			for (auto& [id, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_None)
				{
					spec.AddToOptionsList("All Options", script->m_ScriptName);
				}
			}
		};
		s_SelectSessionInitSpec.ConfirmAction = [&](const std::string& selection)
		{
			if (selection == "None")
			{
				Projects::Project::SetOnCurrentSessionInit(0);
				return;
			}

			for (auto& [id, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_ScriptName == selection)
				{
					Projects::Project::SetOnCurrentSessionInit(id);
					return;
				}
			}
			KG_ERROR("Could not locate current session function in ProjectPanel");
		};

		// Update Connection Terminated
		s_SelectConnectionTerminatedSpec.Label = "Connection Terminated";
		s_SelectConnectionTerminatedSpec.WidgetID = 0xad3c8300fda24bb7;
		s_SelectConnectionTerminatedSpec.LineCount = 3;
		s_SelectConnectionTerminatedSpec.PopupAction = [](UI::SelectOptionSpec& spec)
		{
			spec.GetOptionsList().clear();

			spec.AddToOptionsList("Clear", "None");
			for (auto& [id, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_None)
				{
					spec.AddToOptionsList("All Options", script->m_ScriptName);
				}
			}
		};
		s_SelectConnectionTerminatedSpec.ConfirmAction = [&](const std::string& selection)
		{
			if (selection == "None")
			{
				Projects::Project::SetOnConnectionTerminated(0);
				return;
			}

			for (auto& [id, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_ScriptName == selection)
				{
					Projects::Project::SetOnConnectionTerminated(id);
					return;
				}
			}
			KG_ERROR("Could not locate connection terminated function in ProjectPanel");
		};

		// Update Session User Slot
		s_SelectUpdateSessionSlotSpec.Label = "Update Session User Slot";
		s_SelectUpdateSessionSlotSpec.WidgetID = 0x182e65506c2843f4;
		s_SelectUpdateSessionSlotSpec.LineCount = 3;
		s_SelectUpdateSessionSlotSpec.PopupAction = [](UI::SelectOptionSpec& spec)
		{
			spec.GetOptionsList().clear();

			spec.AddToOptionsList("Clear", "None");
			for (auto& [id, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_UInt16)
				{
					spec.AddToOptionsList("All Options", script->m_ScriptName);
				}
			}
		};
		s_SelectUpdateSessionSlotSpec.ConfirmAction = [&](const std::string& selection)
		{
			if (selection == "None")
			{
				Projects::Project::SetOnUpdateSessionUserSlot(0);
				return;
			}

			for (auto& [id, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_ScriptName == selection)
				{
					Projects::Project::SetOnUpdateSessionUserSlot(id);
					return;
				}
			}
			KG_ERROR("Could not locate update session user slot function in ProjectPanel");
		};

		// Update On Start Session
		s_SelectStartSessionSpec.Label = "Start Session";
		s_SelectStartSessionSpec.WidgetID = 0xdda2adc6a5f44e58;
		s_SelectStartSessionSpec.LineCount = 3;
		s_SelectStartSessionSpec.PopupAction = [](UI::SelectOptionSpec& spec)
		{
			spec.GetOptionsList().clear();

			spec.AddToOptionsList("Clear", "None");
			for (auto& [id, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_None)
				{
					spec.AddToOptionsList("All Options", script->m_ScriptName);
				}
			}
		};
		s_SelectStartSessionSpec.ConfirmAction = [&](const std::string& selection)
		{
			if (selection == "None")
			{
				Projects::Project::SetOnStartSession(0);
				return;
			}

			for (auto& [id, script] : Assets::AssetManager::GetScriptMap())
			{
				if (script->m_ScriptName == selection)
				{
					Projects::Project::SetOnStartSession(id);
					return;
				}
			}
			KG_ERROR("Could not locate start session function in ProjectPanel");
		};

		// Update On Session Ready Check
		s_SelectSessionReadyCheckSpec.Label = "Session Ready Check";
		s_SelectSessionReadyCheckSpec.WidgetID = 0x352214211be04ab5;
		s_SelectSessionReadyCheckSpec.LineCount = 3;
		s_SelectSessionReadyCheckSpec.PopupAction = [](UI::SelectOptionSpec& spec)
		{
			spec.GetOptionsList().clear();

			spec.AddToOptionsList("Clear", "None");
			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				spec.AddToOptionsList("All Options", name);
			}
		};
		s_SelectSessionReadyCheckSpec.ConfirmAction = [&](const std::string& selection)
		{
			if (selection == "None")
			{
				Projects::Project::SetProjectOnSessionReadyCheckConfirm("None");
				return;
			}

			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				if (name == selection)
				{
					Projects::Project::SetProjectOnSessionReadyCheckConfirm(name);
					return;
				}
			}
			KG_ERROR("Could not locate session ready check function in ProjectPanel");
		};

		// Update On Receive Signal
		s_SelectReceiveSignalSpec.Label = "Receive Signal";
		s_SelectReceiveSignalSpec.WidgetID = 0x97a9f99e14df4139;
		s_SelectReceiveSignalSpec.LineCount = 3;
		s_SelectReceiveSignalSpec.PopupAction = [](UI::SelectOptionSpec& spec)
		{
			spec.GetOptionsList().clear();

			spec.AddToOptionsList("Clear", "None");
			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				spec.AddToOptionsList("All Options", name);
			}
		};
		s_SelectReceiveSignalSpec.ConfirmAction = [&](const std::string& selection)
		{
			if (selection == "None")
			{
				Projects::Project::SetProjectOnReceiveSignal("None");
				return;
			}

			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				if (name == selection)
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
		UI::Editor::StartWindow("Project", &s_EditorLayer->m_ShowProject);
		// Project Name
		UI::Editor::LabeledText("Project Name", Projects::Project::GetProjectName());
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		// Project Directory
		UI::Editor::LabeledText("Project Directory", Projects::Project::GetProjectDirectory().string());
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		// Select Starting Scene
		s_SelectStartSceneSpec.CurrentOption = Projects::Project::GetStartScenePath(false).string();
		UI::Editor::SelectOptionInline(s_SelectStartSceneSpec);
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		// Default Fullscreen
		s_DefaultFullscreenSpec.ToggleBoolean = Projects::Project::GetIsFullscreen();;
		UI::Editor::Checkbox(s_DefaultFullscreenSpec);
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		// Networking Checkbox
		s_ToggleNetworkSpec.ToggleBoolean = Projects::Project::GetAppIsNetworked();
		UI::Editor::Checkbox(s_ToggleNetworkSpec);
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		// Resolution Select Option
		s_SelectResolutionSpec.CurrentOption = Utility::ScreenResolutionToString(Projects::Project::GetTargetResolution());
		UI::Editor::SelectOptionInline(s_SelectResolutionSpec);
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		// Select On Runtime Start
		s_SelectRuntimeStartSpec.CurrentOption = Projects::Project::GetOnRuntimeStart() ?
			Assets::AssetManager::GetScript(Projects::Project::GetOnRuntimeStart())->m_ScriptName : "None";
		UI::Editor::SelectOptionInline(s_SelectRuntimeStartSpec);
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		// Select On Update User Count
		s_SelectUpdateUserCountSpec.CurrentOption = Projects::Project::GetOnUpdateUserCount() ?
			Assets::AssetManager::GetScript(Projects::Project::GetOnUpdateUserCount())->m_ScriptName : "None";
		UI::Editor::SelectOptionInline(s_SelectUpdateUserCountSpec);
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		// Select On Approve Join Session
		s_SelectApproveJoinSessionSpec.CurrentOption = Projects::Project::GetProjectOnApproveJoinSession();
		UI::Editor::SelectOptionInline(s_SelectApproveJoinSessionSpec);
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		// Select On User Left Session
		s_SelectUserLeftSessionSpec.CurrentOption = Projects::Project::GetOnUserLeftSession() ?
			Assets::AssetManager::GetScript(Projects::Project::GetOnUserLeftSession())->m_ScriptName : "None";
		UI::Editor::SelectOptionInline(s_SelectUserLeftSessionSpec);
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		// Select On Current Session Start
		s_SelectSessionInitSpec.CurrentOption = Projects::Project::GetOnCurrentSessionInit() ?
			Assets::AssetManager::GetScript(Projects::Project::GetOnCurrentSessionInit())->m_ScriptName : "None";
		UI::Editor::SelectOptionInline(s_SelectSessionInitSpec);
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		// Select On Connection Terminated
		s_SelectConnectionTerminatedSpec.CurrentOption = Projects::Project::GetOnConnectionTerminated() ?
			Assets::AssetManager::GetScript(Projects::Project::GetOnConnectionTerminated())->m_ScriptName : "None";
		UI::Editor::SelectOptionInline(s_SelectConnectionTerminatedSpec);
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		// Select On Update Session User Slot
		s_SelectUpdateSessionSlotSpec.CurrentOption = Projects::Project::GetOnUpdateSessionUserSlot() ?
			Assets::AssetManager::GetScript(Projects::Project::GetOnUpdateSessionUserSlot())->m_ScriptName : "None";
		UI::Editor::SelectOptionInline(s_SelectUpdateSessionSlotSpec);
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		// Select On Start Session
		s_SelectStartSessionSpec.CurrentOption = Projects::Project::GetOnStartSession() ?
			Assets::AssetManager::GetScript(Projects::Project::GetOnStartSession())->m_ScriptName : "None";
		
		UI::Editor::SelectOptionInline(s_SelectStartSessionSpec);
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		// Select On Session Ready Check
		s_SelectSessionReadyCheckSpec.CurrentOption = Projects::Project::GetProjectOnSessionReadyCheckConfirm();
		UI::Editor::SelectOptionInline(s_SelectSessionReadyCheckSpec);
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		// Select On Receive Signal Function
		s_SelectReceiveSignalSpec.CurrentOption = Projects::Project::GetProjectOnReceiveSignal();
		UI::Editor::SelectOptionInline(s_SelectReceiveSignalSpec);
		UI::Editor::Spacing(UI::SpacingAmount::Small);


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
		if (ImGui::ImageButton((ImTextureID)(uint64_t)UI::Editor::s_IconSettings->GetRendererID(), ImVec2(17, 17), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
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
					if (ImGui::ImageButton(("Delete Generator##AppTickGeneratorDelete" + std::to_string(generatorValue)).c_str(), (ImTextureID)(uint64_t)UI::Editor::s_IconDelete->GetRendererID(),
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


		UI::Editor::EndWindow();
	}
}
