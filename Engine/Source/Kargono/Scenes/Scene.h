#pragma once

#include "Kargono/Core/Timestep.h"
#include "Kargono/Core/UUID.h"
#include "Modules/Rendering/EditorPerspectiveCamera.h"
#include "Modules/Physics2D/Physics2D.h"
#include "Kargono/Math/Math.h"
#include "Modules/Assets/Asset.h"
#include "Modules/ECS/Registry.h"
#include "Kargono/Memory/HeapAlloc.h"
#include "Modules/Rendering/RenderingService.h"
#include "Modules/ECS/Entity.h"

#include <vector>
#include <unordered_map>


// Forward Declarations
class Shader;
struct Buffer;

namespace Kargono::Scenes
{
	// Forward Declarations
	struct ShaderSpecification;

	class Scene 
	{
	public:
		//====================
		// Constructor/Destructor
		//====================
		Scene();
		~Scene() = default;
	public:
		//====================
		// Lifecycle Functions
		//====================
		void OnRuntimeStart();
		void OnRuntimeStop();
		void OnRender(Rendering::Camera& camera, const Math::mat4& transform);
		void OnUpdate(Timestep ts);
	public:
		//====================
		// Events
		//====================
		void OnViewportResize(uint32_t width, uint32_t height);
	private:
		// Helpers
		void RegisterAllComponents();
	public:
		//====================
		// Create Scenes
		//====================
		Ref<Scene> CreateSceneCopy();
	public:
		//====================
		// Get Entity Components
		//====================
		// TODO: These belong in the scripting section
		Math::vec3 TransformComponentGetTranslation(UUID entityID);
		void TransformComponentSetTranslation(UUID entityID, Math::vec3 newTranslation);
		std::string_view TagComponentGetTag(UUID entityID);
		void Rigidbody2DComponent_SetLinearVelocity(UUID entityID, Math::vec2 linearVelocity);
		Math::vec2 Rigidbody2DComponent_GetLinearVelocity(UUID entityID);
		void SetCustomComponentField(UUID entityID, Assets::AssetHandle projectComponentID, uint64_t fieldLocation, void* value);
		void* GetCustomComponentField(UUID entityID, Assets::AssetHandle projectComponentID, uint64_t fieldLocation);

	public:
		//====================
		// Modify Interaction State
		//====================
		void SetHoveredEntity(ECS::Entity entity)
		{
			m_HoveredEntity = entity;
		}
		void SetSelectedEntity(ECS::Entity entity)
		{
			m_SelectedEntity = entity;
		}
		void SetPrimaryCameraEntity(ECS::Entity entity)
		{
			m_PrimaryCameraEntity = entity;
		}
		void ClearSelectedEntity()
		{
			m_SelectedEntity = {};
		}
		void ClearHoveredEntity()
		{
			m_HoveredEntity = {};
		}
		void ClearPrimaryCameraEntity()
		{
			m_PrimaryCameraEntity = {};
		}
	public:
		//====================
		// Getters/Setters
		//====================
		Physics::PhysicsSpecification& GetPhysicsSpecification() 
		{ 
			return m_PhysicsSpecification; 
		}
		ECS::Entity& GetHoveredEntity()
		{
			return m_HoveredEntity;
		}
		ECS::Entity& GetSelectedEntity()
		{
			return m_SelectedEntity;
		}
		ECS::Entity& GetPrimaryCameraEntity() 
		{ 
			return m_PrimaryCameraEntity; 
		}
		bool IsRunning() const 
		{ 
			return m_IsRunning; 
		}
	public:
		// Registry of entities
		ECS::Registry m_EntityRegistry;
		Memory::HeapAllocator m_SceneAlloc{};
		// Physics Spec
		Physics::PhysicsSpecification m_PhysicsSpecification{};
		// Scene description data
		Math::vec4 m_BackgroundColor{ 0.1f, 0.1f, 0.1f, 1.0f };
		// Scene State Fields
		bool m_IsRunning{ false };
		ECS::Entity m_HoveredEntity{};
		ECS::Entity m_SelectedEntity{};
		ECS::Entity m_PrimaryCameraEntity{};
	private:
		// Friend Declarations
		friend class ECS::Entity;
		friend class SceneContext;
	};

	class SceneContext
	{
	public:
		//====================
		// Constructors/Destructors
		//====================
		SceneContext() = default;
		~SceneContext() = default;
	public:
		//====================
		// LifeCycle Functions
		//====================
		void Init();
		void Terminate();
	public:
		//====================
		// Query Active Scene
		//====================
		bool IsSceneActive(UUID sceneID);
	public:
		//====================
		// Set New Scene
		//====================
		void TransitionScene(Assets::AssetHandle newSceneHandle);
		void TransitionScene(Ref<Scene> newScene);
		void TransitionSceneFromHandle(Assets::AssetHandle sceneID);
	public:
		//====================
		// Getters/Setters
		//====================
		Ref<Scene> GetActiveScene();
		Assets::AssetHandle GetActiveSceneHandle();
		void SetActiveScene(Ref<Scene> newScene, Assets::AssetHandle newHandle);
	public:
		//====================
		// Public Fields
		//====================
		// Rendering
		Rendering::RendererInputSpec m_RenderSceneSpec{};
	private:
		//====================
		// Internal Fields
		//====================
		Ref<Scene> m_ActiveScene{ nullptr };
		Assets::AssetHandle m_ActiveSceneHandle{ Assets::k_EmptyHandle };
	};

	class SceneService // TODO: EWWWWW UGHHHHHHH
	{
	public:
		//==============================
		// Getters/Setters
		//==============================
		static SceneContext& GetActiveContext() { return s_SceneContext; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline SceneContext s_SceneContext{};
	};
}
