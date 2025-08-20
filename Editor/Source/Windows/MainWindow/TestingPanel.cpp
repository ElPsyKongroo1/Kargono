#include "Windows/MainWindow/TestingPanel.h"
#include "Windows/MainWindow/ViewportPanel.h"

#include "EditorApp.h"
#include "Kargono/Utility/DebugGlobals.h"
#include "Kargono/Utility/Timers.h"
#include "Modules/Scripting/ScriptCompilerService.h"
#include "Kargono/Utility/Random.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Kargono/Memory/StackAlloc.h"
#include "Kargono/Memory/SystemAlloc.h"
#include "Kargono/Memory/HeapAlloc.h"
#include "Kargono/Core/DataStructures.h"
#include "Kargono/Utility/CompilerInfo.h"

#include "Modules/ECSTest/DataStructures/SparseSetTest.h"
#include "Modules/ECSTest/RegistryTest.h"
#include "Modules/Core/CoreModule.h"

#include <sstream>
#include <cstdio>

namespace Kargono::Panels
{
	Register_Module(Editor)

	static SparseArray<uint64_t> s_SparseArray{64};
	//static ECS::SparseSet s_SparseSet{ 10, 10 };

	static Memory::HeapAllocator s_TestHeapAlloc{};
	static ECS::Registry s_DataRegistry{};
	static EditorUI::EditIntegerSpec s_EntityIDSpec;

	static EditorApp* s_EditorApp{ nullptr };
	static Windows::MainWindow* s_MainWindow{ nullptr };

	static EditorUI::EditTextSpec s_CompilePath {};

	struct TransformTest
	{
		Math::vec2 location{};
		Math::vec2 size{};
	};

	Register_Module_Type(TransformTest)

	struct HealthTest
	{
		float headHealth{};
		float torsoHealth{};
		Math::vec2 armsHealth{};
		Math::vec2 legsHealth{};
	};

	Register_Module_Type(HealthTest)

	struct BloodTest
	{
		float indexFinger{};
		float ringFinger{};
		float pinkyFinger{};
		float thumbFinger{};
		float middleFinger{};
	};

	Register_Module_Type(BloodTest)

	struct DataStruct
	{
		float ahaha{ 1.0f };
		FixedBufStr32 m_Text{ "aba" };
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

		Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals()};
		// Checkbox initialization
		s_EditTestBool_1.m_Label = "Test Bool 1";
		s_EditTestBool_1.m_CurrentBoolean = globals.m_TestBool_1;
		s_EditTestBool_1.m_ConfirmAction = [](EditorUI::CheckboxSpec& spec) 
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestBool_1 = spec.m_CurrentBoolean;
		};

		s_EditTestBool_2.m_Label = "Test Bool 2";
		s_EditTestBool_2.m_CurrentBoolean = globals.m_TestBool_2;
		s_EditTestBool_2.m_ConfirmAction = [](EditorUI::CheckboxSpec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestBool_2 = spec.m_CurrentBoolean;
		};

		s_EditTestBool_3.m_Label = "Test Bool 3";
		s_EditTestBool_3.m_CurrentBoolean = globals.m_TestBool_3;
		s_EditTestBool_3.m_ConfirmAction = [](EditorUI::CheckboxSpec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestBool_3 = spec.m_CurrentBoolean;
		};


		// Float initialization
		s_EditTestFloat_1.m_Label = "Test Float 1";
		s_EditTestFloat_1.m_CurrentFloat = globals.m_TestFloat_1;
		s_EditTestFloat_1.m_ConfirmAction = [](EditorUI::EditFloatSpec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestFloat_1 = spec.m_CurrentFloat;
		};

		s_EditTestFloat_2.m_Label = "Test Float 2";
		s_EditTestFloat_2.m_CurrentFloat = globals.m_TestFloat_2;
		s_EditTestFloat_2.m_ConfirmAction = [](EditorUI::EditFloatSpec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestFloat_2 = spec.m_CurrentFloat;
		};

		s_EditTestFloat_3.m_Label = "Test Float 3";
		s_EditTestFloat_3.m_CurrentFloat = globals.m_TestFloat_3;
		s_EditTestFloat_3.m_ConfirmAction = [](EditorUI::EditFloatSpec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestFloat_3 = spec.m_CurrentFloat;
		};

		// Integer initialization
		s_EditTestInt_1.m_Label = "Test Int 1";
		s_EditTestInt_1.m_CurrentInteger = globals.m_TestInt_1;
		s_EditTestInt_1.m_ConfirmAction = [](EditorUI::EditIntegerSpec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestInt_1 = spec.m_CurrentInteger;
		};

		s_EditTestInt_2.m_Label = "Test Int 2";
		s_EditTestInt_2.m_CurrentInteger = globals.m_TestInt_2;
		s_EditTestInt_2.m_ConfirmAction = [](EditorUI::EditIntegerSpec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestInt_2 = spec.m_CurrentInteger;
		};

		s_EditTestInt_3.m_Label = "Test Int 3";
		s_EditTestInt_3.m_CurrentInteger = globals.m_TestInt_3;
		s_EditTestInt_3.m_ConfirmAction = [](EditorUI::EditIntegerSpec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestInt_3 = spec.m_CurrentInteger;
		};

		// Unsigned Integer initialization
		s_EditTestUInt_1.m_Label = "Test UInt 1";
		s_EditTestUInt_1.m_CurrentInteger = globals.m_TestUInt_1;
		s_EditTestUInt_1.m_Bounds = { 0, 10'000 };
		s_EditTestUInt_1.m_ConfirmAction = [](EditorUI::EditIntegerSpec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestUInt_1 = spec.m_CurrentInteger;
		};

		s_EditTestUInt_2.m_Label = "Test UInt 2";
		s_EditTestUInt_2.m_CurrentInteger = globals.m_TestUInt_2;
		s_EditTestUInt_2.m_Bounds = { 0, 10'000 };
		s_EditTestUInt_2.m_ConfirmAction = [](EditorUI::EditIntegerSpec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestUInt_2 = spec.m_CurrentInteger;
		};

		s_EditTestUInt_3.m_Label = "Test UInt 3";
		s_EditTestUInt_3.m_CurrentInteger = globals.m_TestUInt_3;
		s_EditTestUInt_3.m_Bounds = { 0, 10'000 };
		s_EditTestUInt_3.m_ConfirmAction = [](EditorUI::EditIntegerSpec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestUInt_3 = spec.m_CurrentInteger;
		};

		// Vec2 initialization
		s_EditTestVec2_1.m_Label = "Test Vec2 1";
		s_EditTestVec2_1.m_CurrentVec2 = globals.m_TestVec2_1;
		s_EditTestVec2_1.m_ConfirmAction = [](EditorUI::EditVec2Spec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestVec2_1 = spec.m_CurrentVec2;
		};

		s_EditTestVec2_2.m_Label = "Test Vec2 2";
		s_EditTestVec2_2.m_CurrentVec2 = globals.m_TestVec2_2;
		s_EditTestVec2_2.m_ConfirmAction = [](EditorUI::EditVec2Spec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestVec2_2 = spec.m_CurrentVec2;
		};

		s_EditTestVec2_3.m_Label = "Test Vec2 3";
		s_EditTestVec2_3.m_CurrentVec2 = globals.m_TestVec2_3;
		s_EditTestVec2_3.m_ConfirmAction = [](EditorUI::EditVec2Spec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestVec2_3 = spec.m_CurrentVec2;
		};

		// Vec3 initialization
		s_EditTestVec3_1.m_Label = "Test Vec3 1";
		s_EditTestVec3_1.m_CurrentVec3 = globals.m_TestVec3_1;
		s_EditTestVec3_1.m_ConfirmAction = [](EditorUI::EditVec3Spec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestVec3_1 = spec.m_CurrentVec3;
		};

		s_EditTestVec3_2.m_Label = "Test Vec3 2";
		s_EditTestVec3_2.m_CurrentVec3 = globals.m_TestVec3_2;
		s_EditTestVec3_2.m_ConfirmAction = [](EditorUI::EditVec3Spec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestVec3_2 = spec.m_CurrentVec3;
		};

		s_EditTestVec3_3.m_Label = "Test Vec3 3";
		s_EditTestVec3_3.m_CurrentVec3 = globals.m_TestVec3_3;
		s_EditTestVec3_3.m_ConfirmAction = [](EditorUI::EditVec3Spec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestVec3_3 = spec.m_CurrentVec3;
		};

		// Vec4 initialization
		s_EditTestVec4_1.m_Label = "Test Vec4 1";
		s_EditTestVec4_1.m_CurrentVec4 = globals.m_TestVec4_1;
		s_EditTestVec4_1.m_ConfirmAction = [](EditorUI::EditVec4Spec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestVec4_1 = spec.m_CurrentVec4;
		};

		s_EditTestVec4_2.m_Label = "Test Vec4 2";
		s_EditTestVec4_2.m_CurrentVec4 = globals.m_TestVec4_2;
		s_EditTestVec4_2.m_ConfirmAction = [](EditorUI::EditVec4Spec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestVec4_2 = spec.m_CurrentVec4;
		};

		s_EditTestVec4_3.m_Label = "Test Vec4 3";
		s_EditTestVec4_3.m_CurrentVec4 = globals.m_TestVec4_3;
		s_EditTestVec4_3.m_ConfirmAction = [](EditorUI::EditVec4Spec& spec)
		{
			Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };
			globals.m_TestVec4_3 = spec.m_CurrentVec4;
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

		s_DataRegistry.Init(&s_TestHeapAlloc);

		s_EntityIDSpec.m_Label = "Active Entity ID";
		s_EntityIDSpec.m_Bounds = {0, 1'000};

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
		Utility::DebugGlobals& globals{ Utility::DebugGlobalsService::GetDebugGlobals() };

		// Bool initialization
		s_EditTestBool_1.m_CurrentBoolean = globals.m_TestBool_1;
		EditorUI::EditorUIService::Checkbox(s_EditTestBool_1);

		s_EditTestBool_2.m_CurrentBoolean = globals.m_TestBool_2;
		EditorUI::EditorUIService::Checkbox(s_EditTestBool_2);

		s_EditTestBool_3.m_CurrentBoolean = globals.m_TestBool_3;
		EditorUI::EditorUIService::Checkbox(s_EditTestBool_3);

		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Float initialization
		s_EditTestFloat_1.m_CurrentFloat = globals.m_TestFloat_1;
		EditorUI::EditorUIService::EditFloat(s_EditTestFloat_1);

		s_EditTestFloat_2.m_CurrentFloat = globals.m_TestFloat_2;
		EditorUI::EditorUIService::EditFloat(s_EditTestFloat_2);

		s_EditTestFloat_3.m_CurrentFloat = globals.m_TestFloat_3;
		EditorUI::EditorUIService::EditFloat(s_EditTestFloat_3);

		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Integer initialization
		s_EditTestInt_1.m_CurrentInteger = globals.m_TestInt_1;
		EditorUI::EditorUIService::EditInteger(s_EditTestInt_1);

		s_EditTestInt_2.m_CurrentInteger = globals.m_TestInt_2;
		EditorUI::EditorUIService::EditInteger(s_EditTestInt_2);

		s_EditTestInt_3.m_CurrentInteger = globals.m_TestInt_3;
		EditorUI::EditorUIService::EditInteger(s_EditTestInt_3);

		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Unsigned Integer initialization
		s_EditTestUInt_1.m_CurrentInteger = globals.m_TestUInt_1;
		EditorUI::EditorUIService::EditInteger(s_EditTestUInt_1);

		s_EditTestUInt_2.m_CurrentInteger = globals.m_TestUInt_2;
		EditorUI::EditorUIService::EditInteger(s_EditTestUInt_2);

		s_EditTestUInt_3.m_CurrentInteger = globals.m_TestUInt_3;
		EditorUI::EditorUIService::EditInteger(s_EditTestUInt_3);

		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Vec2 initialization
		s_EditTestVec2_1.m_CurrentVec2 = globals.m_TestVec2_1;
		EditorUI::EditorUIService::EditVec2(s_EditTestVec2_1);

		s_EditTestVec2_2.m_CurrentVec2 = globals.m_TestVec2_2;
		EditorUI::EditorUIService::EditVec2(s_EditTestVec2_2);

		s_EditTestVec2_3.m_CurrentVec2 = globals.m_TestVec2_3;
		EditorUI::EditorUIService::EditVec2(s_EditTestVec2_3);

		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Vec3 initialization
		s_EditTestVec3_1.m_CurrentVec3 = globals.m_TestVec3_1;
		EditorUI::EditorUIService::EditVec3(s_EditTestVec3_1);

		s_EditTestVec3_2.m_CurrentVec3 = globals.m_TestVec3_2;
		EditorUI::EditorUIService::EditVec3(s_EditTestVec3_2);

		s_EditTestVec3_3.m_CurrentVec3 = globals.m_TestVec3_3;
		EditorUI::EditorUIService::EditVec3(s_EditTestVec3_3);

		EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);

		// Vec4 initialization
		s_EditTestVec4_1.m_CurrentVec4 = globals.m_TestVec4_1;
		EditorUI::EditorUIService::EditVec4(s_EditTestVec4_1);

		s_EditTestVec4_2.m_CurrentVec4 = globals.m_TestVec4_2;
		EditorUI::EditorUIService::EditVec4(s_EditTestVec4_2);

		s_EditTestVec4_3.m_CurrentVec4 = globals.m_TestVec4_3;
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
			s_TestPlot.AddValue(Utility::STLRandomService::GetActiveRandom().GenerateRandomFloat(0.0f, 30.0f));
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
				result->m_Value = (uint64_t)Utility::STLRandomService::GetActiveRandom().GenerateRandomInteger(0, 500);
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
				size_t randomChoice = (size_t)Utility::STLRandomService::GetActiveRandom().GenerateRandomInteger(0, (int32_t)allIndices.size() - 1);

				KG_TRACE_INFO("Removing value {} and index {}", s_SparseArray[allIndices[randomChoice]], allIndices[randomChoice]);
				s_SparseArray.Remove(allIndices[randomChoice]);
			}
			else 
			{
				KG_TRACE_INFO("Failed to remove. No elements in sparse array");
			}
			
		}

		static int testIndex{ 1 };

		ImGui::DragInt("Sparse Set Index", &testIndex, 1, 0, 100);
		/*
		if (ImGui::Button("Add Sparse Set Index"))
		{
			if (s_SparseSet.InsertElement(testIndex) == ECS::k_InvalidDenseIndex)
			{
				KG_TRACE_INFO("Add operation failed!");
			}
			else
			{
				KG_TRACE_INFO("Add operation success!!");
			}
		}

		if (ImGui::Button("Delete Sparse Set Index"))
		{
			if (s_SparseSet.DeleteElement(testIndex) == ECS::k_InvalidDenseIndex)
			{
				KG_TRACE_INFO("Delete operation failed!");
			}
			else
			{
				KG_TRACE_INFO("Delete operation success!!");
			}
		}

		if (ImGui::Button("Print Sparse Set"))
		{
			KG_TRACE_INFO(s_SparseSet.Print());
		}
		*/

		EditorUI::EditorUIService::Text("Current Entity ID");
		EditorUI::EditorUIService::EditInteger(s_EntityIDSpec);

		if (ImGui::Button("Add Entity"))
		{
			Expected<ECS::EntityID> newID{ s_DataRegistry.CreateEntity() };
			if (newID)
			{
				KG_TRACE_INFO("Add operation success!");
			}
			else
			{
				KG_TRACE_INFO("Add operation failed!");
			}
		}

		if (ImGui::Button("Delete Entity"))
		{
			if (s_DataRegistry.DestroyEntity(s_EntityIDSpec.m_CurrentInteger))
			{
				KG_TRACE_INFO("Delete operation success!");
			}
			else
			{
				KG_TRACE_INFO("Delete operation failed!");
			}
		}

		if (ImGui::Button("Print All Entities"))
		{
			std::stringstream ss;
			ss << "All Entities: ";
			for (ECS::EntityID id : s_DataRegistry.GetAllEntities())
			{
				ss << id << ' ';
			}
			ss << '\n';
			KG_TRACE_INFO(ss.str());
		}

		EditorUI::EditorUIService::Text("Add / Remove Components");
		if (ImGui::Button("Add Transform Component"))
		{
			TransformTest testTransform{};
			bool success = s_DataRegistry.AddComponent<TransformTest>(s_EntityIDSpec.m_CurrentInteger, testTransform);

			if (success)
			{
				KG_TRACE_INFO("Add transform succeeded");
			}
			else
			{
				KG_TRACE_INFO("Add transform failed");
			}
		}
		
		if (ImGui::Button("Delete Transform Component"))
		{
			bool success = s_DataRegistry.RemoveComponent<TransformTest>(s_EntityIDSpec.m_CurrentInteger);

			if (success)
			{
				KG_TRACE_INFO("Delete transform succeeded");
			}
			else
			{
				KG_TRACE_INFO("Delete transform failed");
			}
		}

		if (ImGui::Button("Move Entity Up"))
		{
			ExpectedRef<TransformTest> transformRef = s_DataRegistry.GetComponent<TransformTest>(s_EntityIDSpec.m_CurrentInteger);
			if (!transformRef)
			{
				KG_TRACE_INFO("[[ERROR]]: Could not locate component!!!!!");
			}
			else
			{
				TransformTest& transform{ transformRef.value().get() };

				transform.location.y += 1.0f;
			}
		}

		if (ImGui::Button("Add Health Component"))
		{
			HealthTest testTransform{};
			bool success = s_DataRegistry.AddComponent<HealthTest>(s_EntityIDSpec.m_CurrentInteger, testTransform);

			if (success)
			{
				KG_TRACE_INFO("Add health succeeded");
			}
			else
			{
				KG_TRACE_INFO("Add health failed");
			}
		}

		if (ImGui::Button("Delete Health Component"))
		{
			bool success = s_DataRegistry.RemoveComponent<HealthTest>(s_EntityIDSpec.m_CurrentInteger);

			if (success)
			{
				KG_TRACE_INFO("Delete health succeeded");
			}
			else
			{
				KG_TRACE_INFO("Delete health failed");
			}
		}

		if (ImGui::Button("Add Transform To All Entities"))
		{
			std::span<ECS::EntityID> allEntities = s_DataRegistry.GetAllEntities();
			for (ECS::EntityID id : allEntities)
			{
				ExpectedRef<TransformTest> transformRef = s_DataRegistry.GetComponent<TransformTest>(id);
				if (!transformRef)
				{
					TransformTest test{};
					s_DataRegistry.AddComponent<TransformTest>(id, test);
				}
			}
		}

		if (ImGui::Button("Delete Transform To All Entities"))
		{
			auto transformView = s_DataRegistry.GetPackedView<TransformTest>();
			for (ECS::EntityID id : transformView)
			{
				KG_ASSERT(s_DataRegistry.RemoveComponent<TransformTest>(id));
			}
		}

		if (ImGui::Button("Add Health To All Entities"))
		{
			std::span<ECS::EntityID> allEntities = s_DataRegistry.GetAllEntities();
			for (ECS::EntityID id : allEntities)
			{
				ExpectedRef<HealthTest> transformRef = s_DataRegistry.GetComponent<HealthTest>(id);
				if (!transformRef)
				{
					HealthTest test{};
					s_DataRegistry.AddComponent<HealthTest>(id, test);
				}
			}
		}

		if (ImGui::Button("Delete Health To All Entities"))
		{
			auto transformView = s_DataRegistry.GetPackedView<HealthTest>();
			for (ECS::EntityID id : transformView)
			{
				KG_ASSERT(s_DataRegistry.RemoveComponent<HealthTest>(id));
			}
		}

		EditorUI::EditorUIService::Text("Get Views");
		if (ImGui::Button("Print Out All Transform Components"))
		{
			auto transformView = s_DataRegistry.GetPackedView<TransformTest>();
			for (ECS::EntityID id : transformView)
			{
				ExpectedRef<TransformTest> transformRef = s_DataRegistry.GetComponent<TransformTest>(id);
				if (!transformRef)
				{
					KG_TRACE_INFO("[[ERROR]]: Could not locate component!!!!!");
					continue;
				}
				
				TransformTest& transform{ transformRef.value().get() };

				KG_TRACE_INFO("Entity {} | Position x: {} y: {} | Size x: {}, y: {}", 
					id, 
					transform.location.x, transform.location.y,
					transform.size.x, transform.size.y
				);
			}
		}

		if (ImGui::Button("Print Out All Health Components"))
		{
			auto healthView = s_DataRegistry.GetPackedView<HealthTest>();
			for (ECS::EntityID id : healthView)
			{
				ExpectedRef<HealthTest> healthRef = s_DataRegistry.GetComponent<HealthTest>(id);
				if (!healthRef)
				{
					KG_TRACE_INFO("[[ERROR]]: Could not locate component!!!!!");
					continue;
				}

				HealthTest& health{ healthRef.value().get() };

				KG_TRACE_INFO("Entity {} | Head: {} Torso: {} | Arms Left: {}, Right: {} | Legs Left: {}, Right: {}",
					id,
					health.headHealth, health.torsoHealth,
					health.armsHealth.x, health.armsHealth.y,
					health.legsHealth.x, health.legsHealth.y
				);
			}
		}

		if (ImGui::Button("Print Out All Transform & Health Components"))
		{
			auto combinedView = s_DataRegistry.GetPackedView<TransformTest, HealthTest>();
			for (ECS::EntityID id : combinedView)
			{
				ExpectedRef<TransformTest> transformRef = s_DataRegistry.GetComponent<TransformTest>(id);
				if (!transformRef)
				{
					KG_TRACE_INFO("[[ERROR]]: Could not locate component!!!!!");
					continue;
				}

				TransformTest& transform{ transformRef.value().get() };

				KG_TRACE_INFO("Entity {} | Position x: {} y: {} | Size x: {}, y: {}",
					id,
					transform.location.x, transform.location.y,
					transform.size.x, transform.size.y
				);

				ExpectedRef<HealthTest> healthRef = s_DataRegistry.GetComponent<HealthTest>(id);
				if (!healthRef)
				{
					KG_TRACE_INFO("[[ERROR]]: Could not locate component!!!!!");
					continue;
				}

				HealthTest& health{ healthRef.value().get() };

				KG_TRACE_INFO("Entity {} | Head: {} Torso: {} | Arms Left: {}, Right: {} | Legs Left: {}, Right: {}",
					id,
					health.headHealth, health.torsoHealth,
					health.armsHealth.x, health.armsHealth.y,
					health.legsHealth.x, health.legsHealth.y
				);
			}
		}

		if (ImGui::Button("Move All Entities Up By One"))
		{
			auto transformView = s_DataRegistry.GetPackedView<TransformTest>();
			for (ECS::EntityID id : transformView)
			{
				ExpectedRef<TransformTest> transformRef = s_DataRegistry.GetComponent<TransformTest>(id);
				if (!transformRef)
				{
					KG_TRACE_INFO("[[ERROR]]: Could not locate component!!!!!");
					continue;
				}

				TransformTest& transform{ transformRef.value().get() };

				transform.location.y += 1.0f;
			}
		}


		if (ImGui::Button("Print Transform Name"))
		{
			Utility::ReturnTemplateNames<TransformTest> values = Utility::CompilerInfo::GetTemplateArgumentNames<TransformTest>();
			KG_TRACE_INFO("Transform Component. Stringified: {}",
				values[0]);
		}

		if (ImGui::Button("Print Health Name"))
		{
			Utility::ReturnTemplateNames<HealthTest> values = Utility::CompilerInfo::GetTemplateArgumentNames<HealthTest>();
			KG_TRACE_INFO("Health Component. Stringified: {}",
				values[0]);
		}

		if (ImGui::Button("Print TestingPanel Name"))
		{
			Utility::ReturnTemplateNames<TestingPanel> values = Utility::CompilerInfo::GetTemplateArgumentNames<TestingPanel>();
			KG_TRACE_INFO("Testing Panel. Stringified: {}",
				values[0]);
		}


		if (ImGui::Button("Print All Struct Names"))
		{
			auto values = Utility::CompilerInfo::GetTemplateArgumentNames<TransformTest, HealthTest, TestingPanel>();
			
			KG_TRACE_INFO("Print All Struct Names:");
			for (std::string_view str : values)
			{
				KG_TRACE_INFO(str);
			}
		}

		if (ImGui::Button("Test out Compile time Hashing"))
		{
			uint32_t value{ Utility::FileSystem::CRCFromString("This is some text") };
			KG_TRACE_INFO(value);
		}

		if (ImGui::Button("Print Out Metaprogramming Unique Identifier"))
		{
			constexpr auto value{ GetUniqueIdentifier<TestComponent>() };
			KG_TRACE_INFO(value.CString());
		}

		if (ImGui::Button("Print Identifier"))
		{
			auto value{ GetTypeName<TestComponent>() };
			KG_TRACE_INFO(value);

			//uint32_t value{ Utility::FileSystem::CRCFromString("This is some text") };
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
