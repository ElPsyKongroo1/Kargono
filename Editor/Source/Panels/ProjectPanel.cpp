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

		// Check if an active scene handle is valid
		Assets::AssetHandle startSceneHandle = Projects::ProjectService::GetActiveStartSceneHandle();
		if (startSceneHandle == Assets::EmptyHandle || !Assets::AssetService::HasScene(startSceneHandle))
		{
			// Ensure some scenes are available
			KG_ASSERT(Assets::AssetService::GetSceneRegistrySize() != 0);
			
			// Set the start scene to a random available scene
			const Assets::AssetRegistry& sceneRegistry = Assets::AssetService::GetSceneRegistry();
			auto iter = sceneRegistry.begin();
			KG_ASSERT(iter != sceneRegistry.end());

			startSceneHandle = iter->first;
		}
		
		m_SelectStartSceneSpec.CurrentOption = {
			 Assets::AssetService::GetSceneRegistry().at(startSceneHandle).Data.FileLocation.string(),
			startSceneHandle};
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

			const Assets::AssetInfo asset = Assets::AssetService::GetSceneRegistry().at(entry.Handle);
			Projects::ProjectService::SetActiveStartingSceneHandle(entry.Handle);
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
		if (Projects::ProjectService::GetActiveStartGameStateHandle() != 0)
		{
			m_SelectStartGameStateSpec.CurrentOption = { Assets::AssetService::GetGameStateRegistry().at
			(Projects::ProjectService::GetActiveStartGameStateHandle()).Data.FileLocation.string(),
			Projects::ProjectService::GetActiveStartGameStateHandle()};
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

			if (Projects::ProjectService::GetActiveStartGameStateHandle() != Assets::EmptyHandle)
			{
				m_SelectStartGameStateSpec.CurrentOption = { Assets::AssetService::GetGameStateRegistry().at
				(Projects::ProjectService::GetActiveStartGameStateHandle()).Data.FileLocation.string(),
				Projects::ProjectService::GetActiveStartGameStateHandle() };
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
			Projects::ProjectService::SetActiveStartGameStateHandle(selection.Handle);

		};

		// Runtime Start Spec
		m_SelectRuntimeStartSpec.Label = "Runtime Start";
		m_SelectRuntimeStartSpec.Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectRuntimeStartSpec.LineCount = 3;
		m_SelectRuntimeStartSpec.CurrentOption = { Projects::ProjectService::GetActiveOnRuntimeStartHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnRuntimeStartHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnRuntimeStartHandle()};
		m_SelectRuntimeStartSpec.PopupAction = [&]()
		{
			m_SelectRuntimeStartSpec.ClearOptions();

			m_SelectRuntimeStartSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				KG_ASSERT(handle != Assets::EmptyHandle);
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}

				m_SelectRuntimeStartSpec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}

			m_SelectRuntimeStartSpec.CurrentOption = { Projects::ProjectService::GetActiveOnRuntimeStartHandle() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnRuntimeStartHandle())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnRuntimeStartHandle() };
		};
		m_SelectRuntimeStartSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find runtime start function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnRuntimeStartHandle(selection.Handle);
		};
		m_SelectRuntimeStartSpec.OnEdit = [&]() 
		{
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&] (EditorUI::TooltipEntry& entry) 
			{
				m_SelectRuntimeStartSpec.OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
				// Open create script dialog in script editor
				s_EditorApp->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
				{
					// Ensure handle provides a script in the registry
					if (!Assets::AssetService::HasScript(scriptHandle))
					{
						KG_WARN("Could not find runtime start function in Project Panel");
						return;
					}

					// Ensure function type matches definition
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
					if (script->m_FuncType != WrappedFuncType::Void_None)
					{
						KG_WARN("Incorrect function type returned when linking script to usage point");
						return;
					}

					// Fill the new script handle
					Projects::ProjectService::SetActiveOnRuntimeStartHandle(scriptHandle);
					m_SelectRuntimeStartSpec.CurrentOption = { script->m_ScriptName, scriptHandle };
				}, {});

			} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.TooltipActive = true;

		};


		// Update User Count Spec
		m_SelectUpdateUserCountSpec.Label = "Update User Count";
		m_SelectUpdateUserCountSpec.Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectUpdateUserCountSpec.LineCount = 3;
		m_SelectUpdateUserCountSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUpdateUserCountHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUpdateUserCountHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnUpdateUserCountHandle() };
		m_SelectUpdateUserCountSpec.PopupAction = [&]()
		{
			m_SelectUpdateUserCountSpec.ClearOptions();

			m_SelectUpdateUserCountSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				KG_ASSERT(handle != Assets::EmptyHandle);
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_UInt32)
				{
					continue;
				}

				m_SelectUpdateUserCountSpec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}

			m_SelectUpdateUserCountSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUpdateUserCountHandle() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUpdateUserCountHandle())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnUpdateUserCountHandle() };
		};
		m_SelectUpdateUserCountSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find Update User Count function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnUpdateUserCountHandle(selection.Handle);
		};
		m_SelectUpdateUserCountSpec.OnEdit = [&]()
		{
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				m_SelectUpdateUserCountSpec.OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
					// Open create script dialog in script editor
					s_EditorApp->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_UInt32, [&](Assets::AssetHandle scriptHandle)
					{
							// Ensure handle provides a script in the registry
							if (!Assets::AssetService::HasScript(scriptHandle))
							{
								KG_WARN("Could not find update user count function in Project Panel");
								return;
							}

							// Ensure function type matches definition
							Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
							if (script->m_FuncType != WrappedFuncType::Void_UInt32)
							{
								KG_WARN("Incorrect function type returned when linking script to usage point");
								return;
							}

							// Fill the new script handle
							Projects::ProjectService::SetActiveOnUpdateUserCountHandle(scriptHandle);
							m_SelectUpdateUserCountSpec.CurrentOption = { script->m_ScriptName, scriptHandle };
						}, {"userCount"});

					} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.TooltipActive = true;

		};


		// Approve Join Session Spec
		m_SelectApproveJoinSessionSpec.Label = "Approve Join Session";
		m_SelectApproveJoinSessionSpec.Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectApproveJoinSessionSpec.LineCount = 3;
		m_SelectApproveJoinSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnApproveJoinSessionHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnApproveJoinSessionHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnApproveJoinSessionHandle() };
		m_SelectApproveJoinSessionSpec.PopupAction = [&]()
		{
			m_SelectApproveJoinSessionSpec.ClearOptions();

			m_SelectApproveJoinSessionSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				KG_ASSERT(handle != Assets::EmptyHandle);
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_UInt16)
				{
					continue;
				}

				m_SelectApproveJoinSessionSpec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}

			m_SelectApproveJoinSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnApproveJoinSessionHandle() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnApproveJoinSessionHandle())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnApproveJoinSessionHandle() };
		};
		m_SelectApproveJoinSessionSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find Approve Join Session function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnApproveJoinSessionHandle(selection.Handle);
		};
		m_SelectApproveJoinSessionSpec.OnEdit = [&]()
		{
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				m_SelectApproveJoinSessionSpec.OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
					// Open create script dialog in script editor
					s_EditorApp->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_UInt16, [&](Assets::AssetHandle scriptHandle)
					{
							// Ensure handle provides a script in the registry
							if (!Assets::AssetService::HasScript(scriptHandle))
							{
								KG_WARN("Could not find Approve Join Session function in Project Panel");
								return;
							}

							// Ensure function type matches definition
							Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
							if (script->m_FuncType != WrappedFuncType::Void_UInt16)
							{
								KG_WARN("Incorrect function type returned when linking script to usage point");
								return;
							}

							// Fill the new script handle
							Projects::ProjectService::SetActiveOnApproveJoinSessionHandle(scriptHandle);
							m_SelectApproveJoinSessionSpec.CurrentOption = { script->m_ScriptName, scriptHandle };
						}, {"sessionSlot"});

					} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.TooltipActive = true;

		};

		// User Left Session Spec
		m_SelectUserLeftSessionSpec.Label = "User Left Session";
		m_SelectUserLeftSessionSpec.Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectUserLeftSessionSpec.LineCount = 3;
		m_SelectUserLeftSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUserLeftSessionHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUserLeftSessionHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnUserLeftSessionHandle() };
		m_SelectUserLeftSessionSpec.PopupAction = [&]()
			{
				m_SelectUserLeftSessionSpec.ClearOptions();

				m_SelectUserLeftSessionSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
				{
					KG_ASSERT(handle != Assets::EmptyHandle);
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
					if (script->m_FuncType != WrappedFuncType::Void_UInt16)
					{
						continue;
					}

					m_SelectUserLeftSessionSpec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
				}

				m_SelectUserLeftSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUserLeftSessionHandle() ?
					Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUserLeftSessionHandle())->m_ScriptName : "None",
					Projects::ProjectService::GetActiveOnUserLeftSessionHandle() };
			};
		m_SelectUserLeftSessionSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find User Left Session function in Project Panel");
					return;
				}
				Projects::ProjectService::SetActiveOnUserLeftSessionHandle(selection.Handle);
			};
		m_SelectUserLeftSessionSpec.OnEdit = [&]()
			{
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					m_SelectUserLeftSessionSpec.OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						// Open create script dialog in script editor
						s_EditorApp->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_UInt16, [&](Assets::AssetHandle scriptHandle)
						{
								// Ensure handle provides a script in the registry
								if (!Assets::AssetService::HasScript(scriptHandle))
								{
									KG_WARN("Could not find User Left Session function in Project Panel");
									return;
								}

								// Ensure function type matches definition
								Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
								if (script->m_FuncType != WrappedFuncType::Void_UInt16)
								{
									KG_WARN("Incorrect function type returned when linking script to usage point");
									return;
								}

								// Fill the new script handle
								Projects::ProjectService::SetActiveOnUserLeftSessionHandle(scriptHandle);
								m_SelectUserLeftSessionSpec.CurrentOption = { script->m_ScriptName, scriptHandle };
							}, {"sessionSlot"});

						} };
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.TooltipActive = true;

			};




		// Session Initialization Spec
		m_SelectSessionInitSpec.Label = "Session Initialization";
		m_SelectSessionInitSpec.Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectSessionInitSpec.LineCount = 3;
		m_SelectSessionInitSpec.CurrentOption = { Projects::ProjectService::GetActiveOnCurrentSessionInitHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnCurrentSessionInitHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnCurrentSessionInitHandle() };
		m_SelectSessionInitSpec.PopupAction = [&]()
			{
				m_SelectSessionInitSpec.ClearOptions();

				m_SelectSessionInitSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
				{
					KG_ASSERT(handle != Assets::EmptyHandle);
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
					if (script->m_FuncType != WrappedFuncType::Void_None)
					{
						continue;
					}

					m_SelectSessionInitSpec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
				}

				m_SelectSessionInitSpec.CurrentOption = { Projects::ProjectService::GetActiveOnCurrentSessionInitHandle() ?
					Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnCurrentSessionInitHandle())->m_ScriptName : "None",
					Projects::ProjectService::GetActiveOnCurrentSessionInitHandle() };
			};
		m_SelectSessionInitSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find Session Initialization function in Project Panel");
					return;
				}
				Projects::ProjectService::SetActiveOnCurrentSessionInitHandle(selection.Handle);
			};
		m_SelectSessionInitSpec.OnEdit = [&]()
			{
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					m_SelectSessionInitSpec.OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						// Open create script dialog in script editor
						s_EditorApp->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
						{
								// Ensure handle provides a script in the registry
								if (!Assets::AssetService::HasScript(scriptHandle))
								{
									KG_WARN("Could not find Session Initialization function in Project Panel");
									return;
								}

								// Ensure function type matches definition
								Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
								if (script->m_FuncType != WrappedFuncType::Void_None)
								{
									KG_WARN("Incorrect function type returned when linking script to usage point");
									return;
								}

								// Fill the new script handle
								Projects::ProjectService::SetActiveOnCurrentSessionInitHandle(scriptHandle);
								m_SelectSessionInitSpec.CurrentOption = { script->m_ScriptName, scriptHandle };
							}, {});

						} };
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.TooltipActive = true;

			};


		// Connection Terminated Spec
		m_SelectConnectionTerminatedSpec.Label = "Connection Terminated";
		m_SelectConnectionTerminatedSpec.Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectConnectionTerminatedSpec.LineCount = 3;
		m_SelectConnectionTerminatedSpec.CurrentOption = { Projects::ProjectService::GetActiveOnConnectionTerminatedHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnConnectionTerminatedHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnConnectionTerminatedHandle() };
		m_SelectConnectionTerminatedSpec.PopupAction = [&]()
		{
			m_SelectConnectionTerminatedSpec.ClearOptions();

			m_SelectConnectionTerminatedSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				KG_ASSERT(handle != Assets::EmptyHandle);
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}

				m_SelectConnectionTerminatedSpec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}

			m_SelectConnectionTerminatedSpec.CurrentOption = { Projects::ProjectService::GetActiveOnConnectionTerminatedHandle() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnConnectionTerminatedHandle())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnConnectionTerminatedHandle() };
		};
		m_SelectConnectionTerminatedSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find Connection Terminated function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnConnectionTerminatedHandle(selection.Handle);
		};
		m_SelectConnectionTerminatedSpec.OnEdit = [&]()
		{
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				m_SelectConnectionTerminatedSpec.OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
					// Open create script dialog in script editor
					s_EditorApp->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
					{
							// Ensure handle provides a script in the registry
							if (!Assets::AssetService::HasScript(scriptHandle))
							{
								KG_WARN("Could not find Connection Terminated function in Project Panel");
								return;
							}

							// Ensure function type matches definition
							Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
							if (script->m_FuncType != WrappedFuncType::Void_None)
							{
								KG_WARN("Incorrect function type returned when linking script to usage point");
								return;
							}

							// Fill the new script handle
							Projects::ProjectService::SetActiveOnConnectionTerminatedHandle(scriptHandle);
							m_SelectConnectionTerminatedSpec.CurrentOption = { script->m_ScriptName, scriptHandle };
						}, {});

					} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.TooltipActive = true;
		};

		// Update Session User Slot Spec
		m_SelectUpdateSessionSlotSpec.Label = "Update Session User Slot";
		m_SelectUpdateSessionSlotSpec.Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectUpdateSessionSlotSpec.LineCount = 3;
		m_SelectUpdateSessionSlotSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUpdateSessionUserSlotHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUpdateSessionUserSlotHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnUpdateSessionUserSlotHandle() };
		m_SelectUpdateSessionSlotSpec.PopupAction = [&]()
		{
			m_SelectUpdateSessionSlotSpec.ClearOptions();

			m_SelectUpdateSessionSlotSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				KG_ASSERT(handle != Assets::EmptyHandle);
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_UInt16)
				{
					continue;
				}

				m_SelectUpdateSessionSlotSpec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}

			m_SelectUpdateSessionSlotSpec.CurrentOption = { Projects::ProjectService::GetActiveOnUpdateSessionUserSlotHandle() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUpdateSessionUserSlotHandle())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnUpdateSessionUserSlotHandle() };
		};
		m_SelectUpdateSessionSlotSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find Update Session User Slot function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnUpdateSessionUserSlotHandle(selection.Handle);
		};
		m_SelectUpdateSessionSlotSpec.OnEdit = [&]()
		{
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				m_SelectUpdateSessionSlotSpec.OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
					// Open create script dialog in script editor
					s_EditorApp->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_UInt16, [&](Assets::AssetHandle scriptHandle)
					{
							// Ensure handle provides a script in the registry
							if (!Assets::AssetService::HasScript(scriptHandle))
							{
								KG_WARN("Could not find Update Session User Slot function in Project Panel");
								return;
							}

							// Ensure function type matches definition
							Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
							if (script->m_FuncType != WrappedFuncType::Void_UInt16)
							{
								KG_WARN("Incorrect function type returned when linking script to usage point");
								return;
							}

							// Fill the new script handle
							Projects::ProjectService::SetActiveOnUpdateSessionUserSlotHandle(scriptHandle);
							m_SelectUpdateSessionSlotSpec.CurrentOption = { script->m_ScriptName, scriptHandle };
						}, {"sessionSlot"});

					} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.TooltipActive = true;
		};


		// Start Session Spec
		m_SelectStartSessionSpec.Label = "Start Session";
		m_SelectStartSessionSpec.Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectStartSessionSpec.LineCount = 3;
		m_SelectStartSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnStartSessionHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnStartSessionHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnStartSessionHandle() };
		m_SelectStartSessionSpec.PopupAction = [&]()
			{
				m_SelectStartSessionSpec.ClearOptions();

				m_SelectStartSessionSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
				{
					KG_ASSERT(handle != Assets::EmptyHandle);
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
					if (script->m_FuncType != WrappedFuncType::Void_None)
					{
						continue;
					}

					m_SelectStartSessionSpec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
				}

				m_SelectStartSessionSpec.CurrentOption = { Projects::ProjectService::GetActiveOnStartSessionHandle() ?
					Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnStartSessionHandle())->m_ScriptName : "None",
					Projects::ProjectService::GetActiveOnStartSessionHandle() };
			};
		m_SelectStartSessionSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find Start Session function in Project Panel");
					return;
				}
				Projects::ProjectService::SetActiveOnStartSessionHandle(selection.Handle);
			};
		m_SelectStartSessionSpec.OnEdit = [&]()
			{
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					m_SelectStartSessionSpec.OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						// Open create script dialog in script editor
						s_EditorApp->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
						{
								// Ensure handle provides a script in the registry
								if (!Assets::AssetService::HasScript(scriptHandle))
								{
									KG_WARN("Could not find Start Session function in Project Panel");
									return;
								}

								// Ensure function type matches definition
								Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
								if (script->m_FuncType != WrappedFuncType::Void_None)
								{
									KG_WARN("Incorrect function type returned when linking script to usage point");
									return;
								}

								// Fill the new script handle
								Projects::ProjectService::SetActiveOnStartSessionHandle(scriptHandle);
								m_SelectStartSessionSpec.CurrentOption = { script->m_ScriptName, scriptHandle };
							}, {});

						} };
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.TooltipActive = true;
			};


		// Session Ready Check Spec
		m_SelectSessionReadyCheckSpec.Label = "Session Ready Check";
		m_SelectSessionReadyCheckSpec.Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectSessionReadyCheckSpec.LineCount = 3;
		m_SelectSessionReadyCheckSpec.CurrentOption = { Projects::ProjectService::GetActiveOnSessionReadyCheckConfirmHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnSessionReadyCheckConfirmHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnSessionReadyCheckConfirmHandle() };
		m_SelectSessionReadyCheckSpec.PopupAction = [&]()
			{
				m_SelectSessionReadyCheckSpec.ClearOptions();

				m_SelectSessionReadyCheckSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
				{
					KG_ASSERT(handle != Assets::EmptyHandle);
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
					if (script->m_FuncType != WrappedFuncType::Void_None)
					{
						continue;
					}

					m_SelectSessionReadyCheckSpec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
				}

				m_SelectSessionReadyCheckSpec.CurrentOption = { Projects::ProjectService::GetActiveOnSessionReadyCheckConfirmHandle() ?
					Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnSessionReadyCheckConfirmHandle())->m_ScriptName : "None",
					Projects::ProjectService::GetActiveOnSessionReadyCheckConfirmHandle() };
			};
		m_SelectSessionReadyCheckSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find Session Ready Check function in Project Panel");
					return;
				}
				Projects::ProjectService::SetActiveOnSessionReadyCheckConfirmHandle(selection.Handle);
			};
		m_SelectSessionReadyCheckSpec.OnEdit = [&]()
			{
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					m_SelectSessionReadyCheckSpec.OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						// Open create script dialog in script editor
						s_EditorApp->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
						{
								// Ensure handle provides a script in the registry
								if (!Assets::AssetService::HasScript(scriptHandle))
								{
									KG_WARN("Could not find Session Ready Check function in Project Panel");
									return;
								}

								// Ensure function type matches definition
								Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
								if (script->m_FuncType != WrappedFuncType::Void_None)
								{
									KG_WARN("Incorrect function type returned when linking script to usage point");
									return;
								}

								// Fill the new script handle
								Projects::ProjectService::SetActiveOnSessionReadyCheckConfirmHandle(scriptHandle);
								m_SelectSessionReadyCheckSpec.CurrentOption = { script->m_ScriptName, scriptHandle };
							}, {});

						} };
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.TooltipActive = true;
			};


		// On Receive Signal Spec
		m_SelectReceiveSignalSpec.Label = "On Receive Signal";
		m_SelectReceiveSignalSpec.Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectReceiveSignalSpec.LineCount = 3;
		m_SelectReceiveSignalSpec.CurrentOption = { Projects::ProjectService::GetActiveOnReceiveSignalHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnReceiveSignalHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnReceiveSignalHandle() };
		m_SelectReceiveSignalSpec.PopupAction = [&]()
			{
				m_SelectReceiveSignalSpec.ClearOptions();

				m_SelectReceiveSignalSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
				{
					KG_ASSERT(handle != Assets::EmptyHandle);
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
					if (script->m_FuncType != WrappedFuncType::Void_UInt16)
					{
						continue;
					}

					m_SelectReceiveSignalSpec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
				}

				m_SelectReceiveSignalSpec.CurrentOption = { Projects::ProjectService::GetActiveOnReceiveSignalHandle() ?
					Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnReceiveSignalHandle())->m_ScriptName : "None",
					Projects::ProjectService::GetActiveOnReceiveSignalHandle() };
			};
		m_SelectReceiveSignalSpec.ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.Handle) && selection.Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find On Receive Signal function in Project Panel");
					return;
				}
				Projects::ProjectService::SetActiveOnReceiveSignalHandle(selection.Handle);
			};
		m_SelectReceiveSignalSpec.OnEdit = [&]()
			{
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					m_SelectReceiveSignalSpec.OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						// Open create script dialog in script editor
						s_EditorApp->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_UInt16, [&](Assets::AssetHandle scriptHandle)
						{
								// Ensure handle provides a script in the registry
								if (!Assets::AssetService::HasScript(scriptHandle))
								{
									KG_WARN("Could not find On Receive Signal function in Project Panel");
									return;
								}

								// Ensure function type matches definition
								Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
								if (script->m_FuncType != WrappedFuncType::Void_UInt16)
								{
									KG_WARN("Incorrect function type returned when linking script to usage point");
									return;
								}

								// Fill the new script handle
								Projects::ProjectService::SetActiveOnReceiveSignalHandle(scriptHandle);
								m_SelectReceiveSignalSpec.CurrentOption = { script->m_ScriptName, scriptHandle };
							}, {"signal"});

						} };
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.TooltipActive = true;
			};

		m_SelectScriptTooltip.m_Label = "Script Tooltip";

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
			m_MessageTypeTable.ClearList();
			for (std::string& label : Projects::ProjectService::GetAllMessageTypes())
			{
				m_MessageTypeTable.InsertListEntry(label, "", [&](EditorUI::ListEntry& entry, std::size_t iteration)
				{
					m_ActiveAIMessageType = entry.Label;
					m_EditMessageTypePopup.OpenPopup = true;
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


		EditorUI::EditorUIService::List(m_MessageTypeTable);
		EditorUI::EditorUIService::EditText(m_CreateMessageTypePopup);
		EditorUI::EditorUIService::GenericPopup(m_EditMessageTypePopup);
		EditorUI::EditorUIService::Tooltip(m_SelectScriptTooltip);
		

		EditorUI::EditorUIService::EndWindow();
	}
	bool ProjectPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	bool ProjectPanel::OnAssetEvent(Events::Event* event)
	{
		Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;

		// Manage script deletion event
		if (manageAsset->GetAssetType() == Assets::AssetType::Script &&
			manageAsset->GetAction() == Events::ManageAssetAction::Delete)
		{
			if (m_SelectRuntimeStartSpec.CurrentOption.Handle == manageAsset->GetAssetID())
			{
				m_SelectRuntimeStartSpec.CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectUpdateUserCountSpec.CurrentOption.Handle == manageAsset->GetAssetID())
			{
				m_SelectUpdateUserCountSpec.CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectApproveJoinSessionSpec.CurrentOption.Handle == manageAsset->GetAssetID())
			{
				m_SelectApproveJoinSessionSpec.CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectUserLeftSessionSpec.CurrentOption.Handle == manageAsset->GetAssetID())
			{
				m_SelectUserLeftSessionSpec.CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectSessionInitSpec.CurrentOption.Handle == manageAsset->GetAssetID())
			{
				m_SelectSessionInitSpec.CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectConnectionTerminatedSpec.CurrentOption.Handle == manageAsset->GetAssetID())
			{
				m_SelectConnectionTerminatedSpec.CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectUpdateSessionSlotSpec.CurrentOption.Handle == manageAsset->GetAssetID())
			{
				m_SelectUpdateSessionSlotSpec.CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectStartSessionSpec.CurrentOption.Handle == manageAsset->GetAssetID())
			{
				m_SelectStartSessionSpec.CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectSessionReadyCheckSpec.CurrentOption.Handle == manageAsset->GetAssetID())
			{
				m_SelectSessionReadyCheckSpec.CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectReceiveSignalSpec.CurrentOption.Handle == manageAsset->GetAssetID())
			{
				m_SelectReceiveSignalSpec.CurrentOption = { "None", Assets::EmptyHandle };
			}
		}
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
