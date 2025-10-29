#pragma once

#include "Kargono/Core/Timestep.h"
#include "Kargono/Core/UUID.h"
#include "Modules/Cameras/PerspectiveCamera.h"
#include "Modules/Physics2D/Physics2D.h"
#include "Kargono/Math/Math.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/Metadata.h"
#include "Modules/Assets/Module/AssetTag.h"
#include "Modules/ECS/Registry.h"
#include "Modules/Memory/HeapAlloc.h"
#include "Modules/Rendering/RenderingService.h"
#include "Modules/ECS/Entity.h"
#include "Modules/Scenes/Module/SceneModule.h"

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
		//==============================
		// Asset Config Info
		//==============================
		constexpr static FixedBufStr32 GetAssetName()
		{
			return "Scene";
		}
		constexpr static Assets::AssetFlags GetAssetFlags()
		{
			Assets::AssetFlags flags{};
			flags.ClearFlag(Assets::AssetFlag::HasAssetCache);
			flags.SetFlag(Assets::AssetFlag::RequireUniqueName);
			return flags;
		}
		constexpr static FixedBufStr16 GetFileExtension()
		{
			return ".kgscene";
		}
		static void CreateAssetFromName(Assets::Metadata& metadata);
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
		void OnRender(Cameras::CameraProjection& camera, const Math::mat4& transform);
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
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);

	public:
		//====================
		// Remove Asset References
		//====================
		bool RemoveScript(Ref<Scenes::Scene> sceneRef, Assets::AssetHandle scriptHandle);
		bool RemoveAIState(Ref<Scenes::Scene> sceneRef, Assets::AssetHandle aiStateHandle);
		bool RemoveCustomComponent(Ref<Scenes::Scene> sceneRef, Assets::AssetHandle projectCompHandle);
		bool RemoveEmitterConfig(Ref<Scenes::Scene> sceneRef, Assets::AssetHandle emitterConfigHandle);
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
		void SetHoveredEntity(ECS::Entity entity) { m_HoveredEntity = entity; }
		void SetSelectedEntity(ECS::Entity entity) { m_SelectedEntity = entity; }
		void SetPrimaryCameraEntity(ECS::Entity entity) { m_PrimaryCameraEntity = entity; }
		void ClearSelectedEntity() { m_SelectedEntity = {}; }
		void ClearHoveredEntity() { m_HoveredEntity = {}; }
		void ClearPrimaryCameraEntity() { m_PrimaryCameraEntity = {}; }
	public:
		//====================
		// Getters/Setters
		//====================
		Physics::PhysicsSpecification& GetPhysicsSpecification() { return m_PhysicsSpecification; }
		ECS::Entity& GetHoveredEntity() { return m_HoveredEntity; }
		ECS::Entity& GetSelectedEntity() { return m_SelectedEntity; }
		ECS::Entity& GetPrimaryCameraEntity() { return m_PrimaryCameraEntity; }
		bool IsRunning() const { return m_IsRunning; }
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

	Register_Module_Type(Scene, Assets::AssetTag)
}
