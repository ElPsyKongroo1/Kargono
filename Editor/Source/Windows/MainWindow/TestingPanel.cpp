#include "Windows/MainWindow/TestingPanel.h"
#include "Windows/MainWindow/ViewportPanel.h"

#include "EditorApp.h"
#include "Kargono/Utility/DebugGlobals.h"
#include "Kargono/Utility/Timers.h"
#include "Kargono/Scripting/ScriptCompilerService.h"
#include "Kargono/Utility/Random.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Memory/StackAlloc.h"
#include "Kargono/Memory/SystemAlloc.h"
#include "Kargono/Core/DataStructures.h"

#include <sstream>
#include <cstdio>


namespace Kargono::Panels
{
	static SparseArray<uint64_t> s_SparseArray{64};

	static EditorApp* s_EditorApp{ nullptr };
	static Windows::MainWindow* s_MainWindow{ nullptr };

	static EditorUI::EditTextSpec s_CompilePath {};

	struct DataStruct
	{
		float ahaha{ 1.0f };
		FixedString32 m_Text{ "aba" };
	};
	static DataStruct* s_DataStructs[5];
	static Memory::StackAlloc s_DataAllocator{};
	static EditorUI::ButtonSpec s_TestButton{};
	static EditorUI::PlotSpec s_TestPlot{};
	static EditorUI::DropDownSpec s_TestDropdown{};

// TODO: Testing Splines
#if 0
	static std::vector<EditorUI::EditVec3Spec> s_ControlPointWidgets;
#endif

	
	TestingPanel::TestingPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(TestingPanel::OnKeyPressedEditor));

		InitializeDebugGlobalsWidgets();
		InitializeGeneralTestingWidgets();
	}


	void TestingPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_MainWindow->m_ShowTesting);
		// Exit window early if window is not visible
		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		EditorUI::EditorUIService::TitleText("Welcome to the Testing Panel! Stay a while...");

		EditorUI::EditorUIService::BeginTabBar("##TestingPanelTabBar");
		if (EditorUI::EditorUIService::BeginTabItem("General Testing"))
		{
			DrawGeneralTestingWidgets();
			EditorUI::EditorUIService::EndTabItem();
		}
#if defined(KG_DEBUG)
		if (EditorUI::EditorUIService::BeginTabItem("Debug Globals"))
		{
			DrawDebugGlobalWidgets();
			EditorUI::EditorUIService::EndTabItem();
		}
#endif
		EditorUI::EditorUIService::EndTabBar();


		EditorUI::EditorUIService::EndWindow();
	}
	void TestingPanel::InitializeDebugGlobalsWidgets()
	{
#if defined(KG_DEBUG)

		// Checkbox initialization
		s_EditTestBool_1.m_Label = "Test Bool 1";
		s_EditTestBool_1.m_CurrentBoolean = Utility::DebugGlobals::s_TestBool_1;
		s_EditTestBool_1.m_ConfirmAction = [&](EditorUI::CheckboxSpec& spec) 
		{
				Utility::DebugGlobals::s_TestBool_1 = spec.m_CurrentBoolean;
		};

		s_EditTestBool_2.m_Label = "Test Bool 2";
		s_EditTestBool_2.m_CurrentBoolean = Utility::DebugGlobals::s_TestBool_2;
		s_EditTestBool_2.m_ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
			{
				Utility::DebugGlobals::s_TestBool_2 = spec.m_CurrentBoolean;
			};

		s_EditTestBool_3.m_Label = "Test Bool 3";
		s_EditTestBool_3.m_CurrentBoolean = Utility::DebugGlobals::s_TestBool_3;
		s_EditTestBool_3.m_ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
			{
				Utility::DebugGlobals::s_TestBool_3 = spec.m_CurrentBoolean;
			};


		// Float initialization
		s_EditTestFloat_1.m_Label = "Test Float 1";
		s_EditTestFloat_1.m_CurrentFloat = Utility::DebugGlobals::s_TestFloat_1;
		s_EditTestFloat_1.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
			{
				Utility::DebugGlobals::s_TestFloat_1 = spec.m_CurrentFloat;
			};

		s_EditTestFloat_2.m_Label = "Test Float 2";
		s_EditTestFloat_2.m_CurrentFloat = Utility::DebugGlobals::s_TestFloat_2;
		s_EditTestFloat_2.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
			{
				Utility::DebugGlobals::s_TestFloat_2 = spec.m_CurrentFloat;
			};

		s_EditTestFloat_3.m_Label = "Test Float 3";
		s_EditTestFloat_3.m_CurrentFloat = Utility::DebugGlobals::s_TestFloat_3;
		s_EditTestFloat_3.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
			{
				Utility::DebugGlobals::s_TestFloat_3 = spec.m_CurrentFloat;
			};

		// Integer initialization
		s_EditTestInt_1.m_Label = "Test Int 1";
		s_EditTestInt_1.m_CurrentInteger = Utility::DebugGlobals::s_TestInt_1;
		s_EditTestInt_1.m_ConfirmAction = [&](EditorUI::EditIntegerSpec& spec)
			{
				Utility::DebugGlobals::s_TestInt_1 = spec.m_CurrentInteger;
			};

		s_EditTestInt_2.m_Label = "Test Int 2";
		s_EditTestInt_2.m_CurrentInteger = Utility::DebugGlobals::s_TestInt_2;
		s_EditTestInt_2.m_ConfirmAction = [&](EditorUI::EditIntegerSpec& spec)
			{
				Utility::DebugGlobals::s_TestInt_2 = spec.m_CurrentInteger;
			};

		s_EditTestInt_3.m_Label = "Test Int 3";
		s_EditTestInt_3.m_CurrentInteger = Utility::DebugGlobals::s_TestInt_3;
		s_EditTestInt_3.m_ConfirmAction = [&](EditorUI::EditIntegerSpec& spec)
			{
				Utility::DebugGlobals::s_TestInt_3 = spec.m_CurrentInteger;
			};

		// Unsigned Integer initialization
		s_EditTestUInt_1.m_Label = "Test UInt 1";
		s_EditTestUInt_1.m_CurrentInteger = Utility::DebugGlobals::s_TestUInt_1;
		s_EditTestUInt_1.m_Bounds = { 0, 10'000 };
		s_EditTestUInt_1.m_ConfirmAction = [&](EditorUI::EditIntegerSpec& spec)
			{
				Utility::DebugGlobals::s_TestUInt_1 = spec.m_CurrentInteger;
			};

		s_EditTestUInt_2.m_Label = "Test UInt 2";
		s_EditTestUInt_2.m_CurrentInteger = Utility::DebugGlobals::s_TestUInt_2;
		s_EditTestUInt_2.m_Bounds = { 0, 10'000 };
		s_EditTestUInt_2.m_ConfirmAction = [&](EditorUI::EditIntegerSpec& spec)
			{
				Utility::DebugGlobals::s_TestUInt_2 = spec.m_CurrentInteger;
			};

		s_EditTestUInt_3.m_Label = "Test UInt 3";
		s_EditTestUInt_3.m_CurrentInteger = Utility::DebugGlobals::s_TestUInt_3;
		s_EditTestUInt_3.m_Bounds = { 0, 10'000 };
		s_EditTestUInt_3.m_ConfirmAction = [&](EditorUI::EditIntegerSpec& spec)
			{
				Utility::DebugGlobals::s_TestUInt_3 = spec.m_CurrentInteger;
			};

		// Vec2 initialization
		s_EditTestVec2_1.m_Label = "Test Vec2 1";
		s_EditTestVec2_1.m_CurrentVec2 = Utility::DebugGlobals::s_TestVec2_1;
		s_EditTestVec2_1.m_ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec2_1 = spec.m_CurrentVec2;
			};

		s_EditTestVec2_2.m_Label = "Test Vec2 2";
		s_EditTestVec2_2.m_CurrentVec2 = Utility::DebugGlobals::s_TestVec2_2;
		s_EditTestVec2_2.m_ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec2_2 = spec.m_CurrentVec2;
			};

		s_EditTestVec2_3.m_Label = "Test Vec2 3";
		s_EditTestVec2_3.m_CurrentVec2 = Utility::DebugGlobals::s_TestVec2_3;
		s_EditTestVec2_3.m_ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec2_3 = spec.m_CurrentVec2;
			};

		// Vec3 initialization
		s_EditTestVec3_1.m_Label = "Test Vec3 1";
		s_EditTestVec3_1.m_CurrentVec3 = Utility::DebugGlobals::s_TestVec3_1;
		s_EditTestVec3_1.m_ConfirmAction = [&](EditorUI::EditVec3Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec3_1 = spec.m_CurrentVec3;
			};

		s_EditTestVec3_2.m_Label = "Test Vec3 2";
		s_EditTestVec3_2.m_CurrentVec3 = Utility::DebugGlobals::s_TestVec3_2;
		s_EditTestVec3_2.m_ConfirmAction = [&](EditorUI::EditVec3Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec3_2 = spec.m_CurrentVec3;
			};

		s_EditTestVec3_3.m_Label = "Test Vec3 3";
		s_EditTestVec3_3.m_CurrentVec3 = Utility::DebugGlobals::s_TestVec3_3;
		s_EditTestVec3_3.m_ConfirmAction = [&](EditorUI::EditVec3Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec3_3 = spec.m_CurrentVec3;
			};

		// Vec4 initialization
		s_EditTestVec4_1.m_Label = "Test Vec4 1";
		s_EditTestVec4_1.m_CurrentVec4 = Utility::DebugGlobals::s_TestVec4_1;
		s_EditTestVec4_1.m_ConfirmAction = [&](EditorUI::EditVec4Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec4_1 = spec.m_CurrentVec4;
			};

		s_EditTestVec4_2.m_Label = "Test Vec4 2";
		s_EditTestVec4_2.m_CurrentVec4 = Utility::DebugGlobals::s_TestVec4_2;
		s_EditTestVec4_2.m_ConfirmAction = [&](EditorUI::EditVec4Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec4_2 = spec.m_CurrentVec4;
			};

		s_EditTestVec4_3.m_Label = "Test Vec4 3";
		s_EditTestVec4_3.m_CurrentVec4 = Utility::DebugGlobals::s_TestVec4_3;
		s_EditTestVec4_3.m_ConfirmAction = [&](EditorUI::EditVec4Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec4_3 = spec.m_CurrentVec4;
			};
#endif

	}
	void TestingPanel::InitializeGeneralTestingWidgets()
	{
		s_CompilePath.m_Label = "File to Compile";
		s_CompilePath.m_CurrentOption = "test.kgscript";

		size_t bufferSize{ sizeof(DataStruct) * 10 };
		s_DataAllocator.Init(Memory::System::GenAlloc(bufferSize, alignof(DataStruct)), bufferSize);
		s_DataStructs[0] = s_DataAllocator.Alloc<DataStruct>();
		s_DataStructs[1] = s_DataAllocator.Alloc<DataStruct>();
		s_DataStructs[2] = s_DataAllocator.Alloc<DataStruct>();
		s_DataStructs[3] = s_DataAllocator.Alloc<DataStruct>();

		s_DataStructs[0]->m_Text = "This is the first data struct";
		s_DataStructs[1]->m_Text = "This is the second data struct";
		s_DataStructs[2]->m_Text = "This is the third data struct";
		s_DataStructs[3]->m_Text = "This is the fourth data struct";

		s_TestButton.m_Label = "Issa Button";
		s_TestButton.m_Button.m_OnPress = [](EditorUI::Button& spec) 
		{
			KG_WARN("We pressed a button!");
		};

		s_TestPlot.m_Label = "Test Plot";
		s_TestPlot.SetBufferSize(50);

		s_TestDropdown.m_Label = "Test Dropdown";
		EditorUI::OptionEntry* entry = s_TestDropdown.CreateOption();
		KG_ASSERT(entry);
		entry->m_Handle = 5;
		entry->m_Label = "Entry One";
		s_TestDropdown.SetCurrentOption(entry->m_Handle);

		EditorUI::OptionEntry* entry2 = s_TestDropdown.CreateOption();
		KG_ASSERT(entry2);
		entry2->m_Handle = 2;
		entry2->m_Label = "Entry Two";

		// TODO Testing Splines
#if 0
		// TODO: Please Remove
		Math::Spline testSpline;
		testSpline.m_Points.push_back({ 10.0f, 41.0f, 0.0f });
		testSpline.m_Points.push_back({ 40.0f, 41.0f, 0.0f });
		testSpline.m_Points.push_back({ 70.0f, 41.0f, 0.0f });
		testSpline.m_Points.push_back({ 100.0f, 41.0f, 0.0f });
		testSpline.m_Points.push_back({ 130.0f, 41.0f, 0.0f });
		testSpline.m_Points.push_back({ 160.0f, 41.0f, 0.0f });
		testSpline.m_Looped = true;
		s_EditorApp->m_MainWindow->m_ViewportPanel->m_DebugSplines.push_back(testSpline);

		std::vector<Math::vec3>& allPoints = s_EditorApp->m_MainWindow->m_ViewportPanel->m_DebugSplines.at(0).m_Points;
		size_t iteration{ 0 };
		s_ControlPointWidgets.resize(allPoints.size());
		for (Math::vec3& point : allPoints)
		{
			EditorUI::EditVec3Spec& currentSpec = s_ControlPointWidgets.at(iteration);
			currentSpec.m_Label = "Control" + std::to_string(iteration);
			currentSpec.m_ScrollSpeed = 0.5f;
			currentSpec.m_ProvidedData = CreateRef<size_t>(iteration);
			currentSpec.m_ConfirmAction = [&](EditorUI::EditVec3Spec& spec)
				{
					// Get provided data
					size_t iteration = *(size_t*)spec.m_ProvidedData.get();

					Math::Spline& spline = s_EditorApp->m_MainWindow->m_ViewportPanel->m_DebugSplines.at(0);
					spline.m_Points.at(iteration) = spec.m_CurrentVec3;
				};
			iteration++;
		}
#endif
	}
	void TestingPanel::DrawDebugGlobalWidgets()
	{
#if defined(KG_DEBUG)
		// Bool initialization
		s_EditTestBool_1.m_CurrentBoolean = Utility::DebugGlobals::s_TestBool_1;
		EditorUI::EditorUIService::Checkbox(s_EditTestBool_1);

		s_EditTestBool_2.m_CurrentBoolean = Utility::DebugGlobals::s_TestBool_2;
		EditorUI::EditorUIService::Checkbox(s_EditTestBool_2);

		s_EditTestBool_3.m_CurrentBoolean = Utility::DebugGlobals::s_TestBool_3;
		EditorUI::EditorUIService::Checkbox(s_EditTestBool_3);

		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Float initialization
		s_EditTestFloat_1.m_CurrentFloat = Utility::DebugGlobals::s_TestFloat_1;
		EditorUI::EditorUIService::EditFloat(s_EditTestFloat_1);

		s_EditTestFloat_2.m_CurrentFloat = Utility::DebugGlobals::s_TestFloat_2;
		EditorUI::EditorUIService::EditFloat(s_EditTestFloat_2);

		s_EditTestFloat_3.m_CurrentFloat = Utility::DebugGlobals::s_TestFloat_3;
		EditorUI::EditorUIService::EditFloat(s_EditTestFloat_3);

		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Integer initialization
		s_EditTestInt_1.m_CurrentInteger = Utility::DebugGlobals::s_TestInt_1;
		EditorUI::EditorUIService::EditInteger(s_EditTestInt_1);

		s_EditTestInt_2.m_CurrentInteger = Utility::DebugGlobals::s_TestInt_2;
		EditorUI::EditorUIService::EditInteger(s_EditTestInt_2);

		s_EditTestInt_3.m_CurrentInteger = Utility::DebugGlobals::s_TestInt_3;
		EditorUI::EditorUIService::EditInteger(s_EditTestInt_3);

		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Unsigned Integer initialization
		s_EditTestUInt_1.m_CurrentInteger = Utility::DebugGlobals::s_TestUInt_1;
		EditorUI::EditorUIService::EditInteger(s_EditTestUInt_1);

		s_EditTestUInt_2.m_CurrentInteger = Utility::DebugGlobals::s_TestUInt_2;
		EditorUI::EditorUIService::EditInteger(s_EditTestUInt_2);

		s_EditTestUInt_3.m_CurrentInteger = Utility::DebugGlobals::s_TestUInt_3;
		EditorUI::EditorUIService::EditInteger(s_EditTestUInt_3);

		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Vec2 initialization
		s_EditTestVec2_1.m_CurrentVec2 = Utility::DebugGlobals::s_TestVec2_1;
		EditorUI::EditorUIService::EditVec2(s_EditTestVec2_1);

		s_EditTestVec2_2.m_CurrentVec2 = Utility::DebugGlobals::s_TestVec2_2;
		EditorUI::EditorUIService::EditVec2(s_EditTestVec2_2);

		s_EditTestVec2_3.m_CurrentVec2 = Utility::DebugGlobals::s_TestVec2_3;
		EditorUI::EditorUIService::EditVec2(s_EditTestVec2_3);

		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Vec3 initialization
		s_EditTestVec3_1.m_CurrentVec3 = Utility::DebugGlobals::s_TestVec3_1;
		EditorUI::EditorUIService::EditVec3(s_EditTestVec3_1);

		s_EditTestVec3_2.m_CurrentVec3 = Utility::DebugGlobals::s_TestVec3_2;
		EditorUI::EditorUIService::EditVec3(s_EditTestVec3_2);

		s_EditTestVec3_3.m_CurrentVec3 = Utility::DebugGlobals::s_TestVec3_3;
		EditorUI::EditorUIService::EditVec3(s_EditTestVec3_3);

		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Vec4 initialization
		s_EditTestVec4_1.m_CurrentVec4 = Utility::DebugGlobals::s_TestVec4_1;
		EditorUI::EditorUIService::EditVec4(s_EditTestVec4_1);

		s_EditTestVec4_2.m_CurrentVec4 = Utility::DebugGlobals::s_TestVec4_2;
		EditorUI::EditorUIService::EditVec4(s_EditTestVec4_2);

		s_EditTestVec4_3.m_CurrentVec4 = Utility::DebugGlobals::s_TestVec4_3;
		EditorUI::EditorUIService::EditVec4(s_EditTestVec4_3);

		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);
#endif
	}
	void TestingPanel::DrawGeneralTestingWidgets()
	{
		
		EditorUI::EditorUIService::EditText(s_CompilePath);

		if (ImGui::Button("Compile File"))
		{
			KG_TRACE_CRITICAL(Scripting::ScriptCompilerService::CompileScriptFile("./../Projects/Pong/Assets/" + s_CompilePath.m_CurrentOption));
		}

		static size_t s_Count{ 4 };

		for (size_t i{ 0 }; i < s_Count; i++)
		{
			EditorUI::EditorUIService::Text(s_DataStructs[i]->m_Text.CString());
		}

		if (EditorUI::EditorUIService::Button(s_TestButton))
		{
			s_TestPlot.AddValue(Utility::RandomService::GenerateRandomFloat(0.0f, 30.0f));
		}

		EditorUI::EditorUIService::DropDown(s_TestDropdown);

		EditorUI::EditorUIService::Plot(s_TestPlot);


		if (ImGui::Button("Add random to sparse list"))
		{
			auto result = s_SparseArray.EmplaceLowest();
			if (!result)
			{
				KG_TRACE_INFO("Aye, we failed to add");
			}
			else
			{
				result->m_Value = (uint64_t)Utility::RandomService::GenerateRandomInteger(0, 500);
				KG_TRACE_INFO("Aye, we added one at {}, its value is {}", result->m_ArrayIndex, result->m_Value);
			}
		}

		if (ImGui::Button("List out all indices"))
		{
			KG_TRACE_INFO("==========");
			for (size_t index : s_SparseArray.GetActiveIndices())
			{
				KG_TRACE_INFO("We got an index at {} with a value of {}", index, s_SparseArray[index]);
			}
			KG_TRACE_INFO("==========");
		}

		if (ImGui::Button("List out values"))
		{
			KG_TRACE_INFO("==========");
			for (uint64_t value : s_SparseArray)
			{
				KG_TRACE_INFO("We got a value of {}", value);
			}
			KG_TRACE_INFO("==========");
		}

		if (ImGui::Button("Remove Random"))
		{
			std::vector<size_t> allIndices{ s_SparseArray.GetActiveIndices()};

			if (allIndices.size() > 0)
			{
				size_t randomChoice = (size_t)Utility::RandomService::GenerateRandomInteger(0, (int32_t)allIndices.size() - 1);

				KG_TRACE_INFO("Removing value {} and index {}", s_SparseArray[allIndices[randomChoice]], allIndices[randomChoice]);
				s_SparseArray.Remove(allIndices[randomChoice]);
			}
			else 
			{
				KG_TRACE_INFO("Failed to remove. No elements in sparse array");
			}
			
		}

		// TODO: Testing Splines
#if 0 
		Math::Spline& spline = s_EditorApp->m_MainWindow->m_ViewportPanel->m_DebugSplines.at(0);
		size_t iteration{ 0 };
		for (Math::vec3& point : spline.m_Points)
		{
			EditorUI::EditVec3Spec& spec = s_ControlPointWidgets.at(iteration);
			spec.m_CurrentVec3 = spline.m_Points.at(iteration);
			EditorUI::EditorUIService::EditVec3(spec);
			iteration++;
		}
#endif
		

		
	}
	bool TestingPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	bool TestingPanel::OnInputEvent(Events::Event* /*event*/)
	{
		return false;
	}
}
