#include "kgpch.h"

#include "Kargono/Scenes/Scene.h"
#include "Modules/ECSInternal/CustomComponent.h"
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

#include "Modules/Core/Components/IDComponent.h"
#include "Modules/Core/Components/TagComponent.h"
#include "Modules/Core/Components/TransformComponent.h"
#include "Modules/Scripting/Components/OnCreateComponent.h"
#include "Modules/Scripting/Components/OnUpdateComponent.h"
#include "Modules/Rendering/Components/CameraComponent.h"
#include "Modules/Rendering/Components/ShapeComponent.h"
#include "Modules/Physics2D/Components/BoxCollider2DComponent.h"
#include "Modules/Physics2D/Components/RigidBody2DComponent.h"
#include "Modules/Physics2D/Components/CircleCollider2DComponent.h"
#include "Modules/Particles/Components/ParticleEmitterComponent.h"

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

		bool success = m_EntityRegistry.m_Registry.Init(&m_SceneAlloc);
		KG_ASSERT(success);

		RegisterAllComponents();
		
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

	void Scene::RegisterAllComponents()
	{
		// TODO: Replace this mechanism w/ the module reflection system
		ECSInternal::RegistryInternal& registry = m_EntityRegistry.m_Registry;
		if (!registry.IsComponentRegistered<TagComponent>())
		{
			registry.RegisterComponent<TagComponent>();
		}
		if (!registry.IsComponentRegistered<TransformComponent>())
		{
			registry.RegisterComponent<TransformComponent>();
		}
		if (!registry.IsComponentRegistered<IDComponent>())
		{
			registry.RegisterComponent<IDComponent>();
		}
		if (!registry.IsComponentRegistered<Scripting::OnCreateComponent>())
		{
			registry.RegisterComponent<Scripting::OnCreateComponent>();
		}
		if (!registry.IsComponentRegistered<Scripting::OnUpdateComponent>())
		{
			registry.RegisterComponent<Scripting::OnUpdateComponent>();
		}
		if (!registry.IsComponentRegistered<Rendering::CameraComponent>())
		{
			registry.RegisterComponent<Rendering::CameraComponent>();
		}
		if (!registry.IsComponentRegistered<Rendering::ShapeComponent>())
		{
			registry.RegisterComponent<Rendering::ShapeComponent>();
		}
		if (!registry.IsComponentRegistered<Physics2D::Rigidbody2DComponent>())
		{
			registry.RegisterComponent<Physics2D::Rigidbody2DComponent>();
		}
		if (!registry.IsComponentRegistered<Physics2D::BoxCollider2DComponent>())
		{
			registry.RegisterComponent<Physics2D::BoxCollider2DComponent>();
		}
		if (!registry.IsComponentRegistered<Physics2D::CircleCollider2DComponent>())
		{
			registry.RegisterComponent<Physics2D::CircleCollider2DComponent>();
		}
		if (!registry.IsComponentRegistered<Particles::ParticleEmitterComponent>())
		{
			registry.RegisterComponent<Particles::ParticleEmitterComponent>();
		}

		// Custom Components
		for (auto& [handle, info] : Assets::AssetService::GetCustomComponentRegistry())
		{
			Ref<ECSInternal::CustomComponent> component = Assets::AssetService::GetCustomComponent(handle);
			KG_ASSERT(component);

			if (component->m_ComponentSize == 0 || registry.IsComponentRegistered(component->m_Identifier))
			{
				continue;
			}
			
			// Register component
			ECSInternal::ComponentMetadata metadata = component->GenerateMetadata();
			registry.RegisterComponent(component->m_Identifier, metadata);
		}

	}

	void Scene::AddCustomComponentRegistry(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ECSInternal::CustomComponent> component = Assets::AssetService::GetCustomComponent(projectComponentHandle);
		KG_ASSERT(component);
		KG_ASSERT(component->m_Identifier != ECSInternal::k_InvalidComponentIdentifier);

		if (component->m_ComponentSize == 0)
		{
			return;
		}

		ECSInternal::ComponentMetadata metadata = component->GenerateMetadata();

		// Register component
		m_EntityRegistry.m_Registry.RegisterComponent(component->m_Identifier, metadata);
	}

	void Scene::ClearCustomComponentRegistry(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ECSInternal::CustomComponent> component = Assets::AssetService::GetCustomComponent(projectComponentHandle);
		KG_ASSERT(component);

		if (component->m_ComponentSize == 0)
		{
			return;
		}

		// Clear the component store
		m_EntityRegistry.m_Registry.ClearComponentStore(component->m_Identifier);
	}

	std::size_t Scene::GetCustomComponentCount(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ECSInternal::CustomComponent> component = Assets::AssetService::GetCustomComponent(projectComponentHandle);
		KG_ASSERT(component);

		if (component->m_ComponentSize == 0)
		{
			return 0;
		}

		KG_ASSERT(component->m_Identifier != ECSInternal::k_InvalidComponentIdentifier);

		return m_EntityRegistry.m_Registry.GetComponentCount(component->m_Identifier);
	}

	ECS::Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		ECS::Entity entity = { m_EntityRegistry.m_Registry.CreateEntity().value() , &m_EntityRegistry};
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		TagComponent& tag = entity.AddComponent<TagComponent>();
		tag.m_Tag = name.empty() ? "Entity" : name;

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
		auto classInstanceView = GetAllEntitiesWith<Scripting::OnCreateComponent>();
		for (auto enttEntityID : classInstanceView)
		{
			ECS::Entity entity = { enttEntityID, &m_EntityRegistry };
			Scripting::OnCreateComponent& component = entity.GetComponent<Scripting::OnCreateComponent>();
			Assets::AssetHandle scriptHandle = component.m_OnCreateScriptHandle;
			if (scriptHandle != Assets::EmptyHandle)
			{
				Utility::CallWrappedVoidEntity(component.m_OnCreateScript->m_Function, entity.GetUUID());
			}
		}

		// Insert entities with ClassInstanceComponents into m_ScriptClassToEntityList map
		auto view = GetAllEntitiesWith<TagComponent>();
		for (ECSInternal::EntityID id : view)
		{
			ECS::Entity entity = { id, &m_EntityRegistry };
			TagComponent& component = entity.GetComponent<TagComponent>();
			if (!m_GroupToEntityList.contains(component.m_Group))
			{
				m_GroupToEntityList.insert({ component.m_Group, {}});
			}

			m_GroupToEntityList.at(component.m_Group).push_back(entity.GetUUID());
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

		// Copy over components
		m_EntityRegistry.m_Registry.CopyComponents(entity.GetInternalID(), newEntity.GetInternalID());

		return newEntity;
	}

	ECS::Entity Scene::FindEntityByName(const std::string& name)
	{
		auto view = m_EntityRegistry.m_Registry.GetFlatView<TagComponent>();
		for (ECSInternal::EntityID entity : view)
		{
			const TagComponent& tc = m_EntityRegistry.m_Registry.GetComponent<TagComponent>(entity).value();
			if (tc.m_Tag == name.c_str()) 
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

	ECS::Entity Scene::GetEntityByEnttID(ECSInternal::EntityID id)
	{
		// Ensure enttID is valid for this scene's registry
		if (m_EntityRegistry.m_Registry.HasEntity(id))
		{
			return { id, &m_EntityRegistry };
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
		auto view = m_EntityRegistry.m_Registry.GetFlatView<Rendering::CameraComponent>();
		for (ECSInternal::EntityID entity : view)
		{
			Rendering::CameraComponent& cameraComponent = m_EntityRegistry.m_Registry.GetComponent<Rendering::CameraComponent>(entity).value();
			
			cameraComponent.m_Camera.OnViewportResize();
		}

	}

	ECS::Entity Scene::GetPrimaryCameraEntity()
	{
		// TODO: This is ridiculous
		auto view = m_EntityRegistry.m_Registry.GetFlatView<Rendering::CameraComponent>();
		for (auto entity: view)
		{
			Rendering::CameraComponent& camera = m_EntityRegistry.m_Registry.GetComponent<Rendering::CameraComponent>(entity).value();
			if (camera.m_Primary)
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
			auto view = m_EntityRegistry.m_Registry.GetFlatView<TransformComponent, Rendering::ShapeComponent>();
			for (ECSInternal::EntityID entity : view)
			{

				TransformComponent& transform = m_EntityRegistry.m_Registry.GetComponent<TransformComponent>(entity).value();
				Rendering::ShapeComponent& shape = m_EntityRegistry.m_Registry.GetComponent<Rendering::ShapeComponent>(entity).value();
				s_InputSpec.m_Shader = shape.m_Shader;
				s_InputSpec.m_Buffer = shape.m_ShaderData;
				s_InputSpec.m_Entity = static_cast<uint32_t>(entity);
				s_InputSpec.m_EntityRegistry = &m_EntityRegistry.m_Registry;
				s_InputSpec.m_ShapeComponent = &shape;
				s_InputSpec.m_TransformMatrix = transform.GetTransform();

				for (const auto& PerObjectSceneFunction : shape.m_Shader->GetFillDataObjectScene())
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
		auto view = m_EntityRegistry.m_Registry.GetFlatView<Scripting::OnUpdateComponent>();
		for (ECSInternal::EntityID enttEntityID : view)
		{
			ECS::Entity entity = { enttEntityID, &m_EntityRegistry };
			Scripting::OnUpdateComponent& component = entity.GetComponent<Scripting::OnUpdateComponent>();
			Assets::AssetHandle scriptHandle = component.m_OnUpdateScriptHandle;
			if (scriptHandle != Assets::EmptyHandle)
			{
				Utility::CallWrappedVoidEntityFloat(component.m_OnUpdateScript->m_Function, entity.GetUUID(), ts);
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
		KG_ASSERT(entity.HasComponent<TransformComponent>());
		return entity.GetComponent<TransformComponent>().m_Translation;
	}
	void SceneService::TransformComponentSetTranslation(UUID entityID, Math::vec3 newTranslation)
	{
		KG_ASSERT(s_ActiveScene);
		ECS::Entity entity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<TransformComponent>());
		entity.GetComponent<TransformComponent>().m_Translation = newTranslation;
		if (entity.HasComponent<Physics2D::Rigidbody2DComponent>())
		{
			auto& rigidBody2DComp = entity.GetComponent<Physics2D::Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rigidBody2DComp.m_RuntimeBody;
			body->SetTransform({ newTranslation.x, newTranslation.y }, body->GetAngle());
		}
	}
	const std::string& SceneService::TagComponentGetTag(UUID entityID)
	{
		KG_ASSERT(s_ActiveScene);
		ECS::Entity entity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<TagComponent>());
		TagComponent& tagComponent = entity.GetComponent<TagComponent>();

		// TODO: REMOVE THE HELL OUT OF THIS, TEMPORARY FIX TO MAKE IT COMPILE, I HATE THIS, I HATE IT SO MUCH
		// JUST USE A STRING VIEW YEAH??
		static std::string tagBuffer{tagComponent.m_Tag.CString()};
		return tagBuffer;
	}
	void SceneService::Rigidbody2DComponent_SetLinearVelocity(UUID entityID, Math::vec2 linearVelocity)
	{
		KG_ASSERT(s_ActiveScene);
		ECS::Entity entity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<Physics2D::Rigidbody2DComponent>());
		auto& rigidBody2DComp = entity.GetComponent<Physics2D::Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rigidBody2DComp.m_RuntimeBody;
		body->SetLinearVelocity(b2Vec2(linearVelocity.x, linearVelocity.y));
	}
	Math::vec2 SceneService::Rigidbody2DComponent_GetLinearVelocity(UUID entityID)
	{
		KG_ASSERT(s_ActiveScene);
		ECS::Entity entity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<Physics2D::Rigidbody2DComponent>());
		auto& rigidBody2DComp = entity.GetComponent<Physics2D::Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rigidBody2DComp.m_RuntimeBody;
		const b2Vec2& linearVelocity = body->GetLinearVelocity();
		return Math::vec2(linearVelocity.x, linearVelocity.y);
	}
	void SceneService::SetCustomComponentField(UUID entityID, Assets::AssetHandle projectComponentID, uint64_t fieldLocation, void* value)
	{
		// Get the indicated entity
		ECS::Entity currentEntity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(currentEntity);

		// Get the indicated custom component
		Ref<ECSInternal::CustomComponent> projectComponent = Assets::AssetService::GetCustomComponent(projectComponentID);
		KG_ASSERT(projectComponent);
		KG_ASSERT(fieldLocation < projectComponent->m_DataOffsets.size());

		// Set indicated field value
		uint8_t* componentDataRef = (uint8_t*)currentEntity.GetCustomComponentData(projectComponentID);

		// Get field data pointer
		uint8_t* fieldDataRef = componentDataRef + projectComponent->m_DataOffsets.at(fieldLocation);

		// Set the data
		Utility::TransferDataForWrappedVarBuffer(projectComponent->m_DataTypes.at(fieldLocation), value, fieldDataRef);
	}
	void* SceneService::GetCustomComponentField(UUID entityID, Assets::AssetHandle projectComponentID, uint64_t fieldLocation)
	{
		// Get the indicated entity
		ECS::Entity currentEntity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(currentEntity);

		// Get the indicated custom component
		Ref<ECSInternal::CustomComponent> projectComponent = Assets::AssetService::GetCustomComponent(projectComponentID);
		KG_ASSERT(projectComponent);
		KG_ASSERT(fieldLocation < projectComponent->m_DataOffsets.size());

		// Set indicated field value
		uint8_t* componentDataRef = (uint8_t*)currentEntity.GetCustomComponentData(projectComponentID);

		// Get field data pointer
		uint8_t* fieldDataRef = componentDataRef + projectComponent->m_DataOffsets.at(fieldLocation);

		// Get the data
		return fieldDataRef;
	}

	Assets::AssetHandle SceneService::FindEntityHandleByName(const std::string& name)
	{
		for (auto& [handle, enttID] : s_ActiveScene->m_EntityRegistry.m_EntityMap)
		{
			ECS::Entity entity{ enttID, &s_ActiveScene->m_EntityRegistry };
			if (entity.HasComponent<TagComponent>())
			{
				TagComponent& tagComponent = entity.GetComponent<TagComponent>();
				if (tagComponent.m_Tag == name.c_str())
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
		// TODO: Re-implement this function with compile to known component identifiers '
		// (not strings)
		return true;
		/*
		if (!Utility::s_EntityHasComponentFunc.contains(componentName))
		{
			KG_ERROR("Invalid Component name provided.")
				return false;
		}
		KG_ASSERT(s_ActiveScene);
		ECS::Entity activeEntity = s_ActiveScene->GetEntityByUUID(entityID);
		KG_ASSERT(activeEntity);
		return Utility::s_EntityHasComponentFunc.at(componentName)(activeEntity);
		*/
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


