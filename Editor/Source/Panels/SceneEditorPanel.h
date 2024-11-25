#pragma once

#include "Kargono.h"

#include <unordered_map>
#include <variant>

namespace Kargono::Panels
{
	struct ProjectComponentWidgetData
	{
		EditorUI::CollapsingHeaderSpec m_Header;
		std::vector<std::variant<EditorUI::EditFloatSpec, EditorUI::EditVec4Spec,
			EditorUI::EditVec2Spec, EditorUI::EditVec3Spec, EditorUI::EditTextSpec,
			EditorUI::CheckboxSpec, EditorUI::EditIntegerSpec>> m_Fields;
	};

	struct SceneEditorTreeEntryData
	{
		ECS::ComponentType m_ComponentType{ ECS::ComponentType::None };
		Assets::AssetHandle m_ProjectComponentHandle{ Assets::EmptyHandle };
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
		void InitializeTagComponent();
		void InitializeTransformComponent();
		void InitializeRigidbody2DComponent();
		void InitializeBoxCollider2DComponent();
		void InitializeCircleCollider2DComponent();
		void InitializeCameraComponent();
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
		void DrawOnUpdateComponent(ECS::Entity entity);
		void DrawAIStateComponent(ECS::Entity entity);
		void DrawOnCreateComponent(ECS::Entity entity);
		void DrawShapeComponent(ECS::Entity entity);
		void DrawProjectComponent(ECS::Entity entity, Assets::AssetHandle handle);

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
		void CreateSceneEntityInTree(ECS::Entity entity);
	private:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{"Scene Editor"};
		ECS::ComponentType m_DisplayedComponent{ECS::ComponentType::None };
		Assets::AssetHandle m_DisplayedProjectComponentHandle {Assets::EmptyHandle};
		std::unordered_map<Assets::AssetHandle, ProjectComponentWidgetData> m_AllProjectComponents{};

	private:
		//=========================
		// Widgets
		//=========================
		// Scene Hierarchy Tree
		EditorUI::PanelHeaderSpec m_MainSceneHeader{};
		EditorUI::TreeSpec m_SceneHierarchyTree {};
		int32_t m_AddComponentEntity {};
		EditorUI::SelectOptionSpec m_AddComponent{};
		EditorUI::TooltipSpec m_SelectScriptTooltip{};

		// Tag Component
		EditorUI::CollapsingHeaderSpec m_TagHeader{};
		EditorUI::EditTextSpec m_TagEdit{};
		EditorUI::EditTextSpec m_TagGroupEdit{};

		// Transform Component
		EditorUI::CollapsingHeaderSpec m_TransformHeader{};
		EditorUI::EditVec3Spec m_TransformEditTranslation{};
		EditorUI::EditVec3Spec m_TransformEditScale{};
		EditorUI::EditVec3Spec m_TransformEditRotation{};

		// Class Instance Component
		EditorUI::CollapsingHeaderSpec m_ClassInstanceHeader{};
		EditorUI::SelectOptionSpec m_SelectClassOption{};
		EditorUI::ListSpec m_InstanceFieldsTable{};
		std::string m_CurrentClassField {};
		int32_t m_CurrentClassFieldLocation {};
		EditorUI::GenericPopupSpec m_EditClassFieldPopup{};
		EditorUI::EditVariableSpec m_EditFieldValue {};

		// Rigid Body 2D Component
		EditorUI::CollapsingHeaderSpec m_Rigidbody2DHeader{};
		EditorUI::RadioSelectorSpec m_Rigidbody2DType {};
		EditorUI::CheckboxSpec m_RigidBody2DFixedRotation {};
		EditorUI::SelectOptionSpec m_SelectRigidBody2DCollisionStartScript{};
		EditorUI::SelectOptionSpec m_SelectRigidBody2DCollisionEndScript{};
		
		// Box Collider 2D Component
		EditorUI::CollapsingHeaderSpec m_BoxCollider2DHeader{};
		EditorUI::EditVec2Spec m_BoxColliderOffset{};
		EditorUI::EditVec2Spec m_BoxColliderSize{};
		EditorUI::EditFloatSpec m_BoxColliderDensity{};
		EditorUI::EditFloatSpec m_BoxColliderFriction{};
		EditorUI::EditFloatSpec m_BoxColliderRestitution{};
		EditorUI::EditFloatSpec m_BoxColliderRestitutionThreshold{};
		EditorUI::CheckboxSpec m_BoxColliderIsSensor{};

		// Circle Collider 2D Component
		EditorUI::CollapsingHeaderSpec m_CircleCollider2DHeader{};
		EditorUI::EditVec2Spec m_CircleColliderOffset{};
		EditorUI::EditFloatSpec m_CircleColliderRadius{};
		EditorUI::EditFloatSpec m_CircleColliderDensity{};
		EditorUI::EditFloatSpec m_CircleColliderFriction{};
		EditorUI::EditFloatSpec m_CircleColliderRestitution{};
		EditorUI::EditFloatSpec m_CircleColliderRestitutionThreshold{};
		EditorUI::CheckboxSpec m_CircleColliderIsSensor{};

		// OnUpdate Component
		EditorUI::CollapsingHeaderSpec m_OnUpdateHeader{};
		EditorUI::SelectOptionSpec m_SelectOnUpdateScript{};

		// AI State Component
		EditorUI::CollapsingHeaderSpec m_AIStateHeader{};
		EditorUI::SelectOptionSpec m_SelectCurrentState{};
		EditorUI::SelectOptionSpec m_SelectPreviousState{};
		EditorUI::SelectOptionSpec m_SelectGlobalState{};

		// OnCreate Component
		EditorUI::CollapsingHeaderSpec m_OnCreateHeader{};
		EditorUI::SelectOptionSpec m_SelectOnCreateScript{};

		// Camera Component
		EditorUI::CollapsingHeaderSpec m_CameraHeader{};
		EditorUI::CheckboxSpec m_CameraPrimary {};
		EditorUI::RadioSelectorSpec m_CameraProjection {};
		EditorUI::EditFloatSpec m_CameraPerspectiveFOV{};
		EditorUI::EditFloatSpec m_CameraPerspectiveNearPlane{};
		EditorUI::EditFloatSpec m_CameraPerspectiveFarPlane{};
		EditorUI::EditFloatSpec m_CameraOrthographicSize{};
		EditorUI::EditFloatSpec m_CameraOrthographicNearPlane{};
		EditorUI::EditFloatSpec m_CameraOrthographicFarPlane{};

		// Shape Component
		EditorUI::CollapsingHeaderSpec m_ShapeHeader{};
		EditorUI::SelectOptionSpec m_ShapeSelect {};
		EditorUI::SelectOptionSpec m_ShapeColorType {};
		EditorUI::EditVec4Spec m_ShapeColor {};
		EditorUI::CheckboxSpec m_ShapeAddTexture {};
		EditorUI::SelectOptionSpec m_ShapeSetTexture {};
		EditorUI::EditFloatSpec m_ShapeTilingFactor{};
		EditorUI::CheckboxSpec m_ShapeAddCircle {};
		EditorUI::EditFloatSpec m_ShapeCircleThickness{};
		EditorUI::EditFloatSpec m_ShapeCircleFade{};
		EditorUI::CheckboxSpec m_ShapeAddProjection {};
		EditorUI::CheckboxSpec m_ShapeAddEntityID {};
	private:
		friend class PropertiesPanel;
	};
}
