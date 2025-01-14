#include "Windows/MainWindow/ProjectPanel.h"

#include "EditorApp.h"

#include "Kargono/Utility/Operations.h"
#include "Kargono/Rendering/Texture.h"
#include "Kargono/Utility/Time.h"
#include "Kargono/Scenes/Scene.h"

static Kargono::EditorApp* s_EditorApp { nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{
	void ProjectPanel::InitializeStaticResources()
	{
		// Resolution Specification
		m_SelectStartSceneSpec.m_Label = "Starting Scene";
		//m_SelectStartSceneSpec.WidgetID = 0x75ebbc8750034f81;
		m_SelectStartSceneSpec.m_LineCount = 2;

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
		
		m_SelectStartSceneSpec.m_CurrentOption = {
			 Assets::AssetService::GetSceneRegistry().at(startSceneHandle).Data.FileLocation.filename().string(),
			startSceneHandle};
		m_SelectStartSceneSpec.m_PopupAction = [&]()
		{
			m_SelectStartSceneSpec.GetAllOptions().clear();
			for (auto& [handle, asset] : Assets::AssetService::GetSceneRegistry())
			{
				m_SelectStartSceneSpec.AddToOptions("All Options", asset.Data.FileLocation.filename().string(), handle);
			}
			m_SelectStartSceneSpec.m_CurrentOption = {
				Assets::AssetService::GetSceneRegistry().at(Projects::ProjectService::GetActiveStartSceneHandle()).Data.FileLocation.filename().string(),
			Projects::ProjectService::GetActiveStartSceneHandle()};
		};
		m_SelectStartSceneSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{
			if (!Assets::AssetService::GetSceneRegistry().contains(entry.m_Handle))
			{
				KG_WARN("Could not find scene using asset handle in Project Panel");
				return;
			}

			const Assets::AssetInfo asset = Assets::AssetService::GetSceneRegistry().at(entry.m_Handle);
			Projects::ProjectService::SetActiveStartingSceneHandle(entry.m_Handle);
		};

		// Default Full Screen
		m_DefaultFullscreenSpec.m_Label = "Default Fullscreen";
		m_DefaultFullscreenSpec.m_ConfirmAction = [](EditorUI::CheckboxSpec& spec)
		{
			Projects::ProjectService::SetActiveIsFullscreen(spec.m_CurrentBoolean);
		};

		// Set Networking Specification
		m_ToggleNetworkSpec.m_Label = "Networking";
		m_ToggleNetworkSpec.m_ConfirmAction = [](EditorUI::CheckboxSpec& spec)
		{
			Projects::ProjectService::SetActiveAppIsNetworked(spec.m_CurrentBoolean);
		};

		// Resolution Specification
		m_SelectResolutionSpec.m_Label = "Target Resolution";
		m_SelectResolutionSpec.m_LineCount = 4;
		m_SelectResolutionSpec.m_CurrentOption = {
			Utility::ScreenResolutionToString(Projects::ProjectService::GetActiveTargetResolution()),
			Assets::EmptyHandle};
		m_SelectResolutionSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			Projects::ProjectService::SetActiveTargetResolution(Utility::StringToScreenResolution(selection.m_Label));
		};
		m_SelectResolutionSpec.m_PopupAction = [&]()
		{
			m_SelectResolutionSpec.m_CurrentOption = {
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
		m_SelectStartGameStateSpec.m_Label = "Start Game State";
		m_SelectStartGameStateSpec.m_LineCount = 3;
		if (Projects::ProjectService::GetActiveStartGameStateHandle() != 0)
		{
			m_SelectStartGameStateSpec.m_CurrentOption = { Assets::AssetService::GetGameStateRegistry().at
			(Projects::ProjectService::GetActiveStartGameStateHandle()).Data.FileLocation.filename().string(),
			Projects::ProjectService::GetActiveStartGameStateHandle()};
		}
		else
		{
			m_SelectStartGameStateSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
		}
		m_SelectStartGameStateSpec.m_PopupAction = [&]()
		{
			m_SelectStartGameStateSpec.ClearOptions();
			m_SelectStartGameStateSpec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetGameStateRegistry())
			{
				m_SelectStartGameStateSpec.AddToOptions("All Options", asset.Data.FileLocation.filename().string(), handle);
			}

			if (Projects::ProjectService::GetActiveStartGameStateHandle() != Assets::EmptyHandle)
			{
				m_SelectStartGameStateSpec.m_CurrentOption = { Assets::AssetService::GetGameStateRegistry().at
				(Projects::ProjectService::GetActiveStartGameStateHandle()).Data.FileLocation.filename().string(),
				Projects::ProjectService::GetActiveStartGameStateHandle() };
			}
			else
			{
				m_SelectStartGameStateSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
			}

		};

		m_SelectStartGameStateSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetGameState(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not locate starting game state in ProjectPanel");
				return;
			}
			Projects::ProjectService::SetActiveStartGameStateHandle(selection.m_Handle);

		};

		// Runtime Start Spec
		m_SelectRuntimeStartSpec.m_Label = "Runtime Start";
		m_SelectRuntimeStartSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectRuntimeStartSpec.m_LineCount = 3;
		m_SelectRuntimeStartSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnRuntimeStartHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnRuntimeStartHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnRuntimeStartHandle()};
		m_SelectRuntimeStartSpec.m_PopupAction = [&]()
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

			m_SelectRuntimeStartSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnRuntimeStartHandle() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnRuntimeStartHandle())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnRuntimeStartHandle() };
		};
		m_SelectRuntimeStartSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find runtime start function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnRuntimeStartHandle(selection.m_Handle);
		};
		m_SelectRuntimeStartSpec.m_OnEdit = [&]() 
		{
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&] (EditorUI::TooltipEntry& entry) 
			{
				m_SelectRuntimeStartSpec.m_OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
				// Open create script dialog in script editor
				s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
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
					m_SelectRuntimeStartSpec.m_CurrentOption = { script->m_ScriptName, scriptHandle };
				}, {});

			} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.m_TooltipActive = true;

		};


		// Update User Count Spec
		m_SelectUpdateUserCountSpec.m_Label = "Update User Count";
		m_SelectUpdateUserCountSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectUpdateUserCountSpec.m_LineCount = 3;
		m_SelectUpdateUserCountSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnUpdateUserCountHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUpdateUserCountHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnUpdateUserCountHandle() };
		m_SelectUpdateUserCountSpec.m_PopupAction = [&]()
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

			m_SelectUpdateUserCountSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnUpdateUserCountHandle() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUpdateUserCountHandle())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnUpdateUserCountHandle() };
		};
		m_SelectUpdateUserCountSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find Update User Count function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnUpdateUserCountHandle(selection.m_Handle);
		};
		m_SelectUpdateUserCountSpec.m_OnEdit = [&]()
		{
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				m_SelectUpdateUserCountSpec.m_OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
					// Open create script dialog in script editor
					s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_UInt32, [&](Assets::AssetHandle scriptHandle)
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
							m_SelectUpdateUserCountSpec.m_CurrentOption = { script->m_ScriptName, scriptHandle };
						}, {"userCount"});

					} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.m_TooltipActive = true;

		};


		// Approve Join Session Spec
		m_SelectApproveJoinSessionSpec.m_Label = "Approve Join Session";
		m_SelectApproveJoinSessionSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectApproveJoinSessionSpec.m_LineCount = 3;
		m_SelectApproveJoinSessionSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnApproveJoinSessionHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnApproveJoinSessionHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnApproveJoinSessionHandle() };
		m_SelectApproveJoinSessionSpec.m_PopupAction = [&]()
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

			m_SelectApproveJoinSessionSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnApproveJoinSessionHandle() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnApproveJoinSessionHandle())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnApproveJoinSessionHandle() };
		};
		m_SelectApproveJoinSessionSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find Approve Join Session function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnApproveJoinSessionHandle(selection.m_Handle);
		};
		m_SelectApproveJoinSessionSpec.m_OnEdit = [&]()
		{
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				m_SelectApproveJoinSessionSpec.m_OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
					// Open create script dialog in script editor
					s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_UInt16, [&](Assets::AssetHandle scriptHandle)
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
							m_SelectApproveJoinSessionSpec.m_CurrentOption = { script->m_ScriptName, scriptHandle };
						}, {"sessionSlot"});

					} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.m_TooltipActive = true;

		};

		// User Left Session Spec
		m_SelectUserLeftSessionSpec.m_Label = "User Left Session";
		m_SelectUserLeftSessionSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectUserLeftSessionSpec.m_LineCount = 3;
		m_SelectUserLeftSessionSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnUserLeftSessionHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUserLeftSessionHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnUserLeftSessionHandle() };
		m_SelectUserLeftSessionSpec.m_PopupAction = [&]()
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

				m_SelectUserLeftSessionSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnUserLeftSessionHandle() ?
					Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUserLeftSessionHandle())->m_ScriptName : "None",
					Projects::ProjectService::GetActiveOnUserLeftSessionHandle() };
			};
		m_SelectUserLeftSessionSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find User Left Session function in Project Panel");
					return;
				}
				Projects::ProjectService::SetActiveOnUserLeftSessionHandle(selection.m_Handle);
			};
		m_SelectUserLeftSessionSpec.m_OnEdit = [&]()
			{
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					m_SelectUserLeftSessionSpec.m_OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						// Open create script dialog in script editor
						s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_UInt16, [&](Assets::AssetHandle scriptHandle)
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
								m_SelectUserLeftSessionSpec.m_CurrentOption = { script->m_ScriptName, scriptHandle };
							}, {"sessionSlot"});

						} };
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.m_TooltipActive = true;

			};




		// Session Initialization Spec
		m_SelectSessionInitSpec.m_Label = "Session Initialization";
		m_SelectSessionInitSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectSessionInitSpec.m_LineCount = 3;
		m_SelectSessionInitSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnCurrentSessionInitHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnCurrentSessionInitHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnCurrentSessionInitHandle() };
		m_SelectSessionInitSpec.m_PopupAction = [&]()
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

				m_SelectSessionInitSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnCurrentSessionInitHandle() ?
					Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnCurrentSessionInitHandle())->m_ScriptName : "None",
					Projects::ProjectService::GetActiveOnCurrentSessionInitHandle() };
			};
		m_SelectSessionInitSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find Session Initialization function in Project Panel");
					return;
				}
				Projects::ProjectService::SetActiveOnCurrentSessionInitHandle(selection.m_Handle);
			};
		m_SelectSessionInitSpec.m_OnEdit = [&]()
			{
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					m_SelectSessionInitSpec.m_OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						// Open create script dialog in script editor
						s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
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
								m_SelectSessionInitSpec.m_CurrentOption = { script->m_ScriptName, scriptHandle };
							}, {});

						} };
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.m_TooltipActive = true;

			};


		// Connection Terminated Spec
		m_SelectConnectionTerminatedSpec.m_Label = "Connection Terminated";
		m_SelectConnectionTerminatedSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectConnectionTerminatedSpec.m_LineCount = 3;
		m_SelectConnectionTerminatedSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnConnectionTerminatedHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnConnectionTerminatedHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnConnectionTerminatedHandle() };
		m_SelectConnectionTerminatedSpec.m_PopupAction = [&]()
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

			m_SelectConnectionTerminatedSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnConnectionTerminatedHandle() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnConnectionTerminatedHandle())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnConnectionTerminatedHandle() };
		};
		m_SelectConnectionTerminatedSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find Connection Terminated function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnConnectionTerminatedHandle(selection.m_Handle);
		};
		m_SelectConnectionTerminatedSpec.m_OnEdit = [&]()
		{
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				m_SelectConnectionTerminatedSpec.m_OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
					// Open create script dialog in script editor
					s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
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
							m_SelectConnectionTerminatedSpec.m_CurrentOption = { script->m_ScriptName, scriptHandle };
						}, {});

					} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.m_TooltipActive = true;
		};

		// Update Session User Slot Spec
		m_SelectUpdateSessionSlotSpec.m_Label = "Update Session User Slot";
		m_SelectUpdateSessionSlotSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectUpdateSessionSlotSpec.m_LineCount = 3;
		m_SelectUpdateSessionSlotSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnUpdateSessionUserSlotHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUpdateSessionUserSlotHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnUpdateSessionUserSlotHandle() };
		m_SelectUpdateSessionSlotSpec.m_PopupAction = [&]()
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

			m_SelectUpdateSessionSlotSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnUpdateSessionUserSlotHandle() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnUpdateSessionUserSlotHandle())->m_ScriptName : "None",
				Projects::ProjectService::GetActiveOnUpdateSessionUserSlotHandle() };
		};
		m_SelectUpdateSessionSlotSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find Update Session User Slot function in Project Panel");
				return;
			}
			Projects::ProjectService::SetActiveOnUpdateSessionUserSlotHandle(selection.m_Handle);
		};
		m_SelectUpdateSessionSlotSpec.m_OnEdit = [&]()
		{
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				m_SelectUpdateSessionSlotSpec.m_OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
					// Open create script dialog in script editor
					s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_UInt16, [&](Assets::AssetHandle scriptHandle)
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
							m_SelectUpdateSessionSlotSpec.m_CurrentOption = { script->m_ScriptName, scriptHandle };
						}, {"sessionSlot"});

					} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.m_TooltipActive = true;
		};


		// Start Session Spec
		m_SelectStartSessionSpec.m_Label = "Start Session";
		m_SelectStartSessionSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectStartSessionSpec.m_LineCount = 3;
		m_SelectStartSessionSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnStartSessionHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnStartSessionHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnStartSessionHandle() };
		m_SelectStartSessionSpec.m_PopupAction = [&]()
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

				m_SelectStartSessionSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnStartSessionHandle() ?
					Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnStartSessionHandle())->m_ScriptName : "None",
					Projects::ProjectService::GetActiveOnStartSessionHandle() };
			};
		m_SelectStartSessionSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find Start Session function in Project Panel");
					return;
				}
				Projects::ProjectService::SetActiveOnStartSessionHandle(selection.m_Handle);
			};
		m_SelectStartSessionSpec.m_OnEdit = [&]()
			{
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					m_SelectStartSessionSpec.m_OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						// Open create script dialog in script editor
						s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
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
								m_SelectStartSessionSpec.m_CurrentOption = { script->m_ScriptName, scriptHandle };
							}, {});

						} };
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.m_TooltipActive = true;
			};


		// Session Ready Check Spec
		m_SelectSessionReadyCheckSpec.m_Label = "Session Ready Check";
		m_SelectSessionReadyCheckSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectSessionReadyCheckSpec.m_LineCount = 3;
		m_SelectSessionReadyCheckSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnSessionReadyCheckConfirmHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnSessionReadyCheckConfirmHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnSessionReadyCheckConfirmHandle() };
		m_SelectSessionReadyCheckSpec.m_PopupAction = [&]()
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

				m_SelectSessionReadyCheckSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnSessionReadyCheckConfirmHandle() ?
					Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnSessionReadyCheckConfirmHandle())->m_ScriptName : "None",
					Projects::ProjectService::GetActiveOnSessionReadyCheckConfirmHandle() };
			};
		m_SelectSessionReadyCheckSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find Session Ready Check function in Project Panel");
					return;
				}
				Projects::ProjectService::SetActiveOnSessionReadyCheckConfirmHandle(selection.m_Handle);
			};
		m_SelectSessionReadyCheckSpec.m_OnEdit = [&]()
			{
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					m_SelectSessionReadyCheckSpec.m_OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						// Open create script dialog in script editor
						s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_None, [&](Assets::AssetHandle scriptHandle)
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
								m_SelectSessionReadyCheckSpec.m_CurrentOption = { script->m_ScriptName, scriptHandle };
							}, {});

						} };
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.m_TooltipActive = true;
			};


		// On Receive Signal Spec
		m_SelectReceiveSignalSpec.m_Label = "On Receive Signal";
		m_SelectReceiveSignalSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectReceiveSignalSpec.m_LineCount = 3;
		m_SelectReceiveSignalSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnReceiveSignalHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnReceiveSignalHandle())->m_ScriptName : "None",
			Projects::ProjectService::GetActiveOnReceiveSignalHandle() };
		m_SelectReceiveSignalSpec.m_PopupAction = [&]()
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

				m_SelectReceiveSignalSpec.m_CurrentOption = { Projects::ProjectService::GetActiveOnReceiveSignalHandle() ?
					Assets::AssetService::GetScript(Projects::ProjectService::GetActiveOnReceiveSignalHandle())->m_ScriptName : "None",
					Projects::ProjectService::GetActiveOnReceiveSignalHandle() };
			};
		m_SelectReceiveSignalSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find On Receive Signal function in Project Panel");
					return;
				}
				Projects::ProjectService::SetActiveOnReceiveSignalHandle(selection.m_Handle);
			};
		m_SelectReceiveSignalSpec.m_OnEdit = [&]()
			{
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					m_SelectReceiveSignalSpec.m_OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						// Open create script dialog in script editor
						s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_UInt16, [&](Assets::AssetHandle scriptHandle)
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
								m_SelectReceiveSignalSpec.m_CurrentOption = { script->m_ScriptName, scriptHandle };
							}, {"signal"});

						} };
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.m_TooltipActive = true;
			};

		m_SelectScriptTooltip.m_Label = "Script Tooltip";

	}

	void ProjectPanel::InitializeMessageTypeResources()
	{
		// Manage AI Message Types
		m_MessageTypeTable.m_Label = "All Message Types";
		m_MessageTypeTable.m_Column1Title = "Message Type";
		m_MessageTypeTable.m_Column2Title = "";
		m_MessageTypeTable.m_Expanded = false;
		m_MessageTypeTable.AddToSelectionList("Create New Message Type", [&]()
		{
			m_CreateMessageTypePopup.m_StartPopup = true;
		});
		m_MessageTypeTable.m_OnRefresh = [&]()
		{
			m_MessageTypeTable.ClearList();
			for (std::string& label : Projects::ProjectService::GetAllMessageTypes())
			{
				m_MessageTypeTable.InsertListEntry(label, "", [&](EditorUI::ListEntry& entry, std::size_t iteration)
				{
					m_ActiveAIMessageType = entry.m_Label;
					m_EditMessageTypePopup.m_OpenPopup = true;
				});
			}
		};
		m_MessageTypeTable.m_OnRefresh();

		m_CreateMessageTypePopup.m_Label = "Create New Message Type";
		m_CreateMessageTypePopup.m_Flags |= EditorUI::EditText_PopupOnly;
		m_CreateMessageTypePopup.m_ConfirmAction = [&](EditorUI::EditTextSpec& spec)
		{
			// Ensure input string does not use whitespace
			Utility::Operations::RemoveWhitespaceFromString(m_CreateMessageTypePopup.m_CurrentOption);
			// Create new AI message
			bool success = Projects::ProjectService::AddAIMessageType(m_CreateMessageTypePopup.m_CurrentOption);
			if (!success)
			{
				KG_WARN("Failed to create message type");
				return;
			}
			m_MessageTypeTable.m_OnRefresh();
			s_MainWindow->m_TextEditorPanel->RefreshKGScriptEditor();
		};

		m_EditMessageTypePopup.m_Label = "Edit Message Type";
		m_EditMessageTypePopup.m_PopupAction = [&]()
		{
			m_EditMessageTypeText.m_CurrentOption = m_ActiveAIMessageType;
		};
		m_EditMessageTypePopup.m_ConfirmAction = [&]()
		{
			// Ensure input string does not use whitespace
			Utility::Operations::RemoveWhitespaceFromString(m_CreateMessageTypePopup.m_CurrentOption);

			bool success = Projects::ProjectService::EditAIMessageType(m_ActiveAIMessageType, 
				m_EditMessageTypeText.m_CurrentOption);

			// Create new AI message type
			if (!success)
			{
				KG_WARN("Failed to edit message type");
				return;
			}
			OnRefresh();
			s_MainWindow->m_TextEditorPanel->RefreshKGScriptEditor();
		};
		m_EditMessageTypePopup.m_DeleteAction = [&]()
		{
			bool success = Projects::ProjectService::DeleteAIMessageType(m_ActiveAIMessageType);
			if (!success)
			{
				KG_WARN("Failed to delete section label");
				return;
			}

			OnRefresh();
			s_MainWindow->m_TextEditorPanel->RefreshKGScriptEditor();
		};
		m_EditMessageTypePopup.m_PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditText(m_EditMessageTypeText);
		};

		m_EditMessageTypeText.m_Label = "Message Type";
		m_EditMessageTypeText.m_CurrentOption = "Empty";

	}

	ProjectPanel::ProjectPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(ProjectPanel::OnKeyPressedEditor));
		InitializeStaticResources();
		InitializeMessageTypeResources();
	}
	void ProjectPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_MainWindow->m_ShowProject);

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
		m_DefaultFullscreenSpec.m_CurrentBoolean = Projects::ProjectService::GetActiveIsFullscreen();
		EditorUI::EditorUIService::Checkbox(m_DefaultFullscreenSpec);
		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Networking Checkbox
		m_ToggleNetworkSpec.m_CurrentBoolean = Projects::ProjectService::GetActiveAppIsNetworked();
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
		if (ImGui::DragFloat2("Gravity", glm::value_ptr(s_MainWindow->m_EditorScene->GetPhysicsSpecification().Gravity), 0.05f))
		{
			if (Physics::Physics2DService::GetActivePhysics2DWorld())
			{
				Scenes::SceneService::GetActiveScene()->GetPhysicsSpecification().Gravity = s_MainWindow->m_EditorScene->GetPhysicsSpecification().Gravity;
				Physics::Physics2DService::SetActiveGravity(s_MainWindow->m_EditorScene->GetPhysicsSpecification().Gravity);
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
			manageAsset->GetAction() == Events::ManageAssetAction::PreDelete)
		{
			if (m_SelectRuntimeStartSpec.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectRuntimeStartSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectUpdateUserCountSpec.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectUpdateUserCountSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectApproveJoinSessionSpec.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectApproveJoinSessionSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectUserLeftSessionSpec.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectUserLeftSessionSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectSessionInitSpec.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectSessionInitSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectConnectionTerminatedSpec.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectConnectionTerminatedSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectUpdateSessionSlotSpec.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectUpdateSessionSlotSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectStartSessionSpec.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectStartSessionSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectSessionReadyCheckSpec.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectSessionReadyCheckSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			if (m_SelectReceiveSignalSpec.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectReceiveSignalSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
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
		m_MessageTypeTable.m_OnRefresh();
	}
}
