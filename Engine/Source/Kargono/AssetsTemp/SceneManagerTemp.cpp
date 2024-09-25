#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/SceneManagerTemp.h"

#include "Kargono/Scenes/Scene.h"
#include "Kargono/Scenes/Entity.h"

namespace Kargono::Assets
{
	Ref<Scenes::Scene> SceneManager::InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath)
	{
		Ref<Scenes::Scene> newScene = CreateRef<Scenes::Scene>();
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetPath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgscene file '{0}'\n     {1}", assetPath, e.what());
			return nullptr;
		}

		KG_INFO("Deserializing scene");

		auto physics = data["Physics"];
		newScene->GetPhysicsSpecification().Gravity = physics["Gravity"].as<Math::vec2>();

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent) { name = tagComponent["Tag"].as<std::string>(); }
				//KG_CORE_TRACE("Deserialize entity with ID = {0}, name = {1}", uuid, name);

				Scenes::Entity deserializedEntity = newScene->CreateEntityWithUUID(uuid, name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& tc = deserializedEntity.GetComponent<Scenes::TransformComponent>();
					tc.Translation = transformComponent["Translation"].as<Math::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<Math::vec3>();
					tc.Scale = transformComponent["Scale"].as<Math::vec3>();
				}

				auto classInstanceComponent = entity["ClassInstanceComponent"];
				if (classInstanceComponent)
				{
					auto& cInstComp = deserializedEntity.AddComponent<Scenes::ClassInstanceComponent>();
					cInstComp.ClassHandle = classInstanceComponent["ClassHandle"].as<uint64_t>();
					if (cInstComp.ClassHandle != Assets::EmptyHandle)
					{
						if (!AssetServiceTemp::HasEntityClass(cInstComp.ClassHandle))
						{
							KG_ERROR("Could not find entity class for class instance component");
							return nullptr;
						}
						cInstComp.ClassReference = AssetServiceTemp::GetEntityClass(cInstComp.ClassHandle);

						auto instanceFields = classInstanceComponent["InstanceFields"];
						if (instanceFields)
						{
							for (auto field : instanceFields)
							{
								WrappedVarType type = Utility::StringToWrappedVarType(field["Type"].as<std::string>());
								Ref<WrappedVariable> newField = Utility::DeserializeWrappedVariableData(type, field);
								cInstComp.Fields.push_back(newField);
							}
						}
					}
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<Scenes::CameraComponent>();

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
					auto& sc = deserializedEntity.AddComponent<Scenes::ShapeComponent>();
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
						sc.Texture = AssetManager::GetTexture(textureHandle);
						sc.TextureHandle = textureHandle;
					}

					if (shapeComponent["ShaderHandle"])
					{
						AssetHandle shaderHandle = shapeComponent["ShaderHandle"].as<uint64_t>();
						sc.Shader = AssetManager::GetShader(shaderHandle);
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
							auto [newHandle, newShader] = AssetManager::GetShader(shaderSpec);
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
					auto& rb2d = deserializedEntity.AddComponent<Scenes::Rigidbody2DComponent>();
					rb2d.Type = Utility::StringToRigidBody2DBodyType(rigidbody2DComponent["BodyType"].as<std::string>());
					rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
				}

				auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
				if (boxCollider2DComponent)
				{
					auto& bc2d = deserializedEntity.AddComponent<Scenes::BoxCollider2DComponent>();
					bc2d.Offset = boxCollider2DComponent["Offset"].as<Math::vec2>();
					bc2d.Size = boxCollider2DComponent["Size"].as<Math::vec2>();
					bc2d.Density = boxCollider2DComponent["Density"].as<float>();
					bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
					bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
					bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
				}

				auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
				if (circleCollider2DComponent)
				{
					auto& cc2d = deserializedEntity.AddComponent<Scenes::CircleCollider2DComponent>();
					cc2d.Offset = circleCollider2DComponent["Offset"].as<Math::vec2>();
					cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
					cc2d.Density = circleCollider2DComponent["Density"].as<float>();
					cc2d.Friction = circleCollider2DComponent["Friction"].as<float>();
					cc2d.Restitution = circleCollider2DComponent["Restitution"].as<float>();
					cc2d.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
				}
			}
		}

		return newScene;
	}
}
