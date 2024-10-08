#pragma once

#include "Kargono.h"

namespace Kargono::Panels
{
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
		void InitializeClassInstanceComponent();
		void InitializeTransformComponent();
		void InitializeRigidbody2DComponent();
		void InitializeBoxCollider2DComponent();
		void InitializeCircleCollider2DComponent();
		void InitializeCameraComponent();
		void InitializeShapeComponent();
		void CreateSceneEntityInTree(ECS::Entity entity);
	public:
		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender();
		bool OnSceneEvent(Events::Event* event);
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	public:
		//=========================
		// External API Functions
		//=========================
		void SetSelectedEntity(ECS::Entity entity);
		void RefreshClassInstanceComponent();
		void RefreshTransformComponent();
		void SetDisplayedComponent(ECS::ComponentType type)
		{
			m_DisplayedComponent = type;
		}
	private:
		//=========================
		// Internal Draw Functions
		//=========================
		void DrawAllComponents(ECS::Entity entity);
		void DrawSingleComponent(ECS::Entity entity);
		void DrawTagComponent(ECS::Entity entity);
		void DrawTransformComponent(ECS::Entity entity);
		void DrawClassInstanceComponent(ECS::Entity entity);
		void DrawRigidbody2DComponent(ECS::Entity entity);
		void DrawBoxCollider2DComponent(ECS::Entity entity);
		void DrawCircleCollider2DComponent(ECS::Entity entity);
		void DrawCameraComponent(ECS::Entity entity);
		void DrawShapeComponent(ECS::Entity entity);
		//=========================
		// Other Supporting Functions
		//=========================
		void UpdateComponent();
		void AddColorSection();
		void AddTextureSection();
		void AddCircleShapeSection();
		void AddProjectionMatrixSection();
		void AddEntityIDSection();
	private:
		//=========================
		// Core Panel Data
		//=========================
		std::string m_PanelName{"Scene Editor"};
		ECS::ComponentType m_DisplayedComponent{ECS::ComponentType::None };

	private:
		//=========================
		// Widgets
		//=========================
		// Scene Hierarchy Tree
		EditorUI::PanelHeaderSpec m_MainSceneHeader{};
		EditorUI::TreeSpec m_SceneHierarchyTree {};
		int32_t m_AddComponentEntity {};
		EditorUI::SelectOptionSpec m_AddComponent{};

		// Tag Component
		EditorUI::CollapsingHeaderSpec m_TagHeader{};
		EditorUI::EditTextSpec m_TagEdit{};

		// Transform Component
		EditorUI::CollapsingHeaderSpec m_TransformHeader{};
		EditorUI::EditVec3Spec m_TransformEditTranslation{};
		EditorUI::EditVec3Spec m_TransformEditScale{};
		EditorUI::EditVec3Spec m_TransformEditRotation{};

		// Class Instance Component
		EditorUI::CollapsingHeaderSpec m_ClassInstanceHeader{};
		EditorUI::SelectOptionSpec m_SelectClassOption{};
		EditorUI::TableSpec m_InstanceFieldsTable{};
		std::string m_CurrentClassField {};
		int32_t m_CurrentClassFieldLocation {};
		EditorUI::GenericPopupSpec m_EditClassFieldPopup{};
		EditorUI::EditVariableSpec m_EditFieldValue {};

		// Rigid Body 2D Component
		EditorUI::CollapsingHeaderSpec m_Rigidbody2DHeader{};
		EditorUI::RadioSelectorSpec m_Rigidbody2DType {};
		EditorUI::CheckboxSpec m_RigidBody2DFixedRotation {};

		// Box Collider 2D Component
		EditorUI::CollapsingHeaderSpec m_BoxCollider2DHeader{};
		EditorUI::EditVec2Spec m_BoxColliderOffset{};
		EditorUI::EditVec2Spec m_BoxColliderSize{};
		EditorUI::EditFloatSpec m_BoxColliderDensity{};
		EditorUI::EditFloatSpec m_BoxColliderFriction{};
		EditorUI::EditFloatSpec m_BoxColliderRestitution{};
		EditorUI::EditFloatSpec m_BoxColliderRestitutionThreshold{};

		// Circle Collider 2D Component
		EditorUI::CollapsingHeaderSpec m_CircleCollider2DHeader{};
		EditorUI::EditVec2Spec m_CircleColliderOffset{};
		EditorUI::EditFloatSpec m_CircleColliderRadius{};
		EditorUI::EditFloatSpec m_CircleColliderDensity{};
		EditorUI::EditFloatSpec m_CircleColliderFriction{};
		EditorUI::EditFloatSpec m_CircleColliderRestitution{};
		EditorUI::EditFloatSpec m_CircleColliderRestitutionThreshold{};

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
