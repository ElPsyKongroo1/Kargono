#include "Panels/ScriptEditorPanel.h"

#include "EditorLayer.h"

#include "Kargono.h"

namespace Kargono
{

	static UI::TextInputSpec s_InputScriptNameSpec{};
	static std::string s_InputScriptName{ "Empty"};

	ScriptEditorPanel::ScriptEditorPanel()
	{
		s_InputScriptNameSpec.Label = "Script Name";
		s_InputScriptNameSpec.WidgetID = 0xb321694eb9a94fc0;
		s_InputScriptNameSpec.CurrentOption = s_InputScriptName;
		s_InputScriptNameSpec.ConfirmAction = [&](const std::string& scriptName)
		{
				s_InputScriptName = scriptName;
		};
	}
	void ScriptEditorPanel::OnEditorUIRender()
	{
		ImGui::Begin("Scripts");

		bool deleteScript = false;
		Assets::AssetHandle deleteHandle{};

		uint32_t iterator{ 0 };
		for (auto& [handle, script] : Assets::AssetManager::GetScriptMap())
		{
			ImGui::Text(script->m_ScriptName.c_str());
			ImGui::SameLine();
			ImGui::Text(std::string(handle).c_str());
			ImGui::SameLine();
			if (ImGui::Button(("Delete Script##" + std::to_string(iterator)).c_str()))
			{
				deleteScript = true;
				deleteHandle = handle;
			}

			if (ImGui::Button(("RunScript##" + std::to_string(iterator)).c_str()))
			{
				if (script->m_Function->Type() == WrappedFuncType::Void_None)
				{
					((WrappedVoidNone*)script->m_Function.get())->m_Value();
				}
			}

			iterator++;
		}

		if (deleteScript)
		{
			Assets::AssetManager::DeleteScript(deleteHandle);
		}

		if (ImGui::Button("Create New Script"))
		{
			ImGui::OpenPopup("CreateScriptPopup");
		}

		if (ImGui::Button("CreateDLL")) // TODO: TEMPORARY
		{
			Scripting::ScriptModuleBuilder::CreateDll();
		}

		if (ImGui::Button("OpenDll")) // TODO: AHHHHHHHHHHHHHHHHHHHHHHHHHHH
		{
			Scripting::ScriptCore::OpenDll();
			//Scripting::ScriptCore::OpenDll("ExportBody.dll");
		}

		if (ImGui::BeginPopup("CreateScriptPopup"))
		{
			ImGui::Text("New Script Creator===================");
			s_InputScriptNameSpec.CurrentOption = s_InputScriptName;
			UI::Editor::TextInputModal(s_InputScriptNameSpec);
			if (ImGui::Button("Create Script"))
			{
				std::vector<WrappedVarType> parameters{};
				WrappedVarType returnValue{ WrappedVarType::Void };
				WrappedFuncType functionType{ WrappedFuncType::Void_None };
				auto [handle, successful] = Assets::AssetManager::CreateNewScript(s_InputScriptName, parameters, returnValue, functionType);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::End();
	}
}
