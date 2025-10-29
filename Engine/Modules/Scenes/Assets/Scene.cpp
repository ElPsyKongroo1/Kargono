#include "kgpch.h"

#include "Modules/Scenes/Assets/Scene.h"
#include "Modules/ECSInternal/Assets/CustomComponent.h"
#include "Modules/ECS/Entity.h"
#include "Modules/Physics2D/Physics2D.h"
#include "Modules/Core/Engine.h"
#include "Modules/Input/InputService.h"
#include "Modules/InputMap/Assets/InputMap.h"
#include "Modules/Rendering/Shader.h"
#include "Modules/Events/SceneEvent.h"
#include "Kargono/Projects/Project.h"
#include "Modules/Assets/AssetService.h"
#include "Modules/Particles/ParticleContext.h"

#include "Modules/Core/Components/IDComponent.h"
#include "Modules/Core/Components/TagComponent.h"
#include "Modules/Core/Components/TransformComponent.h"
#include "Modules/Scripting/Components/OnCreateComponent.h"
#include "Modules/Scripting/Components/OnUpdateComponent.h"
#include "Modules/Cameras/Components/CameraComponent.h"
#include "Modules/Rendering/Components/ShapeComponent.h"
#include "Modules/Physics2D/Components/BoxCollider2DComponent.h"
#include "Modules/Physics2D/Components/RigidBody2DComponent.h"
#include "Modules/Physics2D/Components/CircleCollider2DComponent.h"
#include "Modules/Particles/Components/ParticleEmitterComponent.h"
#include "Modules/AI/Components/AIStateComponent.h"

#include "Modules/Assets/AssetService.h"

#include "Modules/ECSInternal/Assets/CustomComponent.h"
#include "Modules/Scenes/Assets/Scene.h"
#include "Modules/ECS/Entity.h"

namespace Kargono::Utility
{
	static bool SerializeEntity(YAML::Emitter& out, ECS::Entity entity)
	{
		KG_ASSERT(entity.HasComponent<IDComponent>(), "Entity does not have a component");
		out << YAML::BeginMap; // Entity Map
		out << YAML::Key << "Entity" << YAML::Value << static_cast<uint64_t>(entity.GetUUID());

		// Create serialization context
		ECSInternal::SerializeComponentContext context;
		context.m_Serializer = &out;

		if (entity.HasComponent<TagComponent>())
		{
			TagComponent& tag = entity.GetComponent<TagComponent>();
			out << YAML::Key << GetTypeName<TagComponent>();
			tag.Serialize((void*)&context);
		}
		if (entity.HasComponent<Scripting::OnUpdateComponent>())
		{
			Scripting::OnUpdateComponent& comp = entity.GetComponent<Scripting::OnUpdateComponent>();
			comp.Serialize((void*)&context);
		}
		if (entity.HasComponent<Scripting::OnCreateComponent>())
		{
			Scripting::OnCreateComponent& comp = 
				entity.GetComponent<Scripting::OnCreateComponent>();
			out << YAML::Key << GetTypeName<Scripting::OnCreateComponent>();
			comp.Serialize((void*)&context);
		}
		if (entity.HasComponent<TransformComponent>())
		{
			TransformComponent& transform = 
				entity.GetComponent<TransformComponent>();
			out << YAML::Key << GetTypeName<TransformComponent>();
			transform.Serialize((void*)&context);
		}

		if (entity.HasComponent<AI::AIStateComponent>())
		{
			AI::AIStateComponent& aiStateComp = 
				entity.GetComponent<AI::AIStateComponent>();
			out << YAML::Key << GetTypeName<AI::AIStateComponent>();
			aiStateComp.Serialize((void*)&context);
		}

		if (entity.HasComponent<Particles::ParticleEmitterComponent>())
		{
			Particles::ParticleEmitterComponent& particleEmitterComp =
				entity.GetComponent<Particles::ParticleEmitterComponent>();
			out << YAML::Key << GetTypeName<Particles::ParticleEmitterComponent>();
			particleEmitterComp.Serialize((void*)&context);
		}

		if (entity.HasComponent<Cameras::CameraComponent>())
		{
			Cameras::CameraComponent& cameraComponent = 
				entity.GetComponent<Cameras::CameraComponent>();
			out << YAML::Key << GetTypeName<Cameras::CameraComponent>();
			cameraComponent.Serialize((void*)&context);
		}

		if (entity.HasComponent<Rendering::ShapeComponent>())
		{
			Rendering::ShapeComponent& shapeComponent = 
				entity.GetComponent<Rendering::ShapeComponent>();
			out << YAML::Key << GetTypeName<Rendering::ShapeComponent>();
			shapeComponent.Serialize((void*)&context);
		}

		if (entity.HasComponent<Physics2D::Rigidbody2DComponent>())
		{
			Physics2D::Rigidbody2DComponent& rb2dComponent = 
				entity.GetComponent<Physics2D::Rigidbody2DComponent>();
			out << YAML::Key << GetTypeName<Physics2D::Rigidbody2DComponent>();
			rb2dComponent.Serialize((void*)&context);
		}

		if (entity.HasComponent<Physics2D::BoxCollider2DComponent>())
		{
			Physics2D::BoxCollider2DComponent& bc2dComponent = 
				entity.GetComponent<Physics2D::BoxCollider2DComponent>();
			out << YAML::Key << GetTypeName<Physics2D::BoxCollider2DComponent>();
			bc2dComponent.Serialize((void*)&context);
		}

		if (entity.HasComponent<Physics2D::CircleCollider2DComponent>())
		{
			Physics2D::CircleCollider2DComponent& cc2dComponent = 
				entity.GetComponent<Physics2D::CircleCollider2DComponent>();
			out << YAML::Key << GetTypeName<Physics2D::CircleCollider2DComponent>();
			cc2dComponent.Serialize((void*)&context);
		}

		// Handle all custom components
		for (auto& [handle, asset] : Assets::AssetService::GetCustomComponentRegistry())
		{
			if (!entity.HasCustomComponentData(handle))
			{
				continue;
			}
			Ref<ECSInternal::CustomComponent> projectComponent = Assets::AssetService::GetCustomComponent(handle);
			uint8_t* componentRef = (uint8_t*)entity.GetCustomComponentData(handle);

			out << YAML::Key << projectComponent->m_Name + "Component";
			out << YAML::BeginMap; // Component Map
			for (size_t iteration{ 0 }; iteration < projectComponent->m_DataOffsets.size(); iteration++)
			{
				SerializeWrappedVarType(out,
					projectComponent->m_DataTypes.at(iteration),
					projectComponent->m_DataNames.at(iteration).CString(),
					componentRef + projectComponent->m_DataOffsets.at(iteration));
			}
			out << YAML::EndMap; // Component Map
		}

		out << YAML::EndMap; // Entity
		return true;
	}
}

namespace Kargono::Scenes
{
	void Scene::CreateAssetFromName(Assets::Metadata& metadata)
	{
		// Create default scene
		Scene defaultScene{};

		// Save Binary into File
		Assets::SerializeAssetContext serializeContext{};
		serializeContext.m_AssetMetadata = &metadata;
		defaultScene.Serialize((void*)&serializeContext);
	}
	void Scene::Serialize(void* context)
	{
		// Get asset context
		Assets::SerializeAssetContext* assetContext = (Assets::SerializeAssetContext*)context;
		KG_ASSERT(assetContext, "Context cannot be null");
		Assets::Metadata* metadata{ assetContext->m_AssetMetadata };
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get context fields
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath<Scene>() };

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

			bool success = Utility::SerializeEntity(out, entity);
			if (!success)
			{
				submitScene = false;
			}
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
		Assets::DeserializeAssetContext* deserializeContext = (Assets::DeserializeAssetContext*)context;
		KG_ASSERT(deserializeContext, "Context cannot be null");
		Assets::Metadata* metadata = deserializeContext->m_AssetMetadata;
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get asset path
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath<Scene>() };

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
			for (YAML::Node entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				// Create deserialization context
				ECSInternal::DeserializeComponentContext context;
				context.m_Node = &entity;

				// Get the entity name
				ECS::Entity deserializedEntity = m_EntityRegistry.CreateEntityWithUUID(uuid);

				YAML::Node tagNode = entity[GetTypeName<TagComponent>()];
				if (tagNode)
				{
					TagComponent& tagComp = deserializedEntity.GetComponent<TagComponent>();
					tagComp.Deserialize((void*)&tagNode);
				}

				YAML::Node transformComponent = entity[GetTypeName<TransformComponent>()];
				if (transformComponent)
				{
					TransformComponent& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Deserialize((void*)&context);
				}

				YAML::Node onUpdateNode = entity[GetTypeName<Scripting::OnUpdateComponent>()];
				if (onUpdateNode)
				{
					Scripting::OnUpdateComponent& component = deserializedEntity.AddComponent<Scripting::OnUpdateComponent>();
					component.Deserialize((void*)&context);
				}

				YAML::Node aiStateNode = entity[GetTypeName<AI::AIStateComponent>()];
				if (aiStateNode)
				{
					AI::AIStateComponent& component = deserializedEntity.AddComponent<AI::AIStateComponent>();
					component.Deserialize((void*)&context);
				}

				YAML::Node onCreateNode = entity[GetTypeName<Scripting::OnCreateComponent>()];
				if (onCreateNode)
				{
					Scripting::OnCreateComponent& component = deserializedEntity.AddComponent<Scripting::OnCreateComponent>();
					component.Deserialize((void*)&context);
				}

				YAML::Node particleEmitterNode = entity[GetTypeName<Particles::ParticleEmitterComponent>()];
				if (particleEmitterNode)
				{
					Particles::ParticleEmitterComponent& component = deserializedEntity.AddComponent<Particles::ParticleEmitterComponent>();
					component.Deserialize((void*)&context);
				}

				YAML::Node cameraComponent = entity[GetTypeName<Cameras::CameraComponent>()];
				if (cameraComponent)
				{
					Cameras::CameraComponent& cc = deserializedEntity.AddComponent<Cameras::CameraComponent>();
					cc.Deserialize((void*)&context);
				}

				YAML::Node shapeComponent = entity[GetTypeName<Rendering::ShapeComponent>()];
				if (shapeComponent)
				{
					Rendering::ShapeComponent& sc = deserializedEntity.AddComponent<Rendering::ShapeComponent>();
					sc.Deserialize((void*)&context);
				}

				YAML::Node rigidbody2DComponent = entity[GetTypeName<Physics2D::Rigidbody2DComponent>()];
				if (rigidbody2DComponent)
				{
					Physics2D::Rigidbody2DComponent& rb2d = deserializedEntity.AddComponent<Physics2D::Rigidbody2DComponent>();
					rb2d.Deserialize((void*)&context);
				}

				YAML::Node boxCollider2DComponent = entity[GetTypeName<Physics2D::BoxCollider2DComponent>()];
				if (boxCollider2DComponent)
				{
					Physics2D::BoxCollider2DComponent& bc2d = deserializedEntity.AddComponent<Physics2D::BoxCollider2DComponent>();
					bc2d.Deserialize((void*)&context);
				}

				YAML::Node circleCollider2DComponent = entity[GetTypeName<Physics2D::CircleCollider2DComponent>()];
				if (circleCollider2DComponent)
				{
					Physics2D::CircleCollider2DComponent& cc2d = deserializedEntity.AddComponent<Physics2D::CircleCollider2DComponent>();
					cc2d.Deserialize((void*)&context);
				}

				// Handle all custom components
				for (auto& [handle, asset] : Assets::AssetService::GetCustomComponentRegistry())
				{
					Ref<ECSInternal::CustomComponent> projectComponent = Assets::AssetService::GetCustomComponent(handle);
					KG_ASSERT(projectComponent);

					YAML::Node projectComponentNode = entity[projectComponent->m_Name + "Component"];
					if (!projectComponentNode)
					{
						continue;
					}
					// Create and get custom component
					if (!deserializedEntity.HasCustomComponentData(handle))
					{
						deserializedEntity.AddCustomComponentData(handle);
					}
					uint8_t* componentRef = (uint8_t*)deserializedEntity.GetCustomComponentData(handle);

					// Load in component data from disk
					for (size_t iteration{ 0 }; iteration < projectComponent->m_DataOffsets.size(); iteration++)
					{
						Utility::DeserializeWrappedVarType(projectComponentNode,
							projectComponent->m_DataTypes.at(iteration),
							projectComponent->m_DataNames.at(iteration).CString(),
							componentRef + projectComponent->m_DataOffsets.at(iteration));
					}
				}
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
	bool Scene::RemoveScript(Ref<Scenes::Scene> sceneRef, Assets::AssetHandle scriptHandle)
	{
		bool sceneModified{ false };
		// Handle UI level function pointers
		// OnUpdate
		auto onUpdateView = sceneRef->m_EntityRegistry.GetView<Scripting::OnUpdateComponent>();
		for (ECSInternal::EntityID enttEntity : onUpdateView)
		{
			ECS::Entity currentEntity{ sceneRef->m_EntityRegistry.GetEntityByECSID(enttEntity) };
			Scripting::OnUpdateComponent& component = currentEntity.GetComponent<Scripting::OnUpdateComponent>();
			if (component.m_OnUpdateScriptHandle == scriptHandle)
			{
				component.m_OnUpdateScriptHandle = Assets::k_EmptyHandle;
				component.m_OnUpdateScript = nullptr;
				sceneModified = true;
			}
		}

		// OnCreate
		auto onCreateView = sceneRef->m_EntityRegistry.GetView<Scripting::OnCreateComponent>();
		for (ECSInternal::EntityID enttEntity : onCreateView)
		{
			ECS::Entity currentEntity{ sceneRef->m_EntityRegistry.GetEntityByECSID(enttEntity) };
			Scripting::OnCreateComponent& component = currentEntity.GetComponent<Scripting::OnCreateComponent>();
			if (component.m_OnCreateScriptHandle == scriptHandle)
			{
				component.m_OnCreateScriptHandle = Assets::k_EmptyHandle;
				component.m_OnCreateScript = nullptr;
				sceneModified = true;
			}
		}

		// Rigidbody
		auto rigidBodyView = sceneRef->m_EntityRegistry.GetView<Physics2D::Rigidbody2DComponent>();
		for (ECSInternal::EntityID enttEntity : rigidBodyView)
		{
			ECS::Entity currentEntity{ sceneRef->m_EntityRegistry.GetEntityByECSID(enttEntity) };
			Physics2D::Rigidbody2DComponent& component = currentEntity.GetComponent<Physics2D::Rigidbody2DComponent>();

			if (component.m_OnCollisionStartScriptHandle == scriptHandle)
			{
				component.m_OnCollisionStartScriptHandle = Assets::k_EmptyHandle;
				component.m_OnCollisionStartScript = nullptr;
				sceneModified = true;
			}

			if (component.m_OnCollisionEndScriptHandle == scriptHandle)
			{
				component.m_OnCollisionEndScriptHandle = Assets::k_EmptyHandle;
				component.m_OnCollisionEndScript = nullptr;
				sceneModified = true;
			}
		}
		return sceneModified;
	}
	bool Scene::RemoveAIState(Ref<Scenes::Scene> sceneRef, Assets::AssetHandle aiStateHandle)
	{
		bool aiStateModified{ false };

		// Check for AIState
		auto aiStateView = sceneRef->m_EntityRegistry.GetView<AI::AIStateComponent>();
		for (ECSInternal::EntityID enttEntity : aiStateView)
		{
			ECS::Entity currentEntity{ sceneRef->m_EntityRegistry.GetEntityByECSID(enttEntity) };
			AI::AIStateComponent& component = currentEntity.GetComponent<AI::AIStateComponent>();

			if (component.m_CurrentStateHandle == aiStateHandle)
			{
				component.m_CurrentStateHandle = Assets::k_EmptyHandle;
				component.m_CurrentStateReference = nullptr;
				aiStateModified = true;
			}
			if (component.m_GlobalStateHandle == aiStateHandle)
			{
				component.m_GlobalStateHandle = Assets::k_EmptyHandle;
				component.m_GlobalStateReference = nullptr;
				aiStateModified = true;
			}
			if (component.m_PreviousStateHandle == aiStateHandle)
			{
				component.m_PreviousStateHandle = Assets::k_EmptyHandle;
				component.m_PreviousStateReference = nullptr;
				aiStateModified = true;
			}
		}
		return aiStateModified;
	}
	bool Scene::RemoveCustomComponent(Ref<Scenes::Scene> sceneRef, Assets::AssetHandle projectCompHandle)
	{
		KG_ASSERT(sceneRef);

		ECS::Registry& registry = sceneRef->m_EntityRegistry;

		// Check if any components are being removed
		size_t componentCount = registry.GetCustomComponentCount(projectCompHandle);

		// Clear component registry
		registry.UnRegisterCustomComponent(projectCompHandle);

		return componentCount > 0;
	}
	bool Scene::RemoveEmitterConfig(Ref<Scenes::Scene> sceneRef, Assets::AssetHandle emitterConfigHandle)
	{
		bool emitterConfigModified{ false };

		// Check for emitterConfig
		auto emitterConfigView = sceneRef->m_EntityRegistry.GetView<Particles::ParticleEmitterComponent>();
		for (ECSInternal::EntityID enttEntity : emitterConfigView)
		{
			ECS::Entity currentEntity{ sceneRef->m_EntityRegistry.GetEntityByECSID(enttEntity) };
			Particles::ParticleEmitterComponent& component = currentEntity.GetComponent<Particles::ParticleEmitterComponent>();

			if (component.m_EmitterConfigHandle == emitterConfigHandle)
			{
				component.m_EmitterConfigHandle = Assets::k_EmptyHandle;
				component.m_EmitterConfigRef = nullptr;
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
		m_EntityRegistry.RegisterComponent<TagComponent>();
		m_EntityRegistry.RegisterComponent<TransformComponent>();
		m_EntityRegistry.RegisterComponent<IDComponent>();
		m_EntityRegistry.RegisterComponent<Scripting::OnCreateComponent>();
		m_EntityRegistry.RegisterComponent<Scripting::OnUpdateComponent>();
		m_EntityRegistry.RegisterComponent<Cameras::CameraComponent>();
		m_EntityRegistry.RegisterComponent<Rendering::ShapeComponent>();
		m_EntityRegistry.RegisterComponent<Physics2D::Rigidbody2DComponent>();
		m_EntityRegistry.RegisterComponent<Physics2D::BoxCollider2DComponent>();
		m_EntityRegistry.RegisterComponent<Physics2D::CircleCollider2DComponent>();
		m_EntityRegistry.RegisterComponent<Particles::ParticleEmitterComponent>();
		m_EntityRegistry.RegisterComponent<AI::AIStateComponent>();

		// Custom Components
		for (auto& [handle, info] : Assets::AssetService::GetCustomComponentRegistry())
		{
			m_EntityRegistry.RegisterCustomComponent(handle);
		}

	}
	
	void Scene::OnRuntimeStart()
	{
		m_IsRunning = true;

		// Invoke OnCreate
		auto classInstanceView = m_EntityRegistry.GetView<Scripting::OnCreateComponent>();
		for (ECSInternal::EntityID id : classInstanceView)
		{
			ECS::Entity entity = { id, &m_EntityRegistry };
			Scripting::OnCreateComponent& component = entity.GetComponent<Scripting::OnCreateComponent>();
			Assets::AssetHandle scriptHandle = component.m_OnCreateScriptHandle;
			if (scriptHandle != Assets::k_EmptyHandle)
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
			auto view = m_EntityRegistry.m_Registry.GetFlatView<TransformComponent, Rendering::ShapeComponent>();
			for (ECSInternal::EntityID entity : view)
			{
				TransformComponent& transform = m_EntityRegistry.m_Registry.GetComponent<TransformComponent>(entity).value();
				Rendering::ShapeComponent& shape = m_EntityRegistry.m_Registry.GetComponent<Rendering::ShapeComponent>(entity).value();
				inputSpec.m_Shader = shape.m_Shader;
				inputSpec.m_Buffer = shape.m_ShaderData;
				inputSpec.m_Entity = static_cast<uint32_t>(entity);
				inputSpec.m_EntityRegistry = &m_EntityRegistry.m_Registry;
				inputSpec.m_ShapeComponent = &shape;
				inputSpec.m_TransformMatrix = transform.GetTransform();

				for (const auto& PerObjectSceneFunction : shape.m_Shader->GetFillDataObjectScene())
				{
					PerObjectSceneFunction(inputSpec);
				}

				Rendering::RenderingService::SubmitDataToRenderer(inputSpec);
			}
		}
		Rendering::RenderingService::EndScene();
	}
	void Scene::OnUpdate(Timestep ts)
	{
		// Invoke OnUpdate
		auto view = m_EntityRegistry.m_Registry.GetFlatView<Scripting::OnUpdateComponent>();
		for (ECSInternal::EntityID enttEntityID : view)
		{
			ECS::Entity entity = { enttEntityID, &m_EntityRegistry };
			Scripting::OnUpdateComponent& component = entity.GetComponent<Scripting::OnUpdateComponent>();
			Assets::AssetHandle scriptHandle = component.m_OnUpdateScriptHandle;
			if (scriptHandle != Assets::k_EmptyHandle)
			{
				Utility::CallWrapped<WrappedVoidEntityFloat>(component.m_OnUpdateScript->m_Function, entity.GetUUID(), ts);
			}
		}
	}

	Math::vec3 Scene::TransformComponentGetTranslation(UUID entityID)
	{
		ECS::Entity entity = m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<TransformComponent>());
		return entity.GetComponent<TransformComponent>().m_Translation;
	}
	void Scene::TransformComponentSetTranslation(UUID entityID, Math::vec3 newTranslation)
	{
		ECS::Entity entity = m_EntityRegistry.GetEntityByUUID(entityID);
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
	std::string_view Scene::TagComponentGetTag(UUID entityID)
	{
		ECS::Entity entity = m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<TagComponent>());
		TagComponent& tagComponent = entity.GetComponent<TagComponent>();

		// TODO: REMOVE THE HELL OUT OF THIS, TEMPORARY FIX TO MAKE IT COMPILE, I HATE THIS, I HATE IT SO MUCH
		// JUST USE A STRING VIEW YEAH??
		static std::string tagBuffer;
		tagBuffer = tagComponent.m_Tag.CString();
		return tagBuffer;
	}
	void Scene::Rigidbody2DComponent_SetLinearVelocity(UUID entityID, Math::vec2 linearVelocity)
	{
		ECS::Entity entity = m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<Physics2D::Rigidbody2DComponent>());
		Physics2D::Rigidbody2DComponent& rigidBody2DComp = entity.GetComponent<Physics2D::Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rigidBody2DComp.m_RuntimeBody;
		body->SetLinearVelocity(b2Vec2(linearVelocity.x, linearVelocity.y));
	}
	Math::vec2 Scene::Rigidbody2DComponent_GetLinearVelocity(UUID entityID)
	{
		ECS::Entity entity = m_EntityRegistry.GetEntityByUUID(entityID);
		KG_ASSERT(entity);
		KG_ASSERT(entity.HasComponent<Physics2D::Rigidbody2DComponent>());
		Physics2D::Rigidbody2DComponent& rigidBody2DComp = entity.GetComponent<Physics2D::Rigidbody2DComponent>();
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
	void* Scene::GetCustomComponentField(UUID entityID, Assets::AssetHandle projectComponentID, uint64_t fieldLocation)
	{
		// Get the indicated entity
		ECS::Entity currentEntity = m_EntityRegistry.GetEntityByUUID(entityID);
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
}