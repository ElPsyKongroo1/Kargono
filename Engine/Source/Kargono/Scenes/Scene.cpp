#include "kgpch.h"

#include "Kargono/Scenes/Scene.h"
#include "Kargono/Scenes/Components.h"
#include "Kargono/Scenes/Entity.h"
#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Rendering/RenderingService.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Input/InputService.h"
#include "Kargono/Rendering/Shader.h"


namespace Kargono::Utility
{
	static std::unordered_map<std::string, std::function<bool(Scenes::Entity)>> s_EntityHasComponentFunc {};

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
			{
				auto view = src.view<Component>();
				for (auto srcEntity : view)
				{
					entt::entity dstEntity = enttMap.at(src.get<Scenes::IDComponent>(srcEntity).ID);

					auto& srcComponent = src.get<Component>(srcEntity);
					dst.emplace_or_replace<Component>(dstEntity, srcComponent);
				}
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(Scenes::ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	template<typename... Component>
	static void CopyComponentIfExists(Scenes::Entity dst, Scenes::Entity src)
	{
		([&]()
			{
				if (src.HasComponent<Component>())
					dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(Scenes::ComponentGroup<Component...>, Scenes::Entity dst, Scenes::Entity src)
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
}


namespace Kargono::Scenes
{
	Ref<Scene> SceneService::s_ActiveScene {nullptr};
	Assets::AssetHandle SceneService::s_ActiveSceneHandle {Assets::EmptyHandle};

	Ref<Scene> SceneService::CreateSceneCopy(Ref<Scene> other)
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
		Utility::CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		return newScene;
		
	}

	Scene::Scene()
	{
		m_HoveredEntity = new Entity();
		m_SelectedEntity = new Entity();
		
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

		// Insert entities with ClassInstanceComponents into m_ScriptClassToEntityList map
		auto view = GetAllEntitiesWith<ClassInstanceComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			const auto& classInstanceComp = entity.GetComponent<ClassInstanceComponent>();
			KG_ASSERT(classInstanceComp.ClassReference);
			if (!m_ScriptClassToEntityList.contains(classInstanceComp.ClassReference->GetName()))
			{
				m_ScriptClassToEntityList.insert({ classInstanceComp.ClassReference->GetName(), {}});
			}

			m_ScriptClassToEntityList.at(classInstanceComp.ClassReference->GetName()).push_back(entity.GetUUID());
		}
	}

	void Scene::OnRuntimeStop()
	{
		m_IsRunning = false;
		m_PhysicsWorld.reset();
		m_PhysicsWorld = nullptr;

		// Script
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
		Utility::CopyComponentIfExists(AllComponents{}, newEntity, entity);
		return newEntity;
	}

	Entity Scene::FindEntityByName(const std::string& name)
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
		if (!m_EntityMap.contains(uuid))
		{
			KG_WARN("Could not find entity by uuid");
			return {};
		}

		return { m_EntityMap.at(uuid), this };
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

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity: view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
			{
				return Entity{ entity, this };
			}
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
	void Scene::OnUpdateInputMode(Timestep ts)
	{
		if (Input::InputModeService::GetActiveInputMode())
		{
			for (auto& inputBinding : Input::InputModeService::GetActiveOnUpdate())
			{
				if (inputBinding->GetScript()->m_ScriptType != Scripting::ScriptType::Class)
				{
					Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();
					KG_ASSERT(keyboardBinding->GetScript());
					if (!Input::InputService::IsKeyPressed(keyboardBinding->GetKeyBinding())) { continue; }
					if (keyboardBinding->GetScript()->m_FuncType == WrappedFuncType::Void_None)
					{
						((WrappedVoidNone*)keyboardBinding->GetScript()->m_Function.get())->m_Value();
					}
					else
					{
						((WrappedVoidFloat*)keyboardBinding->GetScript()->m_Function.get())->m_Value(ts);
					}
				}
				else
				{
					Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();
					if (!Input::InputService::IsKeyPressed(keyboardBinding->GetKeyBinding())) { continue; }
					Ref<Scripting::Script> script = keyboardBinding->GetScript();
					KG_ASSERT(script);
					KG_ASSERT(m_ScriptClassToEntityList.contains(keyboardBinding->GetScript()->m_SectionLabel));
					for (auto entity : m_ScriptClassToEntityList.at(keyboardBinding->GetScript()->m_SectionLabel))
					{
						if (keyboardBinding->GetScript()->m_FuncType == WrappedFuncType::Void_UInt64)
						{
							((WrappedVoidUInt64*)keyboardBinding->GetScript()->m_Function.get())->m_Value(entity);
						}
						else
						{
							((WrappedVoidUInt64Float*)keyboardBinding->GetScript()->m_Function.get())->m_Value(entity, ts);
						}
					}
				}
			}
		}
	}
	void Scene::OnKeyPressed(Events::KeyPressedEvent event)
	{
		if (event.IsRepeat()) { return; }
		if (Input::InputModeService::GetActiveInputMode())
		{
			for (auto& inputBinding : Input::InputModeService::GetActiveOnKeyPressed())
			{
				if (inputBinding->GetScript()->m_ScriptType != Scripting::ScriptType::Class)
				{
					Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();
					if (!Input::InputService::IsKeyPressed(keyboardBinding->GetKeyBinding())) { continue; }
					((WrappedVoidNone*)keyboardBinding->GetScript()->m_Function.get())->m_Value();
				}
			}
		}
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
		Utility::RegisterHasComponent(AllComponents{});
	}
	Math::vec3 SceneService::TransformComponentGetTranslation(UUID entityID)
	{
		KG_ASSERT(s_ActiveScene);
		Scenes::Entity entity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<TransformComponent>());
		return entity.GetComponent<Scenes::TransformComponent>().Translation;
	}
	void SceneService::TransformComponentSetTranslation(UUID entityID, Math::vec3 newTranslation)
	{
		KG_ASSERT(s_ActiveScene);
		Scenes::Entity entity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<TransformComponent>());
		entity.GetComponent<Scenes::TransformComponent>().Translation = newTranslation;
		if (entity.HasComponent<Scenes::Rigidbody2DComponent>())
		{
			auto& rigidBody2DComp = entity.GetComponent<Scenes::Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rigidBody2DComp.RuntimeBody;
			body->SetTransform({ newTranslation.x, newTranslation.y }, body->GetAngle());
		}
	}
	const std::string& SceneService::TagComponentGetTag(UUID entityID)
	{
		KG_ASSERT(s_ActiveScene);
		Scenes::Entity entity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<TagComponent>());
		auto& tagComponent = entity.GetComponent<TagComponent>();
		return tagComponent.Tag;
	}
	void SceneService::Rigidbody2DComponent_SetLinearVelocity(UUID entityID, Math::vec2 linearVelocity)
	{
		KG_ASSERT(s_ActiveScene);
		Scenes::Entity entity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<Rigidbody2DComponent>());
		auto& rigidBody2DComp = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rigidBody2DComp.RuntimeBody;
		body->SetLinearVelocity(b2Vec2(linearVelocity.x, linearVelocity.y));
	}
	Math::vec2 SceneService::Rigidbody2DComponent_GetLinearVelocity(UUID entityID)
	{
		KG_ASSERT(s_ActiveScene);
		Scenes::Entity entity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<Rigidbody2DComponent>());
		auto& rigidBody2DComp = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rigidBody2DComp.RuntimeBody;
		const b2Vec2& linearVelocity = body->GetLinearVelocity();
		return Math::vec2(linearVelocity.x, linearVelocity.y);
	}
	void SceneService::SetEntityFieldByName(UUID entityID, const std::string& fieldName, void* fieldValue)
	{
		Scenes::Entity entity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(s_ActiveScene);
		KG_ASSERT(entity);
		if (!entity.HasComponent<ClassInstanceComponent>())
		{
			KG_ERROR("No valid ClassInstanceComponent associated with entity");
			return;
		}

		ClassInstanceComponent& comp = entity.GetComponent<ClassInstanceComponent>();
		KG_ASSERT(comp.ClassHandle != Assets::EmptyHandle);
		Ref<EntityClass> entityClass = comp.ClassReference;
		KG_ASSERT(entityClass);
		int32_t fieldLocation = entityClass->GetFieldLocation(fieldName);
		KG_ASSERT(fieldLocation != -1);
		comp.Fields.at(fieldLocation)->SetValue(fieldValue);
		return;
		
	}

	void* SceneService::GetEntityFieldByName(UUID entityID, const std::string& fieldName)
	{
		KG_ASSERT(s_ActiveScene);
		Scenes::Entity entity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		if (!entity.HasComponent<ClassInstanceComponent>())
		{
			KG_ERROR("No valid ClassInstanceComponent associated with entity");
			return nullptr;
		}

		ClassInstanceComponent& comp = entity.GetComponent<ClassInstanceComponent>();
		KG_ASSERT(comp.ClassHandle != Assets::EmptyHandle);
		Ref<EntityClass> entityClass = comp.ClassReference;
		KG_ASSERT(entityClass);
		int32_t fieldLocation = entityClass->GetFieldLocation(fieldName);
		KG_ASSERT(fieldLocation != -1);
		return comp.Fields.at(fieldLocation)->GetValue();
		

	}
	Assets::AssetHandle SceneService::FindEntityHandleByName(const std::string& name)
	{
		for (auto& [handle, enttID] : s_ActiveScene->m_EntityMap)
		{
			Entity entity{ enttID, s_ActiveScene.get() };
			if (entity.HasComponent<TagComponent>())
			{
				TagComponent& tagComponent = entity.GetComponent<TagComponent>();
				if (tagComponent.Tag == name)
				{
					return handle;
				}
			}
		}
		KG_WARN("Could not locate entity by name!");
		return Assets::EmptyHandle;
	}

	bool SceneService::CheckActiveHasComponent(UUID entityID, const std::string& componentName)
	{
		if (!Utility::s_EntityHasComponentFunc.contains(componentName))
		{
			KG_ERROR("Invalid Component name provided.")
				return false;
		}
		KG_ASSERT(s_ActiveScene);
		Entity activeEntity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(activeEntity);
		return Utility::s_EntityHasComponentFunc.at(componentName)(activeEntity);
	}
	void SceneService::TransitionScene(Assets::AssetHandle newSceneHandle)
	{
		Ref<Scene> newScene = Assets::AssetManager::GetScene(newSceneHandle);
		TransitionScene(newScene);
	}

	void SceneService::TransitionScene(Ref<Scene> newScene)
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

	void SceneService::TransitionSceneFromName(const std::string& sceneName)
	{
		auto [handle, sceneReference] = Assets::AssetManager::GetScene(sceneName);
		if (sceneReference)
		{
			TransitionScene(sceneReference);
		}
	}
}


