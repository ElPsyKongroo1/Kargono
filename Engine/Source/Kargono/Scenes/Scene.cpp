#include "kgpch.h"

#include "Kargono/Scenes/Scene.h"
#include "Kargono/Scenes/Components.h"
#include "Kargono/Scenes/Entity.h"
#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Rendering/RenderingService.h"
#include "Kargono/Script/ScriptEngine.h"
#include "Kargono/Core/EngineCore.h"
#include "Kargono/Rendering/Shader.h"



namespace Kargono::Scenes
{
	Ref<Scene> Scene::s_ActiveScene {nullptr};
	Assets::AssetHandle Scene::s_ActiveSceneHandle {Assets::EmptyHandle};
	static std::unordered_map<std::string, std::function<bool(Scenes::Entity)>> s_EntityHasComponentFunc;

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

	template<typename ... Component>
	static void RegisterHasComponent()
	{
		([]()
			{
				std::string fullName = typeid(Component).name();
				size_t pos = fullName.find_last_of(':');
				std::string componentName = fullName.substr(pos + 1);
				s_EntityHasComponentFunc[componentName] = [](Scenes::Entity entity) { return entity.HasComponent<Component>(); };
			}(), ...);
	}

	template<typename ... Component>
	static void RegisterHasComponent(Scenes::ComponentGroup<Component ...>)
	{
		RegisterHasComponent<Component ...>();
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
		// Invoke OnCreate
		auto classInstanceView = GetAllEntitiesWith<ClassInstanceComponent>();
		for (auto e : classInstanceView)
		{
			Scenes::Entity entity = { e, this };
			ClassInstanceComponent& classInstanceComp = entity.GetComponent<ClassInstanceComponent>();
			Ref<EntityClass> entityClass = Assets::AssetManager::GetEntityClass(classInstanceComp.ClassHandle);
			KG_ASSERT(entityClass);
			Assets::AssetHandle scriptHandle = classInstanceComp.ClassReference->GetScripts().OnCreateHandle;
			if (scriptHandle != Assets::EmptyHandle)
			{
				((WrappedVoidUInt64*)entityClass->GetScripts().OnCreate->m_Function.get())->m_Value(entity.GetUUID());
			}
		}

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
	bool Scene::CheckHasComponent(UUID entityID, const std::string& componentName)
	{
		if (!s_EntityHasComponentFunc.contains(componentName))
		{
			KG_ERROR("Invalid Component name provided.")
			return false;
		}
		KG_ASSERT(s_ActiveScene);
		Entity activeEntity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(activeEntity);
		return s_EntityHasComponentFunc.at(componentName)(activeEntity);
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
	void Scene::RenderScene(Rendering::Camera& camera, const Math::mat4& transform)
	{
		Rendering::RenderingService::BeginScene(camera, transform);
		// Draw Shapes
		{
			auto view = m_Registry.view<TransformComponent, ShapeComponent>();
			for (auto entity : view)
			{
				const auto& [transform, shape] = view.get<TransformComponent, ShapeComponent>(entity);

				static Rendering::RendererInputSpec inputSpec{};
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

				Rendering::RenderingService::SubmitDataToRenderer(inputSpec);
			}
		}

		Rendering::RenderingService::EndScene();

	}
	void Scene::OnUpdatePhysics(Timestep ts)
	{
		m_PhysicsWorld->OnUpdate(ts);
	}
	void Scene::OnUpdateEntities(Timestep ts)
	{
		auto view = GetAllEntitiesWith<ClassInstanceComponent>();

		// Invoke OnUpdate
		auto classInstanceView = GetAllEntitiesWith<ClassInstanceComponent>();
		for (auto e : classInstanceView)
		{
			Scenes::Entity entity = { e, this };
			ClassInstanceComponent& classInstanceComp = entity.GetComponent<ClassInstanceComponent>();
			Ref<EntityClass> entityClass = classInstanceComp.ClassReference;
			KG_ASSERT(entityClass);
			Assets::AssetHandle scriptHandle = classInstanceComp.ClassReference->GetScripts().OnUpdateHandle;
			if (scriptHandle != Assets::EmptyHandle)
			{
				((WrappedVoidUInt64Float*)entityClass->GetScripts().OnUpdate->m_Function.get())->m_Value(entity.GetUUID(), ts);
			}
		}
	}
	void Scene::FillEntityID(Rendering::RendererInputSpec& inputSpec)
	{
		Rendering::Shader::SetDataAtInputLocation<uint32_t>(inputSpec.Entity, "a_EntityID", inputSpec.Buffer, inputSpec.Shader);
	}

	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		
	}
	void SceneService::Init()
	{
		RegisterHasComponent(AllComponents{});
	}
	Math::vec3 SceneService::TransformComponent_GetTranslation(UUID entityID)
	{
		Scenes::Scene* scene = Scenes::Scene::GetActiveScene().get();
		Scenes::Entity entity = scene->GetEntityByUUID(entityID);

		return entity.GetComponent<Scenes::TransformComponent>().Translation;
	}
	void SceneService::SetEntityFieldByName(UUID entityID, const std::string& fieldName, void* fieldValue)
	{
		Scenes::Scene* scene = Scenes::Scene::GetActiveScene().get();
		Scenes::Entity entity = scene->GetEntityByUUID(entityID);
		KG_ASSERT(scene);
		KG_ASSERT(entity);
		if (!entity.HasComponent<ClassInstanceComponent>())
		{
			KG_ERROR("No valid ClassInstanceComponent associated with entity");
			return;
		}

		ClassInstanceComponent& comp = entity.GetComponent<ClassInstanceComponent>();
		KG_ASSERT(comp.ClassHandle != Assets::EmptyHandle);
		Ref<EntityClass> entityClass = Assets::AssetManager::GetEntityClass(comp.ClassHandle);
		KG_ASSERT(entityClass);
		int32_t fieldLocation = entityClass->GetFieldLocation(fieldName);
		KG_ASSERT(fieldLocation != -1);
		comp.Fields.at(fieldLocation)->SetValue(fieldValue);
		return;
		
	}
}
