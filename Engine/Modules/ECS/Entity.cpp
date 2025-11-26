#include "kgpch.h"

#include "Modules/ECS/Entity.h"
#include "Modules/ECSInternal/Assets/CustomComponent.h"


#include "Modules/Core/Components/Transform.h"
#include "Modules/Core/Components/Tag.h"
#include "Modules/Scripting/Components/OnCreate.h"
#include "Modules/Scripting/Components/OnUpdate.h"
#include "Modules/States/Components/StateMachine.h"
#include "Modules/Particles/Components/ParticleEmitter.h"
#include "Modules/Cameras/Components/CameraComponent.h"
#include "Modules/Physics2D/Components/BoxCollider2D.h"
#include "Modules/Physics2D/Components/CircleCollider2D.h"
#include "Modules/Physics2D/Components/RigidBody2D.h"
#include "Modules/Rendering/Components/ShapeComponent.h"

namespace Kargono::ECS
{
	void Entity::Serialize(void* context)
	{
		// Get serialization context and serializer
		ECS::SerializeEntityContext* entityContext{ (ECS::SerializeEntityContext*)context };
		KG_ASSERT(entityContext);
		KG_ASSERT(entityContext->m_Serializer);
		YAML::Emitter& out{ *entityContext->m_Serializer };

		out << YAML::BeginMap; // Entity Map
		out << YAML::Key << "UUID" << YAML::Value << static_cast<uint64_t>(m_UniqueID);

		// Create serialization componentContext
		ECSInternal::SerializeComponentContext componentContext;
		componentContext.m_Serializer = &out;

		if (HasComponent<Tag>())
		{
			Tag& tag = GetComponent<Tag>();
			out << YAML::Key << Modules::GetTypeName<Tag>();
			tag.Serialize((void*)&componentContext);
		}
		if (HasComponent<Scripting::OnUpdate>())
		{
			Scripting::OnUpdate& comp = GetComponent<Scripting::OnUpdate>();
			comp.Serialize((void*)&componentContext);
		}
		if (HasComponent<Scripting::OnCreate>())
		{
			Scripting::OnCreate& comp =
				GetComponent<Scripting::OnCreate>();
			out << YAML::Key << Modules::GetTypeName<Scripting::OnCreate>();
			comp.Serialize((void*)&componentContext);
		}
		if (HasComponent<Transform>())
		{
			Kargono::Transform& transform =
				GetComponent<Kargono::Transform>();
			out << YAML::Key << Modules::GetTypeName<Kargono::Transform>();
			transform.Serialize((void*)&componentContext);
		}

		if (HasComponent<States::StateMachine>())
		{
			States::StateMachine& aiStateComp = GetComponent<States::StateMachine>();
			out << YAML::Key << Modules::GetTypeName<States::State>();
			aiStateComp.Serialize((void*)&componentContext);
		}

		if (HasComponent<Particles::ParticleEmitter>())
		{
			Particles::ParticleEmitter& particleEmitterComp =
				GetComponent<Particles::ParticleEmitter>();
			out << YAML::Key << Modules::GetTypeName<Particles::ParticleEmitter>();
			particleEmitterComp.Serialize((void*)&componentContext);
		}

		if (HasComponent<Cameras::CameraComponent>())
		{
			Cameras::CameraComponent& cameraComponent =
				GetComponent<Cameras::CameraComponent>();
			out << YAML::Key << Modules::GetTypeName<Cameras::CameraComponent>();
			cameraComponent.Serialize((void*)&componentContext);
		}

		if (HasComponent<Rendering::ShapeComponent>())
		{
			Rendering::ShapeComponent& shapeComponent =
				GetComponent<Rendering::ShapeComponent>();
			out << YAML::Key << Modules::GetTypeName<Rendering::ShapeComponent>();
			shapeComponent.Serialize((void*)&componentContext);
		}

		if (HasComponent<Physics2D::RigidBody2D>())
		{
			Physics2D::RigidBody2D& rb2dComponent =
				GetComponent<Physics2D::RigidBody2D>();
			out << YAML::Key << Modules::GetTypeName<Physics2D::RigidBody2D>();
			rb2dComponent.Serialize((void*)&componentContext);
		}

		if (HasComponent<Physics2D::BoxCollider2D>())
		{
			Physics2D::BoxCollider2D& bc2dComponent =
				GetComponent<Physics2D::BoxCollider2D>();
			out << YAML::Key << Modules::GetTypeName<Physics2D::BoxCollider2D>();
			bc2dComponent.Serialize((void*)&componentContext);
		}

		if (HasComponent<Physics2D::CircleCollider2D>())
		{
			Physics2D::CircleCollider2D& cc2dComponent =
				GetComponent<Physics2D::CircleCollider2D>();
			out << YAML::Key << Modules::GetTypeName<Physics2D::CircleCollider2D>();
			cc2dComponent.Serialize((void*)&componentContext);
		}

		// Handle all custom components
		for (auto& [handle, asset] : Assets::s_CustomComponentManager.GetAssetRegistry())
		{
			if (!HasCustomComponentData(handle))
			{
				continue;
			}
			Assets::AssetRef<ECSInternal::CustomComponent> projectComponent = 
				Assets::s_CustomComponentManager.GetAssetByHandle(handle);
			uint8_t* componentRef = (uint8_t*)GetCustomComponentData(handle);

			out << YAML::Key << projectComponent->m_Name + "Component";
			out << YAML::BeginMap; // Component Map
			for (size_t iteration{ 0 }; iteration < projectComponent->m_DataOffsets.size(); iteration++)
			{
				Utility::SerializeWrappedVarType(out,
					projectComponent->m_DataTypes.at(iteration),
					projectComponent->m_DataNames.at(iteration).CString(),
					componentRef + projectComponent->m_DataOffsets.at(iteration));
			}
			out << YAML::EndMap; // Component Map
		}

		out << YAML::EndMap; // Entity
	}

	void Entity::Deserialize(void* context)
	{
		// Get the context
		ECS::DeserializeEntityContext* entityContext{ (ECS::DeserializeEntityContext*)context};
		KG_ASSERT(entityContext);
		YAML::Node& entityNode{ *entityContext->m_Node };

		// Deserialize the entity
		ECSInternal::DeserializeComponentContext componentContext;
		componentContext.m_Node = entityContext->m_Node;

		// Get unique ID
		m_UniqueID = entityNode["UUID"].as<uint64_t>();

		// Create deserialization componentContext
		YAML::Node tagNode = entityNode[Modules::GetTypeName<Tag>()];
		if (tagNode)
		{
			Tag& tagComp = GetComponent<Tag>();
			tagComp.Deserialize((void*)&tagNode);
		}

		YAML::Node transformComponent = entityNode[Modules::GetTypeName<Kargono::Transform>()];
		if (transformComponent)
		{
			Kargono::Transform& tc = GetComponent<Kargono::Transform>();
			tc.Deserialize((void*)&componentContext);
		}

		YAML::Node onUpdateNode = entityNode[Modules::GetTypeName<Scripting::OnUpdate>()];
		if (onUpdateNode)
		{
			Scripting::OnUpdate& component = AddComponent<Scripting::OnUpdate>();
			component.Deserialize((void*)&componentContext);
		}

		YAML::Node aiStateNode = entityNode[Modules::GetTypeName<States::State>()];
		if (aiStateNode)
		{
			States::StateMachine& component = AddComponent<States::StateMachine>();
			component.Deserialize((void*)&componentContext);
		}

		YAML::Node onCreateNode = entityNode[Modules::GetTypeName<Scripting::OnCreate>()];
		if (onCreateNode)
		{
			Scripting::OnCreate& component = AddComponent<Scripting::OnCreate>();
			component.Deserialize((void*)&componentContext);
		}

		YAML::Node particleEmitterNode = entityNode[Modules::GetTypeName<Particles::ParticleEmitter>()];
		if (particleEmitterNode)
		{
			Particles::ParticleEmitter& component = AddComponent<Particles::ParticleEmitter>();
			component.Deserialize((void*)&componentContext);
		}

		YAML::Node cameraComponent = entityNode[Modules::GetTypeName<Cameras::CameraComponent>()];
		if (cameraComponent)
		{
			Cameras::CameraComponent& cc = AddComponent<Cameras::CameraComponent>();
			cc.Deserialize((void*)&componentContext);
		}

		YAML::Node shapeComponent = entityNode[Modules::GetTypeName<Rendering::Shape>()];
		if (shapeComponent)
		{
			Rendering::ShapeComponent& sc = AddComponent<Rendering::ShapeComponent>();
			sc.Deserialize((void*)&componentContext);
		}

		YAML::Node rigidbody2DComponent = entityNode[Modules::GetTypeName<Physics2D::RigidBody2D>()];
		if (rigidbody2DComponent)
		{
			Physics2D::RigidBody2D& rb2d = AddComponent<Physics2D::RigidBody2D>();
			rb2d.Deserialize((void*)&componentContext);
		}

		YAML::Node boxCollider2DComponent = entityNode[Modules::GetTypeName<Physics2D::BoxCollider2D>()];
		if (boxCollider2DComponent)
		{
			Physics2D::BoxCollider2D& bc2d = AddComponent<Physics2D::BoxCollider2D>();
			bc2d.Deserialize((void*)&componentContext);
		}

		YAML::Node circleCollider2DComponent = entityNode[Modules::GetTypeName<Physics2D::CircleCollider2D>()];
		if (circleCollider2DComponent)
		{
			Physics2D::CircleCollider2D& cc2d = AddComponent<Physics2D::CircleCollider2D>();
			cc2d.Deserialize((void*)&componentContext);
		}

		// Handle all custom components
		for (auto& [handle, asset] : Assets::s_CustomComponentManager.GetAssetRegistry())
		{
			Assets::AssetRef<ECSInternal::CustomComponent> projectComponent = 
				Assets::s_CustomComponentManager.GetAssetByHandle(handle);
			KG_ASSERT(projectComponent);

			YAML::Node projectComponentNode = entityNode[projectComponent->m_Name + "Component"];
			if (!projectComponentNode)
			{
				continue;
			}
			// Create and get custom component
			if (!HasCustomComponentData(handle))
			{
				AddCustomComponentData(handle);
			}
			uint8_t* componentRef = (uint8_t*)GetCustomComponentData(handle);

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

	Entity::Entity(ECSInternal::EntityID handle, Registry* registry)
	{
		KG_ASSERT(registry);

		if (!registry->m_Registry.HasEntity(handle))
		{
			KG_WARN("Invalid entity trying to be created with handle {0} and registry pointer {1}", (int32_t)handle, (void*)registry);
			return;
		}

		m_RegistryEntityID = handle;
		m_Registry = registry;
	}
	void Entity::AddCustomComponentData(Assets::AssetHandle componentHandle)
	{
		// Get the custom component
		Assets::AssetRef<ECSInternal::CustomComponent> component = Assets::s_CustomComponentManager.GetAssetByHandle(componentHandle);
		KG_ASSERT(component);
		KG_ASSERT(component->m_Identifier != ECSInternal::k_InvalidComponentIdentifier);
		if (component->m_ComponentSize == 0)
		{
			return;
		}

		uint8_t* componentReference
		{
			(uint8_t*)m_Registry->m_Registry.CreateComponent(m_RegistryEntityID, component->m_Identifier)
		};
		KG_ASSERT(componentReference);

		// Set initial values of data
		for (size_t iteration{0}; iteration < component->m_DataOffsets.size(); iteration++)
		{
			Utility::InitializeDataForWrappedVarBuffer(
				component->m_DataTypes.at(iteration),
				componentReference + component->m_DataOffsets.at(iteration));
		}

	}
	void* Entity::GetCustomComponentData(Assets::AssetHandle componentHandle)
	{
		Assets::AssetRef<ECSInternal::CustomComponent> component = 
			Assets::s_CustomComponentManager.GetAssetByHandle(componentHandle);
		KG_ASSERT(component);
		KG_ASSERT(component->m_Identifier != ECSInternal::k_InvalidComponentIdentifier);
		if (component->m_ComponentSize == 0)
		{
			return nullptr;
		} 

		return m_Registry->m_Registry.GetComponent(m_RegistryEntityID, component->m_Identifier);
	}
	bool Entity::HasCustomComponentData(Assets::AssetHandle componentHandle)
	{
		Assets::AssetRef<ECSInternal::CustomComponent> component = 
			Assets::s_CustomComponentManager.GetAssetByHandle(componentHandle);
		KG_ASSERT(component);
		if (component->m_ComponentSize == 0)
		{
			return false;
		}
		return m_Registry->m_Registry.HasComponent(m_RegistryEntityID, component->m_Identifier);
	}
	void Entity::RemoveCustomComponentData(Assets::AssetHandle componentHandle)
	{
		Assets::AssetRef<ECSInternal::CustomComponent> component = Assets::s_CustomComponentManager.GetAssetByHandle(componentHandle);
		KG_ASSERT(component);
		KG_ASSERT(component->m_Identifier != ECSInternal::k_InvalidComponentIdentifier);
		if (component->m_ComponentSize == 0)
		{
			return;
		}

		m_Registry->m_Registry.RemoveComponent(m_RegistryEntityID, component->m_Identifier);
	}
}
