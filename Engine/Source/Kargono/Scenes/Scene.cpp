#include "kgpch.h"

#include "Kargono/Scenes/Scene.h"
#include "ECSPlugin/EngineComponents.h"
#include "ECSPlugin/ProjectComponent.h"
#include "ECSPlugin/Entity.h"
#include "Physics2DPlugin/Physics2D.h"
#include "RenderingPlugin/RenderingService.h"
#include "Kargono/Core/Engine.h"
#include "InputPlugin/InputService.h"
#include "InputMapPlugin/InputMap.h"
#include "RenderingPlugin/Shader.h"
#include "EventsPlugin/SceneEvent.h"
#include "Kargono/Projects/Project.h"
#include "AssetsPlugin/AssetService.h"
#include "ParticlesPlugin/ParticleService.h"

namespace Kargono::Scenes
{
	static Rendering::RendererInputSpec s_InputSpec{};

	Ref<Scene> SceneService::CreateSceneCopy(Ref<Scene> other)
	{
		Ref<Scene> newScene = CreateRef<Scene>();
		newScene->m_PhysicsSpecification = other->m_PhysicsSpecification;

		auto& srcSceneRegistry = other->m_EntityRegistry.m_EnTTRegistry;
		auto& dstSceneRegistry = newScene->m_EntityRegistry.m_EnTTRegistry;
		std::unordered_map<UUID, entt::entity> enttMap;

		// Create entities in new scene
		auto idView = srcSceneRegistry.view<ECS::IDComponent>();
		for (auto e : idView)
		{
			UUID uuid = srcSceneRegistry.get<ECS::IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<ECS::TagComponent>(e).Tag;
			ECS::Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		// Copy components (except IDComponent and TagComponent)
		Utility::CopyComponent(ECS::AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		// TODO: There is probably a faster way to get all the entities inside the scene without iterating through all entities
		// TODO: I am just not aware of a view function using raw storage references in entt
		// Transfer custom components into new scene
		// Handle all project components
		for (auto& [entityHandle, enttID] : other->m_EntityRegistry.m_EntityMap)
		{
			// Copy over data
			ECS::Entity existingEntity{ other->GetEntityByUUID(entityHandle) };
			ECS::Entity newEntity{ newScene->GetEntityByUUID(entityHandle) };
			for (auto& [handle, asset] : Assets::AssetService::GetProjectComponentRegistry())
			{
				Ref<ECS::ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(handle);
				KG_ASSERT(projectComponent);

				if (existingEntity.HasProjectComponentData(handle))
				{
					if (!newEntity.HasProjectComponentData(handle))
					{
						// Add project component into registry
						newEntity.AddProjectComponentData(handle);
					}

					// Get source and destination data buffers
					uint8_t* sourceDataPtr = (uint8_t*)existingEntity.GetProjectComponentData(handle);
					uint8_t* destinationDataPtr = (uint8_t*)newEntity.GetProjectComponentData(handle);
					std::memcpy(destinationDataPtr, sourceDataPtr, projectComponent->m_BufferSize);
				}
			}
		}

		return newScene;
		
	}

	Scene::Scene()
	{
		m_HoveredEntity = new ECS::Entity();
		m_SelectedEntity = new ECS::Entity();

		RegisterAllProjectComponents();
		
	}
	Scene::~Scene()
	{
		delete m_HoveredEntity;
		delete m_SelectedEntity;
	}


	ECS::Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	void Scene::RegisterAllProjectComponents()
	{
		m_EntityRegistry.m_ProjectComponentStorage.resize(Assets::AssetService::GetProjectComponentRegistry().size());

		for (auto& [handle, asset] : Assets::AssetService::GetProjectComponentRegistry())
		{
			Ref<ECS::ProjectComponent> component = Assets::AssetService::GetProjectComponent(handle);
			KG_ASSERT(component);

			if (component->m_BufferSize == 0)
			{
				continue;
			}
			ECS::ProjectComponentStorage& newStorage = m_EntityRegistry.m_ProjectComponentStorage.at(component->m_BufferSlot);

			// Create new storage value
			ECS::EntityRegistryService::RegisterProjectComponentWithEnTTRegistry(newStorage, m_EntityRegistry, component->m_BufferSize, component->m_Name);
		}
	}

	void Scene::AddProjectComponentRegistry(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ECS::ProjectComponent> component = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(component);

		if (component->m_BufferSize == 0)
		{
			return;
		}

		if (component->m_BufferSlot >= m_EntityRegistry.m_ProjectComponentStorage.size())
		{
			m_EntityRegistry.m_ProjectComponentStorage.resize(component->m_BufferSlot + 1);
		}

		ECS::ProjectComponentStorage& newStorage = m_EntityRegistry.m_ProjectComponentStorage.at(component->m_BufferSlot);

		// Create new storage value
		ECS::EntityRegistryService::RegisterProjectComponentWithEnTTRegistry(newStorage, m_EntityRegistry, component->m_BufferSize, component->m_Name);
		
	}

	void Scene::ClearProjectComponentRegistry(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ECS::ProjectComponent> component = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(component);

		if (component->m_BufferSize == 0)
		{
			return;
		}

		KG_ASSERT(component->m_BufferSlot < m_EntityRegistry.m_ProjectComponentStorage.size());

		// Get storage and clear registry
		ECS::ProjectComponentStorage& currentStorage = m_EntityRegistry.m_ProjectComponentStorage.at(component->m_BufferSlot);
		currentStorage.m_ClearProjectComponentRegistry(currentStorage.m_EnTTStorageReference, m_EntityRegistry.m_EnTTRegistry, component->m_Name);
	}

	std::size_t Scene::GetProjectComponentCount(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ECS::ProjectComponent> component = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(component);
		KG_ASSERT(component->m_BufferSlot < m_EntityRegistry.m_ProjectComponentStorage.size());

		if (component->m_BufferSize == 0)
		{
			return 0;
		}

		// Get storage and clear registry
		ECS::ProjectComponentStorage& currentStorage = m_EntityRegistry.m_ProjectComponentStorage.at(component->m_BufferSlot);
		return currentStorage.m_GetProjectComponentCount(currentStorage.m_EnTTStorageReference);
	}

	ECS::Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		ECS::Entity entity = { m_EntityRegistry.m_EnTTRegistry.create(), &m_EntityRegistry };
		entity.AddComponent<ECS::IDComponent>(uuid);
		entity.AddComponent<ECS::TransformComponent>();
		ECS::TagComponent& tag = entity.AddComponent<ECS::TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		m_EntityRegistry.m_EntityMap[uuid] = entity;

		Events::ManageEntity event = { entity.GetUUID(), this, Events::ManageEntityAction::Create };
		EngineService::OnEvent(&event);

		return entity;
	}

	void Scene::DestroyEntity(ECS::Entity entity)
	{
		Events::ManageEntity event = {entity.GetUUID(), this , Events::ManageEntityAction::Delete};
		EngineService::OnEvent(&event);

		m_EntityRegistry.m_EntityMap.erase(entity.GetUUID());
		if (m_EntityRegistry.m_EnTTRegistry.valid(entity))
		{
			m_EntityRegistry.m_EnTTRegistry.destroy(entity);
		}
	}

	void Scene::DestroyAllEntities()
	{
		if (m_EntityRegistry.m_EntityMap.empty()) { return; }
		for (auto& [uuid, entity] : m_EntityRegistry.m_EntityMap)
		{
			if (m_EntityRegistry.m_EnTTRegistry.valid(entity))
			{
				m_EntityRegistry.m_EnTTRegistry.destroy(entity);
			}
		}
		m_EntityRegistry.m_EntityMap.clear();
	}
	
	void Scene::OnRuntimeStart()
	{
		m_IsRunning = true;

		// Invoke OnCreate
		auto classInstanceView = GetAllEntitiesWith<ECS::OnCreateComponent>();
		for (auto enttEntityID : classInstanceView)
		{
			ECS::Entity entity = { enttEntityID, &m_EntityRegistry };
			ECS::OnCreateComponent& component = entity.GetComponent<ECS::OnCreateComponent>();
			Assets::AssetHandle scriptHandle = component.OnCreateScriptHandle;
			if (scriptHandle != Assets::EmptyHandle)
			{
				Utility::CallWrappedVoidEntity(component.OnCreateScript->m_Function, entity.GetUUID());
			}
		}

		// Insert entities with ClassInstanceComponents into m_ScriptClassToEntityList map
		auto view = GetAllEntitiesWith<ECS::TagComponent>();
		for (auto enttID : view)
		{
			ECS::Entity entity = { enttID, &m_EntityRegistry };
			ECS::TagComponent& component = entity.GetComponent<ECS::TagComponent>();
			if (!m_GroupToEntityList.contains(component.Group))
			{
				m_GroupToEntityList.insert({ component.Group, {}});
			}

			m_GroupToEntityList.at(component.Group).push_back(entity.GetUUID());
		}
	}

	void Scene::OnRuntimeStop()
	{
		m_IsRunning = false;

		// Script
		m_GroupToEntityList.clear();
	}

	ECS::Entity Scene::DuplicateEntity(ECS::Entity entity)
	{
		// Copy name because we're going to modify component data structure
		std::string name = entity.GetName();
		ECS::Entity newEntity = CreateEntity(name);
		Utility::CopyComponentIfExists(ECS::AllComponents{}, newEntity, entity);
		return newEntity;
	}

	ECS::Entity Scene::FindEntityByName(const std::string& name)
	{
		auto view = m_EntityRegistry.m_EnTTRegistry.view<ECS::TagComponent>();
		for (auto entity : view)
		{
			const ECS::TagComponent& tc = view.get<ECS::TagComponent>(entity);
			if (tc.Tag == name) { return ECS::Entity{ entity, & m_EntityRegistry }; }
		}
		return {};
	}


	ECS::Entity Scene::GetEntityByUUID(UUID uuid)
	{
		if (!m_EntityRegistry.m_EntityMap.contains(uuid))
		{
			KG_WARN("Could not find entity by uuid");
			return {};
		}

		return { m_EntityRegistry.m_EntityMap.at(uuid), &m_EntityRegistry };
	}

	ECS::Entity Scene::GetEntityByEnttID(entt::entity enttID)
	{
		// Ensure enttID is valid for this scene's registry
		if (m_EntityRegistry.m_EnTTRegistry.valid(enttID))
		{
			return { enttID, &m_EntityRegistry };
		}

		// Return empty entity
		return {};
	}

	bool Scene::CheckEntityExists(entt::entity entity)
	{
		return m_EntityRegistry.m_EnTTRegistry.valid(entity);
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		UNREFERENCED_PARAMETER(width);
		UNREFERENCED_PARAMETER(height);
		// Resize non-fixed
		auto view = m_EntityRegistry.m_EnTTRegistry.view<ECS::CameraComponent>();
		for (entt::entity entity : view)
		{
			ECS::CameraComponent& cameraComponent = view.get<ECS::CameraComponent>(entity);
			
			cameraComponent.Camera.OnViewportResize();
		}

	}

	ECS::Entity Scene::GetPrimaryCameraEntity()
	{
		// TODO: This is ridiculous
		auto view = m_EntityRegistry.m_EnTTRegistry.view<ECS::CameraComponent>();
		for (auto entity: view)
		{
			const auto& camera = view.get<ECS::CameraComponent>(entity);
			if (camera.Primary)
			{
				return ECS::Entity{ entity, & m_EntityRegistry };
			}
		}
		return {};
	}
	void Scene::RenderScene(Rendering::Camera& camera, const Math::mat4& transformMatrix)
	{
		Rendering::RenderingService::BeginScene(camera, transformMatrix);
		// Draw Shapes
		{
			auto view = m_EntityRegistry.m_EnTTRegistry.view<ECS::TransformComponent, ECS::ShapeComponent>();
			for (entt::entity entity : view)
			{
				const auto& [transform, shape] = view.get<ECS::TransformComponent, ECS::ShapeComponent>(entity);
				s_InputSpec.m_Shader = shape.Shader;
				s_InputSpec.m_Buffer = shape.ShaderData;
				s_InputSpec.m_Entity = static_cast<uint32_t>(entity);
				s_InputSpec.m_EntityRegistry = &m_EntityRegistry.m_EnTTRegistry;
				s_InputSpec.m_ShapeComponent = &shape;
				s_InputSpec.m_TransformMatrix = transform.GetTransform();

				for (const auto& PerObjectSceneFunction : shape.Shader->GetFillDataObjectScene())
				{
					PerObjectSceneFunction(s_InputSpec);
				}

				Rendering::RenderingService::SubmitDataToRenderer(s_InputSpec);
			}
		}
		Rendering::RenderingService::EndScene();
	}
	void Scene::OnUpdateEntities(Timestep ts)
	{
		// Invoke OnUpdate
		auto classInstanceView = GetAllEntitiesWith<ECS::OnUpdateComponent>();
		for (entt::entity enttEntityID : classInstanceView)
		{
			ECS::Entity entity = { enttEntityID, &m_EntityRegistry };
			ECS::OnUpdateComponent& component = entity.GetComponent<ECS::OnUpdateComponent>();
			Assets::AssetHandle scriptHandle = component.OnUpdateScriptHandle;
			if (scriptHandle != Assets::EmptyHandle)
			{
				Utility::CallWrappedVoidEntityFloat(component.OnUpdateScript->m_Function, entity.GetUUID(), ts);
			}
		}
	}

	void SceneService::Init()
	{
		Utility::RegisterHasComponent(ECS::AllComponents{});
	}

	void SceneService::Terminate()
	{
		// Custom closing of input spec
		s_InputSpec.m_Shader = nullptr;
		s_InputSpec.m_Texture = nullptr;
		s_InputSpec.m_ShapeComponent = nullptr;
		s_InputSpec.m_CurrentDrawBuffer = nullptr;

		s_ActiveScene.reset();
		s_ActiveSceneHandle = Assets::EmptyHandle;
	}
	Math::vec3 SceneService::TransformComponentGetTranslation(UUID entityID)
	{
		KG_ASSERT(s_ActiveScene);
		ECS::Entity entity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<ECS::TransformComponent>());
		return entity.GetComponent<ECS::TransformComponent>().Translation;
	}
	void SceneService::TransformComponentSetTranslation(UUID entityID, Math::vec3 newTranslation)
	{
		KG_ASSERT(s_ActiveScene);
		ECS::Entity entity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<ECS::TransformComponent>());
		entity.GetComponent<ECS::TransformComponent>().Translation = newTranslation;
		if (entity.HasComponent<ECS::Rigidbody2DComponent>())
		{
			auto& rigidBody2DComp = entity.GetComponent<ECS::Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rigidBody2DComp.RuntimeBody;
			body->SetTransform({ newTranslation.x, newTranslation.y }, body->GetAngle());
		}
	}
	const std::string& SceneService::TagComponentGetTag(UUID entityID)
	{
		KG_ASSERT(s_ActiveScene);
		ECS::Entity entity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<ECS::TagComponent>());
		ECS::TagComponent& tagComponent = entity.GetComponent<ECS::TagComponent>();
		return tagComponent.Tag;
	}
	void SceneService::Rigidbody2DComponent_SetLinearVelocity(UUID entityID, Math::vec2 linearVelocity)
	{
		KG_ASSERT(s_ActiveScene);
		ECS::Entity entity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<ECS::Rigidbody2DComponent>());
		auto& rigidBody2DComp = entity.GetComponent<ECS::Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rigidBody2DComp.RuntimeBody;
		body->SetLinearVelocity(b2Vec2(linearVelocity.x, linearVelocity.y));
	}
	Math::vec2 SceneService::Rigidbody2DComponent_GetLinearVelocity(UUID entityID)
	{
		KG_ASSERT(s_ActiveScene);
		ECS::Entity entity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<ECS::Rigidbody2DComponent>());
		auto& rigidBody2DComp = entity.GetComponent<ECS::Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rigidBody2DComp.RuntimeBody;
		const b2Vec2& linearVelocity = body->GetLinearVelocity();
		return Math::vec2(linearVelocity.x, linearVelocity.y);
	}
	void SceneService::SetProjectComponentField(UUID entityID, Assets::AssetHandle projectComponentID, uint64_t fieldLocation, void* value)
	{
		// Get the indicated entity
		ECS::Entity currentEntity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(currentEntity);

		// Get the indicated project component
		Ref<ECS::ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(projectComponentID);
		KG_ASSERT(projectComponent);
		KG_ASSERT(fieldLocation < projectComponent->m_DataLocations.size());

		// Set indicated field value
		uint8_t* componentDataRef = (uint8_t*)currentEntity.GetProjectComponentData(projectComponentID);

		// Get field data pointer
		uint8_t* fieldDataRef = componentDataRef + projectComponent->m_DataLocations.at(fieldLocation);

		// Set the data
		Utility::TransferDataForWrappedVarBuffer(projectComponent->m_DataTypes.at(fieldLocation), value, fieldDataRef);
	}
	void* SceneService::GetProjectComponentField(UUID entityID, Assets::AssetHandle projectComponentID, uint64_t fieldLocation)
	{
		// Get the indicated entity
		ECS::Entity currentEntity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(currentEntity);

		// Get the indicated project component
		Ref<ECS::ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(projectComponentID);
		KG_ASSERT(projectComponent);
		KG_ASSERT(fieldLocation < projectComponent->m_DataLocations.size());

		// Set indicated field value
		uint8_t* componentDataRef = (uint8_t*)currentEntity.GetProjectComponentData(projectComponentID);

		// Get field data pointer
		uint8_t* fieldDataRef = componentDataRef + projectComponent->m_DataLocations.at(fieldLocation);

		// Get the data
		return fieldDataRef;
	}

	Assets::AssetHandle SceneService::FindEntityHandleByName(const std::string& name)
	{
		for (auto& [handle, enttID] : s_ActiveScene->m_EntityRegistry.m_EntityMap)
		{
			ECS::Entity entity{ enttID, &s_ActiveScene->m_EntityRegistry };
			if (entity.HasComponent<ECS::TagComponent>())
			{
				ECS::TagComponent& tagComponent = entity.GetComponent<ECS::TagComponent>();
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
		ECS::Entity activeEntity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(activeEntity);
		return Utility::s_EntityHasComponentFunc.at(componentName)(activeEntity);
	}
	bool SceneService::IsSceneActive(UUID sceneID)
	{
		KG_ASSERT(s_ActiveScene);
		KG_ASSERT(s_ActiveSceneHandle != Assets::EmptyHandle);
		return sceneID == s_ActiveSceneHandle;
	}
	void SceneService::TransitionScene(Assets::AssetHandle newSceneHandle)
	{
		Ref<Scene> newScene = Assets::AssetService::GetScene(newSceneHandle);
		if (!newScene)
		{
			KG_WARN("Could not locate scene by scene handle");
			return;
		}
		TransitionScene(newScene);
		s_ActiveSceneHandle = newSceneHandle;
		Ref<Events::ManageScene> event = CreateRef<Events::ManageScene>(newSceneHandle, Events::ManageSceneAction::Open);
		EngineService::SubmitToEventQueue(event);
		
	}

	void SceneService::TransitionScene(Ref<Scene> newScene)
	{
		if (!newScene) { return; }

		Physics::Physics2DService::Terminate();
		s_ActiveScene->OnRuntimeStop();
		s_ActiveScene->DestroyAllEntities();
		s_ActiveScene.reset();

		s_ActiveScene = newScene;

		*s_ActiveScene->m_HoveredEntity = {};
		*s_ActiveScene->m_SelectedEntity = {};

		Physics::Physics2DService::Init(Scenes::SceneService::GetActiveScene().get(), Scenes::SceneService::GetActiveScene()->m_PhysicsSpecification);
		s_ActiveScene->OnRuntimeStart();
	}

	void SceneService::TransitionSceneFromHandle(Assets::AssetHandle sceneID)
	{
		Ref<Scenes::Scene> sceneReference = Assets::AssetService::GetScene(sceneID);
		if (sceneReference)
		{
			Particles::ParticleService::ClearEmitters();
			TransitionScene(sceneReference);

			s_ActiveSceneHandle = sceneID;
			Ref<Events::ManageScene> event = CreateRef<Events::ManageScene>(sceneID, Events::ManageSceneAction::Open);
			EngineService::SubmitToEventQueue(event);

			Particles::ParticleService::LoadSceneEmitters(sceneReference);
		}
		else
		{
			KG_WARN("Attempt to transition scenes, however, new scene reference could not be found!");
		}
	}

	void SceneService::SetActiveScene(Ref<Scene> newScene, Assets::AssetHandle newHandle)
	{
		s_ActiveScene = newScene;
		s_ActiveSceneHandle = newHandle;

		Ref<Events::ManageScene> event = CreateRef<Events::ManageScene>(newHandle, Events::ManageSceneAction::Open);
		EngineService::SubmitToEventQueue(event);
	}
}


