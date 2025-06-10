#include "Windows/MainWindow/ProjectPanel.h"

#include "EditorApp.h"

#include "Kargono/Utility/Operations.h"
#include "Modules/Rendering/Texture.h"
#include "Kargono/Utility/Time.h"
#include "Kargono/Scenes/Scene.h"
#include "Modules/Network/Server.h"

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
		Assets::AssetHandle startSceneHandle = Projects::ProjectService::GetActiveContext().GetStartSceneHandle();
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
			 Assets::AssetService::GetSceneRegistry().at(startSceneHandle).Data.FileLocation.filename().string().c_str(),
			startSceneHandle};
		m_SelectStartSceneSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.GetAllOptions().clear();
			for (auto& [handle, asset] : Assets::AssetService::GetSceneRegistry())
			{
				spec.AddToOptions("All Options", asset.Data.FileLocation.filename().string(), handle);
			}
			spec.m_CurrentOption = {
				Assets::AssetService::GetSceneRegistry().at(Projects::ProjectService::GetActiveContext().GetStartSceneHandle()).Data.FileLocation.filename().string().c_str(),
			Projects::ProjectService::GetActiveContext().GetStartSceneHandle()};
		};
		m_SelectStartSceneSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{
			if (!Assets::AssetService::GetSceneRegistry().contains(entry.m_Handle))
			{
				KG_WARN("Could not find scene using asset handle in Project Panel");
				return;
			}

			const Assets::AssetInfo asset = Assets::AssetService::GetSceneRegistry().at(entry.m_Handle);
			Projects::ProjectService::GetActiveContext().SetStartingSceneHandle(entry.m_Handle);
		};

		// Default Full Screen
		m_DefaultFullscreenSpec.m_Label = "Default Fullscreen";
		m_DefaultFullscreenSpec.m_ConfirmAction = [](EditorUI::CheckboxWidget& spec)
		{
			Projects::ProjectService::GetActiveContext().SetIsFullscreen(spec.m_CurrentBoolean);
		};

		// Set Networking Specification
		m_ToggleNetworkSpec.m_Label = "Networking";
		m_ToggleNetworkSpec.m_ConfirmAction = [](EditorUI::CheckboxWidget& spec)
		{
			Projects::ProjectService::GetActiveContext().SetAppIsNetworked(spec.m_CurrentBoolean);
		};

		// Resolution Specification
		m_SelectResolutionSpec.m_Label = "Target Resolution";
		m_SelectResolutionSpec.m_LineCount = 4;
		m_SelectResolutionSpec.m_CurrentOption = {
			Utility::ScreenResolutionToString(Projects::ProjectService::GetActiveContext().GetTargetResolution()),
			Assets::EmptyHandle};
		m_SelectResolutionSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			// Set the target resolution
			Projects::ProjectService::GetActiveContext().SetTargetResolution((ScreenResolution)(uint64_t)selection.m_Handle);

			// Revalidate the editor's viewport
			s_MainWindow->m_ViewportPanel->SetViewportAspectRatio(Utility::ScreenResolutionToAspectRatio((ScreenResolution)(uint64_t)selection.m_Handle));
		};
		m_SelectResolutionSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.m_CurrentOption = 
			{
				Utility::ScreenResolutionToString(Projects::ProjectService::GetActiveContext().GetTargetResolution()),
			(uint64_t)Projects::ProjectService::GetActiveContext().GetTargetResolution()
			};
		};

		for (ScreenResolution option : s_AllScreenResolutions)
		{
			if (option == ScreenResolution::None)
			{
				continue;
			}

			m_SelectResolutionSpec.AddToOptions
			(
				Utility::ScreenResolutionToCategoryTitle(option),
				Utility::ScreenResolutionToString(option),
				(uint64_t)option
			);
		}

		// Select Start Game State
		m_SelectStartGameStateSpec.m_Label = "Start Game State";
		m_SelectStartGameStateSpec.m_LineCount = 3;
		if (Projects::ProjectService::GetActiveContext().GetStartGameStateHandle() != 0)
		{
			m_SelectStartGameStateSpec.m_CurrentOption = { Assets::AssetService::GetGameStateRegistry().at
			(Projects::ProjectService::GetActiveContext().GetStartGameStateHandle()).Data.FileLocation.filename().string().c_str(),
			Projects::ProjectService::GetActiveContext().GetStartGameStateHandle()};
		}
		else
		{
			m_SelectStartGameStateSpec.m_CurrentOption = { "None", Assets::EmptyHandle };
		}
		m_SelectStartGameStateSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetGameStateRegistry())
			{
				spec.AddToOptions("All Options", asset.Data.FileLocation.filename().string(), handle);
			}

			if (Projects::ProjectService::GetActiveContext().GetStartGameStateHandle() != Assets::EmptyHandle)
			{
				spec.m_CurrentOption = { Assets::AssetService::GetGameStateRegistry().at
				(Projects::ProjectService::GetActiveContext().GetStartGameStateHandle()).Data.FileLocation.filename().string().c_str(),
				Projects::ProjectService::GetActiveContext().GetStartGameStateHandle() };
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
			Projects::ProjectService::GetActiveContext().SetStartGameStateHandle(selection.m_Handle);

		};

		// Runtime Start Spec
		m_SelectRuntimeStartSpec.m_Label = "Runtime Start";
		m_SelectRuntimeStartSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectRuntimeStartSpec.m_LineCount = 3;
		m_SelectRuntimeStartSpec.m_CurrentOption = { Projects::ProjectService::GetActiveContext().GetOnRuntimeStartHandle() ?
			Assets::AssetService::GetScript(Projects::ProjectService::GetActiveContext().GetOnRuntimeStartHandle())->m_ScriptName.c_str() : "None",
			Projects::ProjectService::GetActiveContext().GetOnRuntimeStartHandle()};
		m_SelectRuntimeStartSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				KG_ASSERT(handle != Assets::EmptyHandle);
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_None)
				{
					continue;
				}

				spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}

			spec.m_CurrentOption = { Projects::ProjectService::GetActiveContext().GetOnRuntimeStartHandle() ?
				Assets::AssetService::GetScript(Projects::ProjectService::GetActiveContext().GetOnRuntimeStartHandle())->m_ScriptName.c_str() : "None",
				Projects::ProjectService::GetActiveContext().GetOnRuntimeStartHandle() };
		};
		m_SelectRuntimeStartSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find runtime start function in Project Panel");
				return;
			}
			Projects::ProjectService::GetActiveContext().SetOnRuntimeStartHandle(selection.m_Handle);
		};
		m_SelectRuntimeStartSpec.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&] (EditorUI::TooltipEntry& entry) 
			{
				UNREFERENCED_PARAMETER(entry);
				m_SelectRuntimeStartSpec.m_OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
				UNREFERENCED_PARAMETER(entry);
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
					Projects::ProjectService::GetActiveContext().SetOnRuntimeStartHandle(scriptHandle);
					m_SelectRuntimeStartSpec.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
				}, {});

			} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.m_TooltipActive = true;

		};
	}

	ProjectPanel::ProjectPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(ProjectPanel::OnKeyPressedEditor));

		InitializeStaticResources();
		m_ClientOptions.Init(&m_SelectScriptTooltip);
		m_ServerOptions.Init();
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
		// Project Settings Header
		EditorUI::EditorUIService::BeginTabBar("ProjectSettingsBar");

		if (EditorUI::EditorUIService::BeginTabItem("General"))
		{
			// Project Directory
			EditorUI::EditorUIService::LabeledText("Project Directory", Projects::ProjectService::GetActiveContext().GetProjectPaths().m_ProjectDirectory.string().c_str());
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

			// Select Starting Scene
			m_SelectStartSceneSpec.RenderOptions();
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

			// Networking Checkbox
			m_ToggleNetworkSpec.m_CurrentBoolean = Projects::ProjectService::GetActiveContext().GetAppIsNetworked();
			m_ToggleNetworkSpec.RenderCheckbox();
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

			// Default Fullscreen
			m_DefaultFullscreenSpec.m_CurrentBoolean = Projects::ProjectService::GetActiveContext().GetIsFullscreen();
			m_DefaultFullscreenSpec.RenderCheckbox();
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

			// Select Starting Game State
			m_SelectStartGameStateSpec.RenderOptions();
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

			// Resolution Select Option
			m_SelectResolutionSpec.RenderOptions();
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

			// Select On Runtime Start
			m_SelectRuntimeStartSpec.RenderOptions();
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

			EditorUI::EditorUIService::EndTabItem();
		}

		if (EditorUI::EditorUIService::BeginTabItem("Network"))
		{
			EditorUI::EditorUIService::BeginTabBar("ProjectNetworkBar");

			if (EditorUI::EditorUIService::BeginTabItem("Client"))
			{
				m_ClientOptions.OnEditorUIRender();
				EditorUI::EditorUIService::EndTabItem();
			}

			if (EditorUI::EditorUIService::BeginTabItem("Server"))
			{
				m_ServerOptions.OnEditorUIRender();
				EditorUI::EditorUIService::EndTabItem();
			}

			EditorUI::EditorUIService::EndTabBar();


			EditorUI::EditorUIService::EndTabItem();
		}

		EditorUI::EditorUIService::EndTabBar();

		m_SelectScriptTooltip.RenderTooltip();
		

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
			m_ClientOptions.OnAssetEvent(event);
		}
		return false;
	}
	void ProjectPanel::ResetPanelResources()
	{
		InitializeStaticResources();
	}
	void ProjectPanel::OnRefresh()
	{
		
	}
	void ServerOptions::Init()
	{
		InitWidgets();

		RegisterObservers();
	}
	void ServerOptions::InitWidgets()
	{
		// Commands section
		m_CommandsHeader.m_Label = "Commands";
		m_CommandsHeader.m_Flags = EditorUI::CollapsingHeader_UnderlineTitle;
		m_CommandsHeader.m_Expanded = true;

		m_LifecycleOptions.m_Label = "Lifecycle";
		m_LifecycleOptions.m_Flags |= EditorUI::ButtonBar_Indented;
		m_LifecycleOptions.AddButton("Start", 
		[](EditorUI::Button& button)
		{  
			Network::ServerService::GetActiveContext().Init(Projects::ProjectService::GetActiveContext().GetServerConfig());
		});
		m_LifecycleOptions.AddButton("Close", 
		[](EditorUI::Button& button)
		{
			Network::ServerService::GetActiveContext().Terminate(false);
		});
		m_LifecycleOptions.AddButton("Restart", 
		[](EditorUI::Button& button)
		{
			if (Network::ServerService::IsContextActive())
			{
				Network::ServerService::GetActiveContext().Terminate(false);
			}

			Network::ServerService::GetActiveContext().Init(Projects::ProjectService::GetActiveContext().GetServerConfig());
		});


		// Status section
		m_StatusHeader.m_Label = "Status";
		m_StatusHeader.m_Flags = EditorUI::CollapsingHeader_UnderlineTitle;
		m_StatusHeader.m_Expanded = true;

		m_ConnectionsHeader.m_Label = "All Connections";
		m_ConnectionsHeader.m_Flags = EditorUI::CollapsingHeader_UnderlineTitle;
		m_ConnectionsHeader.m_Expanded = true;

		// Config section
		m_GeneralConfigHeader.m_Label = "General";
		m_GeneralConfigHeader.m_Flags = EditorUI::CollapsingHeader_UnderlineTitle;
		m_GeneralConfigHeader.m_Expanded = true;

		m_ServerIP.m_Label = "Server IPv4";
		m_ServerIP.m_CurrentIVec4 = { 127, 0, 0, 1 };
		m_ServerIP.m_Bounds = { 0, 255 };
		m_ServerIP.m_ConfirmAction = [](EditorUI::EditIVec4Widget& spec) 
		{
			Projects::ProjectService::GetActiveContext().GetServerConfig().m_ServerAddress.SetAddress((Math::u8vec4)spec.m_CurrentIVec4);
		};

		m_ServerPort.m_Label = "Server Port";
		m_ServerPort.m_CurrentInteger = 60'000;
		m_ServerPort.m_Bounds = { 101, 65'535 };
		m_ServerPort.m_ConfirmAction = [](EditorUI::EditIntegerWidget& spec) 
		{
			Projects::ProjectService::GetActiveContext().GetServerConfig().m_ServerAddress.SetNewPort((uint16_t)spec.m_CurrentInteger);
		};

		m_ServerLocation.m_Label = "Local Machine";
		m_ServerLocation.m_CurrentBoolean = true;
		m_ServerLocation.m_ConfirmAction = [](EditorUI::CheckboxWidget& spec) 
		{
			Network::ServerLocation newLoc = spec.m_CurrentBoolean ?
					Network::ServerLocation::LocalMachine :
					Network::ServerLocation::Remote;
			
			Projects::ProjectService::GetActiveContext().GetServerConfig().m_ServerLocation = newLoc;
		};

		m_ServerSecrets.m_Label = "Validation Secrets";
		m_ServerSecrets.m_CurrentIVec4 = { 0, 0, 0, 0 };
		m_ServerSecrets.m_Bounds = { 0, 2'147'483'647 };
		m_ServerSecrets.m_ConfirmAction = [](EditorUI::EditIVec4Widget& spec) 
		{
			Network::ServerConfig& serverConfig = Projects::ProjectService::GetActiveContext().GetServerConfig();
			serverConfig.m_ValidationSecrets = (Math::u64vec4)spec.m_CurrentIVec4;
		};

		
	}
	void ServerOptions::RegisterObservers()
	{
		Network::Server& server{ Network::ServerService::GetActiveContext() };

		Network::ServerNotifiers& serverNotifiers{ server.GetNotifiers() };

		// Server status observers
		serverNotifiers.AddServerActiveObserver(KG_BIND_CLASS_FN(OnNotifyServerActive));


		// Server packet observer(s)
		Network::ReliabilityContextNotifiers& reliabilityNotif{ server.GetNetworkThread().GetReliabilityNotifiers() };
		reliabilityNotif.AddReliabilityStateObserver(KG_BIND_CLASS_FN(OnNotifyReliabilityState));
		reliabilityNotif.AddSendPacketObserver(KG_BIND_CLASS_FN(OnNotifySendServerPacket));
		reliabilityNotif.AddAckPacketObserver(KG_BIND_CLASS_FN(OnNotifyAckServerPacket));
		// Server connection observers
		Network::ServerNetworkNotifiers& netThreadNotifiers{ server.GetNetworkThread().GetNotifiers() };
		netThreadNotifiers.AddConnectObserver(KG_BIND_CLASS_FN(OnNotifyClientConnect));
		netThreadNotifiers.AddDisconnectObserver(KG_BIND_CLASS_FN(OnNotifyClientDisconnect));

	}
	void ServerOptions::OnEditorUIRender()
	{
		EditorUI::EditorUIService::BeginTabBar("ServerOptionsBar");

		if (EditorUI::EditorUIService::BeginTabItem("Status"))
		{
			m_StatusHeader.RenderHeader();
			if (m_StatusHeader.m_Expanded)
			{
				EditorUI::EditorUIService::LabeledText("Status", m_ActiveState ? "Active" : "In-Active",
					EditorUI::LabeledText_Indented);
			}
			m_CommandsHeader.RenderHeader();

			if (m_CommandsHeader.m_Expanded)
			{
				m_LifecycleOptions.RenderBar();
			}

			m_ConnectionsHeader.RenderHeader();
			if (m_ConnectionsHeader.m_Expanded)
			{
				for (ConnectionUI& connection : m_ConnectionUIs)
				{
					connection.OnEditorUIRender();
					EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Medium);
				}
			}
			EditorUI::EditorUIService::EndTabItem();
		}
		if (EditorUI::EditorUIService::BeginTabItem("Config"))
		{
			m_GeneralConfigHeader.RenderHeader();

			if (m_GeneralConfigHeader.m_Expanded)
			{
				Network::ServerConfig config = Projects::ProjectService::GetActiveContext().GetServerConfig();

				m_ServerIP.m_CurrentIVec4 = (Math::ivec4)config.m_ServerAddress.GetAddressUVec4();
				m_ServerIP.RenderIVec4();
				m_ServerPort.m_CurrentInteger = (int32_t)config.m_ServerAddress.GetPort();
				m_ServerPort.RenderInteger();
				m_ServerLocation.m_CurrentBoolean = config.m_ServerLocation == Network::ServerLocation::LocalMachine;
				m_ServerLocation.RenderCheckbox();
				m_ServerSecrets.m_CurrentIVec4 = (Math::ivec4)config.m_ValidationSecrets;
				m_ServerSecrets.RenderIVec4();
			}
			EditorUI::EditorUIService::EndTabItem();
		}

		EditorUI::EditorUIService::EndTabBar();
	}
	void ServerOptions::OnNotifySendServerPacket(Network::ClientIndex clientIndex, Network::PacketSequence seq)
	{
		EngineService::GetActiveEngine().GetThread().SubmitFunction([&, clientIndex, seq]()
		{
			for (ConnectionUI& connection : m_ConnectionUIs)
			{
				if (clientIndex == connection.m_ClientIndex)
				{
					connection.OnNotifySendPacket(seq);
					break;
				}
			}
		});
	}
	void ServerOptions::OnNotifyAckServerPacket(Network::ClientIndex clientIndex, Network::PacketSequence seq, float rtt)
	{
		EngineService::GetActiveEngine().GetThread().SubmitFunction([&, clientIndex, seq, rtt]()
		{
			for (ConnectionUI& connection : m_ConnectionUIs)
			{
				if (clientIndex == connection.m_ClientIndex)
				{
					connection.OnNotifyAckPacket(seq, rtt);
					break;
				}
			}
		});
	}
	void ServerOptions::OnNotifyReliabilityState(Network::ClientIndex index, bool congested, float rtt)
	{
		EngineService::GetActiveEngine().GetThread().SubmitFunction([&, index, congested, rtt]()
		{
			for (ConnectionUI& connection : m_ConnectionUIs)
			{
				if (index == connection.m_ClientIndex)
				{
					connection.m_AverageRTT = rtt;
					connection.m_IsCongested = congested;
					break;
				}
			}
		});
	}
	void ServerOptions::OnNotifyClientConnect(Network::ClientIndex clientIndex)
	{
		EngineService::GetActiveEngine().GetThread().SubmitFunction([&, clientIndex]()
		{
			ConnectionUI& connection = m_ConnectionUIs.emplace_back(clientIndex, (size_t)Network::k_AckBitFieldSize);
			connection.m_ConnectionStatus = Network::ConnectionStatus::Connected;
		});
	}
	void ServerOptions::OnNotifyClientDisconnect(Network::ClientIndex clientIndex)
	{
		EngineService::GetActiveEngine().GetThread().SubmitFunction([&, clientIndex]()
		{
			// Search for connectionUI corresponding to clientIndex
			size_t i{ 0 };
			for (ConnectionUI& connectionUI : m_ConnectionUIs)
			{
				if (connectionUI.m_ClientIndex == clientIndex)
				{
					// Found the corresponding UI for clientIndex
					break;
				}
				i++;
			}

			KG_ASSERT(i < m_ConnectionUIs.size());
			
			// Remove the connection UI
			m_ConnectionUIs.erase(m_ConnectionUIs.begin() + i);
		});
	}
	void ServerOptions::OnNotifyServerActive(bool active)
	{
		EngineService::GetActiveEngine().GetThread().SubmitFunction([&, active]()
		{
			m_ActiveState = active;

			// Reset all visible connections
			m_ConnectionUIs.clear();
		});
	}
	void ClientOptions::Init(EditorUI::TooltipWidget* parentTooltipSpec)
	{
		// Set up tooltip dependency
		KG_ASSERT(parentTooltipSpec);
		m_ParentTooltip = parentTooltipSpec;
		m_ParentTooltip->m_Label = "Script Tooltip";

		// Set up widgets
		InitWidgets();
		RegisterObservers();
		
	}
	void ClientOptions::InitWidgets()
	{
		// Status section
		m_CommandsHeader.m_Label = "Commands";
		m_CommandsHeader.m_Flags = EditorUI::CollapsingHeader_UnderlineTitle;
		m_CommandsHeader.m_Expanded = true;

		m_LifecycleOptions.m_Label = "Lifecycle";
		m_LifecycleOptions.m_Flags |= EditorUI::ButtonBar_Indented;
		m_LifecycleOptions.AddButton("Start",
			[](EditorUI::Button& button)
			{
				Network::ClientService::GetActiveContext().Init(Projects::ProjectService::GetActiveContext().GetServerConfig());
			});
		m_LifecycleOptions.AddButton("Close",
			[](EditorUI::Button& button)
			{
				Network::ClientService::GetActiveContext().Terminate(false);
			});
		m_LifecycleOptions.AddButton("Restart",
			[](EditorUI::Button& button)
			{
				if (Network::ClientService::IsContextActive())
				{
					Network::ClientService::GetActiveContext().Terminate(false);
				}

				Network::ClientService::GetActiveContext().Init(Projects::ProjectService::GetActiveContext().GetServerConfig());
			});

		Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts()};

		m_StatusHeader.m_Label = "Status";
		m_StatusHeader.m_Flags = EditorUI::CollapsingHeader_UnderlineTitle;
		m_StatusHeader.m_Expanded = true;

		m_ConnectionHeader.m_Label = "Connection";
		m_ConnectionHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_ConnectionHeader.m_Expanded = true;
		m_ClientConnectionUI = ConnectionUI(Network::k_InvalidClientIndex, (size_t)Network::k_AckBitFieldSize);

		// Config section
		m_AppScriptsHeader.m_Label = "App Data Handlers";
		m_AppScriptsHeader.m_Flags = EditorUI::CollapsingHeader_UnderlineTitle;
		m_AppScriptsHeader.m_Expanded = true;

		m_SessionScriptsHeader.m_Label = "Session Management";
		m_SessionScriptsHeader.m_Flags = EditorUI::CollapsingHeader_UnderlineTitle;
		m_SessionScriptsHeader.m_Expanded = true;

		m_ConnectionScriptsHeader.m_Label = "Connection Management";
		m_ConnectionScriptsHeader.m_Flags = EditorUI::CollapsingHeader_UnderlineTitle;
		m_ConnectionScriptsHeader.m_Expanded = true;

		m_QueryServerScriptsHeader.m_Label = "Query Server";
		m_QueryServerScriptsHeader.m_Flags = EditorUI::CollapsingHeader_UnderlineTitle;
		m_QueryServerScriptsHeader.m_Expanded = true;

		// Update User Count Spec
		m_SelectUpdateUserCountSpec.m_Label = "Update User Count";
		m_SelectUpdateUserCountSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally
			| EditorUI::SelectOption_Indented;
		m_SelectUpdateUserCountSpec.m_LineCount = 3;
		m_SelectUpdateUserCountSpec.m_CurrentOption = { clientScripts.m_OnUpdateUserCount ?
			Assets::AssetService::GetScript(clientScripts.m_OnUpdateUserCount)->m_ScriptName.c_str() : "None", clientScripts.m_OnUpdateUserCount };
		m_SelectUpdateUserCountSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

			spec.ClearOptions();

			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				KG_ASSERT(handle != Assets::EmptyHandle);
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				if (script->m_FuncType != WrappedFuncType::Void_UInt32)
				{
					continue;
				}

				spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}

			spec.m_CurrentOption = { clientScripts.m_OnUpdateUserCount ?
				Assets::AssetService::GetScript(clientScripts.m_OnUpdateUserCount)->m_ScriptName.c_str() : "None", clientScripts.m_OnUpdateUserCount };
		};
		m_SelectUpdateUserCountSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
		{
			Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

			if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
			{
				KG_WARN("Could not find Update User Count function in Project Panel");
				return;
			}
			clientScripts.m_OnUpdateUserCount = selection.m_Handle;
		};
		m_SelectUpdateUserCountSpec.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
			{
				UNREFERENCED_PARAMETER(spec);
				// Initialize tooltip with options
				m_ParentTooltip->ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					m_SelectUpdateUserCountSpec.m_OpenPopup = true;
				} };
				m_ParentTooltip->AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);

					Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

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
							clientScripts.m_OnUpdateUserCount = scriptHandle;
							m_SelectUpdateUserCountSpec.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
						}, {"userCount"});

				} };
				m_ParentTooltip->AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_ParentTooltip->m_TooltipActive = true;

			};


		// Approve Join Session Spec
		m_SelectApproveJoinSessionSpec.m_Label = "Approve Join Session";
		m_SelectApproveJoinSessionSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally
			| EditorUI::SelectOption_Indented;
		m_SelectApproveJoinSessionSpec.m_LineCount = 3;
		m_SelectApproveJoinSessionSpec.m_CurrentOption = { clientScripts.m_OnApproveJoinSession ?
			Assets::AssetService::GetScript(clientScripts.m_OnApproveJoinSession)->m_ScriptName.c_str() : "None",
			clientScripts.m_OnApproveJoinSession };
		m_SelectApproveJoinSessionSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
			{
				spec.ClearOptions();

				spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
				{
					KG_ASSERT(handle != Assets::EmptyHandle);
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
					if (script->m_FuncType != WrappedFuncType::Void_UInt16)
					{
						continue;
					}

					spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
				}

				Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

				spec.m_CurrentOption = { clientScripts.m_OnApproveJoinSession ?
					Assets::AssetService::GetScript(clientScripts.m_OnApproveJoinSession)->m_ScriptName.c_str() : "None", clientScripts.m_OnApproveJoinSession };
			};
		m_SelectApproveJoinSessionSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find Approve Join Session function in Project Panel");
					return;
				}

				Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

				clientScripts.m_OnApproveJoinSession = selection.m_Handle;
			};
		m_SelectApproveJoinSessionSpec.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
			{
				UNREFERENCED_PARAMETER(spec);
				// Initialize tooltip with options
				m_ParentTooltip->ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					m_SelectApproveJoinSessionSpec.m_OpenPopup = true;
				} };
				m_ParentTooltip->AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						UNREFERENCED_PARAMETER(entry);
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

								Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

								// Fill the new script handle
								clientScripts.m_OnApproveJoinSession = scriptHandle;
								m_SelectApproveJoinSessionSpec.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
							}, {"sessionSlot"});

						} };
				m_ParentTooltip->AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_ParentTooltip->m_TooltipActive = true;

			};

		// User Left Session Spec
		m_SelectUserLeftSessionSpec.m_Label = "User Left Session";
		m_SelectUserLeftSessionSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally
			| EditorUI::SelectOption_Indented;
		m_SelectUserLeftSessionSpec.m_LineCount = 3;
		m_SelectUserLeftSessionSpec.m_CurrentOption = { clientScripts.m_OnUserLeftSession ?
			Assets::AssetService::GetScript(clientScripts.m_OnUserLeftSession)->m_ScriptName.c_str() : "None", clientScripts.m_OnUserLeftSession };
		m_SelectUserLeftSessionSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
			{
				spec.ClearOptions();

				spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
				{
					KG_ASSERT(handle != Assets::EmptyHandle);
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
					if (script->m_FuncType != WrappedFuncType::Void_UInt16)
					{
						continue;
					}

					spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
				}

				Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

				spec.m_CurrentOption = { clientScripts.m_OnUserLeftSession ?
					Assets::AssetService::GetScript(clientScripts.m_OnUserLeftSession)->m_ScriptName.c_str() : "None", clientScripts.m_OnUserLeftSession };
			};
		m_SelectUserLeftSessionSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find User Left Session function in Project Panel");
					return;
				}

				Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };
				clientScripts.m_OnUserLeftSession = selection.m_Handle;
			};
		m_SelectUserLeftSessionSpec.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
			{
				UNREFERENCED_PARAMETER(spec);
				// Initialize tooltip with options
				m_ParentTooltip->ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					m_SelectUserLeftSessionSpec.m_OpenPopup = true;
				} };
				m_ParentTooltip->AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						UNREFERENCED_PARAMETER(entry);
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

								Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

								// Fill the new script handle
								clientScripts.m_OnUserLeftSession = scriptHandle;
								m_SelectUserLeftSessionSpec.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
							}, {"sessionSlot"});

						} };
				m_ParentTooltip->AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_ParentTooltip->m_TooltipActive = true;

			};

		// Session Initialization Spec
		m_SelectSessionInitSpec.m_Label = "Session Initialization";
		m_SelectSessionInitSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally | EditorUI::SelectOption_Indented;
		m_SelectSessionInitSpec.m_LineCount = 3;
		m_SelectSessionInitSpec.m_CurrentOption = { clientScripts.m_OnCurrentSessionInit ?
			Assets::AssetService::GetScript(clientScripts.m_OnCurrentSessionInit)->m_ScriptName.c_str() : "None",
			clientScripts.m_OnCurrentSessionInit };
		m_SelectSessionInitSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
			{
				spec.ClearOptions();

				spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
				{
					KG_ASSERT(handle != Assets::EmptyHandle);
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
					if (script->m_FuncType != WrappedFuncType::Void_None)
					{
						continue;
					}

					spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
				}

				Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

				spec.m_CurrentOption = { clientScripts.m_OnCurrentSessionInit ?
					Assets::AssetService::GetScript(clientScripts.m_OnCurrentSessionInit)->m_ScriptName.c_str() : "None", clientScripts.m_OnCurrentSessionInit };
			};
		m_SelectSessionInitSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find Session Initialization function in Project Panel");
					return;
				}

				Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };
				clientScripts.m_OnCurrentSessionInit = selection.m_Handle;
			};
		m_SelectSessionInitSpec.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
			{
				UNREFERENCED_PARAMETER(spec);
				// Initialize tooltip with options
				m_ParentTooltip->ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					m_SelectSessionInitSpec.m_OpenPopup = true;
				} };
				m_ParentTooltip->AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						UNREFERENCED_PARAMETER(entry);
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

								Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };
								// Fill the new script handle
								clientScripts.m_OnCurrentSessionInit = scriptHandle;
								m_SelectSessionInitSpec.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
							}, {});

						} };
				m_ParentTooltip->AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_ParentTooltip->m_TooltipActive = true;

			};


		// Connection Terminated Spec
		m_SelectConnectionTerminatedSpec.m_Label = "Connection Terminated";
		m_SelectConnectionTerminatedSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally
			| EditorUI::SelectOption_Indented;
		m_SelectConnectionTerminatedSpec.m_LineCount = 3;
		m_SelectConnectionTerminatedSpec.m_CurrentOption = { clientScripts.m_OnConnectionTerminated ?
			Assets::AssetService::GetScript(clientScripts.m_OnConnectionTerminated)->m_ScriptName.c_str() : "None",
			clientScripts.m_OnConnectionTerminated };
		m_SelectConnectionTerminatedSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
			{
				spec.ClearOptions();

				spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
				{
					KG_ASSERT(handle != Assets::EmptyHandle);
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
					if (script->m_FuncType != WrappedFuncType::Void_None)
					{
						continue;
					}

					spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
				}

				Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

				spec.m_CurrentOption = { clientScripts.m_OnConnectionTerminated ?
					Assets::AssetService::GetScript(clientScripts.m_OnConnectionTerminated)->m_ScriptName.c_str() : "None", clientScripts.m_OnConnectionTerminated };
			};
		m_SelectConnectionTerminatedSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find Connection Terminated function in Project Panel");
					return;
				}
				Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };
				clientScripts.m_OnConnectionTerminated = selection.m_Handle;
			};
		m_SelectConnectionTerminatedSpec.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
			{
				UNREFERENCED_PARAMETER(spec);
				// Initialize tooltip with options
				m_ParentTooltip->ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					m_SelectConnectionTerminatedSpec.m_OpenPopup = true;
				} };
				m_ParentTooltip->AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						UNREFERENCED_PARAMETER(entry);
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
								Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };
								clientScripts.m_OnConnectionTerminated = scriptHandle;
								m_SelectConnectionTerminatedSpec.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
							}, {});

						} };
				m_ParentTooltip->AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_ParentTooltip->m_TooltipActive = true;
			};

		// Update Session User Slot Spec
		m_SelectUpdateSessionSlotSpec.m_Label = "Update Session User Slot";
		m_SelectUpdateSessionSlotSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally | EditorUI::SelectOption_Indented;
		m_SelectUpdateSessionSlotSpec.m_LineCount = 3;
		m_SelectUpdateSessionSlotSpec.m_CurrentOption = { clientScripts.m_OnUpdateSessionUserSlot ?
			Assets::AssetService::GetScript(clientScripts.m_OnUpdateSessionUserSlot)->m_ScriptName.c_str() : "None",
			clientScripts.m_OnUpdateSessionUserSlot };
		m_SelectUpdateSessionSlotSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
			{
				spec.ClearOptions();

				spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
				{
					KG_ASSERT(handle != Assets::EmptyHandle);
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
					if (script->m_FuncType != WrappedFuncType::Void_UInt16)
					{
						continue;
					}

					spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
				}

				Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

				spec.m_CurrentOption = { clientScripts.m_OnUpdateSessionUserSlot ?
					Assets::AssetService::GetScript(clientScripts.m_OnUpdateSessionUserSlot)->m_ScriptName.c_str() : "None", clientScripts.m_OnUpdateSessionUserSlot };
			};
		m_SelectUpdateSessionSlotSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find Update Session User Slot function in Project Panel");
					return;
				}

				Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };
				clientScripts.m_OnUpdateSessionUserSlot = selection.m_Handle;
			};
		m_SelectUpdateSessionSlotSpec.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
			{
				UNREFERENCED_PARAMETER(spec);
				// Initialize tooltip with options
				m_ParentTooltip->ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					m_SelectUpdateSessionSlotSpec.m_OpenPopup = true;
				} };
				m_ParentTooltip->AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						UNREFERENCED_PARAMETER(entry);
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
								Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

								clientScripts.m_OnUpdateSessionUserSlot = scriptHandle;
								m_SelectUpdateSessionSlotSpec.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
							}, {"sessionSlot"});

						} };
				m_ParentTooltip->AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_ParentTooltip->m_TooltipActive = true;
			};


		// Start Session Spec
		m_SelectStartSessionSpec.m_Label = "Start Session";
		m_SelectStartSessionSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally |
			EditorUI::SelectOption_Indented;
		m_SelectStartSessionSpec.m_LineCount = 3;
		m_SelectStartSessionSpec.m_CurrentOption = { clientScripts.m_OnStartSession ?
			Assets::AssetService::GetScript(clientScripts.m_OnStartSession)->m_ScriptName.c_str() : "None", clientScripts.m_OnStartSession };
		m_SelectStartSessionSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
			{
				spec.ClearOptions();

				spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
				{
					KG_ASSERT(handle != Assets::EmptyHandle);
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
					if (script->m_FuncType != WrappedFuncType::Void_None)
					{
						continue;
					}

					spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
				}

				Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };
				spec.m_CurrentOption = { clientScripts.m_OnStartSession ?
					Assets::AssetService::GetScript(clientScripts.m_OnStartSession)->m_ScriptName.c_str() : "None", clientScripts.m_OnStartSession };
			};
		m_SelectStartSessionSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find Start Session function in Project Panel");
					return;
				}

				Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };
				clientScripts.m_OnStartSession = selection.m_Handle;
			};
		m_SelectStartSessionSpec.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
			{
				UNREFERENCED_PARAMETER(spec);
				// Initialize tooltip with options
				m_ParentTooltip->ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					m_SelectStartSessionSpec.m_OpenPopup = true;
				} };
				m_ParentTooltip->AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						UNREFERENCED_PARAMETER(entry);
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
								Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };
								clientScripts.m_OnStartSession = scriptHandle;
								m_SelectStartSessionSpec.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
							}, {});

						} };
				m_ParentTooltip->AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_ParentTooltip->m_TooltipActive = true;
			};


		// Session Ready Check Spec
		m_SelectSessionReadyCheckSpec.m_Label = "Session Ready Check";
		m_SelectSessionReadyCheckSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally | EditorUI::SelectOption_Indented;
		m_SelectSessionReadyCheckSpec.m_LineCount = 3;
		m_SelectSessionReadyCheckSpec.m_CurrentOption = { clientScripts.m_OnSessionReadyCheckConfirm ?
			Assets::AssetService::GetScript(clientScripts.m_OnSessionReadyCheckConfirm)->m_ScriptName.c_str() : "None", clientScripts.m_OnSessionReadyCheckConfirm };
		m_SelectSessionReadyCheckSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
			{
				spec.ClearOptions();

				spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
				{
					KG_ASSERT(handle != Assets::EmptyHandle);
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
					if (script->m_FuncType != WrappedFuncType::Void_None)
					{
						continue;
					}

					spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
				}

				Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

				spec.m_CurrentOption = { clientScripts.m_OnSessionReadyCheckConfirm ?
					Assets::AssetService::GetScript(clientScripts.m_OnSessionReadyCheckConfirm)->m_ScriptName.c_str() : "None", clientScripts.m_OnSessionReadyCheckConfirm };
			};
		m_SelectSessionReadyCheckSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find Session Ready Check function in Project Panel");
					return;
				}

				Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };
				clientScripts.m_OnSessionReadyCheckConfirm = selection.m_Handle;
			};
		m_SelectSessionReadyCheckSpec.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
			{
				UNREFERENCED_PARAMETER(spec);
				// Initialize tooltip with options
				m_ParentTooltip->ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					m_SelectSessionReadyCheckSpec.m_OpenPopup = true;
				} };
				m_ParentTooltip->AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						UNREFERENCED_PARAMETER(entry);
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
								Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };
								clientScripts.m_OnSessionReadyCheckConfirm = scriptHandle;
								m_SelectSessionReadyCheckSpec.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
							}, {});

						} };
				m_ParentTooltip->AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_ParentTooltip->m_TooltipActive = true;
			};


		// On Receive Signal Spec
		m_SelectReceiveSignalSpec.m_Label = "On Receive Signal";
		m_SelectReceiveSignalSpec.m_Flags |= EditorUI::SelectOption_HandleEditButtonExternally |
			EditorUI::SelectOption_Indented;
		m_SelectReceiveSignalSpec.m_LineCount = 3;
		m_SelectReceiveSignalSpec.m_CurrentOption = { clientScripts.m_OnReceiveSignal ?
			Assets::AssetService::GetScript(clientScripts.m_OnReceiveSignal)->m_ScriptName.c_str() : "None", clientScripts.m_OnReceiveSignal };
		m_SelectReceiveSignalSpec.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
			{
				spec.ClearOptions();

				spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
				for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
				{
					KG_ASSERT(handle != Assets::EmptyHandle);
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
					if (script->m_FuncType != WrappedFuncType::Void_UInt16)
					{
						continue;
					}

					spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
				}

				Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };

				spec.m_CurrentOption = { clientScripts.m_OnReceiveSignal ?
					Assets::AssetService::GetScript(clientScripts.m_OnReceiveSignal)->m_ScriptName.c_str() : "None", clientScripts.m_OnReceiveSignal };
			};
		m_SelectReceiveSignalSpec.m_ConfirmAction = [&](const EditorUI::OptionEntry& selection)
			{
				if (!Assets::AssetService::GetScriptRegistry().contains(selection.m_Handle) && selection.m_Handle != Assets::EmptyHandle)
				{
					KG_WARN("Could not find On Receive Signal function in Project Panel");
					return;
				}

				Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };
				clientScripts.m_OnReceiveSignal = selection.m_Handle;
			};
		m_SelectReceiveSignalSpec.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
			{
				UNREFERENCED_PARAMETER(spec);
				// Initialize tooltip with options
				m_ParentTooltip->ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					m_SelectReceiveSignalSpec.m_OpenPopup = true;
				} };
				m_ParentTooltip->AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
						UNREFERENCED_PARAMETER(entry);
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
								Network::ClientScripts& clientScripts{ Projects::ProjectService::GetActiveContext().GetClientScripts() };
								clientScripts.m_OnReceiveSignal = scriptHandle;
								m_SelectReceiveSignalSpec.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
							}, {"signal"});

						} };
				m_ParentTooltip->AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_ParentTooltip->m_TooltipActive = true;
			};
	}
	void ClientOptions::RegisterObservers()
	{
		Network::Client& client{ Network::ClientService::GetActiveContext()};

		Network::ClientNotifiers& clientNotifiers{ client.GetNotifiers() };

		// Client status obclients
		clientNotifiers.AddClientActiveObserver(KG_BIND_CLASS_FN(OnNotifyClientActive));


		// Client packet obclient(s)
		Network::ReliabilityContextNotifiers& reliabilityNotif{ client.GetNetworkThread().GetReliabilityNotifiers() };
		reliabilityNotif.AddSendPacketObserver(KG_BIND_CLASS_FN(OnNotifySendClientPacket));
		reliabilityNotif.AddAckPacketObserver(KG_BIND_CLASS_FN(OnNotifyAckClientPacket));
		reliabilityNotif.AddReliabilityStateObserver(KG_BIND_CLASS_FN(OnNotifyReliabilityState));
		// Client connection obclients
		Network::ClientNetworkNotifiers& netThreadNotifiers{ client.GetNetworkThread().GetNotifiers() };
		netThreadNotifiers.AddConnectStatusObserver(KG_BIND_CLASS_FN(OnNotifyConnectStatus));
	}
	void ClientOptions::OnEditorUIRender()
	{
		EditorUI::EditorUIService::BeginTabBar("ClientOptionsBar");

		if (EditorUI::EditorUIService::BeginTabItem("Status"))
		{
			m_StatusHeader.RenderHeader();
			if (m_StatusHeader.m_Expanded)
			{
				EditorUI::EditorUIService::LabeledText("Status", m_ActiveState ? "Active" : "In-Active",
					EditorUI::LabeledText_Indented);
			}

			m_CommandsHeader.RenderHeader();

			if (m_CommandsHeader.m_Expanded)
			{
				m_LifecycleOptions.RenderBar();
			}

			m_ConnectionHeader.RenderHeader();
			if (m_ConnectionHeader.m_Expanded)
			{
				m_ClientConnectionUI.OnEditorUIRender();
			}

			EditorUI::EditorUIService::EndTabItem();
		}
		if (EditorUI::EditorUIService::BeginTabItem("Config"))
		{
			m_AppScriptsHeader.RenderHeader();
			if (m_AppScriptsHeader.m_Expanded)
			{
				// Select On Receive Signal Function
				m_SelectReceiveSignalSpec.RenderOptions();
			}

			m_ConnectionScriptsHeader.RenderHeader();
			if (m_ConnectionScriptsHeader.m_Expanded)
			{
				// Select On Connection Terminated
				m_SelectConnectionTerminatedSpec.RenderOptions();
			}

			m_SessionScriptsHeader.RenderHeader();
			if (m_SessionScriptsHeader.m_Expanded)
			{
				// Select On Current Session Init
				m_SelectSessionInitSpec.RenderOptions();

				// Select On Update Session User Slot
				m_SelectUpdateSessionSlotSpec.RenderOptions();

				// Select On Start Session
				m_SelectStartSessionSpec.RenderOptions();

				// Select On Session Ready Check
				m_SelectSessionReadyCheckSpec.RenderOptions();

				// Select On Approve Join Session
				m_SelectApproveJoinSessionSpec.RenderOptions();

				// Select On User Left Session
				m_SelectUserLeftSessionSpec.RenderOptions();
			}

			m_QueryServerScriptsHeader.RenderHeader();
			if (m_QueryServerScriptsHeader.m_Expanded)
			{
				// Select On Update User Count
				m_SelectUpdateUserCountSpec.RenderOptions();
			}
			EditorUI::EditorUIService::EndTabItem();
		}

		EditorUI::EditorUIService::EndTabBar();
	}
	bool ClientOptions::OnAssetEvent(Events::Event* event)
	{
		Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;

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
		return false;
	}
	void ClientOptions::OnNotifySendClientPacket(Network::ClientIndex /*index*/, Network::PacketSequence seq)
	{
		EngineService::GetActiveEngine().GetThread().SubmitFunction([&, seq]()
		{
			m_ClientConnectionUI.OnNotifySendPacket(seq);
		});
	}
	void ClientOptions::OnNotifyAckClientPacket(Network::ClientIndex /*index*/, Network::PacketSequence seq, float rtt)
	{
		EngineService::GetActiveEngine().GetThread().SubmitFunction([&, seq, rtt]()
		{
			m_ClientConnectionUI.OnNotifyAckPacket(seq, rtt);
		});
	}
	void ClientOptions::OnNotifyReliabilityState(Network::ClientIndex /*index*/, bool congested, float rtt)
	{
		EngineService::GetActiveEngine().GetThread().SubmitFunction([&, congested, rtt]()
		{
			m_ClientConnectionUI.m_IsCongested = congested;
			m_ClientConnectionUI.m_AverageRTT = rtt;
		});
	}
	void ClientOptions::OnNotifyConnectStatus(Network::ConnectionStatus status, Network::ClientIndex clientIndex)
	{
		EngineService::GetActiveEngine().GetThread().SubmitFunction([&, status, clientIndex]()
		{
			m_ClientConnectionUI.m_ConnectionStatus = status;
			m_ClientConnectionUI.m_ClientIndex = clientIndex;
		});
	}
	void ClientOptions::OnNotifyClientActive(bool active)
	{
		EngineService::GetActiveEngine().GetThread().SubmitFunction([&, active]()
		{
			m_ActiveState = active;
		});
	}
	ConnectionUI::ConnectionUI(Network::ClientIndex index, size_t bufferSize) : m_ClientIndex(index)
	{
		KG_ASSERT(bufferSize > 0);

		m_PacketRTTPlot.SetBufferSize(bufferSize);

		InitWidgets();
	}

	ConnectionUI::ConnectionUI()
	{
		InitWidgets();
	}
	
	void ConnectionUI::InitWidgets()
	{
		m_PacketRTTPlot.m_Label = "Round Trip";
		m_PacketRTTPlot.m_Flags |= EditorUI::Plot_Indented;
		m_PacketRTTPlot.m_MaxYVal = k_MaxRoundTripTime;
		m_PacketRTTPlot.SetYAxisLabel("Time (ms)");
	}
	void ConnectionUI::OnEditorUIRender()
	{
		// Display connection status
		const char* status{ Utility::ConnectionStatusToString(m_ConnectionStatus) };
		EditorUI::EditorUIService::LabeledText("Connection Status", status,
			EditorUI::LabeledText_Indented);

		// Display client index
		FixedString16 indexText( m_ClientIndex == Network::k_InvalidClientIndex ?
			"Invalid" : std::to_string(m_ClientIndex).c_str());
		EditorUI::EditorUIService::LabeledText
		(
			"Client Index", 
			indexText.CString(), 
			EditorUI::LabeledText_Indented
		);

		// Display congested status
		EditorUI::EditorUIService::LabeledText("Congested",
			m_IsCongested ? "Yes" : "No",
			EditorUI::LabeledText_Indented);

		// Display the round trip time
		EditorUI::EditorUIService::LabeledText("Average RTT (ms)",
			std::to_string(m_AverageRTT * 1'000.0f).c_str(),
			EditorUI::LabeledText_Indented);
		m_PacketRTTPlot.RenderPlot();
	}
	void ConnectionUI::OnNotifySendPacket(Network::PacketSequence seq)
	{
		m_LastSequence = seq;
		m_PacketRTTPlot.AddValue(k_MaxRoundTripTime);
	}
	void ConnectionUI::OnNotifyAckPacket(Network::PacketSequence seq, float rtt)
	{
		KG_ASSERT(m_LastSequence >= seq);

		m_PacketRTTPlot.UpdateValue(rtt * 1'000, m_LastSequence - seq);
	}
}
