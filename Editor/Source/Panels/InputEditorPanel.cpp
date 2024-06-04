#include "Panels/InputEditorPanel.h"

#include "EditorApp.h"

namespace Kargono
{
	static EditorApp* s_EditorApp { nullptr };

	void InputEditorPanel::InputEditor_Keyboard_OnUpdate()
	{
		if (ImGui::TreeNodeEx("OnUpdate##Keyboard", ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			// Toggle and Delayed Results Variables
			// Delayed Update Variables. Update Data structure after it has been fully output.

			bool updateType = false;
			Math::ivec2 updateTypeValues{};
			bool deleteCustomCalls = false;
			bool deleteScriptClass = false;
			bool switchCustomToScriptClass = false;
			bool switchScriptClassToCustomCalls = false;
			bool updateClassName = false;
			std::string newClassName {};
			InputMode::InputActionBinding* bindingToUpdate{};

			// Enable Delete Option
			static bool deleteMenuToggle = false;

			// Settings Section
			ImGui::SameLine();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			if (ImGui::ImageButton((ImTextureID)(uint64_t)EditorUI::Editor::s_IconSettings->GetRendererID(), ImVec2(17, 17), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
			{
				ImGui::OpenPopup("KeyboardOnUpdateSettings");
			}
			ImGui::PopStyleColor();

			if (ImGui::BeginPopup("KeyboardOnUpdateSettings"))
			{
				if (ImGui::Selectable("Add New Slot"))
				{
					InputMode::AddKeyboardCustomCallsOnUpdateSlot();
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Toggle Delete Option", deleteMenuToggle))
				{
					deleteMenuToggle = deleteMenuToggle ? false : true; // Conditional Toggles Boolean
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			// Main Table that lists audio slots and their corresponding AudioComponents
			static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
			if (ImGui::BeginTable("KeyboardOnUpdateTable", deleteMenuToggle ? 4 : 3, flags))
			{
				ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Keyboard Key", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthStretch);
				if (deleteMenuToggle) { ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, 20.0f); }
				ImGui::TableHeadersRow();
				uint32_t customCallsIterator{ 0 };
				for (auto& binding : InputMode::GetKeyboardCustomCallsOnUpdate())
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 6.0f);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
					if (ImGui::BeginCombo(("##KeyboardCustomCallsOnUpdateType" + std::to_string(customCallsIterator)).c_str(), "CustomCalls", ImGuiComboFlags_NoArrowButton))
					{
						if (ImGui::Selectable("ScriptClass"))
						{
							switchCustomToScriptClass = true;
							bindingToUpdate = binding;
						}

						ImGui::EndCombo();
					}

					ImGui::SameLine();
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 6.0f);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
					if (ImGui::BeginCombo(("##KeyboardCustomCallsOnUpdateKey" + std::to_string(customCallsIterator)).c_str(), ("Key: " + Utility::KeyCodeToString(binding->GetKeyBinding())).c_str(), ImGuiComboFlags_NoArrowButton))
					{
						for (auto& keyCode : Key::s_AllKeyCodes)
						{
							if (ImGui::Selectable(("Key: " + Utility::KeyCodeToString(keyCode)).c_str()))
							{
								binding->SetKeyBinding(keyCode);
							}
						}

						ImGui::EndCombo();
					}

					ImGui::SameLine();
					ImGui::TableSetColumnIndex(2);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 6.0f);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
					if (ImGui::BeginCombo(("##KeyboardCustomCallsOnUpdateFunction" + std::to_string(customCallsIterator)).c_str(), binding->GetFunctionBinding().c_str(), ImGuiComboFlags_NoArrowButton))
					{

						if (ImGui::Selectable("None"))
						{
							binding->SetFunctionBinding("None");
						}

						for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
						{
							if (ImGui::Selectable(name.c_str()))
							{
								binding->SetFunctionBinding(name);
							}
						}

						ImGui::EndCombo();
					}

					if (deleteMenuToggle)
					{
						ImGui::SameLine();
						ImGui::TableSetColumnIndex(3);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
						if (ImGui::ImageButton(("Delete Slot##KeyboardCustomCallsOnUpdateDelete" + std::to_string(customCallsIterator)).c_str(), (ImTextureID)(uint64_t)EditorUI::Editor::s_IconDelete->GetRendererID(),
							ImVec2(17.0f, 17.0f), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
						{
							deleteCustomCalls = true;
							bindingToUpdate = binding;
						}
						ImGui::PopStyleColor();
					}
					customCallsIterator++;
				}

				uint32_t scriptClassIterator{ 0 };
				for (auto& [className, binding] : InputMode::GetKeyboardClassOnUpdate())
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::SetNextItemWidth((ImGui::GetContentRegionAvail().x + 6.0f) / 2);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
					if (ImGui::BeginCombo(("##KeyboardScriptClassOnUpdateType" + std::to_string(scriptClassIterator)).c_str(), "ScriptClass", ImGuiComboFlags_NoArrowButton))
					{

						if (ImGui::Selectable("CustomCalls"))
						{
							switchScriptClassToCustomCalls = true;
							bindingToUpdate = binding;
						}

						ImGui::EndCombo();
					}
					ImGui::SameLine();
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 10.0f);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
					if (ImGui::BeginCombo(("##KeyboardScriptClassOnUpdateClassName" + std::to_string(scriptClassIterator)).c_str(), className.c_str(), ImGuiComboFlags_NoArrowButton))
					{
						if (ImGui::Selectable("None"))
						{
							updateClassName = true;
							bindingToUpdate = binding;
							newClassName = "None";
						}
						for (auto& [name, scriptClass] : Script::ScriptEngine::GetEntityClasses())
						{
							if (ImGui::Selectable(name.c_str()))
							{
								updateClassName = true;
								bindingToUpdate = binding;
								newClassName = name;
							}
						}

						ImGui::EndCombo();
					}

					ImGui::SameLine();
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 6.0f);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
					if (ImGui::BeginCombo(("##KeyboardScriptClassOnUpdateKey" + std::to_string(scriptClassIterator)).c_str(), ("Key: " + Utility::KeyCodeToString(binding->GetKeyBinding())).c_str(), ImGuiComboFlags_NoArrowButton))
					{
						for (auto& keyCode : Key::s_AllKeyCodes)
						{
							if (ImGui::Selectable(("Key: " + Utility::KeyCodeToString(keyCode)).c_str()))
							{
								binding->SetKeyBinding(keyCode);
							}
						}

						ImGui::EndCombo();
					}

					ImGui::SameLine();
					ImGui::TableSetColumnIndex(2);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 6.0f);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
					if (ImGui::BeginCombo(("##KeyboardScriptClassOnUpdateFunction" + std::to_string(scriptClassIterator)).c_str(), binding->GetFunctionBinding().c_str(), ImGuiComboFlags_NoArrowButton))
					{

						if (ImGui::Selectable("None"))
						{
							binding->SetFunctionBinding("None");
						}
						if (Script::ScriptEngine::GetEntityClasses().contains(className))
						{
							for (auto& [name, script] : Script::ScriptEngine::GetEntityClass(className)->GetCustomMethods())
							{
								if (ImGui::Selectable(name.c_str()))
								{
									binding->SetFunctionBinding(name);
								}
							}
						}
						ImGui::EndCombo();
					}

					if (deleteMenuToggle)
					{
						ImGui::SameLine();
						ImGui::TableSetColumnIndex(3);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
						if (ImGui::ImageButton(("Delete Slot##KeyboardScriptClassOnUpdateDelete" + std::to_string(scriptClassIterator)).c_str(), (ImTextureID)(uint64_t)EditorUI::Editor::s_IconDelete->GetRendererID(),
							ImVec2(17.0f, 17.0f), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
						{
							deleteScriptClass = true;
							bindingToUpdate = binding;
						}
						ImGui::PopStyleColor();
					}
					scriptClassIterator++;
				}
				ImGui::EndTable();
			}

			if (updateType) {}
			if (deleteCustomCalls) { InputMode::DeleteKeyboardCustomCallsOnUpdate(bindingToUpdate); }
			if (deleteScriptClass) { InputMode::DeleteKeyboardScriptClassOnUpdate(bindingToUpdate); }
			if (switchCustomToScriptClass)
			{
				bindingToUpdate->SetFunctionBinding("None");
				InputMode::DeleteKeyboardCustomCallsOnUpdate(bindingToUpdate);
				InputMode::AddKeyboardScriptClassOnUpdateSlot();
			}
			if (switchScriptClassToCustomCalls)
			{
				bindingToUpdate->SetFunctionBinding("None");
				InputMode::DeleteKeyboardScriptClassOnUpdate(bindingToUpdate);
				InputMode::AddKeyboardCustomCallsOnUpdateSlot();
			}

			if (updateClassName)
			{
				bindingToUpdate->SetFunctionBinding("None");
				InputMode::UpdateKeyboardClassOnUpdateName(bindingToUpdate, newClassName);
			}
			ImGui::TreePop();
		}
	}

	void InputEditorPanel::InputEditor_Keyboard_OnKeyPressed()
	{
		if (ImGui::TreeNodeEx("OnKeyPressed##Keyboard", ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			// Toggle and Delayed Results Variables
			// Delayed Update Variables. Update Data structure after it has been fully output.

			bool updateType = false;
			Math::ivec2 updateTypeValues{};
			bool deleteCustomCalls = false;
			bool deleteScriptClass = false;
			bool switchCustomToScriptClass = false;
			bool switchScriptClassToCustomCalls = false;
			bool updateClassName = false;
			std::string newClassName {};
			InputMode::InputActionBinding* bindingToUpdate{};

			// Enable Delete Option
			static bool deleteMenuToggle = false;

			// Settings Section
			ImGui::SameLine();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			if (ImGui::ImageButton((ImTextureID)(uint64_t)EditorUI::Editor::s_IconSettings->GetRendererID(), ImVec2(17, 17), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
			{
				ImGui::OpenPopup("KeyboardOnKeyPressedSettings");
			}
			ImGui::PopStyleColor();

			if (ImGui::BeginPopup("KeyboardOnKeyPressedSettings"))
			{
				if (ImGui::Selectable("Add New Slot"))
				{
					InputMode::AddKeyboardCustomCallsOnKeyPressedSlot();
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Toggle Delete Option", deleteMenuToggle))
				{
					deleteMenuToggle = deleteMenuToggle ? false : true; // Conditional Toggles Boolean
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			// Main Table that lists audio slots and their corresponding AudioComponents
			static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
			if (ImGui::BeginTable("KeyboardOnKeyPressedTable", deleteMenuToggle ? 4 : 3, flags))
			{
				ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Keyboard Key", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthStretch);
				if (deleteMenuToggle) { ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, 20.0f); }
				ImGui::TableHeadersRow();
				uint32_t customCallsIterator{ 0 };
				for (auto& binding : InputMode::GetKeyboardCustomCallsOnKeyPressed())
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 6.0f);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
					if (ImGui::BeginCombo(("##KeyboardCustomCallsOnKeyPressedType" + std::to_string(customCallsIterator)).c_str(), "CustomCalls", ImGuiComboFlags_NoArrowButton))
					{
						if (ImGui::Selectable("ScriptClass"))
						{
							switchCustomToScriptClass = true;
							bindingToUpdate = binding;
						}

						ImGui::EndCombo();
					}

					ImGui::SameLine();
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 6.0f);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
					if (ImGui::BeginCombo(("##KeyboardCustomCallsOnKeyPressedKey" + std::to_string(customCallsIterator)).c_str(), ("Key: " + Utility::KeyCodeToString(binding->GetKeyBinding())).c_str(), ImGuiComboFlags_NoArrowButton))
					{
						for (auto& keyCode : Key::s_AllKeyCodes)
						{
							if (ImGui::Selectable(("Key: " + Utility::KeyCodeToString(keyCode)).c_str()))
							{
								binding->SetKeyBinding(keyCode);
							}
						}

						ImGui::EndCombo();
					}

					ImGui::SameLine();
					ImGui::TableSetColumnIndex(2);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 6.0f);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
					if (ImGui::BeginCombo(("##KeyboardCustomCallsOnKeyPressedFunction" + std::to_string(customCallsIterator)).c_str(), binding->GetFunctionBinding().c_str(), ImGuiComboFlags_NoArrowButton))
					{

						if (ImGui::Selectable("None"))
						{
							binding->SetFunctionBinding("None");
						}

						for (auto& [name, script] : Script::ScriptEngine::GetCustomCallMap())
						{
							if (ImGui::Selectable(name.c_str()))
							{
								binding->SetFunctionBinding(name);
							}
						}

						ImGui::EndCombo();
					}

					if (deleteMenuToggle)
					{
						ImGui::SameLine();
						ImGui::TableSetColumnIndex(3);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
						if (ImGui::ImageButton(("Delete Slot##KeyboardCustomCallsOnKeyPressedDelete" + std::to_string(customCallsIterator)).c_str(), (ImTextureID)(uint64_t)EditorUI::Editor::s_IconDelete->GetRendererID(),
							ImVec2(17.0f, 17.0f), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
						{
							deleteCustomCalls = true;
							bindingToUpdate = binding;
						}
						ImGui::PopStyleColor();
					}
					customCallsIterator++;
				}

				uint32_t scriptClassIterator{ 0 };
				for (auto& [className, binding] : InputMode::GetKeyboardClassOnKeyPressed())
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::SetNextItemWidth((ImGui::GetContentRegionAvail().x + 6.0f) / 2);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
					if (ImGui::BeginCombo(("##KeyboardScriptClassOnKeyPressedType" + std::to_string(scriptClassIterator)).c_str(), "ScriptClass", ImGuiComboFlags_NoArrowButton))
					{

						if (ImGui::Selectable("CustomCalls"))
						{
							switchScriptClassToCustomCalls = true;
							bindingToUpdate = binding;
						}

						ImGui::EndCombo();
					}
					ImGui::SameLine();
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 10.0f);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
					if (ImGui::BeginCombo(("##KeyboardScriptClassOnKeyPressedClassName" + std::to_string(scriptClassIterator)).c_str(), className.c_str(), ImGuiComboFlags_NoArrowButton))
					{
						if (ImGui::Selectable("None"))
						{
							updateClassName = true;
							bindingToUpdate = binding;
							newClassName = "None";
						}
						for (auto& [name, scriptClass] : Script::ScriptEngine::GetEntityClasses())
						{
							if (ImGui::Selectable(name.c_str()))
							{
								updateClassName = true;
								bindingToUpdate = binding;
								newClassName = name;
							}
						}

						ImGui::EndCombo();
					}

					ImGui::SameLine();
					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 6.0f);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
					if (ImGui::BeginCombo(("##KeyboardScriptClassOnKeyPressedKey" + std::to_string(scriptClassIterator)).c_str(), ("Key: " + Utility::KeyCodeToString(binding->GetKeyBinding())).c_str(), ImGuiComboFlags_NoArrowButton))
					{
						for (auto& keyCode : Key::s_AllKeyCodes)
						{
							if (ImGui::Selectable(("Key: " + Utility::KeyCodeToString(keyCode)).c_str()))
							{
								binding->SetKeyBinding(keyCode);
							}
						}

						ImGui::EndCombo();
					}

					ImGui::SameLine();
					ImGui::TableSetColumnIndex(2);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 6.0f);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
					if (ImGui::BeginCombo(("##KeyboardScriptClassOnKeyPressedFunction" + std::to_string(scriptClassIterator)).c_str(), binding->GetFunctionBinding().c_str(), ImGuiComboFlags_NoArrowButton))
					{

						if (ImGui::Selectable("None"))
						{
							binding->SetFunctionBinding("None");
						}
						if (Script::ScriptEngine::GetEntityClasses().contains(className))
						{
							for (auto& [name, script] : Script::ScriptEngine::GetEntityClass(className)->GetCustomMethods())
							{
								if (ImGui::Selectable(name.c_str()))
								{
									binding->SetFunctionBinding(name);
								}
							}
						}
						ImGui::EndCombo();
					}

					if (deleteMenuToggle)
					{
						ImGui::SameLine();
						ImGui::TableSetColumnIndex(3);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
						if (ImGui::ImageButton(("Delete Slot##KeyboardScriptClassOnKeyPressedDelete" + std::to_string(scriptClassIterator)).c_str(), (ImTextureID)(uint64_t)EditorUI::Editor::s_IconDelete->GetRendererID(),
							ImVec2(17.0f, 17.0f), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
						{
							deleteScriptClass = true;
							bindingToUpdate = binding;
						}
						ImGui::PopStyleColor();
					}
					scriptClassIterator++;
				}
				ImGui::EndTable();
			}

			if (updateType) {}
			if (deleteCustomCalls) { InputMode::DeleteKeyboardCustomCallsOnKeyPressed(bindingToUpdate); }
			if (deleteScriptClass) { InputMode::DeleteKeyboardScriptClassOnKeyPressed(bindingToUpdate); }
			if (switchCustomToScriptClass)
			{
				bindingToUpdate->SetFunctionBinding("None");
				InputMode::DeleteKeyboardCustomCallsOnKeyPressed(bindingToUpdate);
				InputMode::AddKeyboardScriptClassOnKeyPressedSlot();
			}
			if (switchScriptClassToCustomCalls)
			{
				bindingToUpdate->SetFunctionBinding("None");
				InputMode::DeleteKeyboardScriptClassOnKeyPressed(bindingToUpdate);
				InputMode::AddKeyboardCustomCallsOnKeyPressedSlot();
			}

			if (updateClassName)
			{
				bindingToUpdate->SetFunctionBinding("None");
				InputMode::UpdateKeyboardClassOnKeyPressedName(bindingToUpdate, newClassName);
			}
			ImGui::TreePop();
		}
	}

	void InputEditorPanel::InputEditor_Keyboard_Polling()
	{
		// Delayed Update Variables. Update Data structure after it has been fully output.
		bool updateKeySlot = false;
		Math::ivec2 updateKeySlotValues{};

		bool updateKey = false;
		Math::ivec2 updateKeyValues{};

		// Enable Delete Option
		static bool deleteMenuToggle = false;
		bool deleteKeySlot = false;
		uint16_t slotToDelete{};
		ImGui::SameLine();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		if (ImGui::ImageButton((ImTextureID)(uint64_t)EditorUI::Editor::s_IconSettings->GetRendererID(), ImVec2(17, 17), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
		{
			ImGui::OpenPopup("KeyboardPollingSettings");
		}
		ImGui::PopStyleColor();

		if (ImGui::BeginPopup("KeyboardPollingSettings"))
		{
			if (ImGui::Selectable("Add New Slot"))
			{
				InputMode::AddKeyboardPollingSlot();
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Toggle Delete Option", deleteMenuToggle))
			{
				deleteMenuToggle = deleteMenuToggle ? false : true; // Conditional Toggles Boolean
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		// Main Table that lists audio slots and their corresponding AudioComponents
		static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
		if (ImGui::BeginTable("KeyboardPollingTable", deleteMenuToggle ? 3 : 2, flags))
		{
			ImGui::TableSetupColumn("Slot", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Keyboard Key", ImGuiTableColumnFlags_WidthStretch);
			if (deleteMenuToggle) { ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, 20.0f); }
			ImGui::TableHeadersRow();

			for (auto& [key, value] : InputMode::GetKeyboardPolling())
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 6.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
				if (ImGui::BeginCombo(("##KeyboardPollingSlot" + std::to_string(key)).c_str(), ("Slot: " + std::to_string(key)).c_str(), ImGuiComboFlags_NoArrowButton))
				{
					for (uint16_t iterator{ 0 }; iterator < 50; iterator++)
					{
						if (ImGui::Selectable(("Slot: " + std::to_string(iterator)).c_str()))
						{
							updateKeySlot = true;
							updateKeySlotValues = { key, iterator };
						}
					}
					ImGui::EndCombo();
				}

				ImGui::SameLine();
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 6.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
				if (ImGui::BeginCombo(("##KeyboardPollingKey" + std::to_string(key)).c_str(), ("Key: " + Utility::KeyCodeToString(value)).c_str(), ImGuiComboFlags_NoArrowButton))
				{
					for (auto& keyCode : Key::s_AllKeyCodes)
					{
						if (ImGui::Selectable(("Key: " + Utility::KeyCodeToString(keyCode)).c_str()))
						{
							updateKey = true;
							updateKeyValues = { key, keyCode };
						}
					}

					ImGui::EndCombo();
				}

				if (deleteMenuToggle)
				{
					ImGui::SameLine();
					ImGui::TableSetColumnIndex(2);
					//ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x + 6.0f);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 3.0f);
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
					if (ImGui::ImageButton(("Delete Slot##KeyboardPollingDelete" + std::to_string(key)).c_str(), (ImTextureID)(uint64_t)EditorUI::Editor::s_IconDelete->GetRendererID(),
						ImVec2(17.0f, 17.0f), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)))
					{
						deleteKeySlot = true;
						slotToDelete = key;
					}
					ImGui::PopStyleColor();
				}

			}
			ImGui::EndTable();
		}

		if (updateKeySlot) { InputMode::UpdateKeyboardPollingSlot(updateKeySlotValues.x, updateKeySlotValues.y); }
		if (updateKey) { InputMode::UpdateKeyboardPollingKey(updateKeyValues.x, updateKeyValues.y); }
		if (deleteKeySlot) { InputMode::DeleteKeyboardPollingSlot(slotToDelete); }
	}

	InputEditorPanel::InputEditorPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName,
			KG_BIND_CLASS_FN(InputEditorPanel::OnKeyPressedEditor));
	}

	void InputEditorPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::Editor::StartWindow(m_PanelName, &(s_EditorApp->m_ShowInputEditor));

		if (ImGui::BeginCombo("##Select User Interface", static_cast<bool>(InputMode::s_InputMode) ? Assets::AssetManager::GetInputModeLocation(InputMode::s_InputModeHandle).string().c_str() : "None"))
		{
			if (ImGui::Selectable("None"))
			{
				InputMode::ClearInputEngine();
				
			}
			for (auto& [uuid, asset] : Assets::AssetManager::GetInputModeRegistry())
			{
				if (ImGui::Selectable(asset.Data.IntermediateLocation.string().c_str()))
				{
					InputMode::ClearInputEngine();

					InputMode::LoadInputMode(Assets::AssetManager::GetInputMode(uuid), uuid);
				}
			}
			ImGui::EndCombo();
		}

		ImGui::SameLine();

		if (ImGui::Button("Save Current Input Mode"))
		{
			Assets::AssetManager::SaveInputMode(InputMode::s_InputModeHandle, InputMode::s_InputMode);
		}
		ImGui::SameLine();

		if (ImGui::Button("Create New Input Mode"))
		{
			ImGui::OpenPopup("Create New Input Mode");
		}

		if (ImGui::BeginPopup("Create New Input Mode"))
		{
			static char buffer[256];
			memset(buffer, 0, 256);
			ImGui::InputText("New Input Mode Name", buffer, sizeof(buffer));
			if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
			{
				Assets::AssetHandle newHandle = Assets::AssetManager::CreateNewInputMode(std::string(buffer));
				InputMode::LoadInputMode(Assets::AssetManager::GetInputMode(newHandle), newHandle);

				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (!InputMode::s_InputMode)
		{
			EditorUI::Editor::EndWindow();
			return;
		}

		if (ImGui::BeginTabBar("InputDeviceTab"))
		{
			if (ImGui::BeginTabItem("Keyboard"))
			{
				if (ImGui::CollapsingHeader("Events##Keyboard", ImGuiTreeNodeFlags_DefaultOpen))
				{
					InputEditor_Keyboard_OnUpdate();
					InputEditor_Keyboard_OnKeyPressed();
				}
				if (ImGui::CollapsingHeader("Polling##Keyboard", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap))
				{
					InputEditor_Keyboard_Polling();
				}

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Mouse"))
			{
				ImGui::Text("Unimplemented Yet????");
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		EditorUI::Editor::EndWindow();
	}
	bool InputEditorPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
}
