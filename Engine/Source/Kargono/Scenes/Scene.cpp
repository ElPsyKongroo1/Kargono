#include "kgpch.h"

#include "Kargono/Scenes/Scene.h"
#include "Modules/ECS/EngineComponents.h"
#include "Modules/ECS/ProjectComponent.h"
#include "Modules/ECS/Entity.h"
#include "Modules/Physics2D/Physics2D.h"
#include "Modules/Rendering/RenderingService.h"
#include "Modules/Core/Engine.h"
#include "Modules/Input/InputService.h"
#include "Modules/InputMap/InputMap.h"
#include "Modules/Rendering/Shader.h"
#include "Modules/Events/SceneEvent.h"
#include "Kargono/Projects/Project.h"
#include "Modules/Assets/AssetService.h"
#include "Modules/Particles/ParticleService.h"

namespace Kargono::Scenes
{
	static Rendering::RendererInputSpec s_InputSpec{};

	Ref<Scene> SceneService::CreateSceneCopy(Ref<Scene> other)
	{
		Ref<Scene> newScene = CreateRef<Scene>();
		newScene->m_PhysicsSpecification = other->m_PhysicsSpecification;

		ECSInternal::RegistryInternal& srcSceneRegistry = other->m_EntityRegistry.m_Registry;
		ECSInternal::RegistryInternal& dstSceneRegistry = newScene->m_EntityRegistry.m_Registry;

		// Copy over registry
		srcSceneRegistry.CopyRegistry(dstSceneRegistry);
		newScene->m_EntityRegistry.m_EntityMap = other->m_EntityRegistry.m_EntityMap;

		// Get all new created entities
		std::span<ECSInternal::EntityID> allEntities
		{
			newScene->m_EntityRegistry.m_Registry.GetAllEntities()
		};

		// Send create entity event for all new entities
		for (ECSInternal::EntityID entityID : allEntities)
		{
			ECS::Entity entity{ entityID, &newScene->m_EntityRegistry };
			Events::ManageEntity event = 
			{ 
				entity.GetUUID(), 
				newScene.get(), 
				Events::ManageEntityAction::Create
			};
			EngineService::GetActiveEngine().GetThread().OnEvent(&event);
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
		// TODO: Register All Components
	}

	void Scene::AddProjectComponentRegistry(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ECS::ProjectComponent> component = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(component);

		if (component->m_ComponentSize == 0)
		{
			return;
		}

		// Get identifier
		std::string identifierStr{ "ProjectComponent" "::" + component->m_Name };
		ECSInternal::ComponentIdentifier identifier =
			Utility::FileSystem::CRCFromString(identifierStr.c_str());

		ECSInternal::ComponentMetadata metadata{};
		metadata.m_ComponentSize = component->m_ComponentSize;
		metadata.m_ComponentAlignment = component->m_ComponentAlignment;

		// Register component
		m_EntityRegistry.m_Registry.RegisterComponent(identifier, metadata);
	}

	void Scene::ClearProjectComponentRegistry(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ECS::ProjectComponent> component = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(component);

		if (component->m_ComponentSize == 0)
		{
			return;
		}

		// Get identifier
		std::string identifierStr{ "ProjectComponent" "::" + component->m_Name };
		ECSInternal::ComponentIdentifier identifier =
			Utility::FileSystem::CRCFromString(identifierStr.c_str());

		// Clear the component store
		m_EntityRegistry.m_Registry.ClearComponentStore(identifier);
	}

	std::size_t Scene::GetProjectComponentCount(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ECS::ProjectComponent> component = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(component);

		if (component->m_ComponentSize == 0)
		{
			return 0;
		}

		// Get identifier
		std::string identifierStr{ "ProjectComponent" "::" + component->m_Name };
		ECSInternal::ComponentIdentifier identifier =
			Utility::FileSystem::CRCFromString(identifierStr.c_str());

		return m_EntityRegistry.m_Registry.GetComponentCount(identifier);
	}

	ECS::Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		ECS::Entity entity = { m_EntityRegistry.m_Registry.CreateEntity().value() , &m_EntityRegistry};
		entity.AddComponent<ECS::IDComponent>(uuid);
		entity.AddComponent<ECS::TransformComponent>();
		ECS::TagComponent& tag = entity.AddComponent<ECS::TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		m_EntityRegistry.m_EntityMap[uuid] = entity.GetInternalID();

		Events::ManageEntity event = { entity.GetUUID(), this, Events::ManageEntityAction::Create };
		EngineService::GetActiveEngine().GetThread().OnEvent(&event);

		return entity;
	}

	void Scene::DestroyEntity(ECS::Entity entity)
	{
		Events::ManageEntity event = {entity.GetUUID(), this , Events::ManageEntityAction::Delete};
		EngineService::GetActiveEngine().GetThread().OnEvent(&event);

		m_EntityRegistry.m_EntityMap.erase(entity.GetUUID());
		if (m_EntityRegistry.m_Registry.HasEntity(entity.GetInternalID()))
		{
			m_EntityRegistry.m_Registry.DestroyEntity(entity.GetInternalID());
		}
	}

	void Scene::DestroyAllEntities()
	{
		if (m_EntityRegistry.m_EntityMap.empty()) { return; }
		for (auto& [uuid, entity] : m_EntityRegistry.m_EntityMap)
		{
			if (m_EntityRegistry.m_Registry.HasEntity(entity))
			{
				m_EntityRegistry.m_Registry.DestroyEntity(entity);
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
		auto view = m_EntityRegistry.m_Registry.GetFlatView<ECS::TagComponent>();
		for (ECSInternal::EntityID entity : view)
		{
			const ECS::TagComponent& tc = m_EntityRegistry.m_Registry.GetComponent<ECS::TagComponent>(entity).value();
			if (tc.Tag == name) 
			{ 
				return ECS::Entity 
				{ 
					entity, & m_EntityRegistry 
				}; 
			}
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

	ECS::Entity Scene::GetEntityByEnttID(ECSInternal::EntityID enttID)
	{
		// Ensure enttID is valid for this scene's registry
		if (m_EntityRegistry.m_Registry.HasEntity(enttID))
		{
			return { enttID, &m_EntityRegistry };
		}

		// Return empty entity
		return {};
	}

	bool Scene::CheckEntityExists(ECSInternal::EntityID entity)
	{
		return m_EntityRegistry.m_Registry.HasEntity(entity);
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		UNREFERENCED_PARAMETER(width);
		UNREFERENCED_PARAMETER(height);
		// Resize non-fixed
		auto view = m_EntityRegistry.m_Registry.GetFlatView<ECS::CameraComponent>();
		for (ECSInternal::EntityID entity : view)
		{
			ECS::CameraComponent& cameraComponent = m_EntityRegistry.m_Registry.GetComponent<ECS::CameraComponent>(entity).value();
			
			cameraComponent.Camera.OnViewportResize();
		}

	}

	ECS::Entity Scene::GetPrimaryCameraEntity()
	{
		// TODO: This is ridiculous
		auto view = m_EntityRegistry.m_Registry.GetFlatView<ECS::CameraComponent>();
		for (auto entity: view)
		{
			ECS::CameraComponent& camera = m_EntityRegistry.m_Registry.GetComponent<ECS::CameraComponent>(entity).value();
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
			auto view = m_EntityRegistry.m_Registry.GetFlatView<ECS::TransformComponent, ECS::ShapeComponent>();
			for (ECSInternal::EntityID entity : view)
			{

				ECS::TransformComponent& transform = m_EntityRegistry.m_Registry.GetComponent<ECS::TransformComponent>(entity).value();
				ECS::ShapeComponent& shape = m_EntityRegistry.m_Registry.GetComponent<ECS::ShapeComponent>(entity).value();
				s_InputSpec.m_Shader = shape.Shader;
				s_InputSpec.m_Buffer = shape.ShaderData;
				s_InputSpec.m_Entity = static_cast<uint32_t>(entity);
				s_InputSpec.m_EntityRegistry = &m_EntityRegistry.m_Registry;
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
		auto view = m_EntityRegistry.m_Registry.GetFlatView<ECS::OnUpdateComponent>();
		for (ECSInternal::EntityID enttEntityID : view)
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
		// TODO: Previously had register has component func
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
		EngineService::GetActiveEngine().GetThread().SubmitEvent(event);
		
	}

	void SceneService::TransitionScene(Ref<Scene> newScene)
	{
		if (!newScene) { return; }

		Physics::Physics2DService().GetActiveContext().Terminate();
		Physics::Physics2DService().RemovePhysics2DWorld();
		s_ActiveScene->OnRuntimeStop();
		s_ActiveScene->DestroyAllEntities();
		s_ActiveScene.reset();

		s_ActiveScene = newScene;

		*s_ActiveScene->m_HoveredEntity = {};
		*s_ActiveScene->m_SelectedEntity = {};

		Physics::Physics2DService::CreatePhysics2DWorld();
		Physics::Physics2DService::GetActiveContext().Init
		(
			Scenes::SceneService::GetActiveScene().get(), 
			Scenes::SceneService::GetActiveScene()->m_PhysicsSpecification
		);
		
		s_ActiveScene->OnRuntimeStart();
	}

	void SceneService::TransitionSceneFromHandle(Assets::AssetHandle sceneID)
	{
		Ref<Scenes::Scene> sceneReference = Assets::AssetService::GetScene(sceneID);
		if (sceneReference)
		{
			Particles::ParticleService::GetActiveContext().ClearEmitters();
			TransitionScene(sceneReference);

			s_ActiveSceneHandle = sceneID;
			Ref<Events::ManageScene> event = CreateRef<Events::ManageScene>(sceneID, Events::ManageSceneAction::Open);
			EngineService::GetActiveEngine().GetThread().SubmitEvent(event);

			Particles::ParticleService::GetActiveContext().LoadSceneEmitters(sceneReference);
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
		EngineService::GetActiveEngine().GetThread().SubmitEvent(event);
	}
}


