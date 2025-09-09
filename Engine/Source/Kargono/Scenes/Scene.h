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

#include <vector>
#include <unordered_map>


// Forward Declarations
class Shader;
struct Buffer;
namespace Kargono::ECS { class Entity; }

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
		~Scene();

		void OnRuntimeStart();
		void OnRuntimeStop();

		// Submits render data to the renderer.
		void RenderScene(Rendering::Camera& camera, const Math::mat4& transform);
		// Updates Entities
		void OnUpdateEntities(Timestep ts);
		bool IsRunning() const { return m_IsRunning; }
	public:
		void RegisterAllComponents();
		void AddCustomComponentRegistry(Assets::AssetHandle projectComponentHandle);
		void ClearCustomComponentRegistry(Assets::AssetHandle projectComponentHandle);
		size_t GetCustomComponentCount(Assets::AssetHandle projectComponentHandle);
		//====================
		// Create/Destroy Scene Entities
		//====================
		ECS::Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		ECS::Entity CreateEntity(const std::string& name = std::string());
		ECS::Entity DuplicateEntity(ECS::Entity entity);
		void DestroyEntity(ECS::Entity entity);
		void DestroyAllEntities();

		//====================
		// Query Entity Functions
		//====================
		ECS::Entity FindEntityByName(const std::string& name);
		ECS::Entity GetEntityByUUID(UUID uuid);
		ECS::Entity GetEntityByEnttID(ECSInternal::EntityID enttID);
		ECS::Entity GetPrimaryCameraEntity();
		bool CheckEntityExists(ECSInternal::EntityID entity);
		bool IsEntityValid(ECSInternal::EntityID entity) 
		{ 
			return m_EntityRegistry.m_Registry.HasEntity(entity);
		}

		void OnViewportResize(uint32_t width, uint32_t height);
		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_EntityRegistry.m_Registry.GetFlatView<Components...>();
		}
	public:
		//====================
		// Get Entity Components
		//====================
		bool CheckActiveHasComponent(UUID entityID, std::string_view componentName);
		Math::vec3 TransformComponentGetTranslation(UUID entityID);
		void TransformComponentSetTranslation(UUID entityID, Math::vec3 newTranslation);
		std::string_view TagComponentGetTag(UUID entityID);
		void Rigidbody2DComponent_SetLinearVelocity(UUID entityID, Math::vec2 linearVelocity);
		Math::vec2 Rigidbody2DComponent_GetLinearVelocity(UUID entityID);
		void SetProjectComponentField(UUID entityID, Assets::AssetHandle projectComponentID, uint64_t fieldLocation, void* value);
		void* GetProjectComponentField(UUID entityID, Assets::AssetHandle projectComponentID, uint64_t fieldLocation);
	public:
		//====================
		// Manage Active Scene Entities
		//====================
		Assets::AssetHandle FindEntityHandleByName(std::string_view name);
		//====================
		// Getters/Setters
		//====================
		Physics::PhysicsSpecification& GetPhysicsSpecification() { return m_PhysicsSpecification; }
		ECS::Entity* GetHoveredEntity()
		{
			return m_HoveredEntity;
		}
		ECS::Entity* GetSelectedEntity()
		{
			return m_SelectedEntity;
		}
	public:
		ECS::Registry m_EntityRegistry;
		Memory::HeapAllocator m_SceneAlloc{};

		// Physics Spec
		Physics::PhysicsSpecification m_PhysicsSpecification{};
		// Scene description data
		Math::vec4 m_BackgroundColor{ 0.1f, 0.1f, 0.1f, 1.0f };
		// Scene State Fields
		bool m_IsRunning{ false };
		ECS::Entity* m_HoveredEntity{ nullptr };
		ECS::Entity* m_SelectedEntity{ nullptr };
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
		// Manage Active Scene
		//====================
		static bool CheckActiveHasComponent(UUID entityID, const std::string& componentName);

		static Math::vec3 TransformComponentGetTranslation(UUID entityID);
		static void TransformComponentSetTranslation(UUID entityID, Math::vec3 newTranslation);
		static const std::string& TagComponentGetTag(UUID entityID);
		static void Rigidbody2DComponent_SetLinearVelocity(UUID entityID, Math::vec2 linearVelocity);
		static Math::vec2 Rigidbody2DComponent_GetLinearVelocity(UUID entityID);
		static void SetCustomComponentField(UUID entityID, Assets::AssetHandle projectComponentID, uint64_t fieldLocation, void* value);
		static void* GetCustomComponentField(UUID entityID, Assets::AssetHandle projectComponentID, uint64_t fieldLocation);
		

		//====================
		// Query Active Scene
		//====================
		bool IsSceneActive(UUID sceneID);
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
