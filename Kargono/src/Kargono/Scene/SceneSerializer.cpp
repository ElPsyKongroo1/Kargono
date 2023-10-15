#include "kgpch.h"

#include "Kargono/Scene/SceneSerializer.h"
#include "Kargono/Scene/Entity.h"
#include "Kargono/Scene/Components.h"
#include "Kargono/Project/Project.h"
#include "Kargono/Scripting/ScriptEngine.h"
#include "Kargono/Core/FileSystem.h"

#include "API/Serialization/SerializationAPI.h"

namespace Kargono
{

#define WRITE_SCRIPT_FIELD(FieldType, Type)\
					case FieldType:\
						out << scriptField.GetValue<Type>();\
						break

#define READ_SCRIPT_FIELD(FieldType, Type)                  \
	case FieldType:                            \
	{                                                       \
		Type data = scriptField["Data"].as<Type>();       \
		fieldInstance.SetValue(data);                       \
		break;                                              \
	}

	static std::string RigidBody2DBodyTypeToString(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
		case Rigidbody2DComponent::BodyType::Static:	return "Static";
		case Rigidbody2DComponent::BodyType::Dynamic:	return "Dynamic";
		case Rigidbody2DComponent::BodyType::Kinematic:	return "Kinematic";
		}

		KG_CORE_ASSERT(false, "Unknown body type")
		return {};
	}

	static Rigidbody2DComponent::BodyType StringToRigidBody2DBodyType(const std::string& bodyTypeString)
	{
		if (bodyTypeString == "Static") return Rigidbody2DComponent::BodyType::Static;
		if (bodyTypeString == "Dynamic") return Rigidbody2DComponent::BodyType::Dynamic;
		if (bodyTypeString == "Kinematic") return Rigidbody2DComponent::BodyType::Kinematic;

		KG_CORE_ASSERT(false, "Unknown body type")
		return Rigidbody2DComponent::BodyType::Static;
	}


	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		:m_Scene(scene)
	{
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		KG_CORE_ASSERT(entity.HasComponent<IDComponent>(), "Entity does not have a component");

		out << YAML::BeginMap; // Entity Map
		out << YAML::Key << "Entity" << YAML::Value << static_cast<uint64_t>(entity.GetUUID());
		

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // Component Map
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // Component Map
			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			out << YAML::EndMap; // Component Map
		}
		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // Component Map

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
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
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;
			

			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<ShapeComponent>())
		{
			out << YAML::Key << "ShapeComponent";
			out << YAML::BeginMap; // Component Map
			auto& shapeComponent = entity.GetComponent<ShapeComponent>();
			out << YAML::Key << "CurrentShape" << YAML::Value << Shape::ShapeTypeToString(shapeComponent.CurrentShape);
			if (shapeComponent.Texture)
			{
				out << YAML::Key << "TextureHandle" << YAML::Value << static_cast<uint64_t>(shapeComponent.TextureHandle);
			}
			KG_CORE_ASSERT(sizeof(uint8_t) * 12 == sizeof(Shader::ShaderSpecification), "Please Update Deserialization and Serialization. Incorrect size of input data in Scene Serializer!");
			if (shapeComponent.Shader)
			{
				// Add Shader Handle
				out << YAML::Key << "ShaderHandle" << YAML::Value << static_cast<uint64_t>(shapeComponent.ShaderHandle);
				// Add Shader Specification
				const Shader::ShaderSpecification& shaderSpec = shapeComponent.Shader->GetSpecification();
				out << YAML::Key << "ShaderSpecification" << YAML::Value;
				out << YAML::BeginMap;
				out << YAML::Key << "AddFlatColor" << YAML::Value << shaderSpec.AddFlatColor;
				out << YAML::Key << "AddProjectionMatrix" << YAML::Value << shaderSpec.AddProjectionMatrix;
				out << YAML::Key << "AddEntityID" << YAML::Value << shaderSpec.AddEntityID;
				out << YAML::Key << "AddCircleShape" << YAML::Value << shaderSpec.AddCircleShape;
				out << YAML::Key << "AddTexture" << YAML::Value << shaderSpec.AddTexture;
				out << YAML::Key << "RenderType" << YAML::Value << Shape::RenderingTypeToString(shaderSpec.RenderType);
				out << YAML::EndMap;
				// Add Buffer
				out << YAML::Key << "Buffer" << YAML::Value << YAML::Binary(shapeComponent.ShaderData.Data, shapeComponent.ShaderData.Size);
			}
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Component Map
			auto& rb2dComponent = entity.GetComponent<Rigidbody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2dComponent.Type);
			out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // Component Map
			auto& bc2dComponent = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
			out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
			out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.RestitutionThreshold;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; // Component Map
			auto& cc2dComponent = entity.GetComponent<CircleCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << cc2dComponent.Offset;
			out << YAML::Key << "Radius" << YAML::Value << cc2dComponent.Radius;
			out << YAML::Key << "Density" << YAML::Value << cc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << cc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << cc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2dComponent.RestitutionThreshold;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<ScriptComponent>())
		{
			auto& scriptComponent = entity.GetComponent<ScriptComponent>();

			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap; // Component Map
			out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.ClassName;
			

			// Fields
			Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.ClassName);
			const auto& fields = entityClass->GetFields();
			if (fields.size() > 0)
			{
				out << YAML::Key << "ScriptFields" << YAML::Value;
				auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
				out << YAML::BeginSeq;
				for (const auto& [name, field] : fields)
				{

					if (!entityFields.contains(name)) { continue; }

					out << YAML::BeginMap; // Script Fields
					out << YAML::Key << "Name" << YAML::Value << name;
					out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);
					out << YAML::Key << "Data" << YAML::Value;

					ScriptFieldInstance& scriptField = entityFields.at(name);

					switch (field.Type)
					{
						WRITE_SCRIPT_FIELD(ScriptFieldType::Float,		float);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Double,		double);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Bool,		bool);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Char,		char);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Byte,		int8_t);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Short,		int16_t);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Int,		int32_t);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Long,		int64_t);
						WRITE_SCRIPT_FIELD(ScriptFieldType::UByte,		uint8_t);
						WRITE_SCRIPT_FIELD(ScriptFieldType::UShort,		uint16_t);
						WRITE_SCRIPT_FIELD(ScriptFieldType::UInt,		uint32_t);
						WRITE_SCRIPT_FIELD(ScriptFieldType::ULong,		uint64_t);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Vector2,	glm::vec2);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Vector3,	glm::vec3);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Vector4,	glm::vec4);
						WRITE_SCRIPT_FIELD(ScriptFieldType::Entity,		uint64_t);

					}

					out << YAML::EndMap; // Script Fields
				}
				out << YAML::EndSeq;
			}


			out << YAML::EndMap; // Component Map
		}
		

		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.each([&](auto entityID)
		{
				Entity entity = { entityID, m_Scene.get() };
				if (!entity) { return; }

				SerializeEntity(out, entity);
		});
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
		
	}
	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		KG_CORE_ASSERT(false, "Not Implemented")
	}
	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			KG_CORE_ERROR("Failed to load .kargono file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		if (!data["Scene"]) { return false; }

		std::string sceneName = data["Scene"].as<std::string>();
		KG_CORE_TRACE("Deserializing scene");

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

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Translation = transformComponent["Translation"].as<glm::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<CameraComponent>();

					const auto& cameraProps = cameraComponent["Camera"];

					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());
					cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
					cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				auto scriptComponent = entity["ScriptComponent"];
				if (scriptComponent)
				{
					auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
					sc.ClassName = scriptComponent["ClassName"].as<std::string>();

					auto scriptFields = scriptComponent["ScriptFields"];
					if (scriptFields)
					{

						Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName);
						if (entityClass)
						{
							KG_CORE_ASSERT(entityClass);
							const auto& fields = entityClass->GetFields();

							auto& entityFields = ScriptEngine::GetScriptFieldMap(deserializedEntity);

							for (auto scriptField : scriptFields)
							{
								std::string name = scriptField["Name"].as<std::string>();
								std::string typeString = scriptField["Type"].as<std::string>();
								ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

								ScriptFieldInstance& fieldInstance = entityFields[name];
								// TODO(): Turn into Log Message
								KG_CORE_ASSERT(fields.contains(name))
									if (!fields.contains(name)) { continue; }
								fieldInstance.Field = fields.at(name);

								switch (type)
								{
									READ_SCRIPT_FIELD(ScriptFieldType::Float, float);
									READ_SCRIPT_FIELD(ScriptFieldType::Double, double);
									READ_SCRIPT_FIELD(ScriptFieldType::Bool, bool);
									READ_SCRIPT_FIELD(ScriptFieldType::Char, char);
									READ_SCRIPT_FIELD(ScriptFieldType::Byte, int8_t);
									READ_SCRIPT_FIELD(ScriptFieldType::Short, int16_t);
									READ_SCRIPT_FIELD(ScriptFieldType::Int, int32_t);
									READ_SCRIPT_FIELD(ScriptFieldType::Long, int64_t);
									READ_SCRIPT_FIELD(ScriptFieldType::UByte, uint8_t);
									READ_SCRIPT_FIELD(ScriptFieldType::UShort, uint16_t);
									READ_SCRIPT_FIELD(ScriptFieldType::UInt, uint32_t);
									READ_SCRIPT_FIELD(ScriptFieldType::ULong, uint64_t);
									READ_SCRIPT_FIELD(ScriptFieldType::Vector2, glm::vec2);
									READ_SCRIPT_FIELD(ScriptFieldType::Vector3, glm::vec3);
									READ_SCRIPT_FIELD(ScriptFieldType::Vector4, glm::vec4);
									READ_SCRIPT_FIELD(ScriptFieldType::Entity, UUID);
								}
							}
						}
					}
				}

				auto shapeComponent = entity["ShapeComponent"];
				if (shapeComponent)
				{
					auto& sc = deserializedEntity.AddComponent<ShapeComponent>();
					sc.CurrentShape = Shape::StringToShapeType(shapeComponent["CurrentShape"].as<std::string>());
					if(sc.CurrentShape != Shape::ShapeTypes::None)
					{
						sc.Vertices = CreateRef<std::vector<glm::vec3>>(Shape::ShapeTypeToShape(sc.CurrentShape).GetVertices());
						sc.Indices = CreateRef<std::vector<uint32_t>>(Shape::ShapeTypeToShape(sc.CurrentShape).GetIndices());
						sc.TextureCoordinates = CreateRef<std::vector<glm::vec2>>(Shape::ShapeTypeToShape(sc.CurrentShape).GetTextureCoordinates());
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
							Shader::ShaderSpecification shaderSpec{};
							// ShaderSpecification Section
							shaderSpec.AddFlatColor = shaderSpecificationNode["AddFlatColor"].as<bool>();
							shaderSpec.AddProjectionMatrix = shaderSpecificationNode["AddProjectionMatrix"].as<bool>();
							shaderSpec.AddEntityID = shaderSpecificationNode["AddEntityID"].as<bool>();
							shaderSpec.AddCircleShape = shaderSpecificationNode["AddCircleShape"].as<bool>();
							shaderSpec.AddTexture = shaderSpecificationNode["AddTexture"].as<bool>();
							shaderSpec.RenderType = Shape::StringToRenderingType(shaderSpecificationNode["RenderType"].as<std::string>());
							auto [newHandle , newShader] = AssetManager::GetShader(shaderSpec);
							shaderHandle = newHandle;
							sc.Shader = newShader;
						}
						sc.ShaderHandle = shaderHandle;
						sc.ShaderSpecification = sc.Shader->GetSpecification();
						YAML::Binary binary = shapeComponent["Buffer"].as<YAML::Binary>();
						Buffer buffer{ binary.size() };
						memcpy_s(buffer.Data, buffer.Size, binary.data(), buffer.Size);
						sc.ShaderData = buffer;
					}
				}

				auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
				if (rigidbody2DComponent)
				{
					auto& rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
					rb2d.Type = StringToRigidBody2DBodyType(rigidbody2DComponent["BodyType"].as<std::string>());
					rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
				}

				auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
				if (boxCollider2DComponent)
				{
					auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();
					bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
					bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
					bc2d.Density = boxCollider2DComponent["Density"].as<float>();
					bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
					bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
					bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
				}

				auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
				if (circleCollider2DComponent)
				{
					auto& cc2d = deserializedEntity.AddComponent<CircleCollider2DComponent>();
					cc2d.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
					cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
					cc2d.Density = circleCollider2DComponent["Density"].as<float>();
					cc2d.Friction = circleCollider2DComponent["Friction"].as<float>();
					cc2d.Restitution = circleCollider2DComponent["Restitution"].as<float>();
					cc2d.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
				}
			}
		}
		return true;

	}
	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		KG_CORE_ASSERT(false, "Not Implemented")
		return false;
	}
}
