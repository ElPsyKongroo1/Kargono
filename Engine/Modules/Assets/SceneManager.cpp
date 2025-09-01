#include "kgpch.h"

#include "Modules/Assets/AssetService.h"
#include "Modules/Assets/SceneManager.h"

#include "Modules/Scripting/Components/OnUpdateComponent.h"
#include "Modules/Scripting/Components/OnCreateComponent.h"
#include "Modules/Core/Components/TransformComponent.h"
#include "Modules/AI/Components/AIStateComponent.h"
#include "Modules/Particles/Components/ParticleEmitterComponent.h"
#include "Modules/Rendering/Components/CameraComponent.h"
#include "Modules/Rendering/Components/ShapeComponent.h"

#include "Modules/Physics2D/Components/BoxCollider2DComponent.h"	
#include "Modules/Physics2D/Components/CircleCollider2DComponent.h"	
#include "Modules/Physics2D/Components/RigidBody2DComponent.h"	

#include "Modules/ECS/ProjectComponent.h"
#include "Kargono/Scenes/Scene.h"
#include "Modules/ECS/Entity.h"

namespace Kargono::Utility
{
	static bool SerializeEntity(YAML::Emitter& out, ECS::Entity entity)
	{
		KG_ASSERT(entity.HasComponent<IDComponent>(), "Entity does not have a component");
		out << YAML::BeginMap; // Entity Map
		out << YAML::Key << "Entity" << YAML::Value << static_cast<uint64_t>(entity.GetUUID());

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // Component Map
			TagComponent& tag = entity.GetComponent<TagComponent>();
			out << YAML::Key << "Tag" << YAML::Value << tag.m_Tag;
			out << YAML::Key << "Group" << YAML::Value << tag.m_Group;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<Scripting::OnUpdateComponent>())
		{
			out << YAML::Key << "OnUpdateComponent";
			out << YAML::BeginMap; // Component Map
			Scripting::OnUpdateComponent& component = entity.GetComponent<Scripting::OnUpdateComponent>();
			out << YAML::Key << "OnUpdateHandle" << YAML::Value << static_cast<uint64_t>(component.m_OnUpdateScriptHandle);
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<Scripting::OnCreateComponent>())
		{
			out << YAML::Key << "OnCreateComponent";
			out << YAML::BeginMap; // Component Map
			Scripting::OnCreateComponent& component = entity.GetComponent<Scripting::OnCreateComponent>();
			out << YAML::Key << "OnCreateHandle" << YAML::Value << static_cast<uint64_t>(component.m_OnCreateScriptHandle);
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // Component Map
			TransformComponent& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.m_Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.m_Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.m_Scale;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<AI::AIStateComponent>())
		{
			out << YAML::Key << "AIStateComponent";
			out << YAML::BeginMap; // Component Map
			AI::AIStateComponent& aiStateComp = entity.GetComponent<AI::AIStateComponent>();
			out << YAML::Key << "CurrentState" << YAML::Value << static_cast<uint64_t>(aiStateComp.m_CurrentStateHandle);
			out << YAML::Key << "PreviousState" << YAML::Value << static_cast<uint64_t>(aiStateComp.m_PreviousStateHandle);
			out << YAML::Key << "GlobalState" << YAML::Value << static_cast<uint64_t>(aiStateComp.m_GlobalStateHandle);
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<Particles::ParticleEmitterComponent>())
		{
			out << YAML::Key << "ParticleEmitterComponent";
			out << YAML::BeginMap; // Component Map
			Particles::ParticleEmitterComponent& emitterComp = entity.GetComponent<Particles::ParticleEmitterComponent>();
			out << YAML::Key << "EmitterHandle" << YAML::Value << static_cast<uint64_t>(emitterComp.m_EmitterConfigHandle);
			out << YAML::EndMap; // Component Map
		}


		if (entity.HasComponent<Rendering::CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // Component Map

			Rendering::CameraComponent& cameraComponent = entity.GetComponent<Rendering::CameraComponent>();
			Scenes::SceneCamera& camera = cameraComponent.m_Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap;
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();

			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap;

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.m_Primary;


			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<Rendering::ShapeComponent>())
		{
			out << YAML::Key << "ShapeComponent";
			out << YAML::BeginMap; // Component Map
			Rendering::ShapeComponent& shapeComponent = entity.GetComponent<Rendering::ShapeComponent>();
			out << YAML::Key << "CurrentShape" << YAML::Value << Utility::ShapeTypeToString(shapeComponent.m_CurrentShape);
			if (shapeComponent.m_VertexColors)
			{
				out << YAML::Key << "VertexColors" << YAML::Value << YAML::BeginSeq;
				for (const auto& color : *shapeComponent.m_VertexColors)
				{
					out << YAML::BeginMap;
					out << YAML::Key << "Color" << YAML::Value << color;
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;
			}
			if (shapeComponent.m_Texture)
			{
				out << YAML::Key << "TextureHandle" << YAML::Value << static_cast<uint64_t>(shapeComponent.m_TextureHandle);
			}

			static_assert(sizeof(uint8_t) * 20 == sizeof(Rendering::ShaderSpecification));
			if (shapeComponent.m_Shader)
			{
				// Add Shader Handle
				out << YAML::Key << "ShaderHandle" << YAML::Value << static_cast<uint64_t>(shapeComponent.m_ShaderHandle);
				// Add Shader Specification
				const Rendering::ShaderSpecification& shaderSpec = shapeComponent.m_Shader->GetSpecification();
				out << YAML::Key << "ShaderSpecification" << YAML::Value;
				out << YAML::BeginMap;
				out << YAML::Key << "ColorInputType" << YAML::Value << Utility::ColorInputTypeToString(shaderSpec.ColorInput);
				out << YAML::Key << "AddProjectionMatrix" << YAML::Value << shaderSpec.AddProjectionMatrix;
				out << YAML::Key << "AddEntityID" << YAML::Value << shaderSpec.AddEntityID;
				out << YAML::Key << "AddCircleShape" << YAML::Value << shaderSpec.AddCircleShape;
				out << YAML::Key << "TextureInput" << YAML::Value << Utility::TextureInputTypeToString(shaderSpec.TextureInput);
				out << YAML::Key << "DrawOutline" << YAML::Value << shaderSpec.DrawOutline;
				out << YAML::Key << "RenderType" << YAML::Value << Utility::RenderingTypeToString(shaderSpec.RenderType);

				out << YAML::EndMap;
				// Add Buffer
				out << YAML::Key << "Buffer" << YAML::Value << YAML::Binary(shapeComponent.m_ShaderData.Data, shapeComponent.m_ShaderData.Size);
			}
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<Physics2D::Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Component Map
			Physics2D::Rigidbody2DComponent& rb2dComponent = entity.GetComponent<Physics2D::Rigidbody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << Utility::RigidBody2DBodyTypeToString(rb2dComponent.m_Type);
			out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.m_FixedRotation;
			out << YAML::Key << "OnCollisionStartHandle" << YAML::Value << static_cast<uint64_t>(rb2dComponent.m_OnCollisionStartScriptHandle);
			out << YAML::Key << "OnCollisionEndHandle" << YAML::Value << static_cast<uint64_t>(rb2dComponent.m_OnCollisionEndScriptHandle);
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<Physics2D::BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // Component Map
			Physics2D::BoxCollider2DComponent& bc2dComponent = entity.GetComponent<Physics2D::BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.m_Offset;
			out << YAML::Key << "Size" << YAML::Value << bc2dComponent.m_Size;
			out << YAML::Key << "Density" << YAML::Value << bc2dComponent.m_Density;
			out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.m_Friction;
			out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.m_Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.m_RestitutionThreshold;
			out << YAML::Key << "IsSensor" << YAML::Value << bc2dComponent.m_IsSensor;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<Physics2D::CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; // Component Map
			Physics2D::CircleCollider2DComponent& cc2dComponent = entity.GetComponent<Physics2D::CircleCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << cc2dComponent.m_Offset;
			out << YAML::Key << "Radius" << YAML::Value << cc2dComponent.m_Radius;
			out << YAML::Key << "Density" << YAML::Value << cc2dComponent.m_Density;
			out << YAML::Key << "Friction" << YAML::Value << cc2dComponent.m_Friction;
			out << YAML::Key << "Restitution" << YAML::Value << cc2dComponent.m_Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2dComponent.m_RestitutionThreshold;
			out << YAML::Key << "IsSensor" << YAML::Value << cc2dComponent.m_IsSensor;
			out << YAML::EndMap; // Component Map
		}

		// Handle all project components
		for (auto& [handle, asset] : Assets::AssetService::GetProjectComponentRegistry())
		{
			if (!entity.HasProjectComponentData(handle))
			{
				continue;
			}
			Ref<ECS::ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(handle);
			uint8_t* componentRef = (uint8_t*)entity.GetProjectComponentData(handle);

			out << YAML::Key << projectComponent->m_Name + "Component";
			out << YAML::BeginMap; // Component Map
			for (size_t iteration{ 0 }; iteration < projectComponent->m_DataLocations.size(); iteration++)
			{
				SerializeWrappedVarType(out, 
					projectComponent->m_DataTypes.at(iteration), 
					projectComponent->m_DataNames.at(iteration).CString(),
					componentRef + projectComponent->m_DataLocations.at(iteration));
			}
			out << YAML::EndMap; // Component Map
		}


		out << YAML::EndMap; // Entity
		return true;
	}
}

namespace Kargono::Assets
{
	void Assets::SceneManager::CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(name);
		// Create Temporary Scene
		Ref<Scenes::Scene> temporaryScene = CreateRef<Scenes::Scene>();

		// Save Binary into File
		SerializeAsset(temporaryScene, assetPath);

		// Load data into In-Memory Metadata object
		Ref<Assets::SceneMetaData> metadata = CreateRef<Assets::SceneMetaData>();
		asset.Data.SpecificFileData = metadata;
	}
	void SceneManager::SerializeAsset(Ref<Scenes::Scene> assetReference, const std::filesystem::path& assetPath)
	{
		bool submitScene = true;
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		{ // Physics
			out << YAML::Key << "Physics" << YAML::BeginMap; // Physics Map
			out << YAML::Key << "Gravity" << YAML::Value << assetReference->m_PhysicsSpecification.Gravity;
			out << YAML::EndMap; // Physics Maps
		}

		// Add background color
		out << YAML::Key << "BackgroundColor" << YAML::Value << assetReference->m_BackgroundColor;

		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		auto view{ assetReference->m_EntityRegistry.m_Registry.GetAllEntities()};

		for (ECSInternal::EntityID entityID : view)
		{
			ECS::Entity entity = { entityID, &assetReference->m_EntityRegistry };
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
	Ref<Scenes::Scene> SceneManager::DeserializeAsset(Assets::AssetInfo& assetInfo, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(assetInfo);
		Ref<Scenes::Scene> newScene = CreateRef<Scenes::Scene>();
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetPath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_WARN("Failed to load .kgscene file '{0}'\n     {1}", assetPath, e.what());
			return nullptr;
		}

		YAML::Node physics = data["Physics"];
		newScene->GetPhysicsSpecification().Gravity = physics["Gravity"].as<Math::vec2>();

		newScene->m_BackgroundColor = data["BackgroundColor"].as<Math::vec4>();

		YAML::Node entities = data["Entities"];
		if (entities)
		{
			for (const YAML::Node& entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				YAML::Node tagNode = entity["TagComponent"];
				if (tagNode) 
				{ 
					name = tagNode["Tag"].as<std::string>(); 
				}

				ECS::Entity deserializedEntity = newScene->CreateEntityWithUUID(uuid, name);

				TagComponent& tagComp = deserializedEntity.GetComponent<TagComponent>();
				tagComp.m_Group = tagNode["Group"].as<std::string>();

				YAML::Node transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					TransformComponent& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.m_Translation = transformComponent["Translation"].as<Math::vec3>();
					tc.m_Rotation = transformComponent["Rotation"].as<Math::vec3>();
					tc.m_Scale = transformComponent["Scale"].as<Math::vec3>();
				}

				YAML::Node onUpdateNode = entity["OnUpdateComponent"];
				if (onUpdateNode)
				{
					Scripting::OnUpdateComponent& component = deserializedEntity.AddComponent<Scripting::OnUpdateComponent>();
					component.m_OnUpdateScriptHandle = onUpdateNode["OnUpdateHandle"].as<uint64_t>();
					component.m_OnUpdateScript = Assets::AssetService::GetScript(component.m_OnUpdateScriptHandle);
				}

				YAML::Node aiStateNode = entity["AIStateComponent"];
				if (aiStateNode)
				{
					AI::AIStateComponent& component = deserializedEntity.AddComponent<AI::AIStateComponent>();

					// Deserialize current state
					component.m_CurrentStateHandle = aiStateNode["CurrentState"].as<uint64_t>();
					component.m_CurrentStateReference = Assets::AssetService::GetAIState(component.m_CurrentStateHandle);
					if (component.m_CurrentStateHandle != Assets::EmptyHandle && !component.m_CurrentStateReference)
					{
						KG_WARN("Valid handle was found, however, could not retrieve a valid reference to current AI state");
					}

					// Deserialize previous state
					component.m_PreviousStateHandle = aiStateNode["PreviousState"].as<uint64_t>();
					component.m_PreviousStateReference = Assets::AssetService::GetAIState(component.m_PreviousStateHandle);
					if (component.m_PreviousStateHandle != Assets::EmptyHandle && !component.m_PreviousStateReference)
					{
						KG_WARN("Valid handle was found, however, could not retrieve a valid reference for previous AI state");
					}

					// Deserialize global state
					component.m_GlobalStateHandle = aiStateNode["GlobalState"].as<uint64_t>();
					component.m_GlobalStateReference = Assets::AssetService::GetAIState(component.m_GlobalStateHandle);
					if (component.m_GlobalStateHandle != Assets::EmptyHandle && !component.m_GlobalStateReference)
					{
						KG_WARN("Valid handle was found, however, could not retrieve a valid reference to global AI State");
					}
				}



				YAML::Node onCreateNode = entity["OnCreateComponent"];
				if (onCreateNode)
				{
					Scripting::OnCreateComponent& component = deserializedEntity.AddComponent<Scripting::OnCreateComponent>();
					component.m_OnCreateScriptHandle = onCreateNode["OnCreateHandle"].as<uint64_t>();
					component.m_OnCreateScript = Assets::AssetService::GetScript(component.m_OnCreateScriptHandle);
				}

				YAML::Node particleEmitterNode = entity["ParticleEmitterComponent"];
				if (particleEmitterNode)
				{
					Particles::ParticleEmitterComponent& component = deserializedEntity.AddComponent<Particles::ParticleEmitterComponent>();
					component.m_EmitterConfigHandle = particleEmitterNode["EmitterHandle"].as<uint64_t>();
					component.m_EmitterConfigRef = Assets::AssetService::GetEmitterConfig(component.m_EmitterConfigHandle);
				}

				YAML::Node cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					Rendering::CameraComponent& cc = deserializedEntity.AddComponent<Rendering::CameraComponent>();

					const auto& cameraProps = cameraComponent["Camera"];

					cc.m_Camera.SetProjectionType((Scenes::SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());
					cc.m_Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
					cc.m_Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.m_Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					cc.m_Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.m_Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.m_Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

					cc.m_Primary = cameraComponent["Primary"].as<bool>();
				}

				YAML::Node shapeComponent = entity["ShapeComponent"];
				if (shapeComponent)
				{
					Rendering::ShapeComponent& sc = deserializedEntity.AddComponent<Rendering::ShapeComponent>();
					sc.m_CurrentShape = Utility::StringToShapeType(shapeComponent["CurrentShape"].as<std::string>());

					if (shapeComponent["VertexColors"])
					{
						YAML::Node vertexColors = shapeComponent["VertexColors"];
						sc.m_VertexColors = CreateRef<std::vector<Math::vec4>>();
						for (const YAML::Node& color : vertexColors)
						{
							sc.m_VertexColors->push_back(color["Color"].as<Math::vec4>());
						}
					}

					if (shapeComponent["TextureHandle"])
					{
						AssetHandle textureHandle = shapeComponent["TextureHandle"].as<uint64_t>();
						sc.m_Texture = AssetService::GetTexture2D(textureHandle);
						sc.m_TextureHandle = textureHandle;
					}

					if (shapeComponent["ShaderHandle"])
					{
						AssetHandle shaderHandle = shapeComponent["ShaderHandle"].as<uint64_t>();
						sc.m_Shader = AssetService::GetShader(shaderHandle);
						if (!sc.m_Shader)
						{
							YAML::Node shaderSpecificationNode = shapeComponent["ShaderSpecification"];
							Rendering::ShaderSpecification shaderSpec{};
							// ShaderSpecification Section
							shaderSpec.ColorInput = Utility::StringToColorInputType(shaderSpecificationNode["ColorInputType"].as<std::string>());
							shaderSpec.AddProjectionMatrix = shaderSpecificationNode["AddProjectionMatrix"].as<bool>();
							shaderSpec.AddEntityID = shaderSpecificationNode["AddEntityID"].as<bool>();
							shaderSpec.AddCircleShape = shaderSpecificationNode["AddCircleShape"].as<bool>();
							shaderSpec.TextureInput = Utility::StringToTextureInputType(shaderSpecificationNode["TextureInput"].as<std::string>());
							shaderSpec.DrawOutline = shaderSpecificationNode["DrawOutline"].as<bool>();
							shaderSpec.RenderType = Utility::StringToRenderingType(shaderSpecificationNode["RenderType"].as<std::string>());
							auto [newHandle, newShader] = AssetService::GetShader(shaderSpec);
							shaderHandle = newHandle;
							sc.m_Shader = newShader;
						}
						sc.m_ShaderHandle = shaderHandle;
						sc.m_ShaderSpecification = sc.m_Shader->GetSpecification();
						YAML::Binary binary = shapeComponent["Buffer"].as<YAML::Binary>();
						Buffer buffer{ binary.size() };
						memcpy(buffer.Data, binary.data(), buffer.Size);
						sc.m_ShaderData = buffer;
						if (sc.m_CurrentShape != Rendering::ShapeTypes::None)
						{
							if (sc.m_ShaderSpecification.RenderType == Rendering::RenderingType::DrawIndex)
							{
								sc.m_Vertices = CreateRef<std::vector<Math::vec3>>(Utility::ShapeTypeToShape(sc.m_CurrentShape).GetIndexVertices());
								sc.m_Indices = CreateRef<std::vector<uint32_t>>(Utility::ShapeTypeToShape(sc.m_CurrentShape).GetIndices());
								sc.m_TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Utility::ShapeTypeToShape(sc.m_CurrentShape).GetIndexTextureCoordinates());
							}

							if (sc.m_ShaderSpecification.RenderType == Rendering::RenderingType::DrawTriangle)
							{
								sc.m_Vertices = CreateRef<std::vector<Math::vec3>>(Utility::ShapeTypeToShape(sc.m_CurrentShape).GetTriangleVertices());
								sc.m_TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Utility::ShapeTypeToShape(sc.m_CurrentShape).GetTriangleTextureCoordinates());
							}
						}
					}
				}

				YAML::Node rigidbody2DComponent = entity["Rigidbody2DComponent"];
				if (rigidbody2DComponent)
				{
					Physics2D::Rigidbody2DComponent& rb2d = deserializedEntity.AddComponent<Physics2D::Rigidbody2DComponent>();
					rb2d.m_Type = Utility::StringToRigidBody2DBodyType(rigidbody2DComponent["BodyType"].as<std::string>());
					rb2d.m_FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();

					// Deserialize on collision scripts
					rb2d.m_OnCollisionStartScriptHandle = rigidbody2DComponent["OnCollisionStartHandle"].as<uint64_t>();
					rb2d.m_OnCollisionStartScript = Assets::AssetService::GetScript(rb2d.m_OnCollisionStartScriptHandle);
					rb2d.m_OnCollisionEndScriptHandle = rigidbody2DComponent["OnCollisionEndHandle"].as<uint64_t>();
					rb2d.m_OnCollisionEndScript = Assets::AssetService::GetScript(rb2d.m_OnCollisionEndScriptHandle);
				}

				YAML::Node boxCollider2DComponent = entity["BoxCollider2DComponent"];
				if (boxCollider2DComponent)
				{
					Physics2D::BoxCollider2DComponent& bc2d = deserializedEntity.AddComponent<Physics2D::BoxCollider2DComponent>();
					bc2d.m_Offset = boxCollider2DComponent["Offset"].as<Math::vec2>();
					bc2d.m_Size = boxCollider2DComponent["Size"].as<Math::vec2>();
					bc2d.m_Density = boxCollider2DComponent["Density"].as<float>();
					bc2d.m_Friction = boxCollider2DComponent["Friction"].as<float>();
					bc2d.m_Restitution = boxCollider2DComponent["Restitution"].as<float>();
					bc2d.m_RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
					bc2d.m_IsSensor = boxCollider2DComponent["IsSensor"].as<bool>();
				}

				YAML::Node circleCollider2DComponent = entity["CircleCollider2DComponent"];
				if (circleCollider2DComponent)
				{
					Physics2D::CircleCollider2DComponent& cc2d = deserializedEntity.AddComponent<Physics2D::CircleCollider2DComponent>();
					cc2d.m_Offset = circleCollider2DComponent["Offset"].as<Math::vec2>();
					cc2d.m_Radius = circleCollider2DComponent["Radius"].as<float>();
					cc2d.m_Density = circleCollider2DComponent["Density"].as<float>();
					cc2d.m_Friction = circleCollider2DComponent["Friction"].as<float>();
					cc2d.m_Restitution = circleCollider2DComponent["Restitution"].as<float>();
					cc2d.m_RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
					cc2d.m_IsSensor = circleCollider2DComponent["IsSensor"].as<bool>();
				}

				// Handle all project components
				for (auto& [handle, asset] : Assets::AssetService::GetProjectComponentRegistry())
				{
					Ref<ECS::ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(handle);
					KG_ASSERT(projectComponent);

					YAML::Node projectComponentNode = entity[projectComponent->m_Name + "Component"];
					if (!projectComponentNode)
					{
						continue;
					}
					// Create and get project component
					if (!deserializedEntity.HasProjectComponentData(handle))
					{
						deserializedEntity.AddProjectComponentData(handle);
					}
					uint8_t* componentRef = (uint8_t*)deserializedEntity.GetProjectComponentData(handle);

					// Load in component data from disk
					for (size_t iteration{ 0 }; iteration < projectComponent->m_DataLocations.size(); iteration++)
					{
						Utility::DeserializeWrappedVarType(projectComponentNode,
							projectComponent->m_DataTypes.at(iteration),
							projectComponent->m_DataNames.at(iteration).CString(),
							componentRef + projectComponent->m_DataLocations.at(iteration));
					}
				}
			}
		}

		return newScene;
	}
	bool SceneManager::RemoveScript(Ref<Scenes::Scene> sceneRef, Assets::AssetHandle scriptHandle)
	{
		bool sceneModified{ false };
		// Handle UI level function pointers
		// OnUpdate
		auto onUpdateView = sceneRef->GetAllEntitiesWith<Scripting::OnUpdateComponent>();
		for (ECSInternal::EntityID enttEntity : onUpdateView)
		{
			ECS::Entity currentEntity{ sceneRef->GetEntityByEnttID(enttEntity) };
			Scripting::OnUpdateComponent& component = currentEntity.GetComponent<Scripting::OnUpdateComponent>();
			if (component.m_OnUpdateScriptHandle == scriptHandle)
			{
				component.m_OnUpdateScriptHandle = Assets::EmptyHandle;
				component.m_OnUpdateScript = nullptr;
				sceneModified = true;
			}
		}

		// OnCreate
		auto onCreateView = sceneRef->GetAllEntitiesWith<Scripting::OnCreateComponent>();
		for (ECSInternal::EntityID enttEntity : onCreateView)
		{
			ECS::Entity currentEntity{ sceneRef->GetEntityByEnttID(enttEntity) };
			Scripting::OnCreateComponent& component = currentEntity.GetComponent<Scripting::OnCreateComponent>();
			if (component.m_OnCreateScriptHandle == scriptHandle)
			{
				component.m_OnCreateScriptHandle = Assets::EmptyHandle;
				component.m_OnCreateScript = nullptr;
				sceneModified = true;
			}
		}

		// Rigidbody
		auto rigidBodyView = sceneRef->GetAllEntitiesWith<Physics2D::Rigidbody2DComponent>();
		for (ECSInternal::EntityID enttEntity : rigidBodyView)
		{
			ECS::Entity currentEntity{ sceneRef->GetEntityByEnttID(enttEntity) };
			Physics2D::Rigidbody2DComponent& component = currentEntity.GetComponent<Physics2D::Rigidbody2DComponent>();

			if (component.m_OnCollisionStartScriptHandle == scriptHandle)
			{
				component.m_OnCollisionStartScriptHandle = Assets::EmptyHandle;
				component.m_OnCollisionStartScript = nullptr;
				sceneModified = true;
			}

			if (component.m_OnCollisionEndScriptHandle == scriptHandle)
			{
				component.m_OnCollisionEndScriptHandle = Assets::EmptyHandle;
				component.m_OnCollisionEndScript = nullptr;
				sceneModified = true;
			}
		}
		return sceneModified;
	}
	bool SceneManager::RemoveAIState(Ref<Scenes::Scene> sceneRef, Assets::AssetHandle aiStateHandle)
	{
		bool aiStateModified{ false };

		// Check for AIState
		auto aiStateView = sceneRef->GetAllEntitiesWith<AI::AIStateComponent>();
		for (ECSInternal::EntityID enttEntity : aiStateView)
		{
			ECS::Entity currentEntity{ sceneRef->GetEntityByEnttID(enttEntity) };
			AI::AIStateComponent& component = currentEntity.GetComponent<AI::AIStateComponent>();
			
			if (component.m_CurrentStateHandle == aiStateHandle)
			{
				component.m_CurrentStateHandle = Assets::EmptyHandle;
				component.m_CurrentStateReference = nullptr;
				aiStateModified = true;
			}
			if (component.m_GlobalStateHandle == aiStateHandle)
			{
				component.m_GlobalStateHandle = Assets::EmptyHandle;
				component.m_GlobalStateReference = nullptr;
				aiStateModified = true;
			}
			if (component.m_PreviousStateHandle == aiStateHandle)
			{
				component.m_PreviousStateHandle = Assets::EmptyHandle;
				component.m_PreviousStateReference = nullptr;
				aiStateModified = true;
			}
		}
		return aiStateModified;
	}
	bool SceneManager::RemoveProjectComponent(Ref<Scenes::Scene> sceneRef, Assets::AssetHandle projectCompHandle)
	{
		KG_ASSERT(sceneRef);

		// Check if any components are being removed
		std::size_t componentCount = sceneRef->GetProjectComponentCount(projectCompHandle);

		// Clear component registry
		sceneRef->ClearProjectComponentRegistry(projectCompHandle);

		return componentCount > 0;
	}
	bool SceneManager::RemoveEmitterConfig(Ref<Scenes::Scene> sceneRef, Assets::AssetHandle emitterConfigHandle)
	{
		bool emitterConfigModified{ false };

		// Check for emitterConfig
		auto emitterConfigView = sceneRef->GetAllEntitiesWith<Particles::ParticleEmitterComponent>();
		for (ECSInternal::EntityID enttEntity : emitterConfigView)
		{
			ECS::Entity currentEntity{ sceneRef->GetEntityByEnttID(enttEntity) };
			Particles::ParticleEmitterComponent& component = currentEntity.GetComponent<Particles::ParticleEmitterComponent>();

			if (component.m_EmitterConfigHandle == emitterConfigHandle)
			{
				component.m_EmitterConfigHandle = Assets::EmptyHandle;
				component.m_EmitterConfigRef = nullptr;
				emitterConfigModified = true;
			}
		}
		return emitterConfigModified;
	}
}
