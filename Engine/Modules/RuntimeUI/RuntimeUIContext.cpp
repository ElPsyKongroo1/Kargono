#include "kgpch.h"

#include "Modules/RuntimeUI/RuntimeUIContext.h"

#include "Modules/Assets/AssetService.h"
#include "Modules/Core/Engine.h"
#include "Kargono/Core/Window.h"
#include "Kargono/Projects/Project.h"

#include "Modules/Rendering/Shader.h"
#include "Modules/ECS/EngineComponents.h"
#include "Kargono/Utility/Operations.h"
#include "Kargono/Math/Interpolation.h"
#include "Modules/EditorUI/EditorUIInclude.h"
#include "Modules/Input/InputService.h"

namespace Kargono::RuntimeUI
{
	void RuntimeUIContext::Init()
	{
		// Initialize Runtime UI Context
		m_ActiveUI = nullptr;
		m_ActiveUIHandle = Assets::EmptyHandle;
		m_DefaultFont = FontService::GetActiveContext().InstantiateEditorFont("Resources/Fonts/arial.ttf");

		// Initialize Window/Widget background Rendering Data
		{
			// Create shader for UI background/quad rendering
			Rendering::ShaderSpecification shaderSpec {Rendering::ColorInputType::FlatColor, Rendering::TextureInputType::None, false, true, true, Rendering::RenderingType::DrawIndex, false};
			auto [uuid, localShader] = Assets::AssetService::GetShader(shaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 1.0f, 1.0f, 1.0f, 1.0f }, 
				Utility::FileSystem::CRCFromString("a_Color"),
				localBuffer, localShader);

			// Create basic shape component for UI quad rendering
			ECS::ShapeComponent* shapeComp = new ECS::ShapeComponent();
			shapeComp->CurrentShape = Rendering::ShapeTypes::Quad;
			shapeComp->Vertices = CreateRef<std::vector<Math::vec3>>(Rendering::Shape::s_Quad.GetIndexVertices());
			shapeComp->Indices = CreateRef<std::vector<uint32_t>>(Rendering::Shape::s_Quad.GetIndices());

			m_BackgroundInputSpec.m_Shader = localShader;
			m_BackgroundInputSpec.m_Buffer = localBuffer;
			m_BackgroundInputSpec.m_ShapeComponent = shapeComp;
		}

		// Initialize texture rendering data
		{
			// Create shader for UI background/quad rendering
			Rendering::ShaderSpecification shaderSpec{ Rendering::ColorInputType::None, Rendering::TextureInputType::ColorTexture, false, true, true, Rendering::RenderingType::DrawIndex, false };
			auto [uuid, localShader] = Assets::AssetService::GetShader(shaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			// Create basic shape component for UI quad rendering
			ECS::ShapeComponent* shapeComp = new ECS::ShapeComponent();
			shapeComp->CurrentShape = Rendering::ShapeTypes::Quad;
			shapeComp->Vertices = CreateRef<std::vector<Math::vec3>>(Rendering::Shape::s_Quad.GetIndexVertices());
			shapeComp->Indices = CreateRef<std::vector<uint32_t>>(Rendering::Shape::s_Quad.GetIndices());
			shapeComp->TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Rendering::Shape::s_Quad.GetIndexTextureCoordinates());
			shapeComp->Shader = localShader;
			shapeComp->Texture = nullptr;
			
			float* tilingFactor = Rendering::Shader::GetInputLocation<float>(
				Utility::FileSystem::CRCFromString("a_TilingFactor"), 
				localBuffer, localShader);
			*tilingFactor = 1.0f;

			m_ImageInputSpec.m_Shader = localShader;
			m_ImageInputSpec.m_Buffer = localBuffer;
			m_ImageInputSpec.m_ShapeComponent = shapeComp;
		}

		// Verify Initialization
		m_Active = true;
		KG_VERIFY(m_Active, "Runtime UI Engine Init");
	}

	void RuntimeUIContext::Terminate()
	{
		// Clear input spec data
		m_BackgroundInputSpec.ClearData();
		m_ImageInputSpec.ClearData();

		// Verify Termination
		m_Active = false;
		KG_VERIFY(!m_Active, "Runtime UI Engine Terminate");
	}

	void RuntimeUIContext::SetActiveUI(Ref<UserInterface> userInterface, Assets::AssetHandle uiHandle)
	{
		if (!userInterface || uiHandle == Assets::EmptyHandle)
		{
			KG_WARN("Attempt to make a user interface active that is null or has an empty handle");
			return;
		}

		// Reset previous active UI
		ClearActiveUI();

		// Set new active UI
		m_ActiveUI = userInterface;
		m_ActiveUIHandle = uiHandle;

		m_ActiveUI->Init(this);
	}

	void RuntimeUIContext::SetActiveUIFromHandle(Assets::AssetHandle uiHandle)
	{
		// Get user interface from asset service system
		Ref<RuntimeUI::UserInterface> uiReference = Assets::AssetService::GetUserInterface(uiHandle);

		// Validate returned user interface
		if (!uiReference)
		{
			KG_WARN("Could not locate user interface. Provided handle did not lead to a valid user interface.");
			return;
		}

		// Set active user interface
		SetActiveUI(uiReference, uiHandle);
	}

	bool RuntimeUIContext::IsUIActiveFromHandle(Assets::AssetHandle uiHandle)
	{
		// Ensure an invalid state is not presented
		if (uiHandle == Assets::EmptyHandle || !m_ActiveUI)
		{
			return false;
		}

		return m_ActiveUIHandle == uiHandle;
	}

	void RuntimeUIContext::ClearActiveUI()
	{
		if (m_ActiveUI)
		{
			m_ActiveUI->Terminate();
		}

		m_ActiveUI = nullptr;
		m_ActiveUIHandle = Assets::EmptyHandle;
	}

	Ref<UserInterface> RuntimeUIContext::GetActiveUI()
	{
		return m_ActiveUI;
	}

	Assets::AssetHandle RuntimeUIContext::GetActiveUIHandle()
	{
		return m_ActiveUIHandle;
	}

	

	void NavigationLinksCalculator::CalculateNavigationLinks()
	{
		// Iterate through all windows 
		for (Window& currentWindow : i_WindowsState->m_Windows)
		{
			// Store the current window
			m_CurrentWindow = &currentWindow;

			// Calculate window scale and position
			m_CurrentWindowTransform.m_Translation = currentWindow.GetLowerCornerPosition(i_ViewportData);
			m_CurrentWindowTransform.m_Size = currentWindow.GetSize(i_ViewportData);

			// Iterate through all widgets in the window
			m_CurrentWidgetParentTransform = m_CurrentWindowTransform;
			for (Ref<Widget> currentWidget : currentWindow.m_Widgets)
			{
				CalculateWidgetNavigationLinks(currentWidget.get());
			}
		}
	}

	void NavigationLinksCalculator::CalculateWidgetNavigationLinks(Widget* currentWidget)
	{
		// Handle container widgets
		ContainerData* containerData = currentWidget->GetContainerData();
		if (containerData)
		{
			// Calculate the current widget's transform information
			Bounds cachedTransform;
			cachedTransform.m_Translation = currentWidget->CalculateWorldPosition(
				m_CurrentWidgetParentTransform.m_Translation, m_CurrentWidgetParentTransform.m_Size);
			cachedTransform.m_Size = currentWidget->CalculateWidgetSize(m_CurrentWidgetParentTransform.m_Size);
			m_CurrentWidgetParentTransform = cachedTransform;

			if (currentWidget->m_WidgetType == WidgetTypes::HorizontalContainerWidget)
			{
				// Handle the verical container case
				HorizontalContainerWidget* horizContainer = (HorizontalContainerWidget*)currentWidget;
				KG_ASSERT(horizContainer);
				size_t iteration{ 0 };
				for (Ref<Widget> containedWidget : containerData->m_ContainedWidgets)
				{
					m_CurrentWidgetParentTransform.m_Translation.x = cachedTransform.m_Translation.x +
						cachedTransform.m_Size.x * horizContainer->m_ColumnWidth * iteration +
						cachedTransform.m_Size.x * horizContainer->m_ColumnSpacing * iteration;
					m_CurrentWidgetParentTransform.m_Size.x = cachedTransform.m_Size.x * horizContainer->m_ColumnWidth;
					CalculateWidgetNavigationLinks(containedWidget.get());
					iteration++;
				}
			}
			else if (currentWidget->m_WidgetType == WidgetTypes::VerticalContainerWidget)
			{
				// Handle the verical container case
				VerticalContainerWidget* vertContainer = (VerticalContainerWidget*)currentWidget;
				KG_ASSERT(vertContainer);
				size_t iteration{ 0 };
				for (Ref<Widget> containedWidget : containerData->m_ContainedWidgets)
				{
					m_CurrentWidgetParentTransform.m_Translation.y = cachedTransform.m_Translation.y + cachedTransform.m_Size.y -
						cachedTransform.m_Size.y * vertContainer->m_RowHeight * (iteration + 1) - 
						cachedTransform.m_Size.y * vertContainer->m_RowSpacing * iteration;
					m_CurrentWidgetParentTransform.m_Size.y = cachedTransform.m_Size.y * vertContainer->m_RowHeight;
					CalculateWidgetNavigationLinks(containedWidget.get());
					iteration++;
				}
			}
			else
			{
				for (Ref<Widget> containedWidget : containerData->m_ContainedWidgets)
				{
					// Calculate the navigation links for each contained widget
					CalculateWidgetNavigationLinks(containedWidget.get());
				}
			}
		}

		// Ensure the widget is selectable
		if (!currentWidget->Selectable())
		{
			return;
		}

		// Get the selection specific data from the widget
		SelectionData* selectionData = currentWidget->GetSelectionData();
		if (!selectionData)
		{
			KG_WARN("Unable to retrieve selection data. May be invalid widget type!");
			return;
		}

		// Calculate the position and size of the current widget
		m_CurrentWidget = currentWidget;
		m_CurrentWidgetPosition = m_CurrentWidget->CalculateWorldPosition
		(
			m_CurrentWidgetParentTransform.m_Translation, 
			m_CurrentWidgetParentTransform.m_Size
		);
		m_CurrentWidgetSize = m_CurrentWidget->CalculateWidgetSize(m_CurrentWidgetParentTransform.m_Size);
		m_CurrentWidgetCenterPosition = 
		{ 
			m_CurrentWidgetPosition.x + (m_CurrentWidgetSize.x * 0.5f), 
			m_CurrentWidgetPosition.y + (m_CurrentWidgetSize.y * 0.5f) 
		};

		// Calculate navigation links for the current widget
		selectionData->m_NavigationLinks.m_RightWidgetID = CalculateNavigationLink(Direction::Right);
		selectionData->m_NavigationLinks.m_LeftWidgetID = CalculateNavigationLink(Direction::Left);
		selectionData->m_NavigationLinks.m_UpWidgetID = CalculateNavigationLink(Direction::Up);
		selectionData->m_NavigationLinks.m_DownWidgetID = CalculateNavigationLink(Direction::Down);
	}

	WidgetID NavigationLinksCalculator::CalculateNavigationLink(Direction direction)
	{
		// Initialize calculation variables
		m_CurrentBestChoiceID = k_InvalidWidgetID;
		m_CurrentBestDistance = std::numeric_limits<float>::max();
		m_CurrentDirection = direction;

		// Iterate through each potential widget and decide which widget makes sense to navigate to
		for (Ref<Widget> potentialChoice : m_CurrentWindow->m_Widgets)
		{
			m_PotentialWidgetParentTransform = m_CurrentWindowTransform;
			CompareCurrentAndPotentialWidget(potentialChoice.get());
		}

		// Return the index of the best widget choice if it exists
		return m_CurrentBestChoiceID;
	}

	void NavigationLinksCalculator::CompareCurrentAndPotentialWidget(Widget* potentialWidget)
	{
		// Calculate the position and size of the potential widget
		Math::vec3 potentialWidgetPosition = potentialWidget->CalculateWorldPosition
		(
			m_PotentialWidgetParentTransform.m_Translation,
			m_PotentialWidgetParentTransform.m_Size
		);
		Math::vec3 potentialWidgetSize = potentialWidget->CalculateWidgetSize(m_PotentialWidgetParentTransform.m_Size);
		Math::vec2 potentialWidgetCenterPosition = { potentialWidgetPosition.x + (potentialWidgetSize.x * 0.5f), potentialWidgetPosition.y + (potentialWidgetSize.y * 0.5f) };

		// Check if the widget has nested widgets
		ContainerData* containerData = potentialWidget->GetContainerData();
		m_PotentialWidgetParentTransform.m_Translation = potentialWidgetPosition;
		m_PotentialWidgetParentTransform.m_Size = potentialWidgetSize;

		if (containerData)
		{
			if (potentialWidget->m_WidgetType == WidgetTypes::HorizontalContainerWidget)
			{
				// Handle the verical container case
				size_t iteration{ 0 };
				for (Ref<Widget> containedWidget : containerData->m_ContainedWidgets)
				{
					HorizontalContainerWidget* horizContainer = (HorizontalContainerWidget*)potentialWidget;
					KG_ASSERT(horizContainer);
					m_PotentialWidgetParentTransform.m_Translation.x = potentialWidgetPosition.x + 
						potentialWidgetSize.x * horizContainer->m_ColumnWidth * iteration +
						potentialWidgetSize.x * horizContainer->m_ColumnSpacing * iteration;
					m_PotentialWidgetParentTransform.m_Size.x = potentialWidgetSize.x * horizContainer->m_ColumnWidth;
					CompareCurrentAndPotentialWidget(containedWidget.get());
					iteration++;
				}
			}
			else if (potentialWidget->m_WidgetType == WidgetTypes::VerticalContainerWidget)
			{
				// Handle the verical container case
				size_t iteration{ 0 };
				for (Ref<Widget> containedWidget : containerData->m_ContainedWidgets)
				{
					VerticalContainerWidget* vertContainer = (VerticalContainerWidget*)potentialWidget;
					KG_ASSERT(vertContainer);
					m_PotentialWidgetParentTransform.m_Translation.y = potentialWidgetPosition.y + potentialWidgetSize.y - potentialWidgetSize.y * vertContainer->m_RowHeight * (iteration + 1) - 
						potentialWidgetSize.y * vertContainer->m_RowSpacing * iteration;
					m_PotentialWidgetParentTransform.m_Size.y = potentialWidgetSize.y * vertContainer->m_RowHeight;
					CompareCurrentAndPotentialWidget(containedWidget.get());
					iteration++;
				}
			}
			else
			{
				// Handle the regular frame container case
				for (Ref<Widget> containedWidget : containerData->m_ContainedWidgets)
				{
					CompareCurrentAndPotentialWidget(containedWidget.get());
				}
			}
		}

		// Skip the current widget and any non-selectable widgets
		if (potentialWidget == m_CurrentWidget || !potentialWidget->Selectable())
		{
			return;
		}


		// Check if the potential widget is within the constraints of the current widget
		float extentDistance{ 0.0f };
		float singleDimensionDistance{ 0.0f };
		float currentWidgetExtent{ 0.0f };
		float potentialWidgetExtent{ 0.0f };

		switch (m_CurrentDirection)
		{
		case Direction::Right:
			// Ensure the current widget's left extent does not overlap with the potential widget's right extent
			currentWidgetExtent = m_CurrentWidgetPosition.x + m_CurrentWidgetSize.x;
			potentialWidgetExtent = potentialWidgetPosition.x;
			if (currentWidgetExtent >= potentialWidgetExtent)
			{
				return;
			}
			// Calculate the distance between the extents
			extentDistance = glm::distance(Math::vec2(currentWidgetExtent, m_CurrentWidgetCenterPosition.y), Math::vec2(potentialWidgetExtent, potentialWidgetCenterPosition.y));
			singleDimensionDistance = glm::distance(m_CurrentWidgetCenterPosition.y, potentialWidgetCenterPosition.y);
			break;
		case Direction::Left:
			// Ensure the current widget's right extent does not overlap with the potential widget's left extent
			currentWidgetExtent = m_CurrentWidgetPosition.x;
			potentialWidgetExtent = potentialWidgetPosition.x + potentialWidgetSize.x;
			if (currentWidgetExtent <= potentialWidgetExtent)
			{
				return;
			}
			// Calculate the distance between the extents
			extentDistance = glm::distance(Math::vec2(currentWidgetExtent, m_CurrentWidgetCenterPosition.y), Math::vec2(potentialWidgetExtent, potentialWidgetCenterPosition.y));
			singleDimensionDistance = glm::distance(m_CurrentWidgetCenterPosition.y, potentialWidgetCenterPosition.y);
			break;
		case Direction::Up:
			// Ensure the current widget's top extent does not overlap with the potential widget's bottom extent
			currentWidgetExtent = m_CurrentWidgetPosition.y + m_CurrentWidgetSize.y;
			potentialWidgetExtent = potentialWidgetPosition.y;
			if (currentWidgetExtent >= potentialWidgetExtent)
			{
				return;
			}
			// Calculate the distance between the extents
			extentDistance = glm::distance(Math::vec2(m_CurrentWidgetCenterPosition.x, currentWidgetExtent), Math::vec2(potentialWidgetCenterPosition.x, potentialWidgetExtent));
			singleDimensionDistance = glm::distance(m_CurrentWidgetCenterPosition.x, potentialWidgetCenterPosition.x);
			break;
		case Direction::Down:
			// Ensure the current widget's bottom extent does not overlap with the potential widget's top extent
			currentWidgetExtent = m_CurrentWidgetPosition.y;
			potentialWidgetExtent = potentialWidgetPosition.y + potentialWidgetSize.y;
			if (currentWidgetExtent <= potentialWidgetExtent)
			{
				return;
			}
			// Calculate the distance between the extents
			extentDistance = glm::distance(Math::vec2(m_CurrentWidgetCenterPosition.x, currentWidgetExtent), Math::vec2(potentialWidgetCenterPosition.x, potentialWidgetExtent));
			singleDimensionDistance = glm::distance(m_CurrentWidgetCenterPosition.x, potentialWidgetCenterPosition.x);
			break;
		default:
			KG_ERROR("Invalid direction provided when calculating navigation links for active user interface");
			break;
		}

		// Magic number found by tinkering with settings until it felt right...
		constexpr float singleDimensionAdjustment{ 0.65f };

		// Calculate final distance factor
		float finalDistanceFactor = extentDistance + singleDimensionAdjustment * singleDimensionDistance;

		// Save current best choice if it is the first choice
		if (m_CurrentBestChoiceID == k_InvalidWidgetID)
		{
			m_CurrentBestChoiceID = potentialWidget->m_ID;
			m_CurrentBestDistance = finalDistanceFactor;
			return;
		}


		// Replace current best choice with the potential choice if it is closer
		if (finalDistanceFactor < m_CurrentBestDistance)
		{
			m_CurrentBestChoiceID = potentialWidget->m_ID;
			m_CurrentBestDistance = finalDistanceFactor;
			return;
		}
	}
}