#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Projects/Project.h"
#include "API/Serialization/SerializationAPI.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Scene/Entity.h"

namespace Kargono::Utility
{
	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		KG_ASSERT(entity.HasComponent<IDComponent>(), "Entity does not have a component");

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


			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<AudioComponent>())
		{
			out << YAML::Key << "AudioComponent";
			out << YAML::BeginMap; // Component Map
			auto& audioComponent = entity.GetComponent<AudioComponent>();
			out << YAML::Key << "Name" << YAML::Value << audioComponent.Name;
			out << YAML::Key << "AudioHandle" << YAML::Value << static_cast<uint64_t>(audioComponent.AudioHandle);
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<MultiAudioComponent>())
		{
			out << YAML::Key << "MultiAudioComponent";
			out << YAML::BeginSeq; // Component Sequence

			for (auto& [key, audioComp] : entity.GetComponent<MultiAudioComponent>().AudioComponents)
			{
				out << YAML::BeginMap; // Audio Component Map
				out << YAML::Key << "Name" << YAML::Value << audioComp.Name;
				out << YAML::Key << "AudioHandle" << YAML::Value << static_cast<uint64_t>(audioComp.AudioHandle);
				out << YAML::EndMap; // Audio Component Map
			}

			out << YAML::EndSeq; // Component Sequence
		}

		if (entity.HasComponent<ShapeComponent>())
		{
			out << YAML::Key << "ShapeComponent";
			out << YAML::BeginMap; // Component Map
			auto& shapeComponent = entity.GetComponent<ShapeComponent>();
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
			KG_ASSERT(sizeof(uint8_t) * 20 == sizeof(ShaderSpecification), "Please Update Deserialization and Serialization. Incorrect size of input data in Scene Serializer!");
			if (shapeComponent.Shader)
			{
				// Add Shader Handle
				out << YAML::Key << "ShaderHandle" << YAML::Value << static_cast<uint64_t>(shapeComponent.ShaderHandle);
				// Add Shader Specification
				const ShaderSpecification& shaderSpec = shapeComponent.Shader->GetSpecification();
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

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Component Map
			auto& rb2dComponent = entity.GetComponent<Rigidbody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << Utility::RigidBody2DBodyTypeToString(rb2dComponent.Type);
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
			Ref<Script::ScriptClass> entityClass = Script::ScriptEngine::GetEntityClass(scriptComponent.ClassName);
			const auto& fields = entityClass->GetFields();
			if (fields.size() > 0)
			{
				out << YAML::Key << "ScriptFields" << YAML::Value;
				auto& entityFields = Script::ScriptEngine::GetScriptFieldMap(entity);
				out << YAML::BeginSeq;
				for (const auto& [name, field] : fields)
				{

					if (!entityFields.contains(name)) { continue; }

					out << YAML::BeginMap; // Script Fields
					out << YAML::Key << "Name" << YAML::Value << name;
					out << YAML::Key << "Type" << YAML::Value << Utility::ScriptFieldTypeToString(field.Type);
					out << YAML::Key << "Data" << YAML::Value;

					Script::ScriptFieldInstance& scriptField = entityFields.at(name);

					switch (field.Type)
					{
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Float, float);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Double, double);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Bool, bool);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Char, char);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Byte, int8_t);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Short, int16_t);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Int, int32_t);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Long, int64_t);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::UByte, uint8_t);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::UShort, uint16_t);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::UInt, uint32_t);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::ULong, uint64_t);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Vector2, Math::vec2);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Vector3, Math::vec3);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Vector4, Math::vec4);
						WRITE_SCRIPT_FIELD(Script::ScriptFieldType::Entity, uint64_t);

					}

					out << YAML::EndMap; // Script Fields
				}
				out << YAML::EndSeq;
			}


			out << YAML::EndMap; // Component Map
		}


		out << YAML::EndMap; // Entity
	}
}

namespace Kargono::Assets
{
	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_SceneRegistry {};

	void AssetManager::DeserializeSceneRegistry()
	{
		// Clear current registry and open registry in current project 
		s_SceneRegistry.clear();
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& sceneRegistryLocation = Projects::Project::GetAssetDirectory() / "Scenes/SceneRegistry.kgreg";

		if (!std::filesystem::exists(sceneRegistryLocation))
		{
			KG_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(sceneRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgscene file '{0}'\n     {1}", sceneRegistryLocation.string(), e.what());
			return;
		}

		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing Scene Registry");

		// Opening all assets 
		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Assets::Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				// Retrieving metadata for asset 
				auto metadata = asset["MetaData"];
				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Utility::StringToAssetType(metadata["AssetType"].as<std::string>());

				// Retrieving shader specific metadata 
				if (newAsset.Data.Type == Assets::Scene)
				{
					Ref<Assets::SceneMetaData> sceneMetaData = CreateRef<Assets::SceneMetaData>();
					newAsset.Data.SpecificFileData = sceneMetaData;
				}

				// Add asset to in memory registry 
				s_SceneRegistry.insert({ newAsset.Handle, newAsset });
			}
		}
	}

	void AssetManager::SerializeSceneRegistry()
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& sceneRegistryLocation = Projects::Project::GetAssetDirectory() / "Scenes/SceneRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "Scene";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_SceneRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);
			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		Utility::FileSystem::CreateNewDirectory(sceneRegistryLocation.parent_path());

		std::ofstream fout(sceneRegistryLocation);
		fout << out.c_str();
	}

	void AssetManager::SerializeScene(Ref<Kargono::Scene> scene, const std::filesystem::path& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		{ // Physics
			out << YAML::Key << "Physics" << YAML::BeginMap; // Physics Map
			out << YAML::Key << "Gravity" << YAML::Value << scene->m_PhysicsSpecification.Gravity;
			out << YAML::EndMap; // Physics Maps
		}

		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		scene->m_Registry.each([&](auto entityID)
			{
				Entity entity = { entityID, scene.get() };
				if (!entity) { return; }

				Utility::SerializeEntity(out, entity);
			});
		out << YAML::EndSeq;
		out << YAML::EndMap; // Start of File Map

		std::ofstream fout(filepath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized Scene at {}", filepath);
	}

	bool AssetManager::CheckSceneExists(const std::string& sceneName)
	{
		// Create Checksum
		const std::string currentCheckSum = Utility::FileSystem::ChecksumFromString(sceneName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_SceneRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return true;
			}
		}

		return false;
	}

	bool AssetManager::DeserializeScene(Ref<Kargono::Scene> scene, const std::filesystem::path& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgscene file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		KG_INFO("Deserializing scene");

		auto physics = data["Physics"];
		scene->GetPhysicsSpecification().Gravity = physics["Gravity"].as<Math::vec2>();

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

				Entity deserializedEntity = scene->CreateEntityWithUUID(uuid, name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Translation = transformComponent["Translation"].as<Math::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<Math::vec3>();
					tc.Scale = transformComponent["Scale"].as<Math::vec3>();
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
				}

				auto audioComponent = entity["AudioComponent"];
				if (audioComponent)
				{
					auto& audioComp = deserializedEntity.AddComponent<AudioComponent>();
					audioComp.Name = audioComponent["Name"].as<std::string>();
					audioComp.AudioHandle = audioComponent["AudioHandle"].as<uint64_t>();
					audioComp.Audio = AssetManager::GetAudio(audioComp.AudioHandle);
				}

				auto multiAudioComponent = entity["MultiAudioComponent"];
				if (multiAudioComponent)
				{
					auto& multiAudioComp = deserializedEntity.AddComponent<MultiAudioComponent>();

					for (auto audioComp : multiAudioComponent)
					{
						AudioComponent newComponent{};
						newComponent.Name = audioComp["Name"].as<std::string>();
						newComponent.AudioHandle = audioComp["AudioHandle"].as<uint64_t>();
						newComponent.Audio = AssetManager::GetAudio(newComponent.AudioHandle);
						multiAudioComp.AudioComponents.insert({ newComponent.Name, newComponent });
					}
				}

				auto scriptComponent = entity["ScriptComponent"];
				if (scriptComponent)
				{
					auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
					sc.ClassName = scriptComponent["ClassName"].as<std::string>();

					auto scriptFields = scriptComponent["ScriptFields"];
					if (scriptFields)
					{

						Ref<Script::ScriptClass> entityClass = Script::ScriptEngine::GetEntityClass(sc.ClassName);
						if (entityClass)
						{
							KG_ASSERT(entityClass);
							const auto& fields = entityClass->GetFields();

							auto& entityFields = Script::ScriptEngine::GetScriptFieldMap(deserializedEntity);

							for (auto scriptField : scriptFields)
							{
								std::string name = scriptField["Name"].as<std::string>();
								std::string typeString = scriptField["Type"].as<std::string>();
								Script::ScriptFieldType type = Utility::ScriptFieldTypeFromString(typeString);

								Script::ScriptFieldInstance& fieldInstance = entityFields[name];
								// TODO(): Turn into Log Message
								KG_ASSERT(fields.contains(name))
									if (!fields.contains(name)) { continue; }
								fieldInstance.Field = fields.at(name);

								switch (type)
								{
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Float, float);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Double, double);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Bool, bool);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Char, char);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Byte, int8_t);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Short, int16_t);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Int, int32_t);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Long, int64_t);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::UByte, uint8_t);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::UShort, uint16_t);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::UInt, uint32_t);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::ULong, uint64_t);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Vector2, Math::vec2);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Vector3, Math::vec3);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Vector4, Math::vec4);
									READ_SCRIPT_FIELD(Script::ScriptFieldType::Entity, UUID);
								}
							}
						}
					}
				}

				auto shapeComponent = entity["ShapeComponent"];
				if (shapeComponent)
				{
					auto& sc = deserializedEntity.AddComponent<ShapeComponent>();
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
							ShaderSpecification shaderSpec{};
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
						if (sc.CurrentShape != ShapeTypes::None)
						{
							if (sc.ShaderSpecification.RenderType == RenderingType::DrawIndex)
							{
								sc.Vertices = CreateRef<std::vector<Math::vec3>>(Utility::ShapeTypeToShape(sc.CurrentShape).GetIndexVertices());
								sc.Indices = CreateRef<std::vector<uint32_t>>(Utility::ShapeTypeToShape(sc.CurrentShape).GetIndices());
								sc.TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Utility::ShapeTypeToShape(sc.CurrentShape).GetIndexTextureCoordinates());
							}

							if (sc.ShaderSpecification.RenderType == RenderingType::DrawTriangle)
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
					auto& rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
					rb2d.Type = Utility::StringToRigidBody2DBodyType(rigidbody2DComponent["BodyType"].as<std::string>());
					rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
				}

				auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
				if (boxCollider2DComponent)
				{
					auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();
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
					auto& cc2d = deserializedEntity.AddComponent<CircleCollider2DComponent>();
					cc2d.Offset = circleCollider2DComponent["Offset"].as<Math::vec2>();
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

	AssetHandle AssetManager::CreateNewScene(const std::string& sceneName)
	{
		// Create Checksum
		const std::string currentCheckSum = Utility::FileSystem::ChecksumFromString(sceneName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets
		for (const auto& [handle, asset] : s_SceneRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};
		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create File
		CreateSceneFile(sceneName, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and Create Scene
		s_SceneRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeSceneRegistry(); // Update Registry File on Disk

		return newHandle;
	}

	void AssetManager::SaveScene(AssetHandle sceneHandle, Ref<Kargono::Scene> scene)
	{
		if (!s_SceneRegistry.contains(sceneHandle))
		{
			KG_ERROR("Attempt to save scene that does not exist in registry");
			return;
		}
		Assets::Asset sceneAsset = s_SceneRegistry[sceneHandle];
		SerializeScene(scene, (Projects::Project::GetAssetDirectory() / sceneAsset.Data.IntermediateLocation).string());
	}

	Ref<Kargono::Scene> AssetManager::GetScene(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no active project when retreiving scene!");

		if (s_SceneRegistry.contains(handle))
		{
			auto asset = s_SceneRegistry[handle];
			return InstantiateScene(asset);
		}

		KG_ERROR("No scene is associated with provided handle!");
		return nullptr;
	}
	std::tuple<AssetHandle, Ref<Kargono::Scene>> AssetManager::GetScene(const std::filesystem::path& filepath)
	{
		KG_ASSERT(Projects::Project::GetActive(), "Attempt to use Project Field without active project!");

		std::filesystem::path scenePath {};

		if (Utility::FileSystem::DoesPathContainSubPath(Projects::Project::GetAssetDirectory(), filepath))
		{
			scenePath = Utility::FileSystem::GetRelativePath(Projects::Project::GetAssetDirectory(), filepath);
		}
		else
		{
			scenePath = filepath;
		}

		for (auto& [assetHandle, asset] : s_SceneRegistry)
		{
			if (asset.Data.IntermediateLocation.compare(scenePath) == 0)
			{
				return std::make_tuple(assetHandle, InstantiateScene(asset));
			}
		}
		// Return empty scene if scene does not exist
		KG_WARN("No Scene Associated with provided handle. Returned new empty scene");
		AssetHandle newHandle = CreateNewScene(filepath.stem().string());
		return std::make_tuple(newHandle, GetScene(newHandle));
	}

	Ref<Kargono::Scene> AssetManager::InstantiateScene(const Assets::Asset& sceneAsset)
	{
		Ref<Kargono::Scene> newScene = CreateRef<Kargono::Scene>();
		DeserializeScene(newScene, (Projects::Project::GetAssetDirectory() / sceneAsset.Data.IntermediateLocation).string());
		return newScene;
	}


	void AssetManager::ClearSceneRegistry()
	{
		s_SceneRegistry.clear();
	}

	void AssetManager::CreateSceneFile(const std::string& sceneName, Assets::Asset& newAsset)
	{
		// Create Temporary Scene
		Ref<Kargono::Scene> temporaryScene = CreateRef<Kargono::Scene>();

		// Save Binary Intermediate into File
		std::string scenePath = "Scenes/" + sceneName + ".kgscene";
		std::filesystem::path intermediateFullPath = Projects::Project::GetAssetDirectory() / scenePath;
		SerializeScene(temporaryScene, intermediateFullPath.string());

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Scene;
		newAsset.Data.IntermediateLocation = scenePath;
		Ref<Assets::SceneMetaData> metadata = CreateRef<Assets::SceneMetaData>();
		newAsset.Data.SpecificFileData = metadata;
	}
}
