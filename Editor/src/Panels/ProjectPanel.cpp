#include "Panels/ProjectPanel.h"

#include "Kargono.h"

namespace Kargono
{
	static UI::SelectOptionSpec s_SelectResolutionSpec {};
	static UI::SimpleCheckboxSpec ToggleNetworkSpec {};

	void InitializeStaticResources()
	{
		// Networking Specification
		ToggleNetworkSpec.Label = "Networking";
		ToggleNetworkSpec.WidgetID = 0xef051e3d9bce4b41;
		ToggleNetworkSpec.ConfirmAction = [](bool value)
		{
			Projects::Project::SetAppIsNetworked(value);
		};

		// Resolution Specification
		s_SelectResolutionSpec.Label = "Target Resolution";
		s_SelectResolutionSpec.WidgetID = 0x46a8cd8fbde44223;
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
	}

	ProjectPanel::ProjectPanel()
	{
		InitializeStaticResources();
	}
	void ProjectPanel::OnEditorUIRender()
	{
		ImGui::Begin("Project");
		ImGui::TextUnformatted("Project Name:");
		ImGui::Separator();
		ImGui::Text(Projects::Project::GetProjectName().c_str());
		ImGui::NewLine();

		ImGui::TextUnformatted("Project Directory:");
		ImGui::Separator();
		ImGui::Text(Projects::Project::GetProjectDirectory().string().c_str());
		ImGui::NewLine();

		ImGui::TextUnformatted("Starting Scene:");
		ImGui::Separator();
		if (ImGui::BeginCombo("##Select Starting Scene", Projects::Project::GetStartScenePath(false).string().c_str()))
		{
			for (auto& [uuid, asset] : Assets::AssetManager::GetSceneRegistry())
			{
				if (ImGui::Selectable(asset.Data.IntermediateLocation.string().c_str()))
				{
					Projects::Project::SetStartingScene(uuid, asset.Data.IntermediateLocation);
				}
			}
			ImGui::EndCombo();
		}
		ImGui::NewLine();

		ImGui::TextUnformatted("Default Game Fullscreen:");
		ImGui::Separator();
		bool projectFullscreen = Projects::Project::GetIsFullscreen();
		if (ImGui::Checkbox("Default Game Fullscreen", &projectFullscreen))
		{
			Projects::Project::SetIsFullscreen(projectFullscreen);
		}
		ImGui::NewLine();

		// Networking Checkbox
		ToggleNetworkSpec.ToggleBoolean = Projects::Project::GetAppIsNetworked();
		UI::Editor::SimpleCheckbox(ToggleNetworkSpec);
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		// Resolution Select Option
		s_SelectResolutionSpec.CurrentOption = Utility::ScreenResolutionToString(Projects::Project::GetTargetResolution());
		UI::Editor::SelectOption(s_SelectResolutionSpec);
		UI::Editor::Spacing(UI::SpacingAmount::Small);

		ImGui::TextUnformatted("OnRuntimeStart Function:");
		ImGui::Separator();
		if (ImGui::BeginCombo("##ProjectOnRuntimeStart", Projects::Project::GetProjectOnRuntimeStart().c_str()))
		{
			if (ImGui::Selectable("None"))
			{
				Projects::Project::SetProjectOnRuntimeStart("None");
			}

			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				if (ImGui::Selectable(name.c_str()))
				{
					Projects::Project::SetProjectOnRuntimeStart(name);
				}
			}

			ImGui::EndCombo();
		}
		ImGui::NewLine();

		ImGui::TextUnformatted("OnUpdateUserCount Function:");
		ImGui::Separator();
		if (ImGui::BeginCombo("##ProjectOnUpdateUserCount", Projects::Project::GetProjectOnUpdateUserCount().c_str()))
		{
			if (ImGui::Selectable("None"))
			{
				Projects::Project::SetProjectOnUpdateUserCount("None");
			}

			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				if (ImGui::Selectable(name.c_str()))
				{
					Projects::Project::SetProjectOnUpdateUserCount(name);
				}
			}

			ImGui::EndCombo();
		}
		ImGui::NewLine();

		ImGui::TextUnformatted("OnApproveJoinSession Function:");
		ImGui::Separator();
		if (ImGui::BeginCombo("##ProjectOnApproveJoinSession", Projects::Project::GetProjectOnApproveJoinSession().c_str()))
		{
			if (ImGui::Selectable("None"))
			{
				Projects::Project::SetProjectOnApproveJoinSession("None");
			}

			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				if (ImGui::Selectable(name.c_str()))
				{
					Projects::Project::SetProjectOnApproveJoinSession(name);
				}
			}

			ImGui::EndCombo();
		}
		ImGui::NewLine();

		ImGui::TextUnformatted("OnUserLeftSession Function:");
		ImGui::Separator();
		if (ImGui::BeginCombo("##ProjectOnUserLeftSession", Projects::Project::GetProjectOnUserLeftSession().c_str()))
		{
			if (ImGui::Selectable("None"))
			{
				Projects::Project::SetProjectOnUserLeftSession("None");
			}

			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				if (ImGui::Selectable(name.c_str()))
				{
					Projects::Project::SetProjectOnUserLeftSession(name);
				}
			}

			ImGui::EndCombo();
		}
		ImGui::NewLine();

		ImGui::TextUnformatted("OnCurrentSessionInit Function:");
		ImGui::Separator();
		if (ImGui::BeginCombo("##ProjectOnCurrentSessionInit", Projects::Project::GetProjectOnCurrentSessionInit().c_str()))
		{
			if (ImGui::Selectable("None"))
			{
				Projects::Project::SetProjectOnCurrentSessionInit("None");
			}

			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				if (ImGui::Selectable(name.c_str()))
				{
					Projects::Project::SetProjectOnCurrentSessionInit(name);
				}
			}

			ImGui::EndCombo();
		}
		ImGui::NewLine();

		ImGui::TextUnformatted("OnConnectionTerminated Function:");
		ImGui::Separator();
		if (ImGui::BeginCombo("##ProjectOnConnectionTerminated", Projects::Project::GetProjectOnConnectionTerminated().c_str()))
		{
			if (ImGui::Selectable("None"))
			{
				Projects::Project::SetProjectOnConnectionTerminated("None");
			}

			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				if (ImGui::Selectable(name.c_str()))
				{
					Projects::Project::SetProjectOnConnectionTerminated(name);
				}
			}

			ImGui::EndCombo();
		}
		ImGui::NewLine();

		ImGui::TextUnformatted("OnUpdateSessionUserSlot Function:");
		ImGui::Separator();
		if (ImGui::BeginCombo("##ProjectOnUpdateSessionUserSlot", Projects::Project::GetProjectOnUpdateSessionUserSlot().c_str()))
		{
			if (ImGui::Selectable("None"))
			{
				Projects::Project::SetProjectOnUpdateSessionUserSlot("None");
			}

			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				if (ImGui::Selectable(name.c_str()))
				{
					Projects::Project::SetProjectOnUpdateSessionUserSlot(name);
				}
			}

			ImGui::EndCombo();
		}
		ImGui::NewLine();

		ImGui::TextUnformatted("OnStartSession Function:");
		ImGui::Separator();
		if (ImGui::BeginCombo("##ProjectOnStartSession", Projects::Project::GetProjectOnStartSession().c_str()))
		{
			if (ImGui::Selectable("None"))
			{
				Projects::Project::SetProjectOnStartSession("None");
			}

			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				if (ImGui::Selectable(name.c_str()))
				{
					Projects::Project::SetProjectOnStartSession(name);
				}
			}

			ImGui::EndCombo();
		}
		ImGui::NewLine();

		ImGui::TextUnformatted("OnSessionReadyCheckConfirm Function:");
		ImGui::Separator();
		if (ImGui::BeginCombo("##ProjectOnSessionReadyCheckConfirm", Projects::Project::GetProjectOnSessionReadyCheckConfirm().c_str()))
		{
			if (ImGui::Selectable("None"))
			{
				Projects::Project::SetProjectOnSessionReadyCheckConfirm("None");
			}

			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				if (ImGui::Selectable(name.c_str()))
				{
					Projects::Project::SetProjectOnSessionReadyCheckConfirm(name);
				}
			}

			ImGui::EndCombo();
		}
		ImGui::NewLine();

		ImGui::TextUnformatted("OnReceiveSignal Function:");
		ImGui::Separator();
		if (ImGui::BeginCombo("##ProjectOnReceiveSignal", Projects::Project::GetProjectOnReceiveSignal().c_str()))
		{
			if (ImGui::Selectable("None"))
			{
				Projects::Project::SetProjectOnReceiveSignal("None");
			}

			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				if (ImGui::Selectable(name.c_str()))
				{
					Projects::Project::SetProjectOnReceiveSignal(name);
				}
			}

			ImGui::EndCombo();
		}
		ImGui::NewLine();




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


		ImGui::End();
	}
}
