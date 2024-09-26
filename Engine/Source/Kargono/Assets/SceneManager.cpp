#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Scenes/Entity.h"

#include "API/Serialization/yamlcppAPI.h"

namespace Kargono::Utility
{
	static bool SerializeEntity(YAML::Emitter& out, Scenes::Entity entity)
	{
		KG_ASSERT(entity.HasComponent<Scenes::IDComponent>(), "Entity does not have a component");
		out << YAML::BeginMap; // Entity Map
		out << YAML::Key << "Entity" << YAML::Value << static_cast<uint64_t>(entity.GetUUID());

		if (entity.HasComponent<Scenes::TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // Component Map
			auto& tag = entity.GetComponent<Scenes::TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<Scenes::ClassInstanceComponent>())
		{
			out << YAML::Key << "ClassInstanceComponent";
			Scenes::ClassInstanceComponent& comp = entity.GetComponent<Scenes::ClassInstanceComponent>();
			out << YAML::BeginMap; // Component Map
			out << YAML::Key << "ClassHandle" << YAML::Value << static_cast<uint64_t>(comp.ClassHandle);
			if (comp.ClassHandle != Assets::EmptyHandle)
			{
				Ref<Scenes::EntityClass> entityClass = Assets::AssetManager::GetEntityClass(comp.ClassHandle);
				if (!entityClass)
				{
					KG_ERROR("Attempt to serialize ClassInstanceComponent without valid entityClass");
					return false;
				}
				if (entityClass->GetFields().size() != comp.Fields.size())
				{
					KG_ERROR("Attempt to serialize ClassInstanceComponent where class and instance fields are unaligned");
					return false;
				}

				out << YAML::Key << "InstanceFields" << YAML::Value << YAML::BeginSeq; // Begin Fields
				uint32_t iteration{ 0 };
				for (auto& fieldValue : comp.Fields)
				{
					const Scenes::ClassField& fieldType = entityClass->GetFields().at(iteration);
					if (fieldType.Type != fieldValue->Type())
					{
						KG_ERROR("Attempt to serialize ClassInstanceComponent with incorrect types");
						return false;
					}
					out << YAML::BeginMap; // Begin Field Map
					out << YAML::Key << "Name" << YAML::Value << fieldType.Name;
					out << YAML::Key << "Type" << YAML::Value << Utility::WrappedVarTypeToString(fieldType.Type);
					SerializeWrappedVariableData(fieldValue, out);
					out << YAML::EndMap; // End Field Map
					iteration++;
				}
				out << YAML::EndSeq; // End Fields
			}
			
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<Scenes::TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // Component Map
			auto& tc = entity.GetComponent<Scenes::TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			out << YAML::EndMap; // Component Map
		}
		if (entity.HasComponent<Scenes::CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // Component Map

			auto& cameraComponent = entity.GetComponent<Scenes::CameraComponent>();
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

		if (entity.HasComponent<Scenes::ShapeComponent>())
		{
			out << YAML::Key << "ShapeComponent";
			out << YAML::BeginMap; // Component Map
			auto& shapeComponent = entity.GetComponent<Scenes::ShapeComponent>();
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

		if (entity.HasComponent<Scenes::Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Component Map
			auto& rb2dComponent = entity.GetComponent<Scenes::Rigidbody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << Utility::RigidBody2DBodyTypeToString(rb2dComponent.Type);
			out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<Scenes::BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // Component Map
			auto& bc2dComponent = entity.GetComponent<Scenes::BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
			out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
			out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.RestitutionThreshold;
			out << YAML::EndMap; // Component Map
		}

		if (entity.HasComponent<Scenes::CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; // Component Map
			auto& cc2dComponent = entity.GetComponent<Scenes::CircleCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << cc2dComponent.Offset;
			out << YAML::Key << "Radius" << YAML::Value << cc2dComponent.Radius;
			out << YAML::Key << "Density" << YAML::Value << cc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << cc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << cc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2dComponent.RestitutionThreshold;
			out << YAML::EndMap; // Component Map
		}


		out << YAML::EndMap; // Entity
		return true;
	}
}

namespace Kargono::Assets
{
	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_SceneRegistry {};

	void AssetManager::DeserializeSceneRegistry()
	{
		// Clear current registry and open registry in current project 
		s_SceneRegistry.clear();
		KG_ASSERT(Projects::ProjectService::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& sceneRegistryLocation = Projects::ProjectService::GetActiveAssetDirectory() / "Scenes/SceneRegistry.kgreg";

		if (!std::filesystem::exists(sceneRegistryLocation))
		{
			KG_WARN("No .kgregistry file exists in project path!");
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
				newAsset.Data.FileLocation = metadata["FileLocation"].as<std::string>();
				newAsset.Data.Type = Utility::StringToAssetType(metadata["AssetType"].as<std::string>());

				// Retrieving shader specific metadata 
				if (newAsset.Data.Type == Assets::AssetType::Scene)
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
		KG_ASSERT(Projects::ProjectService::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& sceneRegistryLocation = Projects::ProjectService::GetActiveAssetDirectory() / "Scenes/SceneRegistry.kgreg";
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
			out << YAML::Key << "FileLocation" << YAML::Value << asset.Data.FileLocation.string();
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

	void AssetManager::SerializeScene(Ref<Scenes::Scene> scene, const std::filesystem::path& filepath)
	{
		bool submitScene = true;
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
			Scenes::Entity entity = { entityID, scene.get() };
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
			std::ofstream fout(filepath);
			fout << out.c_str();
			KG_INFO("Successfully Serialized Scene at {}", filepath);
		}
		else
		{
			KG_WARN("Failed to Serialize Scene");
		}
		
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

	void AssetManager::SaveScene(AssetHandle sceneHandle, Ref<Scenes::Scene> scene)
	{
		if (!s_SceneRegistry.contains(sceneHandle))
		{
			KG_ERROR("Attempt to save scene that does not exist in registry");
			return;
		}
		Assets::Asset sceneAsset = s_SceneRegistry[sceneHandle];
		SerializeScene(scene, (Projects::ProjectService::GetActiveAssetDirectory() / sceneAsset.Data.FileLocation).string());
	}

	void AssetManager::CreateSceneFile(const std::string& sceneName, Assets::Asset& newAsset)
	{
		// Create Temporary Scene
		Ref<Scenes::Scene> temporaryScene = CreateRef<Scenes::Scene>();

		// Save Binary into File
		std::string scenePath = "Scenes/" + sceneName + ".kgscene";
		std::filesystem::path fullPath = Projects::ProjectService::GetActiveAssetDirectory() / scenePath;
		SerializeScene(temporaryScene, fullPath.string());

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Scene;
		newAsset.Data.FileLocation = scenePath;
		Ref<Assets::SceneMetaData> metadata = CreateRef<Assets::SceneMetaData>();
		newAsset.Data.SpecificFileData = metadata;
	}

//===================================================================================================================================================
	std::tuple<AssetHandle, Ref<Scenes::Scene>> AssetManager::GetScene(const std::filesystem::path& filepath)
	{
		KG_ASSERT(Projects::ProjectService::GetActive(), "Attempt to use Project Field without active project!");

		std::filesystem::path scenePath {};

		if (Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveAssetDirectory(), filepath))
		{
			scenePath = Utility::FileSystem::GetRelativePath(Projects::ProjectService::GetActiveAssetDirectory(), filepath);
		}
		else
		{
			scenePath = filepath;
		}

		for (auto& [assetHandle, asset] : s_SceneRegistry)
		{
			if (asset.Data.FileLocation.compare(scenePath) == 0)
			{
				return std::make_tuple(assetHandle, InstantiateScene(asset));
			}
		}
		// Return empty scene if scene does not exist
		KG_WARN("No Scene Associated with provided handle. Returned new empty scene");
		AssetHandle newHandle = CreateNewScene(filepath.stem().string());
		return std::make_tuple(newHandle, GetScene(newHandle));
	}
	
	void AssetManager::ClearSceneRegistry()
	{
		s_SceneRegistry.clear();
	}

	bool AssetManager::DeserializeScene(Ref<Scenes::Scene> scene, const std::filesystem::path& filepath)
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

				Scenes::Entity deserializedEntity = scene->CreateEntityWithUUID(uuid, name);

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
						if (!s_EntityClassRegistry.contains(cInstComp.ClassHandle))
						{
							KG_ERROR("Could not find entity class for class instance component");
							return false;
						}
						cInstComp.ClassReference = GetEntityClass(cInstComp.ClassHandle);

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
		return true;

	}
	Ref<Scenes::Scene> AssetManager::InstantiateScene(const Assets::Asset& sceneAsset)
	{
		Ref<Scenes::Scene> newScene = CreateRef<Scenes::Scene>();
		DeserializeScene(newScene, (Projects::ProjectService::GetActiveAssetDirectory() / sceneAsset.Data.FileLocation).string());
		return newScene;
	}

	Ref<Scenes::Scene> AssetManager::GetScene(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::ProjectService::GetActive(), "There is no active project when retreiving scene!");

		if (s_SceneRegistry.contains(handle))
		{
			auto asset = s_SceneRegistry[handle];
			return InstantiateScene(asset);
		}

		KG_ERROR("No scene is associated with provided handle!");
		return nullptr;
	}
}
