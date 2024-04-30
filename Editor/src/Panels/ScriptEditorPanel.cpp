#include "Panels/ScriptEditorPanel.h"

#include "EditorLayer.h"

#include "Kargono.h"

namespace Kargono
{
	static EditorLayer* s_EditorLayer { nullptr };

	static UI::TextInputSpec s_InputScriptNameSpec{};
	static UI::SelectOptionSpec s_InputScriptFuncSpec{};
	static std::string s_InputScriptName{ "Empty"};
	static WrappedFuncType s_InputScriptFunc{ WrappedFuncType::Void_None};

	ScriptEditorPanel::ScriptEditorPanel()
	{
		s_EditorLayer = EditorLayer::GetCurrentLayer();

		s_InputScriptNameSpec.Label = "Script Name";
		s_InputScriptNameSpec.WidgetID = 0xb321694eb9a94fc0;
		s_InputScriptNameSpec.CurrentOption = s_InputScriptName;
		s_InputScriptNameSpec.ConfirmAction = [&](const std::string& scriptName)
		{
				s_InputScriptName = scriptName;
		};

		s_InputScriptFuncSpec.Label = "Function Name";
		s_InputScriptFuncSpec.WidgetID = 0x2b84d1b4e4304b26;
		s_InputScriptFuncSpec.CurrentOption = "Void_None";
		for (auto func : s_AllWrappedFuncs)
		{
			s_InputScriptFuncSpec.AddToOptionsList("All Options", Utility::WrappedFuncTypeToString(func));
		}
		s_InputScriptFuncSpec.ConfirmAction = [&](const std::string& scriptName)
		{
			s_InputScriptFunc = Utility::StringToWrappedFuncType(scriptName);
		};
	}
	void ScriptEditorPanel::OnEditorUIRender()
	{
		UI::Editor::StartWindow("Scripts", &s_EditorLayer->m_ShowScriptEditor);

		bool deleteScript = false;
		Assets::AssetHandle deleteHandle{};

		uint32_t iterator{ 0 };
		for (auto& [handle, script] : Assets::AssetManager::GetScriptMap())
		{
			ImGui::Text(script->m_ScriptName.c_str());
			ImGui::SameLine();
			ImGui::Text(std::string(handle).c_str());
			ImGui::SameLine();
			ImGui::Text(Utility::WrappedFuncTypeToString(script->m_Function->Type()).c_str());

			//ImGui::SameLine();
			/*if (ImGui::Button(("Delete Script##" + std::to_string(iterator)).c_str()))
			{
				deleteScript = true;
				deleteHandle = handle;
			}*/

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
			s_InputScriptFuncSpec.CurrentOption = Utility::WrappedFuncTypeToString(s_InputScriptFunc);
			UI::Editor::SelectOption(s_InputScriptFuncSpec);
			if (ImGui::Button("Create Script"))
			{
				std::vector<WrappedVarType> parameters{ Utility::WrappedFuncTypeToParameterTypes(s_InputScriptFunc) };
				WrappedVarType returnValue{ Utility::WrappedFuncTypeToReturnType(s_InputScriptFunc)};
				WrappedFuncType functionType{ s_InputScriptFunc };
				auto [handle, successful] = Assets::AssetManager::CreateNewScript(s_InputScriptName, parameters, returnValue, functionType);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		UI::Editor::EndWindow();
	}
}
