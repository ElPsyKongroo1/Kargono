#include "kgpch.h"

#include "Kargono/Scene/Scene.h"
#include "Kargono/Scene/Components.h"
#include "Kargono/Scene/Entity.h"
#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Script/ScriptEngine.h"
#include "Kargono/Core/EngineCore.h"
#include "Kargono/Renderer/Shader.h"



namespace Kargono
{
	Ref<Scene> Scene::s_ActiveScene {nullptr};

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
			{
				auto view = src.view<Component>();
				for (auto srcEntity : view)
				{
					entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

					auto& srcComponent = src.get<Component>(srcEntity);
					dst.emplace_or_replace<Component>(dstEntity, srcComponent);
				}
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
			{
				if (src.HasComponent<Component>())
					dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_PhysicsSpecification = other->m_PhysicsSpecification;

		auto& srcSceneRegistry = other->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		// Create entities in new scene
		auto idView = srcSceneRegistry.view<IDComponent>();
		for (auto e : idView)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		// Copy components (except IDComponent and TagComponent)
		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		return newScene;
		
	}

	Scene::Scene()
	{
		m_HoveredEntity = new Entity();
		m_SelectedEntity = new Entity();

		//m_ViewportWidth = Application::GetCurrentApp().GetWindow().GetWidth();
		//m_ViewportHeight = Application::GetCurrentApp().GetWindow().GetHeight();

		
	}
	Scene::~Scene()
	{
		delete m_HoveredEntity;
		delete m_SelectedEntity;
	}


	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		m_EntityMap[uuid] = entity;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_EntityMap.erase(entity.GetUUID());
		if (m_Registry.valid(entity))
		{
			m_Registry.destroy(entity);
		}
	}

	void Scene::DestroyAllEntities()
	{
		if (m_EntityMap.empty()) { return; }
		for (auto& [uuid, entity] : m_EntityMap)
		{
			if (m_Registry.valid(entity))
			{
				m_Registry.destroy(entity);
			}
		}
		m_EntityMap.clear();
	}
	
	void Scene::OnRuntimeStart()
	{
		m_IsRunning = true;
		// Physics
		m_PhysicsWorld = CreateScope<Physics::Physics2DWorld>(this, m_PhysicsSpecification.Gravity);

		// Scripts
		Script::ScriptEngine::OnRuntimeStart(this);

		// Instantiate all script entities
		auto view = GetAllEntitiesWith<ScriptComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			const auto& scriptComponent = entity.GetComponent<ScriptComponent>();
			if (Script::ScriptEngine::EntityClassExists(scriptComponent.ClassName))
			{
				if (!m_ScriptClassToEntityList.contains(scriptComponent.ClassName))
				{
					m_ScriptClassToEntityList.insert({ scriptComponent.ClassName, {} });
				}

				m_ScriptClassToEntityList.at(scriptComponent.ClassName).push_back(entity.GetUUID());
			}

		}
	}

	void Scene::OnRuntimeStop()
	{
		m_IsRunning = false;
		m_PhysicsWorld.reset();
		m_PhysicsWorld = nullptr;

		// Script
		Script::ScriptEngine::OnRuntimeStop();
		m_ScriptClassToEntityList.clear();
	}

	void Scene::OnSimulationStart()
	{
		// Physics
		m_PhysicsWorld = CreateScope<Physics::Physics2DWorld>(this, m_PhysicsSpecification.Gravity);
	}

	void Scene::OnSimulationStop()
	{
		// Physics
		m_PhysicsWorld.reset();
		m_PhysicsWorld = nullptr;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		// Copy name because we're going to modify component data structure
		std::string name = entity.GetName();
		Entity newEntity = CreateEntity(name);
		CopyComponentIfExists(AllComponents{}, newEntity, entity);
		return newEntity;
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			const TagComponent& tc = view.get<TagComponent>(entity);
			if (tc.Tag == name) { return Entity{ entity, this }; }
		}
		return {};
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		// TODO: Maybe should be an assert
		if (m_EntityMap.find(uuid) != m_EntityMap.end()) { return { m_EntityMap.at(uuid), this }; }
		return {};
	}

	bool Scene::CheckEntityExists(entt::entity entity)
	{
		return m_Registry.valid(entity);
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		// Resize non-fixed
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			
			cameraComponent.Camera.OnViewportResize();
					
		}

	}
	std::unordered_map<std::string, std::vector<UUID>>& Scene::GetScriptClassToEntityList()
	{
		return s_ActiveScene->m_ScriptClassToEntityList;
	}
	void Scene::TransitionScene(Assets::AssetHandle newSceneHandle)
	{
		Ref<Scene> newScene = Assets::AssetManager::GetScene(newSceneHandle);
		TransitionScene(newScene);
	}

	void Scene::TransitionScene(Ref<Scene> newScene)
	{
		if (!newScene) { return; }

		s_ActiveScene->OnRuntimeStop();
		s_ActiveScene->DestroyAllEntities();
		s_ActiveScene.reset();

		s_ActiveScene = newScene;

		//Audio::AudioEngine::StopAllAudio();

		*s_ActiveScene->m_HoveredEntity = {};
		*s_ActiveScene->m_SelectedEntity = {};

		s_ActiveScene->OnRuntimeStart();
	}

	void Scene::TransitionSceneFromName(const std::string& sceneName)
	{
		auto [handle, sceneReference] = Assets::AssetManager::GetScene(sceneName);
		if (sceneReference)
		{
			TransitionScene(sceneReference);
		}
	}


	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity: view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary) { return Entity{ entity, this }; }
		}
		return {};
	}
	void Scene::RenderScene(Camera& camera, const Math::mat4& transform)
	{

		Renderer::BeginScene(camera, transform);
		// Draw Shapes
		{
			auto view = m_Registry.view<TransformComponent, ShapeComponent>();
			for (auto entity : view)
			{
				const auto& [transform, shape] = view.get<TransformComponent, ShapeComponent>(entity);

				static RendererInputSpec inputSpec{};
				inputSpec.Shader = shape.Shader;
				inputSpec.Buffer = shape.ShaderData;
				inputSpec.Entity = static_cast<uint32_t>(entity);
				inputSpec.EntityRegistry = &m_Registry;
				inputSpec.ShapeComponent = &shape;
				inputSpec.TransformMatrix = transform.GetTransform();

				for (const auto& PerObjectSceneFunction : shape.Shader->GetFillDataObjectScene())
				{
					PerObjectSceneFunction(inputSpec);
				}

				Renderer::SubmitDataToRenderer(inputSpec);
			}
		}

		Renderer::EndScene();

	}
	void Scene::OnUpdatePhysics(Timestep ts)
	{
		m_PhysicsWorld->OnUpdate(ts);
	}
	void Scene::FillEntityID(RendererInputSpec& inputSpec)
	{
		Shader::SetDataAtInputLocation<uint32_t>(inputSpec.Entity, "a_EntityID", inputSpec.Buffer, inputSpec.Shader);
	}

	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		
	}
}
