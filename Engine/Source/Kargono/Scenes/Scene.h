#pragma once

#include "Kargono/Core/Timestep.h"
#include "Kargono/Core/UUID.h"
#include "Kargono/Rendering/EditorPerspectiveCamera.h"
#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Math/Math.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/ECS/EntityRegistry.h"

#include "API/EntityComponentSystem/enttAPI.h"

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

	//============================================================
	// Scene Class
	//============================================================
	// Main class that holds reference to ECS registry.
	//		The scene is the central point for the game engine
	//		to work. Scenes can be created in the editor, linked
	//		together, and loaded in the runtime. In addition to
	//		holding the underlying ECS registry, there is also
	//		a map to every entity in the current scene using its
	//		UUID.
	// Ex: The main menu of a game is a scene. The game could
	//		transition to another scene that holds the gameplay.
	class Scene 
	{
	public:
		//====================
		// Constructor/Destructor
		//====================
		// This constructor/destructor manages the lifetime of the m_HoveredEntity variable.
		Scene();
		~Scene();

		//====================
		// LifeCycle Functions
		//====================
		// These functions run when the runtime is initiated/ended in both the editor
		//		and the runtime applications. Currently, the OnRuntimeStart function
		//		initiates the Physics System and the Scripting System. The OnRuntimeStop
		//		cleans up the Physics and Scripting systems.
		void OnRuntimeStart();
		void OnRuntimeStop();

		// Submits render data to the renderer.
		void RenderScene(Rendering::Camera& camera, const Math::mat4& transform);
		// Updates Entities
		void OnUpdateEntities(Timestep ts);

		// Other LifeCycle Functions
		// These Getter/Setter Functions query the current state of the scene.
		//		IsRunning and IsPaused allow certain functionality to be stopped based on their
		//		values. Step allows one iteration to occur
		bool IsRunning() const { return m_IsRunning; }
	public:
		void RegisterAllProjectComponents();
		void AddProjectComponentRegistry(Assets::AssetHandle projectComponentHandle);
		void ClearProjectComponentRegistry(Assets::AssetHandle projectComponentHandle);
		std::size_t GetProjectComponentCount(Assets::AssetHandle projectComponentHandle);
		//====================
		// Create/Destroy Scene Entities
		//====================
		// This function creates a new entity inside the underlying m_Registry. This function
		//		creates the new entity in the ECS, adds new Tag/Transform/ID components, and
		//		registers the entity into the m_EntityMap.
		ECS::Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		// This function calls the above function, but also pre-generates a UUID for the new entity
		ECS::Entity CreateEntity(const std::string& name = std::string());
		// This function creates a deep copy of another entity (calls CreateEntity()).
		ECS::Entity DuplicateEntity(ECS::Entity entity);
		// This function removes an entity from the underlying ECS m_Registry and m_EntityMap.
		void DestroyEntity(ECS::Entity entity);
		// This function removes all entities from the underlying ECS m_Registry and clears the m_EntityMap
		void DestroyAllEntities();

		//====================
		// Query Entity Functions
		//====================
		// These functions query the current map of entities
		ECS::Entity FindEntityByName(const std::string& name);
		ECS::Entity GetEntityByUUID(UUID uuid);
		ECS::Entity GetEntityByEnttID(entt::entity enttID);
		ECS::Entity GetPrimaryCameraEntity();
		bool CheckEntityExists(entt::entity entity);
		bool IsEntityValid(entt::entity entity) { return m_EntityRegistry.m_EnTTRegistry.valid(entity); }

		// Update Scene Viewport Size (Not too important) and resize all camera in scene
		//		if they require a fixed ratio (more important).
		void OnViewportResize(uint32_t width, uint32_t height);

		// Templated function that serves as external API to query current m_Registry for
		//		various components. This returns a view that contains all the entities
		//		that have the required components.
		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_EntityRegistry.m_EnTTRegistry.view<Components...>();
		}

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
		ECS::EntityRegistry m_EntityRegistry;
		// This map holds lists of entitys (UUID) using the key of a group.
		//		This map is only filled at runtime while the scripting engine
		//		is working. It is used to easily find all of the entities of a
		//		particular script class.
		std::unordered_map<std::string, std::vector<UUID>> m_GroupToEntityList {};

		// Physics Spec
		Physics::PhysicsSpecification m_PhysicsSpecification{};

		// Scene description data
		Math::vec4 m_BackgroundColor{ 0.1f, 0.1f, 0.1f, 1.0f };

		// Scene State Fields
		bool m_IsRunning = false;
		ECS::Entity* m_HoveredEntity = nullptr;
		ECS::Entity* m_SelectedEntity = nullptr;

	private:
		// Friend Declarations
		friend class ECS::Entity;
		friend class SceneService;
	};

	class SceneService
	{
	public:
		//====================
		// LifeCycle Functions
		//====================
		static void Init();
		static void Terminate();
	public:
		//====================
		// Get Entity Components
		//====================
		static bool CheckActiveHasComponent(UUID entityID, const std::string& componentName);

		static Math::vec3 TransformComponentGetTranslation(UUID entityID);
		static void TransformComponentSetTranslation(UUID entityID, Math::vec3 newTranslation);
		static const std::string& TagComponentGetTag(UUID entityID);
		static void Rigidbody2DComponent_SetLinearVelocity(UUID entityID, Math::vec2 linearVelocity);
		static Math::vec2 Rigidbody2DComponent_GetLinearVelocity(UUID entityID);
		static void SetProjectComponentField(UUID entityID, Assets::AssetHandle projectComponentID, uint64_t fieldLocation, void* value);
		static void* GetProjectComponentField(UUID entityID, Assets::AssetHandle projectComponentID, uint64_t fieldLocation);
		

		//====================
		// Manage Active Scene Entities
		//====================
		static Assets::AssetHandle FindEntityHandleByName(const std::string& name);

		//====================
		// Manage Active Scene
		//====================
		static bool IsSceneActive(UUID sceneID);
		static void TransitionScene(Assets::AssetHandle newSceneHandle);
		static void TransitionScene(Ref<Scene> newScene);
		static void TransitionSceneFromHandle(Assets::AssetHandle sceneID);
		static Ref<Scene> CreateSceneCopy(Ref<Scene> other);

	public:
		//====================
		// Getters/Setters
		//====================
		static Ref<Scene> GetActiveScene()
		{
			return s_ActiveScene;
		}
		static Assets::AssetHandle GetActiveSceneHandle()
		{
			return s_ActiveSceneHandle;
		}
		static void SetActiveScene(Ref<Scene> newScene, Assets::AssetHandle newHandle);
	private:
		//====================
		// Internal Fields
		//====================
		static inline Ref<Scene> s_ActiveScene { nullptr };
		static inline Assets::AssetHandle s_ActiveSceneHandle { Assets::EmptyHandle };
	};
}
