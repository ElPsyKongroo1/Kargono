#include "kgpch.h"

#include "Modules/Scenes/Assets/Scene.h"
#include "Modules/ECSInternal/Assets/CustomComponent.h"
#include "Modules/ECS/Entity.h"
#include "Modules/Physics2D/Physics2D.h"
#include "Modules/Core/Engine.h"
#include "Modules/Input/InputService.h"
#include "Modules/InputMap/Assets/InputMap.h"
#include "Modules/Rendering/Assets/Shader.h"
#include "Modules/Events/SceneEvent.h"
#include "Kargono/Projects/Project.h"

#include "Modules/Particles/ParticleContext.h"

#include "Modules/Core/Components/Tag.h"
#include "Modules/Core/Components/Transform.h"
#include "Modules/Scripting/Components/OnCreate.h"
#include "Modules/Scripting/Components/OnUpdate.h"
#include "Modules/Cameras/Components/CameraComponent.h"
#include "Modules/Rendering/Components/ShapeComponent.h"
#include "Modules/Physics2D/Components/BoxCollider2D.h"
#include "Modules/Physics2D/Components/RigidBody2D.h"
#include "Modules/Physics2D/Components/CircleCollider2D.h"
#include "Modules/Particles/Components/ParticleEmitter.h"
#include "Modules/States/Components/StateMachine.h"
#include "Modules/Scenes/SceneContext.h"

#include "Modules/Scripting/Assets/Script.h"

#include "Modules/ECSInternal/Assets/CustomComponent.h"
#include "Modules/Scenes/Assets/Scene.h"
#include "Modules/ECS/Entity.h"

namespace Kargono::Scenes
{
	void Scene::CreateFromName(Assets::Metadata<Scene>& metadata)
	{
		// Create default scene
		Scene defaultScene{};

		// Save Binary into File
		Assets::SerializeAssetContext<Scene> serializeContext{};
		serializeContext.m_AssetMetadata = &metadata;
		defaultScene.Serialize((void*)&serializeContext);
	}
	void Scene::Serialize(void* context)
	{
		// Get asset context
		Assets::SerializeAssetContext<Scene>* assetContext = (Assets::SerializeAssetContext<Scene>*)context;
		KG_ASSERT(assetContext, "Context cannot be null");
		Assets::Metadata<Scene>* metadata{ assetContext->m_AssetMetadata };
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get context fields
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath() };

		bool submitScene{ true };
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		{ // Physics
			out << YAML::Key << "Physics" << YAML::BeginMap; // Physics Map
			out << YAML::Key << "Gravity" << YAML::Value << m_PhysicsSpecification.m_Gravity;
			out << YAML::EndMap; // Physics Maps
		}
		UUID primaryCameraUUID{ k_EmptyUUID };

		if (GetPrimaryCameraEntity().IsValid())
		{
			primaryCameraUUID = GetPrimaryCameraEntity().GetUUID();
		}

		// Add primary camera
		out << YAML::Key << "PrimaryCamera" << YAML::Value << (uint64_t)primaryCameraUUID;

		// Add background color
		out << YAML::Key << "BackgroundColor" << YAML::Value << m_BackgroundColor;

		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		std::span<ECSInternal::EntityID> view{ m_EntityRegistry.m_Registry.GetAllEntities() };

		for (ECSInternal::EntityID entityID : view)
		{
			ECS::Entity entity = { entityID, &m_EntityRegistry };
			if (!entity) { return; }

			// Create serialization context
			ECS::SerializeEntityContext entityContext{ &out };
			entity.Serialize((void*)&entityContext);

		}
		out << YAML::EndSeq;
		out << YAML::EndMap; // Start of File Map
		if (submitScene)
		{
			std::ofstream fout(assetPath);
			fout << out.c_str();
			KG_INFO("Successfully Serialized Scene at {}", assetPath.string());
		}
		else
		{
			KG_WARN("Failed to Serialize Scene");
		}
	}
	void Scene::Deserialize(void* context)
	{
		// Get context
		Assets::DeserializeAssetContext<Scene>* deserializeContext = (Assets::DeserializeAssetContext<Scene>*)context;
		KG_ASSERT(deserializeContext, "Context cannot be null");
		Assets::Metadata<Scene>* metadata = deserializeContext->m_AssetMetadata;
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get asset path
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath() };

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetPath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_WARN("Failed to load .kgscene file '{0}'\n     {1}", assetPath, e.what());
			return;
		}

		YAML::Node physics = data["Physics"];
		GetPhysicsSpecification().m_Gravity = physics["Gravity"].as<Math::vec2>();

		m_BackgroundColor = data["BackgroundColor"].as<Math::vec4>();

		YAML::Node entities = data["Entities"];
		if (entities)
		{
			// Deserialize each entity
			for (YAML::Node entity : entities)
			{
				// Create entity for registry
				uint64_t uuid = entity["UUID"].as<uint64_t>();
				ECS::Entity newEntity = m_EntityRegistry.CreateEntityWithUUID(uuid);

				// Deserialize the entity
				ECS::DeserializeEntityContext context{ &entity };
				newEntity.Deserialize((void*)&context);
			}
		}

		// Load in primary camera
		UUID primaryCameraID = data["PrimaryCamera"].as<UUID>();
		if (primaryCameraID != k_EmptyUUID)
		{
			ECS::Entity primaryCameraEntity = m_EntityRegistry.GetEntityByUUID(primaryCameraID);
			KG_ASSERT(primaryCameraEntity.IsValid() &&
				primaryCameraEntity.HasComponent<Cameras::CameraComponent>());
			SetPrimaryCameraEntity(primaryCameraEntity);
		}
	}
	bool Scene::RemoveScript(Assets::AssetHandle scriptHandle)
	{
		bool sceneModified{ false };
		// Handle UI level function pointers
		// OnUpdate
		auto onUpdateView = m_EntityRegistry.GetView<Scripting::OnUpdate>();
		for (ECSInternal::EntityID enttEntity : onUpdateView)
		{
			ECS::Entity currentEntity{ m_EntityRegistry.GetEntityByECSID(enttEntity) };
			Scripting::OnUpdate& component = currentEntity.GetComponent<Scripting::OnUpdate>();
			if (component.m_OnUpdateScript.GetAssetHandle() == scriptHandle)
			{
				component.m_OnUpdateScript.Reset();
				sceneModified = true;
			}
		}

		// OnCreate
		auto onCreateView = m_EntityRegistry.GetView<Scripting::OnCreate>();
		for (ECSInternal::EntityID enttEntity : onCreateView)
		{
			ECS::Entity currentEntity{ m_EntityRegistry.GetEntityByECSID(enttEntity) };
			Scripting::OnCreate& component = currentEntity.GetComponent<Scripting::OnCreate>();
			if (component.m_OnCreateScript.GetAssetHandle() == scriptHandle)
			{
				component.m_OnCreateScript.Reset();
				sceneModified = true;
			}
		}

		// Rigidbody
		auto rigidBodyView = m_EntityRegistry.GetView<Physics2D::RigidBody2D>();
		for (ECSInternal::EntityID enttEntity : rigidBodyView)
		{
			ECS::Entity currentEntity{ m_EntityRegistry.GetEntityByECSID(enttEntity) };
			Physics2D::RigidBody2D& component = currentEntity.GetComponent<Physics2D::RigidBody2D>();

			if (component.m_OnCollisionStartScript.GetAssetHandle()  == scriptHandle)
			{
				component.m_OnCollisionStartScript.Reset();
				sceneModified = true;
			}

			if (component.m_OnCollisionEndScript.GetAssetHandle() == scriptHandle)
			{
				component.m_OnCollisionEndScript.Reset();
				sceneModified = true;
			}
		}

		return sceneModified;
	}
	bool Scene::RemoveState(Assets::AssetHandle aiStateHandle)
	{
		bool aiStateModified{ false };

		// Check for State
		auto aiStateView = m_EntityRegistry.GetView<States::StateMachine>();
		for (ECSInternal::EntityID enttEntity : aiStateView)
		{
			ECS::Entity currentEntity{ m_EntityRegistry.GetEntityByECSID(enttEntity) };
			States::StateMachine& component = currentEntity.GetComponent<States::StateMachine>();

			if (component.m_CurrentStateReference.GetAssetHandle()  == aiStateHandle)
			{
				component.m_CurrentStateReference.Reset();
				aiStateModified = true;
			}
			if (component.m_GlobalStateReference.GetAssetHandle() == aiStateHandle)
			{
				component.m_GlobalStateReference.Reset();
				aiStateModified = true;
			}
			if (component.m_PreviousStateReference.GetAssetHandle() == aiStateHandle)
			{
				component.m_PreviousStateReference.Reset();
				aiStateModified = true;
			}
		}
		return aiStateModified;
	}
	bool Scene::RemoveCustomComponent(Assets::AssetHandle projectCompHandle)
	{
		ECS::Registry& registry = m_EntityRegistry;

		// Check if any components are being removed
		size_t componentCount = registry.GetCustomComponentCount(projectCompHandle);

		// Clear component registry
		registry.UnRegisterCustomComponent(projectCompHandle);

		return componentCount > 0;
	}
	bool Scene::RemoveEmitterConfig(Assets::AssetHandle emitterConfigHandle)
	{
		bool emitterConfigModified{ false };

		// Check for emitterConfig
		auto emitterConfigView = m_EntityRegistry.GetView<Particles::ParticleEmitter>();
		for (ECSInternal::EntityID enttEntity : emitterConfigView)
		{
			ECS::Entity currentEntity{ m_EntityRegistry.GetEntityByECSID(enttEntity) };
			Particles::ParticleEmitter& component = currentEntity.GetComponent<Particles::ParticleEmitter>();

			if (component.m_EmitterConfigRef.GetAssetHandle() == emitterConfigHandle)
			{
				component.m_EmitterConfigRef.Reset();
				emitterConfigModified = true;
			}
		}
		return emitterConfigModified;
	}

	Ref<Scene> Scene::CreateSceneCopy()
	{
		Ref<Scene> newScene = CreateRef<Scene>();
		newScene->m_PhysicsSpecification = m_PhysicsSpecification;

		ECSInternal::RegistryInternal& srcSceneRegistry = m_EntityRegistry.m_Registry;
		ECSInternal::RegistryInternal& dstSceneRegistry = newScene->m_EntityRegistry.m_Registry;

		// Copy over registry
		srcSceneRegistry.CopyRegistry(dstSceneRegistry);
		newScene->m_EntityRegistry.m_EntityMap = m_EntityRegistry.m_EntityMap;

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
				&newScene->m_EntityRegistry, 
				Events::ManageEntityAction::Create
			};
			EngineService::GetActiveEngine().GetThread().OnEvent(&event);
		}

		return newScene;
		
	}

	Scene::Scene()
	{
		bool success = m_EntityRegistry.m_Registry.Init(&m_SceneAlloc);
		KG_ASSERT(success);

		RegisterAllComponents();
	}

	void Scene::RegisterAllComponents()
	{
		// TODO: Replace this mechanism w/ the module reflection system
		// Load in core components
		m_EntityRegistry.RegisterComponent<Tag>();
		m_EntityRegistry.RegisterComponent<Transform>();
		m_EntityRegistry.RegisterComponent<Scripting::OnCreate>();
		m_EntityRegistry.RegisterComponent<Scripting::OnUpdate>();
		m_EntityRegistry.RegisterComponent<Cameras::CameraComponent>();
		m_EntityRegistry.RegisterComponent<Rendering::ShapeComponent>();
		m_EntityRegistry.RegisterComponent<Physics2D::RigidBody2D>();
		m_EntityRegistry.RegisterComponent<Physics2D::BoxCollider2D>();
		m_EntityRegistry.RegisterComponent<Physics2D::CircleCollider2D>();
		m_EntityRegistry.RegisterComponent<Particles::ParticleEmitter>();
		m_EntityRegistry.RegisterComponent<States::StateMachine>();

		// Custom Components
		for (auto& [handle, info] : Assets::s_CustomComponentManager.GetAssetRegistry())
		{
			m_EntityRegistry.RegisterCustomComponent(handle);
		}

	}
	
	void Scene::OnRuntimeStart()
	{
		m_IsRunning = true;

		// Invoke OnCreate
		auto classInstanceView = m_EntityRegistry.GetView<Scripting::OnCreate>();
		for (ECSInternal::EntityID id : classInstanceView)
		{
			ECS::Entity entity = { id, &m_EntityRegistry };
			Scripting::OnCreate& component = entity.GetComponent<Scripting::OnCreate>();
			Assets::AssetHandle scriptHandle = component.m_OnCreateScript.GetAssetHandle();
			if (scriptHandle.IsValid())
			{
				Utility::CallWrapped<WrappedVoidEntity>(component.m_OnCreateScript->m_Function, entity.GetUUID());
			}
		}
	}

	void Scene::OnRuntimeStop()
	{
		m_IsRunning = false;
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		UNREFERENCED_PARAMETER(width);
		UNREFERENCED_PARAMETER(height);
		// Resize non-fixed
		auto view = m_EntityRegistry.m_Registry.GetFlatView<Cameras::CameraComponent>();
		for (ECSInternal::EntityID entity : view)
		{
			Cameras::CameraComponent& cameraComponent = m_EntityRegistry.m_Registry.GetComponent<Cameras::CameraComponent>(entity).value();
			
			cameraComponent.m_Camera.OnViewportResize();
		}
	}
	void Scene::OnRender(Cameras::CameraProjection& camera, const Math::mat4& transformMatrix)
	{
		Rendering::RenderingService::BeginScene(camera, transformMatrix);
		// Draw Shapes
		{
			Rendering::RendererInputSpec& inputSpec{ SceneService::GetActiveContext().m_RenderSceneSpec };
			auto view = m_EntityRegistry.m_Registry.GetFlatView<Transform, Rendering::ShapeComponent>();
			for (ECSInternal::EntityID entity : view)
			{
				Transform& transform = m_EntityRegistry.m_Registry.GetComponent<Transform>(entity).value();
				Rendering::ShapeComponent& shape = m_EntityRegistry.m_Registry.GetComponent<Rendering::ShapeComponent>(entity).value();
				inputSpec.m_Shader = shape.m_Shader;
				inputSpec.m_Buffer = shape.m_ShaderData;
				inputSpec.m_Entity = static_cast<uint32_t>(entity);
				inputSpec.m_EntityRegistry = &m_EntityRegistry.m_Registry;
				inputSpec.m_ShapeComponent = &shape;
				inputSpec.m_TransformMatrix = transform.GetTransform();

				for (const auto& PerObjectSceneFunction : shape.m_Shader->GetFillDataObjectScene())
				{
					PerObjectSceneFunction(&inputSpec);
				}

				Rendering::RenderingService::SubmitDataToRenderer(inputSpec);
			}
		}
		Rendering::RenderingService::EndScene();
	}
	void Scene::OnUpdate(Timestep ts)
	{
		// Invoke OnUpdate
		auto view = m_EntityRegistry.m_Registry.GetFlatView<Scripting::OnUpdate>();
		for (ECSInternal::EntityID enttEntityID : view)
		{
			ECS::Entity entity = { enttEntityID, &m_EntityRegistry };
			Scripting::OnUpdate& component = entity.GetComponent<Scripting::OnUpdate>();
			Assets::AssetHandle scriptHandle = component.m_OnUpdateScript.GetAssetHandle();
			if (scriptHandle.IsValid())
			{
				Utility::CallWrapped<WrappedVoidEntityFloat>(component.m_OnUpdateScript->m_Function, entity.GetUUID(), ts);
			}
		}
	}

	Math::vec3 Scene::TransformComponentGetTranslation(UUID entityID)
	{
		ECS::Entity entity = m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<Transform>());
		return entity.GetComponent<Transform>().m_Translation;
	}
	void Scene::TransformComponentSetTranslation(UUID entityID, Math::vec3 newTranslation)
	{
		ECS::Entity entity = m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<Transform>());
		entity.GetComponent<Transform>().m_Translation = newTranslation;
		if (entity.HasComponent<Physics2D::RigidBody2D>())
		{
			auto& rigidBody2DComp = entity.GetComponent<Physics2D::RigidBody2D>();
			b2Body* body = (b2Body*)rigidBody2DComp.m_RuntimeBody;
			body->SetTransform({ newTranslation.x, newTranslation.y }, body->GetAngle());
		}
	}
	std::string_view Scene::TagComponentGetTag(UUID entityID)
	{
		ECS::Entity entity = m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<Tag>());
		Tag& tagComponent = entity.GetComponent<Tag>();

		// TODO: REMOVE THE HELL OUT OF THIS, TEMPORARY FIX TO MAKE IT COMPILE, I HATE THIS, I HATE IT SO MUCH
		// JUST USE A STRING VIEW YEAH??
		static std::string tagBuffer;
		tagBuffer = tagComponent.m_Tag.CString();
		return tagBuffer;
	}
	void Scene::RigidBody2D_SetLinearVelocity(UUID entityID, Math::vec2 linearVelocity)
	{
		ECS::Entity entity = m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<Physics2D::RigidBody2D>());
		Physics2D::RigidBody2D& rigidBody2DComp = entity.GetComponent<Physics2D::RigidBody2D>();
		b2Body* body = (b2Body*)rigidBody2DComp.m_RuntimeBody;
		body->SetLinearVelocity(b2Vec2(linearVelocity.x, linearVelocity.y));
	}
	Math::vec2 Scene::RigidBody2D_GetLinearVelocity(UUID entityID)
	{
		ECS::Entity entity = m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<Physics2D::RigidBody2D>());
		Physics2D::RigidBody2D& rigidBody2DComp = entity.GetComponent<Physics2D::RigidBody2D>();
		b2Body* body = (b2Body*)rigidBody2DComp.m_RuntimeBody;
		const b2Vec2& linearVelocity = body->GetLinearVelocity();
		return Math::vec2(linearVelocity.x, linearVelocity.y);
	}
	void Scene::SetCustomComponentField(UUID entityID, Assets::AssetHandle projectComponentID, uint64_t fieldLocation, void* value)
	{
		// Get the indicated entity
		ECS::Entity currentEntity = m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(currentEntity);

		// Get the indicated custom component
		Assets::AssetRef<ECSInternal::CustomComponent> projectComponent = Assets::s_CustomComponentManager.GetAssetByHandle(projectComponentID);
		KG_ASSERT(projectComponent);
		KG_ASSERT(fieldLocation < projectComponent->m_DataOffsets.size());

		// Set indicated field value
		uint8_t* componentDataRef = (uint8_t*)currentEntity.GetCustomComponentData(projectComponentID);

		// Get field data pointer
		uint8_t* fieldDataRef = componentDataRef + projectComponent->m_DataOffsets.at(fieldLocation);

		// Set the data
		Utility::TransferDataForWrappedVarBuffer(projectComponent->m_DataTypes.at(fieldLocation), value, fieldDataRef);
	}
	void* Scene::GetCustomComponentField(UUID entityID, Assets::AssetHandle projectComponentID, uint64_t fieldLocation)
	{
		// Get the indicated entity
		ECS::Entity currentEntity = m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(currentEntity);

		// Get the indicated custom component
		Assets::AssetRef<ECSInternal::CustomComponent> projectComponent = 
			Assets::s_CustomComponentManager.GetAssetByHandle(projectComponentID);
		KG_ASSERT(projectComponent);
		KG_ASSERT(fieldLocation < projectComponent->m_DataOffsets.size());

		// Set indicated field value
		uint8_t* componentDataRef = (uint8_t*)currentEntity.GetCustomComponentData(projectComponentID);

		// Get field data pointer
		uint8_t* fieldDataRef = componentDataRef + projectComponent->m_DataOffsets.at(fieldLocation);

		// Get the data
		return fieldDataRef;
	}
}