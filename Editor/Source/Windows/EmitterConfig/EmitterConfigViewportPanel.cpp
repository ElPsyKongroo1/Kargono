

#include "Windows/EmitterConfig/EmitterConfigViewportPanel.h"

#include "Modules/Rendering/RenderingService.h"
#include "Modules/Input/InputService.h"
#include "Modules/Rendering/Texture.h"
#include "Kargono/Utility/Operations.h"

#include "EditorApp.h"

static Kargono::EditorApp* s_EditorApp{ nullptr };
static Kargono::Windows::EmitterConfigWindow* s_EmitterConfigWindow{ nullptr };

namespace Kargono::Panels
{
	EmitterConfigViewportPanel::EmitterConfigViewportPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EmitterConfigWindow = s_EditorApp->m_EmitterConfigEditorWindow.get();

		InitializeFrameBuffer();
		InitializeOverlayData();

		m_EditorCamera = Rendering::EditorPerspectiveCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		ResetCamera();

		KG_ASSERT(Projects::ProjectService::GetActive());

	}

	EmitterConfigViewportPanel::~EmitterConfigViewportPanel()
	{
		ClearOverlayData();
	}

	void EmitterConfigViewportPanel::InitializeFrameBuffer()
	{
		Engine& engine{ EngineService::GetActiveEngine()};
		Rendering::FramebufferSpecification fbSpec;
		fbSpec.Attachments = { Rendering::FramebufferDataFormat::RGBA8, Rendering::FramebufferDataFormat::RED_INTEGER, Rendering::FramebufferDataFormat::Depth };
		fbSpec.Width = engine.GetWindow().GetWidth();
		fbSpec.Height = engine.GetWindow().GetHeight();
		m_ViewportFramebuffer = Rendering::Framebuffer::Create(fbSpec);
	}

	void EmitterConfigViewportPanel::OnUpdate(Timestep ts)
	{
		KG_PROFILE_FUNCTION();

		// Handle editor camera movement
		FixedString32 focusedWindow{ EditorUI::EditorUIService::GetFocusedWindowName() };
		if (focusedWindow == m_PanelName)
		{
			m_EditorCamera.OnUpdate(ts);
		}

		// Adjust framebuffer & camera viewport size if necessary
		Window& currentWindow = EngineService::GetActiveEngine().GetWindow();
		currentWindow.SetActiveViewport(&m_ViewportData);
		if (Rendering::FramebufferSpecification spec = m_ViewportFramebuffer->GetSpecification();
			(float)m_ViewportData.m_Width > 0.0f && (float)m_ViewportData.m_Height > 0.0f &&
			(spec.Width != m_ViewportData.m_Width || spec.Height != m_ViewportData.m_Height))
		{
			// Update framebuffer and camera viewport size
			m_ViewportFramebuffer->Resize(m_ViewportData.m_Width, m_ViewportData.m_Height);
			m_EditorCamera.SetViewportSize((float)m_ViewportData.m_Width, (float)m_ViewportData.m_Height);
		}

		// Prepare for rendering
		Rendering::RenderingService::ResetStats();
		m_ViewportFramebuffer->Bind();
		Rendering::RendererAPI::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Rendering::RendererAPI::Clear();

		// Clear mouse picking attachment value
		m_ViewportFramebuffer->SetAttachment(1, -1);

		// Draw gridlines
		DrawGridLines();

		// Draw particle bounds
		if (m_DisplaySpawningBoundsOutline)
		{
			DrawParticleSpawningBounds();
		}

		// Draw emitters
		Particles::ParticleService::GetActiveContext().OnUpdate(ts);
		Particles::ParticleService::GetActiveContext().OnRender(m_EditorCamera.GetViewProjection());

		HandleMouseHovering();

		// Complete rendering
		m_ViewportFramebuffer->Unbind();

	}
	void EmitterConfigViewportPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EmitterConfigWindow->m_ShowViewport);
		ImGui::PopStyleVar();

		// Early out if the window is not visible
		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		EditorUI::EditorUIService::AutoCalcViewportSize(m_ScreenViewportBounds, m_ViewportData, m_ViewportFocused, m_ViewportHovered,
			Utility::ScreenResolutionToAspectRatio(Projects::ProjectService::GetActiveTargetResolution()));

		uint64_t textureID = m_ViewportFramebuffer->GetColorAttachmentRendererID();
		ImGui::Image((ImTextureID)textureID, ImVec2{ (float)m_ViewportData.m_Width, (float)m_ViewportData.m_Height }, ImVec2{ 0, 1 },
			ImVec2{ 1, 0 });
		
		// TODO: Handle on mouse hover to select entity

		DrawGizmo();

		DrawToolbarOverlay();

		// End the window
		EditorUI::EditorUIService::EndWindow();
	}

	void EmitterConfigViewportPanel::OnInputEvent(Events::Event* event)
	{
		m_EditorCamera.OnInputEvent(event);
	}

	bool EmitterConfigViewportPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		/*bool control = Input::InputService::IsKeyPressed(Key::LeftControl) || Input::InputService::IsKeyPressed(Key::RightControl);
		bool shift = Input::InputService::IsKeyPressed(Key::LeftShift) || Input::InputService::IsKeyPressed(Key::RightShift);
		bool alt = Input::InputService::IsKeyPressed(Key::LeftAlt) || Input::InputService::IsKeyPressed(Key::RightAlt);*/

		switch (event.GetKeyCode())
		{
		case Key::Tab:
			m_EditorCamera.ToggleMovementType();
			return true;
		default:
			return false;
		}
	}

	// Overlay Data
	static Rendering::RendererInputSpec s_LineInputSpec{};
	static Rendering::RendererInputSpec s_PointInputSpec{};
	static std::vector<Math::vec4> s_RectangleVertexPositions
	{
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{ 0.5f, -0.5f, 0.0f, 1.0f },
			{ 0.5f, 0.5f, 0.0f, 1.0f },
			{ -0.5f, 0.5f, 0.0f, 1.0f }
	};

	static std::vector<uint32_t> s_RectangleIndices
	{
			0, 1,
			1, 2,
			2, 3,
			3, 0
	};
	static Ref<std::vector<Math::vec3>> s_OutputVector {CreateRef<std::vector<Math::vec3>>()};

	void EmitterConfigViewportPanel::InitializeOverlayData()
	{
		// Set up Line Input Specifications for Overlay Calls
		{
			Rendering::ShaderSpecification lineShaderSpec{ Rendering::ColorInputType::FlatColor, Rendering::TextureInputType::None, false, true, false, Rendering::RenderingType::DrawLine, false };
			auto [uuid, localShader] = Assets::AssetService::GetShader(lineShaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			// Add red color to shader
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_Red), 
				Utility::FileSystem::CRCFromString("a_Color"), 
				localBuffer, localShader);

			// Create the shape component
			ECS::ShapeComponent* lineShapeComponent = new ECS::ShapeComponent();
			lineShapeComponent->CurrentShape = Rendering::ShapeTypes::None;
			lineShapeComponent->Vertices = nullptr;

			s_LineInputSpec.m_Shader = localShader;
			s_LineInputSpec.m_Buffer = localBuffer;
			s_LineInputSpec.m_ShapeComponent = lineShapeComponent;
		}
		// Set up Point Input Specifications for Overlay Calls
		{
			Rendering::ShaderSpecification pointShaderSpec{ Rendering::ColorInputType::FlatColor, Rendering::TextureInputType::None, false, true, false, Rendering::RenderingType::DrawPoint, false };
			auto [uuid, localShader] = Assets::AssetService::GetShader(pointShaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 0.0f, 1.0f, 0.0f, 1.0f }, 
				Utility::FileSystem::CRCFromString("a_Color"), 
				localBuffer, localShader);

			ECS::ShapeComponent* pointShapeComponent = new ECS::ShapeComponent();
			pointShapeComponent->CurrentShape = Rendering::ShapeTypes::None;
			pointShapeComponent->Vertices = nullptr;

			s_PointInputSpec.m_Shader = localShader;
			s_PointInputSpec.m_Buffer = localBuffer;
			s_PointInputSpec.m_ShapeComponent = pointShapeComponent;
		}
	}

	void EmitterConfigViewportPanel::ClearOverlayData()
	{
		s_LineInputSpec.ClearData();
	}

	void EmitterConfigViewportPanel::HandleMouseHovering()
	{
	}
	void EmitterConfigViewportPanel::DrawToolbarOverlay()
	{

		//constexpr float k_IconSize{ 36.0f };
		ImGui::PushStyleColor(ImGuiCol_Button, EditorUI::EditorUIService::s_PureEmpty);
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 initialScreenCursorPos = ImGui::GetWindowPos() + ImGui::GetCursorStartPos();
		ImVec2 initialCursorPos = ImGui::GetCursorStartPos();

		ImVec2 windowSize = ImGui::GetWindowSize();
		Ref<Rendering::Texture2D> icon{ nullptr };
		if (m_ToolbarEnabled)
		{

			// Draw Display Options Background
			draw_list->AddRectFilled(ImVec2(initialScreenCursorPos.x + windowSize.x - 80.0f, initialScreenCursorPos.y),
				ImVec2(initialScreenCursorPos.x + (windowSize.x) - 48.0f, initialScreenCursorPos.y + 30.0f),
				ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 12.0f, ImDrawFlags_RoundCornersBottom);

			// Draw Grid Options Background
			draw_list->AddRectFilled(ImVec2(initialScreenCursorPos.x + windowSize.x - 257.0f, initialScreenCursorPos.y),
				ImVec2(initialScreenCursorPos.x + (windowSize.x) - 187.0f, initialScreenCursorPos.y + 30.0f),
				ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 12.0f, ImDrawFlags_RoundCornersBottom);

			// Draw Camera Options Background
			draw_list->AddRectFilled(ImVec2(initialScreenCursorPos.x + windowSize.x - 170.0f, initialScreenCursorPos.y),
				ImVec2(initialScreenCursorPos.x + (windowSize.x) - 100.0f, initialScreenCursorPos.y + 30.0f),
				ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 12.0f, ImDrawFlags_RoundCornersBottom);

			// Draw Toggle Top Bar Background
			draw_list->AddRectFilled(ImVec2(initialScreenCursorPos.x + windowSize.x - 30.0f, initialScreenCursorPos.y),
				ImVec2(initialScreenCursorPos.x + (windowSize.x), initialScreenCursorPos.y + 30.0f),
				ImColor(EditorUI::EditorUIService::s_DarkBackgroundColor), 12.0f, ImDrawFlags_RoundCornersBottomLeft);

			// Camera Options Button
			icon = EditorUI::EditorUIService::s_IconCamera;
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + windowSize.x - 163, initialCursorPos.y + 5));
			if (ImGui::ImageButton("Camera Options",
				(ImTextureID)(uint64_t)icon->GetRendererID(),
				ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
				EditorUI::EditorUIService::s_PureEmpty,
				EditorUI::EditorUIService::s_HighlightColor1))
			{
				ImGui::OpenPopup("Toggle Viewport Camera Options");
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, "Camera Movement Types");
				ImGui::EndTooltip();
			}

			if (ImGui::BeginPopup("Toggle Viewport Camera Options"))
			{
				if (ImGui::BeginMenu("Movement Type"))
				{
					if (ImGui::MenuItem("Model Viewer", 0,
						m_EditorCamera.GetMovementType() == Rendering::EditorPerspectiveCamera::MovementType::ModelView))
					{
						m_EditorCamera.SetMovementType(Rendering::EditorPerspectiveCamera::MovementType::ModelView);
					}
					if (ImGui::MenuItem("FreeFly", 0,
						m_EditorCamera.GetMovementType() == Rendering::EditorPerspectiveCamera::MovementType::FreeFly))
					{
						m_EditorCamera.SetMovementType(Rendering::EditorPerspectiveCamera::MovementType::FreeFly);
					}
					ImGui::EndMenu();
				}

				if (ImGui::MenuItem("Reset Camera"))
				{
					ResetCamera();
				}
				
				ImGui::EndPopup();
			}

			// Camera Speed
			ImGui::SetNextItemWidth(30.0f);
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + windowSize.x - 138, initialCursorPos.y + 6));
			ImGui::DragFloat("##CameraSpeed", &m_EditorCamera.GetMovementSpeed(), 0.5f,
				m_EditorCamera.GetMinMovementSpeed(), m_EditorCamera.GetMaxMovementSpeed(),
				"%.0f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_CenterText);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, "Camera Speed");
				ImGui::EndTooltip();
			}

			// Viewport Display Options Button
			icon = EditorUI::EditorUIService::s_IconDisplay;
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + windowSize.x - 75, initialCursorPos.y + 4));
			if (ImGui::ImageButton("Display Toggle",
				(ImTextureID)(uint64_t)icon->GetRendererID(),
				ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
				EditorUI::EditorUIService::s_PureEmpty,
				EditorUI::EditorUIService::s_HighlightColor1))
			{
				ImGui::OpenPopup("Display Options");
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, "Display Options");
				ImGui::EndTooltip();
			}

			// Display options popup
			if (ImGui::BeginPopup("Display Options"))
			{
				if (ImGui::MenuItem("Display Spawning Bounds Outline", 0, m_DisplaySpawningBoundsOutline))
				{
					Utility::Operations::ToggleBoolean(m_DisplaySpawningBoundsOutline);
				}
				ImGui::EndPopup();
			}


			// Grid Options Button
			icon = EditorUI::EditorUIService::s_IconGrid;
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + windowSize.x - 252, initialCursorPos.y + 4));
			if (ImGui::ImageButton("Grid Toggle",
				(ImTextureID)(uint64_t)icon->GetRendererID(),
				ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
				EditorUI::EditorUIService::s_PureEmpty,
				EditorUI::EditorUIService::s_HighlightColor1))
			{
				ImGui::OpenPopup("Grid Options");
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, "Grid Options");
				ImGui::EndTooltip();
			}

			if (ImGui::BeginPopup("Grid Options"))
			{
				if (ImGui::BeginMenu("X-Y Grid"))
				{
					if (ImGui::MenuItem("Display Infinite Grid", 0, m_DisplayXYMajorGrid))
					{
						Utility::Operations::ToggleBoolean(m_DisplayXYMajorGrid);

						if (!m_DisplayXYMajorGrid && m_DisplayXYMinorGrid)
						{
							Utility::Operations::ToggleBoolean(m_DisplayXYMinorGrid);
						}
					}
					if (m_DisplayXYMajorGrid)
					{
						if (ImGui::MenuItem("Display Local Grid", 0, m_DisplayXYMinorGrid))
						{
							Utility::Operations::ToggleBoolean(m_DisplayXYMinorGrid);
						}
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("X-Z Grid"))
				{
					if (ImGui::MenuItem("Display Infinite Grid", 0, m_DisplayXZMajorGrid))
					{
						Utility::Operations::ToggleBoolean(m_DisplayXZMajorGrid);

						if (!m_DisplayXZMajorGrid && m_DisplayXZMinorGrid)
						{
							Utility::Operations::ToggleBoolean(m_DisplayXZMinorGrid);
						}
					}

					if (m_DisplayXZMajorGrid)
					{
						if (ImGui::MenuItem("Display Local Grid", 0, m_DisplayXZMinorGrid))
						{
							Utility::Operations::ToggleBoolean(m_DisplayXZMinorGrid);
						}
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Y-Z Grid"))
				{
					if (ImGui::MenuItem("Display Infinite Grid", 0, m_DisplayYZMajorGrid))
					{
						Utility::Operations::ToggleBoolean(m_DisplayYZMajorGrid);
						if (!m_DisplayYZMajorGrid && m_DisplayYZMinorGrid)
						{
							Utility::Operations::ToggleBoolean(m_DisplayYZMinorGrid);
						}
					}
					if (m_DisplayYZMajorGrid)
					{
						if (ImGui::MenuItem("Display Local Grid", 0, m_DisplayYZMinorGrid))
						{
							Utility::Operations::ToggleBoolean(m_DisplayYZMinorGrid);
						}
					}

					ImGui::EndMenu();
				}
				ImGui::EndPopup();
			}

			// Grid Spacing
			ImGui::SetNextItemWidth(30.0f);
			ImGui::SetCursorPos(ImVec2(initialCursorPos.x + windowSize.x - 227, initialCursorPos.y + 6));
			ImGui::DragFloat("##GridSpacing", &m_FineGridSpacing, 1.0f,
				1.0f, 50.0f,
				"%.0f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_CenterText);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetNextFrameWantCaptureMouse(false);
				ImGui::BeginTooltip();
				ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, "Local Grid Spacing");
				ImGui::EndTooltip();
			}
		}

		// Toggle Top Bar Button
		icon = m_ToolbarEnabled ? EditorUI::EditorUIService::s_IconCheckbox_Enabled :
			EditorUI::EditorUIService::s_IconCheckbox_Disabled;
		ImGui::SetCursorPos(ImVec2(initialCursorPos.x + windowSize.x - 25, initialCursorPos.y + 4));
		if (ImGui::ImageButton("Toggle Top Bar",
			(ImTextureID)(uint64_t)icon->GetRendererID(),
			ImVec2(14, 14), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
			EditorUI::EditorUIService::s_PureEmpty,
			m_ToolbarEnabled ? EditorUI::EditorUIService::s_HighlightColor1 : EditorUI::EditorUIService::s_DisabledColor))
		{
			Utility::Operations::ToggleBoolean(m_ToolbarEnabled);
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetNextFrameWantCaptureMouse(false);
			ImGui::BeginTooltip();
			ImGui::TextColored(EditorUI::EditorUIService::s_HighlightColor1, m_ToolbarEnabled ? "Close Toolbar" : "Open Toolbar");
			ImGui::EndTooltip();
		}

		ImGui::PopStyleColor();
		
	}

	static Math::vec4 s_DefaultFrustumVertexPositions[8]
	{
		{ -1.0f, -1.0f, 1.0f, 1.0f },			// 0
			{ 1.0f, -1.0f, 1.0f, 1.0f },		// 1
			{ 1.0f, 1.0f, 1.0f, 1.0f },		// 2
			{ -1.0f, 1.0f, 1.0f, 1.0f },		// 3
		{ -1.0f, -1.0f, -1.0f, 1.0f },		// 4
			{ 1.0f, -1.0f, -1.0f, 1.0f },		// 5
			{ 1.0f, 1.0f, -1.0f, 1.0f },		// 6
		{ -1.0f, 1.0f, -1.0f, 1.0f },			// 7
	};

	void EmitterConfigViewportPanel::DrawGridLines()
	{
		Rendering::RenderingService::BeginScene(m_EditorCamera);

		Math::vec3 cameraPosition = m_EditorCamera.GetPosition();
		Math::vec3 fineGridStart =
		{
			/*Utility::Operations::RoundDown((int32_t)(cameraPosition.x - (m_LargeGridSpacing / 2)), (int32_t)m_FineGridSpacing),
			Utility::Operations::RoundDown((int32_t)(cameraPosition.y - (m_LargeGridSpacing / 2)), (int32_t)m_FineGridSpacing),
			Utility::Operations::RoundDown((int32_t)(cameraPosition.z - (m_LargeGridSpacing / 2)), (int32_t)m_FineGridSpacing),*/
			Utility::Operations::RoundDown((int32_t)cameraPosition.x, (int32_t)m_LargeGridSpacing),
			Utility::Operations::RoundDown((int32_t)cameraPosition.y, (int32_t)m_LargeGridSpacing),
			Utility::Operations::RoundDown((int32_t)cameraPosition.z, (int32_t)m_LargeGridSpacing)
		};

		// Set cameraFrustrumVertices 0 - 7 with vertices from camera frustum
		Math::vec3 currentVertex;
		Math::vec3 minimumValues{ std::numeric_limits<float>().max() };
		Math::vec3 maximumValues{ -std::numeric_limits<float>().max() };
		for (size_t i = 0; i < 8; i++)
		{
			Math::vec4 localSpaceCoordinates = glm::inverse(m_EditorCamera.GetProjection()) * s_DefaultFrustumVertexPositions[i];
			localSpaceCoordinates = localSpaceCoordinates / localSpaceCoordinates.w; // Perspective Division
			currentVertex = glm::inverse(m_EditorCamera.GetViewMatrix()) * localSpaceCoordinates;
			if (currentVertex.x > maximumValues.x)
			{
				maximumValues.x = currentVertex.x;
			}
			if (currentVertex.y > maximumValues.y)
			{
				maximumValues.y = currentVertex.y;
			}
			if (currentVertex.z > maximumValues.z)
			{
				maximumValues.z = currentVertex.z;
			}
			if (currentVertex.x < minimumValues.x)
			{
				minimumValues.x = currentVertex.x;
			}
			if (currentVertex.y < minimumValues.y)
			{
				minimumValues.y = currentVertex.y;
			}
			if (currentVertex.z < minimumValues.z)
			{
				minimumValues.z = currentVertex.z;
			}
		}

		// Start Grids
		int32_t currentLine;
		s_OutputVector->clear();
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_GridMajor),
			Utility::FileSystem::CRCFromString("a_Color"), 
			s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
		// X-Y Grid
		if (m_DisplayXYMajorGrid)
		{
			// Create Y Lines
			currentLine = Utility::Operations::RoundDown((int32_t)minimumValues.x, (int32_t)m_LargeGridSpacing);
			while ((float)currentLine < maximumValues.x)
			{
				if (currentLine == 0)
				{
					currentLine += (int32_t)m_LargeGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->emplace_back(currentLine, minimumValues.y, 0.0f);
				s_OutputVector->emplace_back(currentLine, maximumValues.y, 0.0f);
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_LargeGridSpacing;
			}

			// Create X Lines
			currentLine = Utility::Operations::RoundDown((int32_t)minimumValues.y, (int32_t)m_LargeGridSpacing);
			while ((float)currentLine < maximumValues.y)
			{
				if (currentLine == 0)
				{
					currentLine += (int32_t)m_LargeGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->emplace_back(minimumValues.x, currentLine, 0.0f);
				s_OutputVector->emplace_back(maximumValues.x, currentLine, 0.0f);
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_LargeGridSpacing;
			}
		}

		// Y-Z Grid
		if (m_DisplayYZMajorGrid)
		{
			// Create Y Lines
			currentLine = Utility::Operations::RoundDown((int32_t)minimumValues.y, (int32_t)m_LargeGridSpacing);
			while ((float)currentLine < maximumValues.y)
			{
				if (currentLine == 0)
				{
					currentLine += (int32_t)m_LargeGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->emplace_back( 0.0f, currentLine, minimumValues.z );
				s_OutputVector->emplace_back( 0.0f, currentLine, maximumValues.z );
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_LargeGridSpacing;
			}

			// Create Z Lines
			currentLine = Utility::Operations::RoundDown((int32_t)minimumValues.z, (int32_t)m_LargeGridSpacing);
			while ((float)currentLine < maximumValues.z)
			{
				if (currentLine == 0)
				{
					currentLine += (int32_t)m_LargeGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->emplace_back( 0.0f, minimumValues.y, currentLine );
				s_OutputVector->emplace_back( 0.0f, maximumValues.y, currentLine );
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_LargeGridSpacing;
			}
		}

		// X-Z Grid
		if (m_DisplayXZMajorGrid)
		{
			// Create Large X Lines
			currentLine = Utility::Operations::RoundDown((int32_t)minimumValues.x, (int32_t)m_LargeGridSpacing);
			while ((float)currentLine < maximumValues.x)
			{
				if (currentLine == 0)
				{
					currentLine += (int32_t)m_LargeGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->emplace_back( currentLine, 0.0f, minimumValues.z );
				s_OutputVector->emplace_back( currentLine, 0.0f, maximumValues.z );
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_LargeGridSpacing;
			}

			// Create Large Z Lines
			currentLine = Utility::Operations::RoundDown((int32_t)minimumValues.z, (int32_t)m_LargeGridSpacing);
			while ((float)currentLine < maximumValues.z)
			{
				if (currentLine == 0)
				{
					currentLine += (int32_t)m_LargeGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->emplace_back( minimumValues.x, 0.0f, currentLine );
				s_OutputVector->emplace_back( maximumValues.x, 0.0f, currentLine );
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_LargeGridSpacing;
			}
		}

		// Set Color for minor grid lines
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_GridMinor),
			Utility::FileSystem::CRCFromString("a_Color"), 
			s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);

		if (m_DisplayXYMinorGrid)
		{
			// Create X Minor Grid lines
			currentLine = (int32_t)(fineGridStart.x - m_LargeGridSpacing);
			while ((float)currentLine < (fineGridStart.x + 2 * m_LargeGridSpacing))
			{
				if (currentLine % (int32_t)m_LargeGridSpacing == 0)
				{
					currentLine += (int32_t)m_FineGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->emplace_back( currentLine,   fineGridStart.y - m_LargeGridSpacing, 0.0f );
				s_OutputVector->emplace_back( currentLine,fineGridStart.y + 2 * m_LargeGridSpacing , 0.0f );
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_FineGridSpacing;
			}

			// Create Y Minor Grid lines
			currentLine = (int32_t)(fineGridStart.y - m_LargeGridSpacing);
			while ((float)currentLine < (fineGridStart.y + 2 * m_LargeGridSpacing))
			{
				if (currentLine % (int32_t)m_LargeGridSpacing == 0)
				{
					currentLine += (int32_t)m_FineGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->emplace_back(fineGridStart.x - m_LargeGridSpacing, currentLine, 0.0f);
				s_OutputVector->emplace_back( fineGridStart.x + 2 * m_LargeGridSpacing , currentLine, 0.0f );
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_FineGridSpacing;
			}
		}

		if (m_DisplayXZMinorGrid)
		{
			// Create X Minor Grid lines
			currentLine = (int32_t)(fineGridStart.x - m_LargeGridSpacing);
			while ((float)currentLine < (fineGridStart.x + 2 * m_LargeGridSpacing))
			{
				if (currentLine % (int32_t)m_LargeGridSpacing == 0)
				{
					currentLine += (int32_t)m_FineGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->emplace_back( currentLine, 0.0f, fineGridStart.z - m_LargeGridSpacing );
				s_OutputVector->emplace_back( currentLine, 0.0f, fineGridStart.z + 2 * m_LargeGridSpacing );
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_FineGridSpacing;
			}

			// Create Z Minor Grid lines
			currentLine = (int32_t)(fineGridStart.z - m_LargeGridSpacing);
			while ((float)currentLine < (fineGridStart.z + 2 * m_LargeGridSpacing))
			{
				if (currentLine % (int32_t)m_LargeGridSpacing == 0)
				{
					currentLine += (int32_t)m_FineGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->emplace_back( fineGridStart.x - m_LargeGridSpacing, 0.0f, currentLine );
				s_OutputVector->emplace_back( fineGridStart.x + 2 * m_LargeGridSpacing, 0.0f, currentLine );
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_FineGridSpacing;
			}
		}

		if (m_DisplayYZMinorGrid)
		{
			// Create Y Minor Grid lines
			currentLine = (int32_t)(fineGridStart.y - m_LargeGridSpacing);
			while ((float)currentLine < (fineGridStart.y + 2 * m_LargeGridSpacing))
			{
				if (currentLine % (int32_t)m_LargeGridSpacing == 0)
				{
					currentLine += (int32_t)m_FineGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->emplace_back( 0.0f, currentLine,  fineGridStart.z - m_LargeGridSpacing );
				s_OutputVector->emplace_back( 0.0f , currentLine,  fineGridStart.z + (2 * m_LargeGridSpacing) );
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_FineGridSpacing;
			}

			// Create Z Minor Grid lines
			currentLine = (int32_t)(fineGridStart.z - m_LargeGridSpacing);
			while ((float)currentLine < (fineGridStart.z + 2 * m_LargeGridSpacing))
			{
				if (currentLine % (int32_t)m_LargeGridSpacing == 0)
				{
					currentLine += (int32_t)m_FineGridSpacing;
					continue;
				}
				s_OutputVector->clear();
				s_OutputVector->emplace_back( 0.0f, fineGridStart.y - m_LargeGridSpacing, currentLine );
				s_OutputVector->emplace_back( 0.0f , fineGridStart.y + 2 * m_LargeGridSpacing, currentLine );
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
				currentLine += (int32_t)m_FineGridSpacing;
			}
		}

		if (m_DisplayXZMajorGrid || m_DisplayXYMajorGrid || m_DisplayYZMajorGrid)
		{
			// X Axis
			s_OutputVector->clear();
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_HighlightColor1),
				Utility::FileSystem::CRCFromString("a_Color"), 
				s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
			s_OutputVector->emplace_back( minimumValues.x, 0.0f, 0.0f );
			s_OutputVector->emplace_back( maximumValues.x, 0.0f, 0.0f );
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);


			// Y Axis
			s_OutputVector->clear();
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_HighlightColor2),
				Utility::FileSystem::CRCFromString("a_Color"), 
				s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
			s_OutputVector->emplace_back( 0.0f, minimumValues.y, 0.0f );
			s_OutputVector->emplace_back( 0.0f, maximumValues.y, 0.0f );
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);


			// Z Axis
			s_OutputVector->clear();
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_HighlightColor3),
				Utility::FileSystem::CRCFromString("a_Color"), 
				s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);
			s_OutputVector->emplace_back( 0.0f, 0.0f, minimumValues.z );
			s_OutputVector->emplace_back( 0.0f, 0.0f, maximumValues.z );
			s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
		}

		Rendering::RenderingService::EndScene();
	}
	void EmitterConfigViewportPanel::DrawGizmo()
	{
		// TODO: Draw perspective gizmo
	}

	static Math::uvec2 s_CubeIndices[12]
	{
		{0,1},
		{1,2},
		{2,3},
		{3,0},
		{4,5},
		{5,6},
		{6,7},
		{7,4},
		{0,4},
		{1,5},
		{2,6},
		{3,7}
	};

	void EmitterConfigViewportPanel::DrawParticleSpawningBounds()
	{
		// Ensure an emitter config is available
		if (!s_EmitterConfigWindow->m_EditorEmitterConfig)
		{
			return;
		}

		// Get the current bounds
		std::array<Math::vec3, 2>& currentBounds = s_EmitterConfigWindow->m_EditorEmitterConfig->m_SpawningBounds;

		// Do not draw boxes if the bounds are the same
		if (currentBounds[0] == currentBounds[1])
		{
			return;
		}

		Rendering::RenderingService::BeginScene(m_EditorCamera);

		{ // Draw the indicated bound lines for spawning particles
			
			static Math::vec4 selectionColor{ 1.0f, 0.5f, 0.0f, 1.0f };
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(selectionColor, 
				Utility::FileSystem::CRCFromString("a_Color"), 
				s_LineInputSpec.m_Buffer, s_LineInputSpec.m_Shader);

			Math::vec3 lineVertices[8]
			{
				{ currentBounds[0].x, currentBounds[0].y, currentBounds[1].z }, // 0
				{ currentBounds[1].x, currentBounds[0].y, currentBounds[1].z }, // 1
				{ currentBounds[1].x, currentBounds[1].y, currentBounds[1].z }, // 2
				{ currentBounds[0].x, currentBounds[1].y, currentBounds[1].z }, // 3
				{ currentBounds[0].x, currentBounds[0].y, currentBounds[0].z }, // 4
				{ currentBounds[1].x, currentBounds[0].y, currentBounds[0].z }, // 5
				{ currentBounds[1].x, currentBounds[1].y, currentBounds[0].z }, // 6
				{ currentBounds[0].x, currentBounds[1].y, currentBounds[0].z }	// 7
			};

			// Create and submit lines to renderer
			for (Math::uvec2& indices : s_CubeIndices)
			{
				s_OutputVector->clear();
				s_OutputVector->emplace_back(lineVertices[indices.x]);
				s_OutputVector->emplace_back(lineVertices[indices.y]);
				s_LineInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
				Rendering::RenderingService::SubmitDataToRenderer(s_LineInputSpec);
			}
		}

		{ // Draw the bounds as points
			s_OutputVector->clear();
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>(Utility::ImVec4ToMathVec4(EditorUI::EditorUIService::s_Red),
				Utility::FileSystem::CRCFromString("a_Color"), 
				s_PointInputSpec.m_Buffer, s_PointInputSpec.m_Shader);
			s_OutputVector->emplace_back(currentBounds[0]);
			s_OutputVector->emplace_back(currentBounds[1]);
			s_PointInputSpec.m_ShapeComponent->Vertices = s_OutputVector;
			Rendering::RenderingService::SubmitDataToRenderer(s_PointInputSpec);
		}
		
		Rendering::RenderingService::EndScene();
		
	}
	void EmitterConfigViewportPanel::OnOpenEmitterConfig()
	{
		ResetCamera();
	}

	void EmitterConfigViewportPanel::ResetCamera()
	{
		m_EditorCamera.SetFocalPoint({ 0.0f, 0.0f, 0.0f });
		m_EditorCamera.SetDistance(20.0f);
		m_EditorCamera.SetPitch(0.195f);
		m_EditorCamera.SetYaw(-0.372f);
		m_EditorCamera.SetMovementType(Rendering::EditorPerspectiveCamera::MovementType::ModelView);
	}

}


