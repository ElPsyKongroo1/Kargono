#include "kgpch.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Assets/SceneManager.h"

#include "Kargono/ECS/ProjectComponent.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/ECS/Entity.h"

namespace Kargono::Utility
{

	static void SerializeWrappedVarType(YAML::Emitter& out, WrappedVarType type, const std::string& name, void* dataSource)
	{
		switch (type)
		{
		case WrappedVarType::Integer32: 
			out << YAML::Key << name << YAML::Value << *(int32_t*)dataSource;
			return;
		case WrappedVarType::UInteger16:
			out << YAML::Key << name << YAML::Value << *(uint16_t*)dataSource;
			return;
		case WrappedVarType::UInteger32:
			out << YAML::Key << name << YAML::Value << *(uint32_t*)dataSource;
			return;
		case WrappedVarType::UInteger64:
			out << YAML::Key << name << YAML::Value << *(uint64_t*)dataSource;
			return;
		case WrappedVarType::Vector3:
			out << YAML::Key << name << YAML::Value << *(Math::vec3*)dataSource;
			return;
		case WrappedVarType::String:
			out << YAML::Key << name << YAML::Value << *(std::string*)dataSource;
			return;
		case WrappedVarType::Bool:
			out << YAML::Key << name << YAML::Value << *(bool*)dataSource;
			return;
		case WrappedVarType::Float:
			out << YAML::Key << name << YAML::Value << *(float*)dataSource;
			return;
		case WrappedVarType::None:
		case WrappedVarType::Void:
			KG_ERROR("Invalid type provided while serializing data");
			return;
		}
		KG_ERROR("Unknown Type of WrappedVariableType.");
		return;
	}

	static void DeserializeWrappedVarType(YAML::Node& componentNode, WrappedVarType type, const std::string& name, void* destination)
	{
		switch (type)
		{
		case WrappedVarType::Integer32:
			*(int32_t*)destination = componentNode[name].as<int32_t>();
			return;
		case WrappedVarType::UInteger16:
			*(uint16_t*)destination = (uint16_t)componentNode[name].as<uint32_t>();
			return;
		case WrappedVarType::UInteger32:
			*(uint32_t*)destination = componentNode[name].as<uint32_t>();
			return;
		case WrappedVarType::UInteger64:
			*(uint64_t*)destination = componentNode[name].as<uint64_t>();
			return;
		case WrappedVarType::Vector3:
			*(Math::vec3*)destination = componentNode[name].as<Math::vec3>();
			return;
		case WrappedVarType::String:
			*(std::string*)destination = componentNode[name].as<std::string>();
			return;
		case WrappedVarType::Bool:
			*(bool*)destination = componentNode[name].as<bool>();
			return;
		case WrappedVarType::Float:
			*(float*)destination = componentNode[name].as<float>();
			return;
		case WrappedVarType::None:
		case WrappedVarType::Void:
			KG_ERROR("Invalid type provided while serializing data");
			return;
		}
		KG_ERROR("Unknown Type of WrappedVariableType.");
		return;
	}

	static bool SerializeEntity(YAML::Emitter& out, ECS::Entity entity)
	{
		KG_ASSERT(entity.HasComponent<ECS::IDComponent>(), "Entity does not have a component");
		out << YAML::BeginMap; // Entity Map
		out << YAML::Key << "Entity" << YAML::Value << static_cast<uint64_t>(entity.GetUUID());

		if (entity.HasComponent<ECS::TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // Component Map
			ECS::TagComponent& tag = entity.GetComponent<ECS::TagComponent>();
			out << YAML::Key << "Tag" << YAML::Value << tag.Tag;
			out << YAML::Key << "Group" << YAML::Value << tag.Group;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<ECS::OnUpdateComponent>())
		{
			out << YAML::Key << "OnUpdateComponent";
			out << YAML::BeginMap; // Component Map
			ECS::OnUpdateComponent& component = entity.GetComponent<ECS::OnUpdateComponent>();
			out << YAML::Key << "OnUpdateHandle" << YAML::Value << static_cast<uint64_t>(component.OnUpdateScriptHandle);
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<ECS::OnCreateComponent>())
		{
			out << YAML::Key << "OnCreateComponent";
			out << YAML::BeginMap; // Component Map
			ECS::OnCreateComponent& component = entity.GetComponent<ECS::OnCreateComponent>();
			out << YAML::Key << "OnCreateHandle" << YAML::Value << static_cast<uint64_t>(component.OnCreateScriptHandle);
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<ECS::TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // Component Map
			auto& tc = entity.GetComponent<ECS::TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<ECS::AIStateComponent>())
		{
			out << YAML::Key << "AIStateComponent";
			out << YAML::BeginMap; // Component Map
			ECS::AIStateComponent& aiStateComp = entity.GetComponent<ECS::AIStateComponent>();
			out << YAML::Key << "CurrentState" << YAML::Value << static_cast<uint64_t>(aiStateComp.CurrentStateHandle);
			out << YAML::Key << "PreviousState" << YAML::Value << static_cast<uint64_t>(aiStateComp.PreviousStateHandle);
			out << YAML::Key << "GlobalState" << YAML::Value << static_cast<uint64_t>(aiStateComp.GlobalStateHandle);
			out << YAML::EndMap; // Component Map
		}


		if (entity.HasComponent<ECS::CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // Component Map

			auto& cameraComponent = entity.GetComponent<ECS::CameraComponent>();
			auto& camera = cameraComponent.Camera;

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

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;


			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<ECS::ShapeComponent>())
		{
			out << YAML::Key << "ShapeComponent";
			out << YAML::BeginMap; // Component Map
			auto& shapeComponent = entity.GetComponent<ECS::ShapeComponent>();
			out << YAML::Key << "CurrentShape" << YAML::Value << Utility::ShapeTypeToString(shapeComponent.CurrentShape);
			if (shapeComponent.VertexColors)
			{
				out << YAML::Key << "VertexColors" << YAML::Value << YAML::BeginSeq;
				for (const auto& color : *shapeComponent.VertexColors)
				{
					out << YAML::BeginMap;
					out << YAML::Key << "Color" << YAML::Value << color;
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;
			}
			if (shapeComponent.Texture)
			{
				out << YAML::Key << "TextureHandle" << YAML::Value << static_cast<uint64_t>(shapeComponent.TextureHandle);
			}
			KG_ASSERT(sizeof(uint8_t) * 20 == sizeof(Rendering::ShaderSpecification), "Please Update Deserialization and Serialization. Incorrect size of input data in Scene Serializer!");
			if (shapeComponent.Shader)
			{
				// Add Shader Handle
				out << YAML::Key << "ShaderHandle" << YAML::Value << static_cast<uint64_t>(shapeComponent.ShaderHandle);
				// Add Shader Specification
				const Rendering::ShaderSpecification& shaderSpec = shapeComponent.Shader->GetSpecification();
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
				out << YAML::Key << "Buffer" << YAML::Value << YAML::Binary(shapeComponent.ShaderData.Data, shapeComponent.ShaderData.Size);
			}
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<ECS::Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Component Map
			auto& rb2dComponent = entity.GetComponent<ECS::Rigidbody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << Utility::RigidBody2DBodyTypeToString(rb2dComponent.Type);
			out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;
			out << YAML::Key << "OnCollisionStartHandle" << YAML::Value << static_cast<uint64_t>(rb2dComponent.OnCollisionStartScriptHandle);
			out << YAML::Key << "OnCollisionEndHandle" << YAML::Value << static_cast<uint64_t>(rb2dComponent.OnCollisionEndScriptHandle);
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<ECS::BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // Component Map
			auto& bc2dComponent = entity.GetComponent<ECS::BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
			out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
			out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.RestitutionThreshold;
			out << YAML::Key << "IsSensor" << YAML::Value << bc2dComponent.IsSensor;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<ECS::CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; // Component Map
			auto& cc2dComponent = entity.GetComponent<ECS::CircleCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << cc2dComponent.Offset;
			out << YAML::Key << "Radius" << YAML::Value << cc2dComponent.Radius;
			out << YAML::Key << "Density" << YAML::Value << cc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << cc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << cc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2dComponent.RestitutionThreshold;
			out << YAML::Key << "IsSensor" << YAML::Value << cc2dComponent.IsSensor;
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
					projectComponent->m_DataNames.at(iteration), 
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
	void Assets::SceneManager::CreateAssetFileFromName(const std::string& name, AssetInfo& asset, const std::filesystem::path& assetPath)
	{
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

		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		assetReference->m_EntityRegistry.m_EnTTRegistry.each([&](auto entityID)
		{
			ECS::Entity entity = { entityID, &assetReference->m_EntityRegistry };
			if (!entity) { return; }

			bool success = Utility::SerializeEntity(out, entity);
			if (!success)
			{
				submitScene = false;
			}
		});
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
	Ref<Scenes::Scene> SceneManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
	{
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

		auto physics = data["Physics"];
		newScene->GetPhysicsSpecification().Gravity = physics["Gravity"].as<Math::vec2>();

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				YAML::Node tagNode = entity["TagComponent"];
				if (tagNode) 
				{ 
					name = tagNode["Tag"].as<std::string>(); 
				}

				ECS::Entity deserializedEntity = newScene->CreateEntityWithUUID(uuid, name);

				ECS::TagComponent& tagComp = deserializedEntity.GetComponent<ECS::TagComponent>();
				tagComp.Group = tagNode["Group"].as<std::string>();

				YAML::Node transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& tc = deserializedEntity.GetComponent<ECS::TransformComponent>();
					tc.Translation = transformComponent["Translation"].as<Math::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<Math::vec3>();
					tc.Scale = transformComponent["Scale"].as<Math::vec3>();
				}

				YAML::Node onUpdateNode = entity["OnUpdateComponent"];
				if (onUpdateNode)
				{
					ECS::OnUpdateComponent& component = deserializedEntity.AddComponent<ECS::OnUpdateComponent>();
					component.OnUpdateScriptHandle = onUpdateNode["OnUpdateHandle"].as<uint64_t>();
					component.OnUpdateScript = Assets::AssetService::GetScript(component.OnUpdateScriptHandle);
				}

				YAML::Node aiStateNode = entity["AIStateComponent"];
				if (aiStateNode)
				{
					ECS::AIStateComponent& component = deserializedEntity.AddComponent<ECS::AIStateComponent>();

					// Deserialize current state
					component.CurrentStateHandle = aiStateNode["CurrentState"].as<uint64_t>();
					component.CurrentStateReference = Assets::AssetService::GetAIState(component.CurrentStateHandle);
					if (component.CurrentStateHandle != Assets::EmptyHandle && !component.CurrentStateReference)
					{
						KG_WARN("Valid handle was found, however, could not retrieve a valid reference to current AI state");
					}

					// Deserialize previous state
					component.PreviousStateHandle = aiStateNode["PreviousState"].as<uint64_t>();
					component.PreviousStateReference = Assets::AssetService::GetAIState(component.PreviousStateHandle);
					if (component.PreviousStateHandle != Assets::EmptyHandle && !component.PreviousStateReference)
					{
						KG_WARN("Valid handle was found, however, could not retrieve a valid reference for previous AI state");
					}

					// Deserialize global state
					component.GlobalStateHandle = aiStateNode["GlobalState"].as<uint64_t>();
					component.GlobalStateReference = Assets::AssetService::GetAIState(component.GlobalStateHandle);
					if (component.GlobalStateHandle != Assets::EmptyHandle && !component.GlobalStateReference)
					{
						KG_WARN("Valid handle was found, however, could not retrieve a valid reference to global AI State");
					}
				}

				YAML::Node onCreateNode = entity["OnCreateComponent"];
				if (onCreateNode)
				{
					ECS::OnCreateComponent& component = deserializedEntity.AddComponent<ECS::OnCreateComponent>();
					component.OnCreateScriptHandle = onCreateNode["OnCreateHandle"].as<uint64_t>();
					component.OnCreateScript = Assets::AssetService::GetScript(component.OnCreateScriptHandle);
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<ECS::CameraComponent>();

					const auto& cameraProps = cameraComponent["Camera"];

					cc.Camera.SetProjectionType((Scenes::SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());
					cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
					cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

					cc.Primary = cameraComponent["Primary"].as<bool>();
				}

				auto shapeComponent = entity["ShapeComponent"];
				if (shapeComponent)
				{
					auto& sc = deserializedEntity.AddComponent<ECS::ShapeComponent>();
					sc.CurrentShape = Utility::StringToShapeType(shapeComponent["CurrentShape"].as<std::string>());

					if (shapeComponent["VertexColors"])
					{
						auto vertexColors = shapeComponent["VertexColors"];
						sc.VertexColors = CreateRef<std::vector<Math::vec4>>();
						for (auto color : vertexColors)
						{
							sc.VertexColors->push_back(color["Color"].as<Math::vec4>());
						}
					}

					if (shapeComponent["TextureHandle"])
					{
						AssetHandle textureHandle = shapeComponent["TextureHandle"].as<uint64_t>();
						sc.Texture = AssetService::GetTexture2D(textureHandle);
						sc.TextureHandle = textureHandle;
					}

					if (shapeComponent["ShaderHandle"])
					{
						AssetHandle shaderHandle = shapeComponent["ShaderHandle"].as<uint64_t>();
						sc.Shader = AssetService::GetShader(shaderHandle);
						if (!sc.Shader)
						{
							auto shaderSpecificationNode = shapeComponent["ShaderSpecification"];
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
							sc.Shader = newShader;
						}
						sc.ShaderHandle = shaderHandle;
						sc.ShaderSpecification = sc.Shader->GetSpecification();
						YAML::Binary binary = shapeComponent["Buffer"].as<YAML::Binary>();
						Buffer buffer{ binary.size() };
						memcpy_s(buffer.Data, buffer.Size, binary.data(), buffer.Size);
						sc.ShaderData = buffer;
						if (sc.CurrentShape != Rendering::ShapeTypes::None)
						{
							if (sc.ShaderSpecification.RenderType == Rendering::RenderingType::DrawIndex)
							{
								sc.Vertices = CreateRef<std::vector<Math::vec3>>(Utility::ShapeTypeToShape(sc.CurrentShape).GetIndexVertices());
								sc.Indices = CreateRef<std::vector<uint32_t>>(Utility::ShapeTypeToShape(sc.CurrentShape).GetIndices());
								sc.TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Utility::ShapeTypeToShape(sc.CurrentShape).GetIndexTextureCoordinates());
							}

							if (sc.ShaderSpecification.RenderType == Rendering::RenderingType::DrawTriangle)
							{
								sc.Vertices = CreateRef<std::vector<Math::vec3>>(Utility::ShapeTypeToShape(sc.CurrentShape).GetTriangleVertices());
								sc.TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Utility::ShapeTypeToShape(sc.CurrentShape).GetTriangleTextureCoordinates());
							}
						}
					}
				}

				auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
				if (rigidbody2DComponent)
				{
					auto& rb2d = deserializedEntity.AddComponent<ECS::Rigidbody2DComponent>();
					rb2d.Type = Utility::StringToRigidBody2DBodyType(rigidbody2DComponent["BodyType"].as<std::string>());
					rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();

					// Deserialize on collision scripts
					rb2d.OnCollisionStartScriptHandle = rigidbody2DComponent["OnCollisionStartHandle"].as<uint64_t>();
					rb2d.OnCollisionStartScript = Assets::AssetService::GetScript(rb2d.OnCollisionStartScriptHandle);
					rb2d.OnCollisionEndScriptHandle = rigidbody2DComponent["OnCollisionEndHandle"].as<uint64_t>();
					rb2d.OnCollisionEndScript = Assets::AssetService::GetScript(rb2d.OnCollisionEndScriptHandle);
				}

				auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
				if (boxCollider2DComponent)
				{
					auto& bc2d = deserializedEntity.AddComponent<ECS::BoxCollider2DComponent>();
					bc2d.Offset = boxCollider2DComponent["Offset"].as<Math::vec2>();
					bc2d.Size = boxCollider2DComponent["Size"].as<Math::vec2>();
					bc2d.Density = boxCollider2DComponent["Density"].as<float>();
					bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
					bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
					bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
					bc2d.IsSensor = boxCollider2DComponent["IsSensor"].as<bool>();
				}

				auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
				if (circleCollider2DComponent)
				{
					auto& cc2d = deserializedEntity.AddComponent<ECS::CircleCollider2DComponent>();
					cc2d.Offset = circleCollider2DComponent["Offset"].as<Math::vec2>();
					cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
					cc2d.Density = circleCollider2DComponent["Density"].as<float>();
					cc2d.Friction = circleCollider2DComponent["Friction"].as<float>();
					cc2d.Restitution = circleCollider2DComponent["Restitution"].as<float>();
					cc2d.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
					cc2d.IsSensor = circleCollider2DComponent["IsSensor"].as<bool>();
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
							projectComponent->m_DataNames.at(iteration),
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
		auto onUpdateView = sceneRef->GetAllEntitiesWith<ECS::OnUpdateComponent>();
		for (entt::entity enttEntity : onUpdateView)
		{
			ECS::Entity currentEntity{ sceneRef->GetEntityByEnttID(enttEntity) };
			ECS::OnUpdateComponent& component = currentEntity.GetComponent<ECS::OnUpdateComponent>();
			if (component.OnUpdateScriptHandle == scriptHandle)
			{
				component.OnUpdateScriptHandle = Assets::EmptyHandle;
				component.OnUpdateScript = nullptr;
				sceneModified = true;
			}
		}

		// OnCreate
		auto onCreateView = sceneRef->GetAllEntitiesWith<ECS::OnCreateComponent>();
		for (entt::entity enttEntity : onCreateView)
		{
			ECS::Entity currentEntity{ sceneRef->GetEntityByEnttID(enttEntity) };
			ECS::OnCreateComponent& component = currentEntity.GetComponent<ECS::OnCreateComponent>();
			if (component.OnCreateScriptHandle == scriptHandle)
			{
				component.OnCreateScriptHandle = Assets::EmptyHandle;
				component.OnCreateScript = nullptr;
				sceneModified = true;
			}
		}

		// Rigidbody
		auto rigidBodyView = sceneRef->GetAllEntitiesWith<ECS::Rigidbody2DComponent>();
		for (entt::entity enttEntity : rigidBodyView)
		{
			ECS::Entity currentEntity{ sceneRef->GetEntityByEnttID(enttEntity) };
			ECS::Rigidbody2DComponent& component = currentEntity.GetComponent<ECS::Rigidbody2DComponent>();

			if (component.OnCollisionStartScriptHandle == scriptHandle)
			{
				component.OnCollisionStartScriptHandle = Assets::EmptyHandle;
				component.OnCollisionStartScript = nullptr;
				sceneModified = true;
			}

			if (component.OnCollisionEndScriptHandle == scriptHandle)
			{
				component.OnCollisionEndScriptHandle = Assets::EmptyHandle;
				component.OnCollisionEndScript = nullptr;
				sceneModified = true;
			}
		}
		return sceneModified;
	}
	bool SceneManager::RemoveAIState(Ref<Scenes::Scene> sceneRef, Assets::AssetHandle aiStateHandle)
	{
		bool aiStateModified{ false };

		// Check for AIState
		auto aiStateView = sceneRef->GetAllEntitiesWith<ECS::AIStateComponent>();
		for (entt::entity enttEntity : aiStateView)
		{
			ECS::Entity currentEntity{ sceneRef->GetEntityByEnttID(enttEntity) };
			ECS::AIStateComponent& component = currentEntity.GetComponent<ECS::AIStateComponent>();
			
			if (component.CurrentStateHandle == aiStateHandle)
			{
				component.CurrentStateHandle = Assets::EmptyHandle;
				component.CurrentStateReference = nullptr;
				aiStateModified = true;
			}
			if (component.GlobalStateHandle == aiStateHandle)
			{
				component.GlobalStateHandle = Assets::EmptyHandle;
				component.GlobalStateReference = nullptr;
				aiStateModified = true;
			}
			if (component.PreviousStateHandle == aiStateHandle)
			{
				component.PreviousStateHandle = Assets::EmptyHandle;
				component.PreviousStateReference = nullptr;
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
}
