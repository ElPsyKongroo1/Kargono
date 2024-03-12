#include "kgpch.h"

#include "Kargono/UI/Editor.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Input/InputMode.h"

#define IMGUI_IMPL_API
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include "ImGuizmo.h"
#include "imgui_internal.h"



namespace Kargono::UI
{
	ImFont* Editor::s_AntaRegular{ nullptr };
	ImFont* Editor::s_AntaSmall{ nullptr };
	ImFont* Editor::s_PlexBold{ nullptr };
	ImFont* Editor::s_OpenSansRegular{ nullptr };
	ImFont* Editor::s_OpenSansBold{ nullptr };
	ImFont* Editor::s_RobotoRegular{ nullptr };

	Ref<Texture2D> Editor::s_IconPlay{};
	Ref<Texture2D> Editor::s_IconPause{};
	Ref<Texture2D> Editor::s_IconStop{};
	Ref<Texture2D> Editor::s_IconStep{};
	Ref<Texture2D> Editor::s_IconSimulate{};
	Ref<Texture2D> Editor::s_IconAddItem{};
	Ref<Texture2D> Editor::s_IconSettings{};
	Ref<Texture2D> Editor::s_IconDelete{};
	Ref<Texture2D> Editor::s_IconEdit{};
	Ref<Texture2D> Editor::s_IconEdit_Active{};
	Ref<Texture2D> Editor::s_IconCancel{};
	Ref<Texture2D> Editor::s_IconConfirm{};
	Ref<Texture2D> Editor::s_IconSearch{};
	Ref<Texture2D> Editor::s_IconCheckbox_Empty_Disabled{};
	Ref<Texture2D> Editor::s_IconCheckbox_Check_Disabled{};
	Ref<Texture2D> Editor::s_IconCheckbox_Empty_Enabled{};
	Ref<Texture2D> Editor::s_IconCheckbox_Check_Enabled{};

	Ref<Texture2D> Editor::s_DirectoryIcon{};
	Ref<Texture2D> Editor::s_GenericFileIcon{};
	Ref<Texture2D> Editor::s_BackIcon{};
	Ref<Texture2D> Editor::s_AudioIcon{};
	Ref<Texture2D> Editor::s_ImageIcon{};
	Ref<Texture2D> Editor::s_BinaryIcon{};
	Ref<Texture2D> Editor::s_SceneIcon{};
	Ref<Texture2D> Editor::s_RegistryIcon{};
	Ref<Texture2D> Editor::s_ScriptProjectIcon{};
	Ref<Texture2D> Editor::s_UserInterfaceIcon{};
	Ref<Texture2D> Editor::s_FontIcon{};
	Ref<Texture2D> Editor::s_InputIcon{};

	void SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = Editor::s_LightPurple_Thin;
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	}

	void Editor::Init()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		float fontSize = 18.0f;
		s_OpenSansBold = io.Fonts->AddFontFromFileTTF("resources/fonts/opensans/static/OpenSans-Bold.ttf", fontSize);
		s_OpenSansRegular = io.Fonts->AddFontFromFileTTF("resources/fonts/opensans/static/OpenSans-Regular.ttf", fontSize);
		s_AntaRegular = io.Fonts->AddFontFromFileTTF("resources/fonts/Anta-Regular.ttf", 20.0f);
		s_AntaSmall = io.Fonts->AddFontFromFileTTF("resources/fonts/Anta-Regular.ttf", 18.0f);
		s_PlexBold = io.Fonts->AddFontFromFileTTF("resources/fonts/IBMPlexMono-Bold.ttf", 29.0f);
		s_RobotoRegular = io.Fonts->AddFontFromFileTTF("resources/fonts/Roboto-Regular.ttf", 18.0f);
		io.FontDefault = s_AntaSmall;
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();
		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		SetDarkThemeColors();

		// Setup Platform/Renderer backends
		Application& app = Application::GetCurrentApp();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		// Set Up Editor Resources
		s_IconPlay = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/play_icon.png").string());
		s_IconPause = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/pause_icon.png").string());
		s_IconSimulate = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/simulate_icon.png").string());
		s_IconStop = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/stop_icon.png").string());
		s_IconStep = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/step_icon.png").string());
		s_IconAddItem = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/add_item.png").string());
		s_IconSettings = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/settings_icon.png").string());
		s_IconDelete = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/delete_icon.png").string());
		s_IconEdit = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/edit_icon.png").string());
		s_IconEdit_Active = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/edit_active_icon.png").string());
		s_IconCancel = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/cancel_icon.png").string());
		s_IconConfirm = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/confirm_icon.png").string());
		s_IconSearch = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/search_icon.png").string());

		s_IconCheckbox_Empty_Disabled = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/checkbox/checkbox_disabled_empty_icon.png").string());
		s_IconCheckbox_Check_Disabled = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/checkbox/checkbox_disabled_check_icon.png").string());
		s_IconCheckbox_Empty_Enabled = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/checkbox/checkbox_enabled_empty_icon.png").string());
		s_IconCheckbox_Check_Enabled = Texture2D::CreateEditorTexture((Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/checkbox/checkbox_enabled_check_icon.png").string());

		s_DirectoryIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/directory_icon.png");
		s_GenericFileIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/generic_file_icon.png");
		s_BackIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/back_icon.png");
		s_AudioIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/audio_icon.png");
		s_ImageIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/image_icon.png");
		s_BinaryIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/binary_icon.png");
		s_SceneIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/scene_icon.png");
		s_RegistryIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/registry_icon.png");
		s_ScriptProjectIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/script_project.png");
		s_FontIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/font.png");
		s_UserInterfaceIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/user_interface_icon.png");
		s_InputIcon = Texture2D::CreateEditorTexture(Application::GetCurrentApp().GetWorkingDirectory() / "resources/icons/content_browser/input_icon.png");

		s_Running = true;
	}

	void Editor::Terminate()
	{
		if (s_Running)
		{
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			s_Running = false;
		}
	}

	void Editor::Begin()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void Editor::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::GetCurrentApp();
		io.DisplaySize = ImVec2(static_cast<float>(app.GetWindow().GetWidth()), static_cast<float>(app.GetWindow().GetHeight()));

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	uint32_t Editor::GetActiveWidgetID()
	{
		return GImGui->ActiveId;
	}

	void Editor::OnEvent(Events::Event& e)
	{
		KG_PROFILE_FUNCTION();

		if (s_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(Events::Mouse) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(Events::Keyboard) & io.WantCaptureKeyboard;
		}

	}

	void Editor::Spacing(float space)
	{
		ImGui::Dummy(ImVec2(0.0f, space));
	}

	void Editor::Spacing(SpacingAmount space)
	{
		switch (space)
		{
			case SpacingAmount::Small:
			{
				ImGui::Dummy(ImVec2(0.0f, 3.0f));
				return;
			}
			case SpacingAmount::Medium:
			{
				ImGui::Dummy(ImVec2(0.0f, 8.0f));
				return;
			}
			case SpacingAmount::Large:
			{
				ImGui::Dummy(ImVec2(0.0f, 15.0f));
				return;
			}
			case SpacingAmount::None:
			default:
			{
				KG_WARN("Invalid SpacingAmount provided to Spacing() enum function");
				return;
			}
		}
	}

	OptionsList GenerateRegexCache(OptionsList& originalList, std::string regexQuery)
	{
		std::regex searchQuery {"(" + regexQuery + ")"};
		std::smatch matches;
		OptionsList returnList{};

		for (auto& [title, options] : originalList)
		{
			std::vector<std::string> returnOptions {};
			for (auto& option : options)
			{
				if (!std::regex_search(option, matches, searchQuery))
				{
					continue;
				}
				returnOptions.push_back(option);
			}
			if (!returnOptions.empty())
			{
				returnList.insert_or_assign(title, returnOptions);
			}
		}

		return returnList;
	}

	void WriteMultilineText(const std::string& text)
	{
		std::string previewOutput{};
		std::string previewRemainder{ text };
		uint32_t iteration{ 0 };
		float cachedDistance{ 0.0f };
		do
		{
			ImVec2 textSize = ImGui::CalcTextSize(previewRemainder.c_str());
			if (textSize.x > 240.0f)
			{
				if (iteration == 0)
				{
					previewOutput = previewRemainder.substr(0, 29);
					previewRemainder = previewRemainder.substr(29, std::string::npos);
					textSize = ImGui::CalcTextSize(previewOutput.c_str());
				}
				else 
				{
					previewOutput = previewRemainder.substr(0, 27);
					previewRemainder = previewRemainder.substr(27, std::string::npos);
					textSize = ImGui::CalcTextSize(previewOutput.c_str());
				}
				
			}
			else
			{
				previewOutput = previewRemainder.c_str();
				previewRemainder.clear();
			}

			if (iteration == 0)
			{
				cachedDistance = ImGui::GetWindowWidth() - 47 - textSize.x;
				ImGui::SameLine(cachedDistance);
			}
			else
			{
				ImGui::SameLine(cachedDistance);
			}

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + iteration * 20.0f);
			ImGui::TextColored(UI::Editor::s_PearlBlue, previewOutput.c_str());
			iteration++;
		} while (!previewRemainder.empty());
	}

	void Editor::SelectOption(SelectOptionSpec& spec)
	{
		// Local Variables
		static std::unordered_map<std::string, std::string> s_CachedSelections {};
		static std::unordered_map<std::string, bool> s_CachedSearching {};
		static std::unordered_map<std::string, OptionsList> s_CachedRegexSearch {};
		std::string id = "##" + std::to_string(spec.WidgetID);
		std::string popUpLabel = "Set " + spec.Label;

		// Display Menu Item
		ImGui::TextColored(s_PureWhite, spec.Label.c_str());

		WriteMultilineText(spec.CurrentOption);

		ImGui::SameLine(ImGui::GetWindowWidth() - 40);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 0.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		if (ImGui::ImageButtonEx(static_cast<ImGuiID>(spec.WidgetID + 1),
			(ImTextureID)(uint64_t)UI::Editor::s_IconEdit->GetRendererID(),
			ImVec2(14, 14), ImVec2{0, 1}, ImVec2{1, 0},
			ImVec4(0.0f, 0.0f, 0.0f, 0.0f), s_PureWhite, 0))
		{
			ImGui::OpenPopup(id.c_str());
		}
		ImGui::PopStyleColor();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::TextColored(s_PearlBlue, "Edit");
			ImGui::EndTooltip();
		}

		// Display Popup
		ImGui::SetNextWindowSize(ImVec2(700.0f, 0.0f));
		if (ImGui::BeginPopupModal(id.c_str(), NULL, ImGuiWindowFlags_NoTitleBar))
		{
			if (spec.PopupAction)
			{
				spec.PopupAction(spec);
			}

			static char searchBuffer[256];

			if (!s_CachedSelections.contains(spec.WidgetID))
			{
				s_CachedSelections.insert_or_assign(spec.WidgetID, spec.CurrentOption);
			}

			if (!s_CachedSearching.contains(spec.WidgetID))
			{
				s_CachedSearching.insert_or_assign(spec.WidgetID, false);
			}

			ImGui::PushFont(UI::Editor::s_PlexBold);
			ImGui::TextColored(s_PearlBlue, popUpLabel.c_str());
			ImGui::PopFont();

			ImGui::PushFont(UI::Editor::s_AntaRegular);
			if (s_CachedSearching.at(spec.WidgetID))
			{
				ImGui::SameLine(ImGui::GetWindowWidth() - 124.0f - 200.0f);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.6f);
				ImGui::SetNextItemWidth(200.0f);

				ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData* data)
				{
					std::string currentData = std::string(data->Buf);
					SelectOptionSpec* providedSpec = (SelectOptionSpec*)data->UserData;
					s_CachedRegexSearch.insert_or_assign(providedSpec->WidgetID, GenerateRegexCache(providedSpec->GetOptionsList(), currentData));
					return 0;
				};

				ImGui::InputText((id + "InputText").c_str(), searchBuffer, sizeof(searchBuffer), ImGuiInputTextFlags_CallbackEdit, callback, (void*)&spec);
			}

			ImGui::SameLine(ImGui::GetWindowWidth() - 112.0f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + -0.6f);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			if (ImGui::ImageButtonEx(static_cast<ImGuiID>(spec.WidgetID + 2), (ImTextureID)(uint64_t)UI::Editor::s_IconSearch->GetRendererID(), ImVec2(28, 28),
				ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), s_PureWhite, 0))
			{
				if (s_CachedSearching.at(spec.WidgetID))
				{
					s_CachedSearching.insert_or_assign(spec.WidgetID, false);
				}
				else
				{
					s_CachedSearching.insert_or_assign(spec.WidgetID, true);
					s_CachedRegexSearch.insert_or_assign(spec.WidgetID, GenerateRegexCache(spec.GetOptionsList(), searchBuffer));
				}
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				const char* searchTooltip = s_CachedSearching.at(spec.WidgetID) ? "Cancel Search" : "Search";
				ImGui::BeginTooltip();
				ImGui::TextColored(s_PearlBlue, searchTooltip);
				ImGui::EndTooltip();
			}

			ImGui::SameLine(ImGui::GetWindowWidth() - 75.0f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + -0.6f);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			if (ImGui::ImageButtonEx(static_cast<ImGuiID>(spec.WidgetID + 3), (ImTextureID)(uint64_t)UI::Editor::s_IconCancel->GetRendererID(), ImVec2(28, 28),
				ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
				s_PureWhite, 0))
			{
				if (!s_CachedSelections.contains(spec.WidgetID) || s_CachedSelections.at(spec.WidgetID) != spec.CurrentOption)
				{
					s_CachedSelections.insert_or_assign(spec.WidgetID, spec.CurrentOption);
				}

				s_CachedSearching.insert_or_assign(spec.WidgetID, false);
				memset(searchBuffer, 0, sizeof(searchBuffer));

				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(s_PearlBlue, "Cancel");
				ImGui::EndTooltip();
			}

			ImGui::SameLine(ImGui::GetWindowWidth() - 38);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + -0.6f);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			if (ImGui::ImageButtonEx(static_cast<ImGuiID>(spec.WidgetID + 4),
				(ImTextureID)(uint64_t)UI::Editor::s_IconConfirm->GetRendererID(),
				ImVec2(28, 28), ImVec2{ 0, 1 },
				ImVec2{ 1, 0 }, ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
				s_PureWhite, 0))
			{
				if (s_CachedSelections.contains(spec.WidgetID))
				{
					spec.ConfirmAction(s_CachedSelections.at(spec.WidgetID));
				}

				s_CachedSearching.insert_or_assign(spec.WidgetID, false);
				memset(searchBuffer, 0, sizeof(searchBuffer));

				ImGui::CloseCurrentPopup();
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextColored(s_PearlBlue, "Confirm");
				ImGui::EndTooltip();
			}
			ImGui::PopStyleColor();

			ImGui::Separator();
			
			for (auto& [title, options] : 
				s_CachedSearching.at(spec.WidgetID) ? s_CachedRegexSearch.at(spec.WidgetID) : spec.GetOptionsList())
			{
				ImGui::Text(title.c_str());
				uint32_t iteration{ 1 };
				bool selectedButton = false;
				for (auto& option : options)
				{
					if (s_CachedSelections.at(spec.WidgetID) == option)
					{
						selectedButton = true;
					}

					if (selectedButton)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, s_DarkPurple);
					}

					if (ImGui::Button((option + id).c_str()))
					{
						spec.CurrentOption = option;
						s_CachedSelections.insert_or_assign(spec.WidgetID, option);
					}
					if (iteration % spec.LineCount != 0 && iteration != 0 && iteration != options.size())
					{
						ImGui::SameLine();
					}

					if (selectedButton)
					{
						ImGui::PopStyleColor();
						selectedButton = false;
					}
					iteration++;
				}
				Spacing(SpacingAmount::Medium);
			}

			ImGui::PopFont();
			ImGui::EndPopup();
		}
	}
	

	void Editor::Checkbox(CheckboxSpec& spec)
	{
		// Local Variables
		std::string id = "##" + std::to_string(spec.WidgetID);
		static std::unordered_map<std::string, bool> s_CachedEditing{};

		if (!s_CachedEditing.contains(spec.WidgetID))
		{
			s_CachedEditing.insert_or_assign(spec.WidgetID, false);
		}

		// Display Item

		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::TextColored(s_PureWhite, spec.Label.c_str());
		ImGui::SameLine(ImGui::GetWindowWidth() - 52 - 14);

		if (s_CachedEditing.at(spec.WidgetID))
		{
			Ref<Texture2D> iconChoice = spec.ToggleBoolean ? UI::Editor::s_IconCheckbox_Check_Enabled : UI::Editor::s_IconCheckbox_Empty_Enabled;
			if (ImGui::ImageButtonEx(static_cast<ImGuiID>(spec.WidgetID + 1),
				(ImTextureID)(uint64_t)iconChoice->GetRendererID(),
				ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
				ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
				s_PureWhite, 0))
			{
				if (spec.ToggleBoolean)
				{
					spec.ConfirmAction(false);
				}
				else
				{
					spec.ConfirmAction(true);
				}
			}
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			Ref<Texture2D> iconChoice = spec.ToggleBoolean ? UI::Editor::s_IconCheckbox_Check_Disabled : UI::Editor::s_IconCheckbox_Empty_Disabled;
			if (ImGui::ImageButtonEx(static_cast<ImGuiID>(spec.WidgetID + 2),
				(ImTextureID)(uint64_t)iconChoice->GetRendererID(),
				ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
				ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
				s_PureWhite, 0))
			{
				
			}
			ImGui::PopStyleColor();
		}

		ImGui::PopStyleColor(2);

		ImGui::SameLine(ImGui::GetWindowWidth() - 40);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 0.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		Ref<Texture2D> iconChoice = s_CachedEditing.at(spec.WidgetID) ? UI::Editor::s_IconEdit_Active : UI::Editor::s_IconEdit;
		if (ImGui::ImageButtonEx(static_cast<ImGuiID>(spec.WidgetID + 3),
			(ImTextureID)(uint64_t)iconChoice->GetRendererID(),
			ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
			ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
			s_PureWhite, 0))
		{
			if (s_CachedEditing.at(spec.WidgetID))
			{
				s_CachedEditing.insert_or_assign(spec.WidgetID, false);
			}
			else
			{
				s_CachedEditing.insert_or_assign(spec.WidgetID, true);
			}
		}
		ImGui::PopStyleColor();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::TextColored(s_PearlBlue, s_CachedEditing.at(spec.WidgetID) ? "Cancel Editing" : "Edit");
			ImGui::EndTooltip();
		}

	}

	void Editor::Text(const std::string& label, const std::string& text)
	{
		// Display Menu Item
		ImGui::TextColored(s_PureWhite, label.c_str());

		WriteMultilineText(text);

	}
}
