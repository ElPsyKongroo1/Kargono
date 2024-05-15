#include "Panels/UIEditorPanel.h"

#include "EditorLayer.h"

#include "Kargono.h"

namespace Kargono
{
	static EditorLayer* s_EditorLayer { nullptr };

	static void DisplayWidgetSpecificInfo(Ref<RuntimeUI::Widget> widget, int32_t selectedWidget)
	{
		switch (widget->WidgetType)
		{
		case RuntimeUI::WidgetTypes::TextWidget:
		{
			RuntimeUI::TextWidget* textWidget = (RuntimeUI::TextWidget*)widget.get();

			char buffer[256] = {};
			strncpy_s(buffer, textWidget->Text.c_str(), sizeof(buffer));
			if (ImGui::Button(textWidget->Text.c_str()))
			{
				ImGui::OpenPopup((std::string("##Input Text") + std::to_string(selectedWidget)).c_str());
			}
			ImGui::SameLine();
			ImGui::Text("Widget Text");

			if (ImGui::BeginPopup((std::string("##Input Text") + std::to_string(selectedWidget)).c_str()))
			{
				ImGui::InputTextMultiline((std::string("##Input Text") + std::to_string(selectedWidget)).c_str(),
					buffer, sizeof(buffer),
					ImVec2(0, 0), ImGuiInputTextFlags_CtrlEnterForNewLine);
				if (ImGui::IsWindowFocused() && ImGui::IsKeyDown(ImGuiKey_Enter) && !(ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)))
				{
					textWidget->SetText(std::string(buffer));
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			ImGui::DragFloat((std::string("Text Size##") + std::to_string(selectedWidget)).c_str(), &textWidget->TextSize,
				0.01f, 0.0f, 5.0f);
			ImGui::ColorEdit4("Text Color", glm::value_ptr(textWidget->TextColor));
			ImGui::Checkbox("Toggle Centered", &textWidget->TextCentered);
			break;
		}
		default:
			KG_ASSERT(false, "Invalid Widget Type Presented!");
			break;
		}
	}

	UIEditorPanel::UIEditorPanel()
	{
		s_EditorLayer = EditorLayer::GetCurrentLayer();
	}

	void UIEditorPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		int32_t windowIteration{ 1 };
		int32_t& windowToDelete = RuntimeUI::Runtime::GetWindowToDelete();
		int32_t& widgetToDelete = RuntimeUI::Runtime::GetWidgetToDelete();
		int32_t& windowsToAddWidget = RuntimeUI::Runtime::GetWindowsToAddWidget();
		RuntimeUI::WidgetTypes& widgetTypeToAdd = RuntimeUI::Runtime::GetWidgetTypeToAdd();
		uint32_t& windowToAdd = RuntimeUI::Runtime::GetWindowToAdd();
		int32_t& selectedWindow = RuntimeUI::Runtime::GetSelectedWindow();
		int32_t& selectedWidget = RuntimeUI::Runtime::GetSelectedWidget();

		EditorUI::Editor::StartWindow("User Interface Editor", &s_EditorLayer->m_ShowUserInterfaceEditor);

		Assets::AssetHandle currentUIHandle = RuntimeUI::Runtime::GetCurrentUIHandle();
		if (ImGui::BeginCombo("##Select User Interface", static_cast<bool>(currentUIHandle) ? Assets::AssetManager::GetUIObjectLocation(currentUIHandle).string().c_str() : "None"))
		{
			if (ImGui::Selectable("None"))
			{
				RuntimeUI::Runtime::ClearUIEngine();
			}
			for (auto& [uuid, asset] : Assets::AssetManager::GetUIObjectRegistry())
			{
				if (ImGui::Selectable(asset.Data.IntermediateLocation.string().c_str()))
				{
					RuntimeUI::Runtime::ClearUIEngine();

					RuntimeUI::Runtime::LoadUIObject(Assets::AssetManager::GetUIObject(uuid), uuid);
				}
			}
			ImGui::EndCombo();
		}

		ImGui::SameLine();

		if (ImGui::Button("Save Current User Interface"))
		{
			if (RuntimeUI::Runtime::SaveCurrentUIIntoUIObject())
			{
				Assets::AssetManager::SaveUIObject(RuntimeUI::Runtime::GetCurrentUIHandle(), RuntimeUI::Runtime::GetCurrentUIObject());
			}
		}
		ImGui::SameLine();

		if (ImGui::Button("Create New User Interface"))
		{
			ImGui::OpenPopup("Create New User Interface");
		}

		if (ImGui::BeginPopup("Create New User Interface"))
		{
			static char buffer[256];
			memset(buffer, 0, 256);
			ImGui::InputText("New User Interface Name", buffer, sizeof(buffer));
			if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
			{
				RuntimeUI::Runtime::ClearUIEngine();

				Assets::AssetHandle newHandle = Assets::AssetManager::CreateNewUIObject(std::string(buffer));
				RuntimeUI::Runtime::LoadUIObject(Assets::AssetManager::GetUIObject(newHandle), newHandle);

				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (!RuntimeUI::Runtime::GetCurrentUIObject())
		{
			EditorUI::Editor::EndWindow();
			return;
		}

		if (ImGui::ColorEdit4("Select Color", glm::value_ptr(RuntimeUI::Runtime::GetSelectColor())))
		{
			RuntimeUI::Runtime::SetSelectedWidgetColor(RuntimeUI::Runtime::GetSelectColor());
		}

		if (ImGui::BeginCombo(("OnMove##" + std::to_string(selectedWidget)).c_str(), RuntimeUI::Runtime::GetFunctionOnMove().empty() ? "None" : RuntimeUI::Runtime::GetFunctionOnMove().c_str()))
		{
			if (ImGui::Selectable("None"))
			{
				RuntimeUI::Runtime::SetFunctionOnMove({});
			}

			for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
			{
				if (script.NumParameters > 0) { continue; }

				if (ImGui::Selectable(name.c_str()))
				{
					RuntimeUI::Runtime::SetFunctionOnMove(name);
				}
			}

			ImGui::EndCombo();
		}

		// Main window
		if (ImGui::BeginTable("All Windows", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			for (auto& window : RuntimeUI::Runtime::GetAllWindows())
			{
				ImGui::AlignTextToFramePadding();
				ImGuiTreeNodeFlags windowFlags = ((selectedWindow == windowIteration && selectedWidget == -1) ? ImGuiTreeNodeFlags_Selected : 0) |
					ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
				bool node_open = ImGui::TreeNodeEx(("Window " + std::to_string(windowIteration)).c_str(), windowFlags);
				if (ImGui::IsItemClicked())
				{
					selectedWindow = windowIteration;
					selectedWidget = -1;
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					ImGui::OpenPopup((std::string("RightClickOptions##UIWindow") + std::to_string(windowIteration)).c_str());
				}

				if (ImGui::BeginPopup((std::string("RightClickOptions##UIWindow") + std::to_string(windowIteration)).c_str()))
				{
					if (ImGui::Selectable((std::string("Add Text Widget##") + std::to_string(windowIteration)).c_str()))
					{
						windowsToAddWidget = windowIteration;
						widgetTypeToAdd = RuntimeUI::WidgetTypes::TextWidget;
					}

					if (ImGui::Selectable((std::string("Delete Window##") + std::to_string(windowIteration)).c_str()))
					{
						windowToDelete = windowIteration;
					}
					ImGui::EndPopup();
				}

				if (node_open)
				{
					uint32_t widgetIteration{ 1 };

					for (auto& widget : window.Widgets)
					{
						ImGuiTreeNodeFlags widgetFlags = ((selectedWidget == widgetIteration) ? ImGuiTreeNodeFlags_Selected : 0) |
							ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
						bool widgetOpened = ImGui::TreeNodeEx(("Widget " + std::to_string(widgetIteration) + std::string(" ##") + std::to_string(windowIteration)).c_str(), widgetFlags);
						if (ImGui::IsItemClicked())
						{
							selectedWindow = windowIteration;
							selectedWidget = widgetIteration;
						}

						if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
						{
							ImGui::OpenPopup((std::string("RightClickOptions##UIWidget") + std::to_string(widgetIteration)).c_str());
						}

						if (ImGui::BeginPopup((std::string("RightClickOptions##UIWidget") + std::to_string(widgetIteration)).c_str()))
						{
							if (ImGui::Selectable((std::string("Delete Widget##") + std::to_string(widgetIteration)).c_str()))
							{
								selectedWindow = windowIteration;
								widgetToDelete = widgetIteration;
							}
							ImGui::EndPopup();
						}

						if (widgetOpened) { ImGui::TreePop(); }
						widgetIteration++;
					}

					ImGui::TreePop();
				}

				windowIteration++;
			}

			if (ImGui::Button("Add Window"))
			{
				windowToAdd++;
			}

			ImGui::TableSetColumnIndex(1);
			if (selectedWindow != -1)
			{
				if (selectedWidget != -1)
				{
					auto& widget = RuntimeUI::Runtime::GetAllWindows().at(selectedWindow - 1).Widgets.at(selectedWidget - 1);
					char buffer[256] = {};
					strncpy_s(buffer, widget->Tag.c_str(), sizeof(buffer));
					ImGui::Text(widget->Tag.c_str());
					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						ImGui::OpenPopup("UpdateWidgetTag");
					}

					if (ImGui::BeginPopup("UpdateWidgetTag"))
					{
						ImGui::InputText("##WidgetTag", buffer, sizeof(buffer));
						if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
						{
							widget->Tag = std::string(buffer);
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
					ImGui::Separator();
					ImGui::Checkbox((std::string("Selectable##") + std::to_string(selectedWidget)).c_str(), &widget->Selectable);
					ImGui::DragFloat2((std::string("Widget Location##") + std::to_string(selectedWidget)).c_str(),
						glm::value_ptr(widget->WindowPosition), 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat2((std::string("Widget Size##") + std::to_string(selectedWidget)).c_str(), glm::value_ptr(widget->Size), 0.01f, 0.0f, 1.0f);
					if (ImGui::ColorEdit4(("Background Color##" + std::to_string(selectedWidget)).c_str(), glm::value_ptr(widget->DefaultBackgroundColor)))
					{
						widget->ActiveBackgroundColor = widget->DefaultBackgroundColor;
					}
					if (ImGui::BeginCombo(("OnPress##" + std::to_string(selectedWidget)).c_str(), widget->FunctionPointers.OnPress.empty() ? "None" : widget->FunctionPointers.OnPress.c_str()))
					{
						if (ImGui::Selectable("None"))
						{
							widget->FunctionPointers.OnPress = {};
						}

						for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
						{
							if (script.NumParameters > 0) { continue; }

							if (ImGui::Selectable(name.c_str()))
							{
								widget->FunctionPointers.OnPress = name;
							}
						}

						ImGui::EndCombo();
					}
					DisplayWidgetSpecificInfo(widget, selectedWidget);
				}
				else
				{
					auto& window = RuntimeUI::Runtime::GetAllWindows().at(selectedWindow - 1);
					char buffer[256] = {};
					strncpy_s(buffer, window.Tag.c_str(), sizeof(buffer));
					ImGui::Text(window.Tag.c_str());
					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						ImGui::OpenPopup("UpdateWindowTag");
					}

					if (ImGui::BeginPopup("UpdateWindowTag"))
					{
						ImGui::InputText("##WindowTag", buffer, sizeof(buffer));
						if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
						{
							window.Tag = std::string(buffer);
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}

					ImGui::Separator();
					int32_t widgetLocation = -1;
					if (window.DefaultActiveWidgetRef)
					{
						auto iterator = std::find(window.Widgets.begin(), window.Widgets.end(), window.DefaultActiveWidgetRef);
						widgetLocation = static_cast<int32_t>(iterator - window.Widgets.begin()) + 1;
					}
					if (ImGui::BeginCombo(("Default Widget##" + std::to_string(selectedWindow)).c_str(),
						widgetLocation == -1 ? "None" : (std::string("Widget ") + std::to_string(widgetLocation)).c_str()))
					{
						if (ImGui::Selectable("None"))
						{
							window.DefaultActiveWidgetRef = nullptr;
						}
						uint32_t iteration{ 0 };
						for (auto& widget : window.Widgets)
						{
							if (ImGui::Selectable(("Widget " + std::to_string(iteration + 1) + " (" + widget->Tag + ")").c_str()))
							{
								window.DefaultActiveWidgetRef = window.Widgets.at(iteration);
							}
							iteration++;
						}
						ImGui::EndCombo();
					}
					bool windowDisplayed = window.GetWindowDisplayed();

					if (ImGui::Checkbox((std::string("Display Window##") + std::to_string(selectedWindow)).c_str(), &windowDisplayed))
					{
						if (windowDisplayed)
						{
							window.DisplayWindow();
						}
						if (!windowDisplayed)
						{
							window.HideWindow();
						}
					}
					ImGui::DragFloat3((std::string("Window Location##") + std::to_string(selectedWindow)).c_str(), glm::value_ptr(window.ScreenPosition), 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat2((std::string("Window Size##") + std::to_string(selectedWindow)).c_str(), glm::value_ptr(window.Size), 0.01f, 0.0f, 1.0f);
					ImGui::ColorEdit4("Background Color", glm::value_ptr(window.BackgroundColor));
				}

			}

			ImGui::EndTable();
		}

		EditorUI::Editor::EndWindow();

		if (windowToDelete != -1)
		{
			RuntimeUI::Runtime::DeleteWindow(static_cast<uint32_t>(windowToDelete - 1));
			windowToDelete = -1;

			selectedWindow = -1;
			selectedWidget = -1;
		}

		if (widgetToDelete != -1)
		{
			auto& window = RuntimeUI::Runtime::GetAllWindows().at(selectedWindow - 1);
			window.DeleteWidget(widgetToDelete - 1);

			widgetToDelete = -1;
			selectedWidget = -1;

		}

		if (windowsToAddWidget != -1)
		{
			auto& windows = RuntimeUI::Runtime::GetAllWindows();
			switch (widgetTypeToAdd)
			{
			case RuntimeUI::WidgetTypes::TextWidget:
			{
				RuntimeUI::Runtime::GetAllWindows().at(windowsToAddWidget - 1).AddTextWidget(CreateRef<RuntimeUI::TextWidget>());
				break;
			}
			default:
			{
				KG_ASSERT(false, "Invalid widgetTypeToAdd value");
				break;
			}
			}

			windowsToAddWidget = -1;
		}

		if (windowToAdd != 0)
		{
			RuntimeUI::Window window1 {};
			window1.Size = Math::vec2(0.4f, 0.4f);
			window1.ScreenPosition = Math::vec3(0.3f, 0.3f, 0.0f);
			RuntimeUI::Runtime::AddWindow(window1);
			windowToAdd = 0;
		}
	}
}
