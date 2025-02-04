#include "Windows/MainWindow/TestingPanel.h"
#include "Windows/MainWindow/ViewportPanel.h"

#include "EditorApp.h"
#include "Kargono/Utility/DebugGlobals.h"
#include "Kargono/Utility/Timers.h"
#include "Kargono/Scripting/ScriptCompilerService.h"
#include "Kargono/Utility/Random.h"

static Kargono::EditorApp* s_EditorApp { nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{
	static EditorUI::EditTextSpec s_TestText {};
	static EditorUI::EditFloatSpec s_TimerTime{};
	static EditorUI::EditIntegerSpec s_RandomTestInteger{};
	static EditorUI::EditMultiLineTextSpec s_MultiLineText{};

// TODO: Testing Splines
#if 0
	static std::vector<EditorUI::EditVec3Spec> s_ControlPointWidgets;
#endif

	static FixedString256 newString;
	static EditorUI::TooltipSpec testTooltip{};

	enum class TestTypes : uint32_t
	{
		None = 0,
		Display
	};

	
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

		EditorUI::EditorUIService::TitleText("Welcome to the Testing Panel!");
		DrawDebugGlobalWidgets();
		DrawGeneralTestingWidgets();

		EditorUI::EditorUIService::EndWindow();
	}
	void TestingPanel::InitializeDebugGlobalsWidgets()
	{
#if defined(KG_DEBUG)
		s_DebugGlobalsHeader.m_Label = "Debug Globals";
		s_DebugGlobalsHeader.m_Expanded = true;
		s_DebugGlobalsHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;

		// Checkbox initialization
		s_EditTestBool_1.m_Label = "Test Bool 1";
		s_EditTestBool_1.m_Flags |= EditorUI::Checkbox_Indented;
		s_EditTestBool_1.m_CurrentBoolean = Utility::DebugGlobals::s_TestBool_1;
		s_EditTestBool_1.m_ConfirmAction = [&](EditorUI::CheckboxSpec& spec) 
		{
				Utility::DebugGlobals::s_TestBool_1 = spec.m_CurrentBoolean;
		};

		s_EditTestBool_2.m_Label = "Test Bool 2";
		s_EditTestBool_2.m_Flags |= EditorUI::Checkbox_Indented;
		s_EditTestBool_2.m_CurrentBoolean = Utility::DebugGlobals::s_TestBool_2;
		s_EditTestBool_2.m_ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
			{
				Utility::DebugGlobals::s_TestBool_2 = spec.m_CurrentBoolean;
			};

		s_EditTestBool_3.m_Label = "Test Bool 3";
		s_EditTestBool_3.m_Flags |= EditorUI::Checkbox_Indented;
		s_EditTestBool_3.m_CurrentBoolean = Utility::DebugGlobals::s_TestBool_3;
		s_EditTestBool_3.m_ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
			{
				Utility::DebugGlobals::s_TestBool_3 = spec.m_CurrentBoolean;
			};


		// Float initialization
		s_EditTestFloat_1.m_Label = "Test Float 1";
		s_EditTestFloat_1.m_CurrentFloat = Utility::DebugGlobals::s_TestFloat_1;
		s_EditTestFloat_1.m_Flags |= EditorUI::EditFloatFlags::EditFloat_Indented;
		s_EditTestFloat_1.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
			{
				Utility::DebugGlobals::s_TestFloat_1 = spec.m_CurrentFloat;
			};

		s_EditTestFloat_2.m_Label = "Test Float 2";
		s_EditTestFloat_2.m_CurrentFloat = Utility::DebugGlobals::s_TestFloat_2;
		s_EditTestFloat_2.m_Flags |= EditorUI::EditFloatFlags::EditFloat_Indented;
		s_EditTestFloat_2.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
			{
				Utility::DebugGlobals::s_TestFloat_2 = spec.m_CurrentFloat;
			};

		s_EditTestFloat_3.m_Label = "Test Float 3";
		s_EditTestFloat_3.m_CurrentFloat = Utility::DebugGlobals::s_TestFloat_3;
		s_EditTestFloat_3.m_Flags |= EditorUI::EditFloatFlags::EditFloat_Indented;
		s_EditTestFloat_3.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
			{
				Utility::DebugGlobals::s_TestFloat_3 = spec.m_CurrentFloat;
			};

		// Integer initialization
		s_EditTestInt_1.m_Label = "Test Int 1";
		s_EditTestInt_1.m_CurrentInteger = Utility::DebugGlobals::s_TestInt_1;
		s_EditTestInt_1.m_Flags |= EditorUI::EditIntegerFlags::EditInteger_Indented;
		s_EditTestInt_1.m_ConfirmAction = [&](EditorUI::EditIntegerSpec& spec)
			{
				Utility::DebugGlobals::s_TestInt_1 = spec.m_CurrentInteger;
			};

		s_EditTestInt_2.m_Label = "Test Int 2";
		s_EditTestInt_2.m_CurrentInteger = Utility::DebugGlobals::s_TestInt_2;
		s_EditTestInt_2.m_Flags |= EditorUI::EditIntegerFlags::EditInteger_Indented;
		s_EditTestInt_2.m_ConfirmAction = [&](EditorUI::EditIntegerSpec& spec)
			{
				Utility::DebugGlobals::s_TestInt_2 = spec.m_CurrentInteger;
			};

		s_EditTestInt_3.m_Label = "Test Int 3";
		s_EditTestInt_3.m_CurrentInteger = Utility::DebugGlobals::s_TestInt_3;
		s_EditTestInt_3.m_Flags |= EditorUI::EditIntegerFlags::EditInteger_Indented;
		s_EditTestInt_3.m_ConfirmAction = [&](EditorUI::EditIntegerSpec& spec)
			{
				Utility::DebugGlobals::s_TestInt_3 = spec.m_CurrentInteger;
			};

		// Unsigned Integer initialization
		s_EditTestUInt_1.m_Label = "Test UInt 1";
		s_EditTestUInt_1.m_CurrentInteger = Utility::DebugGlobals::s_TestUInt_1;
		s_EditTestUInt_1.m_Bounds = { 0, 10'000 };
		s_EditTestUInt_1.m_Flags |= EditorUI::EditIntegerFlags::EditInteger_Indented;
		s_EditTestUInt_1.m_ConfirmAction = [&](EditorUI::EditIntegerSpec& spec)
			{
				Utility::DebugGlobals::s_TestUInt_1 = spec.m_CurrentInteger;
			};

		s_EditTestUInt_2.m_Label = "Test UInt 2";
		s_EditTestUInt_2.m_CurrentInteger = Utility::DebugGlobals::s_TestUInt_2;
		s_EditTestUInt_2.m_Bounds = { 0, 10'000 };
		s_EditTestUInt_2.m_Flags |= EditorUI::EditIntegerFlags::EditInteger_Indented;
		s_EditTestUInt_2.m_ConfirmAction = [&](EditorUI::EditIntegerSpec& spec)
			{
				Utility::DebugGlobals::s_TestUInt_2 = spec.m_CurrentInteger;
			};

		s_EditTestUInt_3.m_Label = "Test UInt 3";
		s_EditTestUInt_3.m_CurrentInteger = Utility::DebugGlobals::s_TestUInt_3;
		s_EditTestUInt_3.m_Bounds = { 0, 10'000 };
		s_EditTestUInt_3.m_Flags |= EditorUI::EditIntegerFlags::EditInteger_Indented;
		s_EditTestUInt_3.m_ConfirmAction = [&](EditorUI::EditIntegerSpec& spec)
			{
				Utility::DebugGlobals::s_TestUInt_3 = spec.m_CurrentInteger;
			};

		// Vec2 initialization
		s_EditTestVec2_1.m_Label = "Test Vec2 1";
		s_EditTestVec2_1.m_CurrentVec2 = Utility::DebugGlobals::s_TestVec2_1;
		s_EditTestVec2_1.m_Flags |= EditorUI::EditVec2Flags::EditVec2_Indented;
		s_EditTestVec2_1.m_ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec2_1 = spec.m_CurrentVec2;
			};

		s_EditTestVec2_2.m_Label = "Test Vec2 2";
		s_EditTestVec2_2.m_CurrentVec2 = Utility::DebugGlobals::s_TestVec2_2;
		s_EditTestVec2_2.m_Flags |= EditorUI::EditVec2Flags::EditVec2_Indented;
		s_EditTestVec2_2.m_ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec2_2 = spec.m_CurrentVec2;
			};

		s_EditTestVec2_3.m_Label = "Test Vec2 3";
		s_EditTestVec2_3.m_CurrentVec2 = Utility::DebugGlobals::s_TestVec2_3;
		s_EditTestVec2_3.m_Flags |= EditorUI::EditVec2Flags::EditVec2_Indented;
		s_EditTestVec2_3.m_ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec2_3 = spec.m_CurrentVec2;
			};

		// Vec3 initialization
		s_EditTestVec3_1.m_Label = "Test Vec3 1";
		s_EditTestVec3_1.m_CurrentVec3 = Utility::DebugGlobals::s_TestVec3_1;
		s_EditTestVec3_1.m_Flags |= EditorUI::EditVec3Flags::EditVec3_Indented;
		s_EditTestVec3_1.m_ConfirmAction = [&](EditorUI::EditVec3Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec3_1 = spec.m_CurrentVec3;
			};

		s_EditTestVec3_2.m_Label = "Test Vec3 2";
		s_EditTestVec3_2.m_CurrentVec3 = Utility::DebugGlobals::s_TestVec3_2;
		s_EditTestVec3_2.m_Flags |= EditorUI::EditVec3Flags::EditVec3_Indented;
		s_EditTestVec3_2.m_ConfirmAction = [&](EditorUI::EditVec3Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec3_2 = spec.m_CurrentVec3;
			};

		s_EditTestVec3_3.m_Label = "Test Vec3 3";
		s_EditTestVec3_3.m_CurrentVec3 = Utility::DebugGlobals::s_TestVec3_3;
		s_EditTestVec3_3.m_Flags |= EditorUI::EditVec3Flags::EditVec3_Indented;
		s_EditTestVec3_3.m_ConfirmAction = [&](EditorUI::EditVec3Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec3_3 = spec.m_CurrentVec3;
			};

		// Vec4 initialization
		s_EditTestVec4_1.m_Label = "Test Vec4 1";
		s_EditTestVec4_1.m_CurrentVec4 = Utility::DebugGlobals::s_TestVec4_1;
		s_EditTestVec4_1.m_Flags |= EditorUI::EditVec4Flags::EditVec4_Indented;
		s_EditTestVec4_1.m_ConfirmAction = [&](EditorUI::EditVec4Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec4_1 = spec.m_CurrentVec4;
			};

		s_EditTestVec4_2.m_Label = "Test Vec4 2";
		s_EditTestVec4_2.m_CurrentVec4 = Utility::DebugGlobals::s_TestVec4_2;
		s_EditTestVec4_2.m_Flags |= EditorUI::EditVec4Flags::EditVec4_Indented;
		s_EditTestVec4_2.m_ConfirmAction = [&](EditorUI::EditVec4Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec4_2 = spec.m_CurrentVec4;
			};

		s_EditTestVec4_3.m_Label = "Test Vec4 3";
		s_EditTestVec4_3.m_CurrentVec4 = Utility::DebugGlobals::s_TestVec4_3;
		s_EditTestVec4_3.m_Flags |= EditorUI::EditVec4Flags::EditVec4_Indented;
		s_EditTestVec4_3.m_ConfirmAction = [&](EditorUI::EditVec4Spec& spec)
			{
				Utility::DebugGlobals::s_TestVec4_3 = spec.m_CurrentVec4;
			};
#endif

	}
	void TestingPanel::InitializeGeneralTestingWidgets()
	{
		s_TestText.m_Label = "File to Compile";
		s_TestText.m_CurrentOption = "test.kgscript";

		s_TimerTime.m_Label = "Timer Time";
		s_TimerTime.m_CurrentFloat = 3.0f;

		s_RandomTestInteger.m_Label = "Intenger Time";
		s_RandomTestInteger.m_CurrentInteger = 5;

		m_TestHeader.m_Label = "directory/directory/file.txt";
		newString = "Hahahaha";

		// Test tooltip api
		testTooltip.m_Label = "Test Tooltip";
		// Create menu items
		EditorUI::TooltipEntry newEntry{ "First Test Entry", [](EditorUI::TooltipEntry& entry)
		{
			KG_WARN("Meow");
		} };

		testTooltip.AddTooltipEntry(std::move(newEntry));
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

		s_MultiLineText.m_Label = "Test Multi Line";
		s_MultiLineText.m_CurrentOption = "This is a paragraph of text bahahahahahahahahahahahaha";
	}
	void TestingPanel::DrawDebugGlobalWidgets()
	{
#if defined(KG_DEBUG)
		EditorUI::EditorUIService::CollapsingHeader(s_DebugGlobalsHeader);

		if (s_DebugGlobalsHeader.m_Expanded)
		{
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
		}
		
#endif
	}
	void TestingPanel::DrawGeneralTestingWidgets()
	{
		if (ImGui::Button("Calculate Navigation Links"))
		{
			RuntimeUI::RuntimeUIService::CalculateWindowNavigationLinks();
		}

		if (ImGui::Button("Crit Log"))
		{
			KG_CRITICAL("Hey we in this bihh {}", Utility::RandomService::GenerateRandomInteger(0, 20));
		}

		if (ImGui::Button("Warn Log"))
		{
			KG_WARN("Hey we in this bihh {}", Utility::RandomService::GenerateRandomInteger(0, 20));
		}


		EditorUI::EditorUIService::EditText(s_TestText);

		if (ImGui::Button("Compile File"))
		{
			KG_TRACE_CRITICAL(Scripting::ScriptCompilerService::CompileScriptFile("./../Projects/Pong/Assets/" + s_TestText.m_CurrentOption));
		}

		if (ImGui::Button("Test Popup"))
		{
			ImGui::OpenPopup("The Test Popup");
		}

		EditorUI::EditorUIService::EditFloat(s_TimerTime);
		EditorUI::EditorUIService::EditInteger(s_RandomTestInteger);

		if (ImGui::Button("Start Timer"))
		{
			Utility::PassiveTimer::CreateTimer(s_TimerTime.m_CurrentFloat, []()
				{
					KG_WARN("The timer has gone off");
				});
		}

		ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing;

		bool closePopup = false;
		if (ImGui::Button("HSDHASDJ"))
		{
			closePopup = true;
		}

		if (ImGui::BeginPopup("The Test Popup", popupFlags))
		{
			EditorUI::EditorUIService::BringCurrentWindowToFront();

			ImGui::Text("Ayooo, the popup is open");
			if (closePopup)
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		EditorUI::EditorUIService::EditMultiLineText(s_MultiLineText);

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

#if 0
		bool backActive = m_CurrentDirectory != std::filesystem::path(m_BaseDirectory);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		if (!backActive)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		}
		// Draw icon for moving a directory back
		if (ImGui::ImageButton((ImTextureID)(uint64_t)EditorUI::EditorUIService::s_IconBack->GetRendererID(),
			{ 24.0f, 24.0f }, { 0, 1 }, { 1, 0 },
			-1, ImVec4(0, 0, 0, 0),
			backActive ? EditorUI::EditorUIService::s_PrimaryTextColor : EditorUI::EditorUIService::s_DisabledColor))
		{
			if (backActive)
			{
				UpdateCurrentDirectory(m_CurrentDirectory.parent_path());
			}
		}
		if (!backActive)
		{
			ImGui::PopStyleColor(2);
		}
		if (backActive && ImGui::BeginDragDropTarget())
		{
			for (auto& payloadName : EditorUI::EditorUIService::s_AllPayloadTypes)
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName.c_str()))
				{
					const wchar_t* payloadPathPointer = (const wchar_t*)payload->Data;
					std::filesystem::path payloadPath(payloadPathPointer);
					Utility::FileSystem::MoveFileToDirectory(payloadPath, m_CurrentDirectory.parent_path());
					break;
				}
			}
			ImGui::EndDragDropTarget();
		}

		// Draw icon for moving a directory forward
		bool forwardActive = m_CurrentDirectory != m_LongestRecentPath && !m_LongestRecentPath.empty();
		ImGui::SameLine();
		if (!forwardActive)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		}
		if (ImGui::ImageButton((ImTextureID)(uint64_t)EditorUI::EditorUIService::s_IconForward->GetRendererID(),
			{ 24.0f, 24.0f }, { 0, 1 }, { 1, 0 },
			-1, ImVec4(0, 0, 0, 0),
			forwardActive ? EditorUI::EditorUIService::s_PrimaryTextColor : EditorUI::EditorUIService::s_DisabledColor))
		{
			if (forwardActive && Utility::FileSystem::DoesPathContainSubPath(m_CurrentDirectory, m_LongestRecentPath))
			{
				std::filesystem::path currentIterationPath{ m_LongestRecentPath };
				std::filesystem::path recentIterationPath{ m_LongestRecentPath };
				while (currentIterationPath != m_CurrentDirectory)
				{
					recentIterationPath = currentIterationPath;
					currentIterationPath = currentIterationPath.parent_path();
				}
				UpdateCurrentDirectory(recentIterationPath);
			}
		}
		if (!forwardActive)
		{
			ImGui::PopStyleColor(2);
		}
		if (forwardActive && ImGui::BeginDragDropTarget())
		{
			for (auto& payloadName : EditorUI::EditorUIService::s_AllPayloadTypes)
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName.c_str()))
				{
					const wchar_t* payloadPathPointer = (const wchar_t*)payload->Data;
					std::filesystem::path payloadPath(payloadPathPointer);
					std::filesystem::path currentIterationPath{ m_LongestRecentPath };
					std::filesystem::path recentIterationPath{ m_LongestRecentPath };
					while (currentIterationPath != m_CurrentDirectory)
					{
						recentIterationPath = currentIterationPath;
						currentIterationPath = currentIterationPath.parent_path();
					}
					Utility::FileSystem::MoveFileToDirectory(payloadPath, recentIterationPath);
					break;
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopStyleColor();

		// Current directory title
		std::filesystem::path activeDirectory = Utility::FileSystem::GetRelativePath(Projects::ProjectService::GetActiveProjectDirectory(), m_CurrentDirectory);

		std::vector<std::string> tokenizedDirectoryPath{};

		while (activeDirectory.filename() != "Assets")
		{
			tokenizedDirectoryPath.push_back(activeDirectory.filename().string());
			activeDirectory = activeDirectory.parent_path();
		}
		tokenizedDirectoryPath.push_back("Assets");

		ImGui::PushFont(EditorUI::EditorUIService::s_FontPlexBold);
		for (int32_t i = (int32_t)(tokenizedDirectoryPath.size()) - 1; i >= 0; --i)
		{
			ImGui::SameLine();
			ImGui::Text(tokenizedDirectoryPath.at(i).c_str());
			if (i != 0)
			{
				ImGui::SameLine();
				ImGui::Text("/");
			}
		}
		ImGui::PopFont();
		ImGui::Separator();
#endif

#if 0
		EditorUI::EditorUIService::NavigationHeader(m_TestHeader);

		EditorUI::EditorUIService::Tooltip(testTooltip);
#endif
	}
	bool TestingPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	bool TestingPanel::OnInputEvent(Events::Event* event)
	{
		return false;
	}
}
