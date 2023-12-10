#pragma once

#include "Kargono/Core/Timestep.h"
#include "Kargono/Core/UUID.h"
#include "Kargono/Renderer/EditorCamera.h"
#include "Kargono/Physics/Physics2D.h"

#include "entt.hpp"
#include "glm/glm.hpp"

// Forward Declarations
class Shader;
struct Buffer;

namespace Kargono 
{

	// Forward Declarations
	class Entity;
	struct Shader::ShaderSpecification;
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
		Scene() = default;
		~Scene() = default;

		//====================
		// LifeCycle Functions
		//====================
		// These functions run when the runtime is initiated/ended in both the editor
		//		and the runtime applications. Currently, the OnRuntimeStart function
		//		initiates the Physics System and the Scripting System. The OnRuntimeStop
		//		cleans up the Physics and Scripting systems.
		void OnRuntimeStart();
		void OnRuntimeStop();

		// These functions are run when the simulation option is started in the editor.
		//		These functions only start and stop the physics system currently.
		void OnSimulationStart();
		void OnSimulationStop();

		// Submits render data to the renderer.
		void RenderScene(Camera& camera, const glm::mat4& transform);
		// Updates Physics
		void OnUpdatePhysics(Timestep ts);

		// Other LifeCycle Functions
		// These Getter/Setter Functions query the current state of the scene.
		//		IsRunning and IsPaused allow certain functionality to be stopped based on their
		//		values. Step allows one iteration to occur
		bool IsRunning() const { return m_IsRunning; }

	private:
		// Supporting private functions for LifeCycle Functions

		// This function template optionally runs code that involves
		//		the current scene and the newly instantiated component.
		template <typename T>
		void OnComponentAdded(Entity entity, T& component);
	public:
		//====================
		// Rendering Pipeline Functions
		//====================
		// These functions are meant to be called selectively in a for-loop to fill buffer
		//		data for a current rendering call. These functions are optionally called in
		//		the RenderScene() function.
		// Ex: To enable mouse picking, the entityID needs to be submitted to the Shader.
		//		This data is filled by the FillEntityID() function below. The buffer is
		//		then sent to the Renderer to continue the process.

		// This function fills the buffer inside inputSpec with the current entityID.
		static void FillEntityID(Shader::RendererInputSpec& inputSpec);

		//====================
		// Copy Scene
		//====================
		// This function creates a deep copy of a scene. This includes copying the viewport size,
		//		copying entities into new registry, and copying corresponding components for
		//		each entity. This function is currently used in the editor
		//		to provide a temporary scene for the runtime and simulation modes to work.
		static Ref<Scene> Copy(Ref<Scene> other);

		//====================
		// Create/Destroy Scene Entities
		//====================
		// This function creates a new entity inside the underlying m_Registry. This function
		//		creates the new entity in the ECS, adds new Tag/Transform/ID components, and
		//		registers the entity into the m_EntityMap.
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		// This function calls the above function, but also pre-generates a UUID for the new entity
		Entity CreateEntity(const std::string& name = std::string());
		// This function creates a deep copy of another entity (calls CreateEntity()).
		Entity DuplicateEntity(Entity entity);
		// This function removes an entity from the underlying ECS m_Registry and m_EntityMap.
		void DestroyEntity(Entity entity);
		// This function removes all entities from the underlying ECS m_Registry and clears the m_EntityMap
		void DestroyAllEntities();

		//====================
		// Query Entity Functions
		//====================
		// These functions query the current map of entities
		Entity FindEntityByName(std::string_view name);
		Entity GetEntityByUUID(UUID uuid);
		Entity GetPrimaryCameraEntity();
		bool CheckEntityExists(entt::entity entity);
		bool IsEntityValid(entt::entity entity) { return m_Registry.valid(entity); }

		// Update Scene Viewport Size (Not too important) and resize all camera in scene
		//		if they require a fixed ratio (more important).
		void OnViewportResize(uint32_t width, uint32_t height);

		// Templated function that serves as external API to query current m_Registry for
		//		various components. This returns a view that contains all the entities
		//		that have the required components.
		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}
		PhysicsSpecification& GetPhysicsSpecification() { return m_PhysicsSpecification; }
		Physics2DWorld* GetPhysicsWorld() { return m_PhysicsWorld.get(); }

	private:
		// Underlying ECS registry that holds actual entities and their components
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		// Entity Map that holds easy to access reference to all entities in the scene.
		std::unordered_map<UUID, entt::entity> m_EntityMap;

		// Physics World
		Scope<Physics2DWorld> m_PhysicsWorld = nullptr;
		PhysicsSpecification m_PhysicsSpecification{};

		// Scene State Fields
		bool m_IsRunning = false;

		// Friend Declarations
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
		friend class AssetManager;
	};
}
