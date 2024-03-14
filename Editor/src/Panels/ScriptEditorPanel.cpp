#include "Panels/ScriptEditorPanel.h"

#include "EditorLayer.h"

#include "Kargono.h"

namespace Kargono
{
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
			ImGui::Text("WE ARE CREATING A NEW SCRIPT");
			if (ImGui::Button("Add New Item HAHAHA"))
			{
				std::string name{"Potato"};
				static uint32_t testIterator {0};
				name.append(std::to_string(testIterator));
				testIterator++;
				std::vector<WrappedVarType> parameters{};
				WrappedVarType returnValue{ WrappedVarType::Void };
				WrappedFuncType functionType{ WrappedFuncType::Void_None };

				Assets::AssetManager::CreateNewScript(name, parameters, returnValue, functionType);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::End();
	}
}
