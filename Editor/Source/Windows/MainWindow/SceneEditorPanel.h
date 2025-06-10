#pragma once

#include "Modules/ECS/EngineComponents.h"
#include "Modules/ECS/Entity.h"
#include "Modules/Events/Event.h"
#include "Modules/Events/KeyEvent.h"
#include "Modules/Assets/Asset.h"
#include "Kargono/Core/FixedString.h"
#include "Modules/EditorUI/EditorUIInclude.h"

#include <unordered_map>
#include <variant>

namespace Kargono::Panels
{
	struct ProjectComponentWidgetData
	{
		EditorUI::CollapsingHeaderWidget m_Header;
		std::vector<std::variant<EditorUI::EditFloatWidget, EditorUI::EditVec4Widget,
			EditorUI::EditVec2Widget, EditorUI::EditVec3Widget, EditorUI::EditTextSpec,
			EditorUI::CheckboxWidget, EditorUI::EditIntegerWidget>> m_Fields;
	};

	struct SceneEditorTreeEntryData
	{
		ECS::ComponentType m_ComponentType{ ECS::ComponentType::None };
		Assets::AssetHandle m_ProjectComponentHandle{ Assets::EmptyHandle };
	};

	enum class ScenePropertiesDisplay : uint16_t
	{
		None = 0,
		Scene,
		Entity
	};

	class SceneEditorPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		SceneEditorPanel();
	private:
		//=========================
		// Internal Inititialization
		//=========================
		void InitializeSceneHierarchy();
		void InitializeSceneOptions();
		void InitializeTagComponent();
		void InitializeTransformComponent();
		void InitializeRigidbody2DComponent();
		void InitializeBoxCollider2DComponent();
		void InitializeCircleCollider2DComponent();
		void InitializeCameraComponent();
		void InitializeParticleEmitterComponent();
		void InitializeOnUpdateComponent();
		void InitializeOnCreateComponent();
		void InitializeAIComponent();
		void InitializeShapeComponent();
		void InitializeProjectComponents();
		void InitializeProjectComponent(Assets::AssetHandle projectComponentHandle);
	public:
		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();
		bool OnSceneEvent(Events::Event* event);
		bool OnAssetEvent(Events::Event* event);
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	public:
		//=========================
		// External API Functions
		//=========================
		void SetSelectedEntity(ECS::Entity entity);
		void RefreshTransformComponent();
		void SetDisplayedComponent(ECS::ComponentType type)
		{
			m_DisplayedComponent = type;
		}
		void SetDisplayedProjectComponent(Assets::AssetHandle handle)
		{
			m_DisplayedProjectComponentHandle = handle;
		}
		ECS::ComponentType GetDisplayedComponent()
		{
			return m_DisplayedComponent;
		}
		Assets::AssetHandle GetDisplayedProjectComponent()
		{
			return m_DisplayedProjectComponentHandle;
		}
	private:
		//=========================
		// Internal Draw Functions
		//=========================
		void DrawAllComponents(ECS::Entity entity);
		void DrawSingleComponent(ECS::Entity entity);
		void DrawTagComponent(ECS::Entity entity);
		void DrawTransformComponent(ECS::Entity entity);
		void DrawRigidbody2DComponent(ECS::Entity entity);
		void DrawBoxCollider2DComponent(ECS::Entity entity);
		void DrawCircleCollider2DComponent(ECS::Entity entity);
		void DrawCameraComponent(ECS::Entity entity);
		void DrawParticleEmitterComponent(ECS::Entity entity);
		void DrawOnUpdateComponent(ECS::Entity entity);
		void DrawAIStateComponent(ECS::Entity entity);
		void DrawOnCreateComponent(ECS::Entity entity);
		void DrawShapeComponent(ECS::Entity entity);
		void DrawProjectComponent(ECS::Entity entity, Assets::AssetHandle handle);
		void DrawSceneOptions();

		//=========================
		// Shape Component Supporting Functions
		//=========================
		void UpdateShapeComponent();
		void DrawShapeComponentColor();
		void DrawShapeComponentTexture();
		void DrawShapeComponentCircle();
		void DrawShapeComponentProjection();
		void DrawShapeComponentEntityID();

		//=========================
		// Scene Hierarchy Supporting Functions
		//=========================
		void CreateSceneEntityInTree(ECS::Entity entity, EditorUI::TreeEntry& sceneEntry);
	private:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{"Scene Editor"};
		ECS::ComponentType m_DisplayedComponent{ECS::ComponentType::None };
		Assets::AssetHandle m_DisplayedProjectComponentHandle {Assets::EmptyHandle};
		std::unordered_map<Assets::AssetHandle, ProjectComponentWidgetData> m_AllProjectComponents{};
		ScenePropertiesDisplay m_CurrentDisplayed{ ScenePropertiesDisplay::None };

	private:
		//=========================
		// Widgets
		//=========================
		
		// Scene Hierarchy Tree
		EditorUI::PanelHeaderWidget m_MainSceneHeader{};
		EditorUI::TreeWidget m_SceneHierarchyTree {};
		int32_t m_AddComponentEntity {};
		EditorUI::SelectOptionWidget m_AddComponent{};
		EditorUI::TooltipWidget m_SelectTooltip{};

		// Scene Options
		EditorUI::EditVec4Widget m_BackgroundColorSpec;
		EditorUI::EditVec2Widget m_Gravity2DSpec;

		// Tag Component
		EditorUI::CollapsingHeaderWidget m_TagHeader{};
		EditorUI::EditTextSpec m_TagEdit{};
		EditorUI::EditTextSpec m_TagGroupEdit{};

		// Transform Component
		EditorUI::CollapsingHeaderWidget m_TransformHeader{};
		EditorUI::EditVec3Widget m_TransformEditTranslation{};
		EditorUI::EditVec3Widget m_TransformEditScale{};
		EditorUI::EditVec3Widget m_TransformEditRotation{};

		// Class Instance Component
		EditorUI::CollapsingHeaderWidget m_ClassInstanceHeader{};
		EditorUI::SelectOptionWidget m_SelectClassOption{};
		EditorUI::ListWidget m_InstanceFieldsTable{};
		std::string m_CurrentClassField {};
		int32_t m_CurrentClassFieldLocation {};
		EditorUI::GenericPopupWidget m_EditClassFieldPopup{};
		EditorUI::EditVariableWidget m_EditFieldValue {};

		// Rigid Body 2D Component
		EditorUI::CollapsingHeaderWidget m_Rigidbody2DHeader{};
		EditorUI::RadioSelectWidget m_Rigidbody2DType {};
		EditorUI::CheckboxWidget m_RigidBody2DFixedRotation {};
		EditorUI::SelectOptionWidget m_SelectRigidBody2DCollisionStartScript{};
		EditorUI::SelectOptionWidget m_SelectRigidBody2DCollisionEndScript{};
		
		// Box Collider 2D Component
		EditorUI::CollapsingHeaderWidget m_BoxCollider2DHeader{};
		EditorUI::EditVec2Widget m_BoxColliderOffset{};
		EditorUI::EditVec2Widget m_BoxColliderSize{};
		EditorUI::EditFloatWidget m_BoxColliderDensity{};
		EditorUI::EditFloatWidget m_BoxColliderFriction{};
		EditorUI::EditFloatWidget m_BoxColliderRestitution{};
		EditorUI::EditFloatWidget m_BoxColliderRestitutionThreshold{};
		EditorUI::CheckboxWidget m_BoxColliderIsSensor{};

		// Circle Collider 2D Component
		EditorUI::CollapsingHeaderWidget m_CircleCollider2DHeader{};
		EditorUI::EditVec2Widget m_CircleColliderOffset{};
		EditorUI::EditFloatWidget m_CircleColliderRadius{};
		EditorUI::EditFloatWidget m_CircleColliderDensity{};
		EditorUI::EditFloatWidget m_CircleColliderFriction{};
		EditorUI::EditFloatWidget m_CircleColliderRestitution{};
		EditorUI::EditFloatWidget m_CircleColliderRestitutionThreshold{};
		EditorUI::CheckboxWidget m_CircleColliderIsSensor{};

		// OnUpdate Component
		EditorUI::CollapsingHeaderWidget m_OnUpdateHeader{};
		EditorUI::SelectOptionWidget m_SelectOnUpdateScript{};

		// AI State Component
		EditorUI::CollapsingHeaderWidget m_AIStateHeader{};
		EditorUI::SelectOptionWidget m_SelectCurrentState{};
		EditorUI::SelectOptionWidget m_SelectPreviousState{};
		EditorUI::SelectOptionWidget m_SelectGlobalState{};

		// OnCreate Component
		EditorUI::CollapsingHeaderWidget m_OnCreateHeader{};
		EditorUI::SelectOptionWidget m_SelectOnCreateScript{};

		// Camera Component
		EditorUI::CollapsingHeaderWidget m_CameraHeader{};
		EditorUI::CheckboxWidget m_CameraPrimary {};
		EditorUI::RadioSelectWidget m_CameraProjection {};
		EditorUI::EditFloatWidget m_CameraPerspectiveFOV{};
		EditorUI::EditFloatWidget m_CameraPerspectiveNearPlane{};
		EditorUI::EditFloatWidget m_CameraPerspectiveFarPlane{};
		EditorUI::EditFloatWidget m_CameraOrthographicSize{};
		EditorUI::EditFloatWidget m_CameraOrthographicNearPlane{};
		EditorUI::EditFloatWidget m_CameraOrthographicFarPlane{};

		// Particle Emitter Component
		EditorUI::CollapsingHeaderWidget m_ParticleEmitterHeader{};
		EditorUI::SelectOptionWidget m_SelectParticleEmitter{};

		// Shape Component
		EditorUI::CollapsingHeaderWidget m_ShapeHeader{};
		EditorUI::SelectOptionWidget m_ShapeSelect {};
		EditorUI::SelectOptionWidget m_ShapeColorType {};
		EditorUI::EditVec4Widget m_ShapeColor {};
		EditorUI::CheckboxWidget m_ShapeAddTexture {};
		EditorUI::SelectOptionWidget m_ShapeSetTexture {};
		EditorUI::EditFloatWidget m_ShapeTilingFactor{};
		EditorUI::CheckboxWidget m_ShapeAddCircle {};
		EditorUI::EditFloatWidget m_ShapeCircleThickness{};
		EditorUI::EditFloatWidget m_ShapeCircleFade{};
		EditorUI::CheckboxWidget m_ShapeAddProjection {};
		EditorUI::CheckboxWidget m_ShapeAddEntityID {};
	private:
		friend class PropertiesPanel;
	};
}
