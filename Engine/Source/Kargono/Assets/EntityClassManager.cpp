#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/Scenes/Entity.h"

#include "API/Serialization/yamlcppAPI.h"

namespace Kargono::Utility
{
	static void TransferClassInstanceFieldData(Ref<Scenes::Scene> scene, Ref<Scenes::EntityClass> entityClass, Assets::AssetHandle entityHandle,
		const std::unordered_map<uint32_t, uint32_t>& transferMap)
	{
		if (scene)
		{
			for (auto entity : scene->GetAllEntitiesWith<Scenes::ClassInstanceComponent>())
			{
				Scenes::Entity currentEntity { entity, scene.get() };
				Scenes::ClassInstanceComponent& component = currentEntity.GetComponent<Scenes::ClassInstanceComponent>();
				if (component.ClassHandle == entityHandle)
				{
					component.ClassReference = entityClass;
					auto oldVariables = component.Fields;
					component.Fields.clear();
					for (auto& [name, type] : entityClass->GetFields())
					{
						component.Fields.push_back(Utility::WrappedVarTypeToWrappedVariable(type));
					}

					// Transfer Data
					for (auto [oldLoc, newLoc] : transferMap)
					{
						component.Fields.at(newLoc)->SetValue(oldVariables.at(oldLoc)->GetValue());
					}
				}
				
			}
		}
	}

	static void ClearClassReferenceFromScene(Ref<Scenes::Scene> scene,Ref<Scenes::EntityClass> entityClass, Assets::AssetHandle entityHandle)
	{
		if (scene)
		{
			for (auto entity : scene->GetAllEntitiesWith<Scenes::ClassInstanceComponent>())
			{
				Scenes::Entity currentEntity { entity, scene.get() };
				Scenes::ClassInstanceComponent& component = currentEntity.GetComponent<Scenes::ClassInstanceComponent>();
				if (component.ClassHandle == entityHandle)
				{
					component.ClassHandle = Assets::EmptyHandle;
					component.ClassReference = nullptr;
				 	bool success = component.Fields.empty();
				}

			}
		}
	}
}

namespace Kargono::Assets
{
	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_EntityClassRegistry {};

	void AssetManager::DeserializeEntityClassRegistry()
	{
		// Clear current registry and open registry in current project 
		s_EntityClassRegistry.clear();
		KG_ASSERT(Projects::ProjectService::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& EntityClassRegistryLocation = Projects::ProjectService::GetActiveAssetDirectory() / "EntityClass/EntityClassRegistry.kgreg";

		if (!std::filesystem::exists(EntityClassRegistryLocation))
		{
			KG_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(EntityClassRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgstate file '{0}'\n     {1}", EntityClassRegistryLocation.string(), e.what());
			return;
		}

		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing EntityClass Registry");

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

				// Retrieving EntityClass specific metadata 
				if (newAsset.Data.Type == Assets::EntityClass)
				{
					Ref<Assets::EntityClassMetaData> EntityClassMetaData = CreateRef<Assets::EntityClassMetaData>();
					EntityClassMetaData->Name = metadata["Name"].as<std::string>();

					newAsset.Data.SpecificFileData = EntityClassMetaData;
				}

				// Add asset to in memory registry 
				s_EntityClassRegistry.insert({ newAsset.Handle, newAsset });
			}
		}
	}

	void AssetManager::SerializeEntityClassRegistry()
	{
		KG_ASSERT(Projects::ProjectService::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& EntityClassRegistryLocation = Projects::ProjectService::GetActiveAssetDirectory() / "EntityClass/EntityClassRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "EntityClass";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_EntityClassRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);
			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);
			if (asset.Data.Type == Assets::AssetType::EntityClass)
			{
				Assets::EntityClassMetaData* metadata = static_cast<Assets::EntityClassMetaData*>(asset.Data.SpecificFileData.get());

				out << YAML::Key << "Name" << YAML::Value << metadata->Name;
			}

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		Utility::FileSystem::CreateNewDirectory(EntityClassRegistryLocation.parent_path());

		std::ofstream fout(EntityClassRegistryLocation);
		fout << out.c_str();
	}

	void AssetManager::SerializeEntityClass(Ref<Scenes::EntityClass> EntityClass, const std::filesystem::path& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map

		out << YAML::Key << "Name" << YAML::Value << EntityClass->GetName(); // Output State Name
		out << YAML::Key << "FieldTypes" << YAML::Value;
		out << YAML::BeginSeq; // Start Fields

		for (auto& [name, field] : EntityClass->m_FieldTypes)
		{
			out << YAML::BeginMap; // Start Field

			out << YAML::Key << "Name" << YAML::Value << name; // Name/Map Key
			out << YAML::Key << "Type" << YAML::Value << Utility::WrappedVarTypeToString(field); // Field Type

			out << YAML::EndMap; // End Field
		}

		out << YAML::EndSeq; // End Fields

		out << YAML::Key << "OnPhysicsCollisionStart" << YAML::Value <<
			static_cast<uint64_t>(EntityClass->m_Scripts.OnPhysicsCollisionStartHandle);
		out << YAML::Key << "OnPhysicsCollisionEnd" << YAML::Value <<
			static_cast<uint64_t>(EntityClass->m_Scripts.OnPhysicsCollisionEndHandle);
		out << YAML::Key << "OnCreate" << YAML::Value <<
			static_cast<uint64_t>(EntityClass->m_Scripts.OnCreateHandle);
		out << YAML::Key << "OnUpdate" << YAML::Value <<
			static_cast<uint64_t>(EntityClass->m_Scripts.OnUpdateHandle);

		out << YAML::Key << "AllScripts" << YAML::Value;
		out << YAML::BeginSeq; // Start AllScripts

		for (auto& script : EntityClass->m_Scripts.AllClassScripts)
		{
			out << YAML::Value << static_cast<uint64_t>(script); // Script ID
		}

		out << YAML::EndSeq; // End AllScripts

		out << YAML::EndMap; // End of File Map

		std::ofstream fout(filepath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized EntityClass at {}", filepath);
	}

	bool AssetManager::CheckEntityClassExists(const std::string& EntityClassName)
	{
		// Create Checksum
		const std::string currentCheckSum = Utility::FileSystem::ChecksumFromString(EntityClassName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_EntityClassRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return true;
			}
		}

		return false;
	}

	bool AssetManager::DeserializeEntityClass(Ref<Scenes::EntityClass> EntityClass, const std::filesystem::path& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgui file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		KG_INFO("Deserializing entity class");

		EntityClass->m_Name = data["Name"].as<std::string>();

		// Get Fields
		{
			auto fields = data["FieldTypes"];
			if (fields)
			{
				auto& newFieldsMap = EntityClass->m_FieldTypes;
				for (auto field : fields)
				{
					std::string fieldName = field["Name"].as<std::string>();
					WrappedVarType fieldType = Utility::StringToWrappedVarType(field["Type"].as<std::string>());
					if (!EntityClass->AddField(fieldName, fieldType))
					{
						KG_WARN("Unable to add field inside deserialize entity class function");
					}

				}
			}
		}

		// Get Function Slots
		{
			Scenes::EntityScripts& scripts = EntityClass->m_Scripts;
			scripts.OnPhysicsCollisionStartHandle =
				static_cast<Assets::AssetHandle>(data["OnPhysicsCollisionStart"].as<uint64_t>());
			if (scripts.OnPhysicsCollisionStartHandle != Assets::EmptyHandle)
			{
				scripts.OnPhysicsCollisionStart =
					Assets::AssetManager::GetScript(scripts.OnPhysicsCollisionStartHandle).get();
			}
			scripts.OnPhysicsCollisionEndHandle =
				static_cast<Assets::AssetHandle>(data["OnPhysicsCollisionEnd"].as<uint64_t>());
			if (scripts.OnPhysicsCollisionEndHandle != Assets::EmptyHandle)
			{
				scripts.OnPhysicsCollisionEnd =
					Assets::AssetManager::GetScript(scripts.OnPhysicsCollisionEndHandle).get();
			}
			scripts.OnCreateHandle =
				static_cast<Assets::AssetHandle>(data["OnCreate"].as<uint64_t>());
			if (scripts.OnCreateHandle != Assets::EmptyHandle)
			{
				scripts.OnCreate =
					Assets::AssetManager::GetScript(scripts.OnCreateHandle).get();
			}
			scripts.OnUpdateHandle =
				static_cast<Assets::AssetHandle>(data["OnUpdate"].as<uint64_t>());
			if (scripts.OnUpdateHandle != Assets::EmptyHandle)
			{
				scripts.OnUpdate =
					Assets::AssetManager::GetScript(scripts.OnUpdateHandle).get();
			}
		}

		// Get Class Functions
		{
			auto allScripts = data["AllScripts"];
			if (allScripts)
			{
				std::set<Assets::AssetHandle>& classScripts = EntityClass->m_Scripts.AllClassScripts;
				for (auto script : allScripts)
				{
					classScripts.insert(static_cast<Assets::AssetHandle>(script.as<uint64_t>()));
				}
			}
		}

		return true;

	}

	AssetHandle AssetManager::CreateNewEntityClass(const std::string& EntityClassName)
	{
		// Create Checksum
		const std::string currentCheckSum = Utility::FileSystem::ChecksumFromString(EntityClassName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets
		for (const auto& [handle, asset] : s_EntityClassRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate entity class asset");
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};
		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create File
		CreateEntityClassFile(EntityClassName, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and return handle.
		s_EntityClassRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeEntityClassRegistry(); // Update Registry File on Disk

		return newHandle;
	}

	void AssetManager::SaveEntityClass(AssetHandle entityClassHandle, Ref<Scenes::EntityClass> entityClass, Ref<Scenes::Scene> editorScene)
	{
		if (!s_EntityClassRegistry.contains(entityClassHandle))
		{
			KG_ERROR("Attempt to save EntityClass that does not exist in registry");
			return;
		}
		// Store Field Location for Processing ClassInstanceComponent Data
		std::vector<Scenes::ClassField> oldFields = GetEntityClass(entityClassHandle)->GetFields();
		std::vector<Scenes::ClassField> newFields = entityClass->GetFields();

		Assets::Asset EntityClassAsset = s_EntityClassRegistry[entityClassHandle];
		SerializeEntityClass(entityClass, (Projects::ProjectService::GetActiveAssetDirectory() / EntityClassAsset.Data.IntermediateLocation).string());

		// Create map that associates old data locations with new data locations for ClassInstanceComponents
		std::unordered_map<uint32_t, uint32_t> transferFieldDataMap {};
		uint32_t newFieldsIteration{ 0 };
		for (auto& [newName, newType] : newFields)
		{
			uint32_t oldFieldsIteration{ 0 };
			for (auto& [oldName, oldType] : oldFields)
			{
				if (oldName == newName && oldType == newType)
				{
					transferFieldDataMap.insert_or_assign(oldFieldsIteration, newFieldsIteration);
					break;
				}
				oldFieldsIteration++;
			}
			newFieldsIteration++;
		}

		// Update Active Scene
		Ref<Scenes::EntityClass> newEntityClass = GetEntityClass(entityClassHandle);
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();

		Utility::TransferClassInstanceFieldData(activeScene, newEntityClass, entityClassHandle, transferFieldDataMap);
		// Update Editor Scene if applicable
		if (editorScene)
		{
			Utility::TransferClassInstanceFieldData(editorScene, newEntityClass,entityClassHandle, transferFieldDataMap);
		}

		for (auto& [handle, asset] : s_SceneRegistry)
		{
			const Ref<Scenes::Scene> scene = GetScene(handle);
			if (!scene)
			{
				KG_WARN("Unable to load scene in SaveEntityClass");
				continue;
			}

			Utility::TransferClassInstanceFieldData(scene, newEntityClass, entityClassHandle, transferFieldDataMap);
			SaveScene(handle, scene);
		}
		
	}

	void AssetManager::DeleteEntityClass(AssetHandle handle, Ref<Scenes::Scene> editorScene)
	{
		if (!s_EntityClassRegistry.contains(handle))
		{
			KG_WARN("Failed to delete EntityClass in AssetManager");
			return;
		}

		// Remove entity class references from all scenes
		Ref<Scenes::EntityClass> newEntityClass = GetEntityClass(handle);
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();

		// Update Active Scene if applicable
		Utility::ClearClassReferenceFromScene(activeScene, newEntityClass, handle);
		// Update Editor Scene if applicable
		if (editorScene)
		{
			Utility::ClearClassReferenceFromScene(editorScene, newEntityClass, handle);
		}
		// Update all scenes in scene registry
		for (auto& [handle, asset] : s_SceneRegistry)
		{
			const Ref<Scenes::Scene> scene = GetScene(handle);
			if (!scene)
			{
				KG_WARN("Unable to load scene in SaveEntityClass");
				continue;
			}

			Utility::ClearClassReferenceFromScene(scene, newEntityClass, handle);
			SaveScene(handle, scene);
		}


		Utility::FileSystem::DeleteSelectedFile(Projects::ProjectService::GetActiveAssetDirectory() /
			s_EntityClassRegistry.at(handle).Data.IntermediateLocation);

		s_EntityClassRegistry.erase(handle);

		SerializeEntityClassRegistry();
	}

	std::filesystem::path AssetManager::GetEntityClassLocation(const AssetHandle& handle)
	{
		if (!s_EntityClassRegistry.contains(handle))
		{
			KG_ERROR("Attempt to save EntityClass that does not exist in registry");
			return "";
		}
		return s_EntityClassRegistry[handle].Data.IntermediateLocation;
	}

	Ref<Scenes::EntityClass> AssetManager::GetEntityClass(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::ProjectService::GetActive(), "There is no active project when retreiving EntityClass!");

		if (s_EntityClassRegistry.contains(handle))
		{
			auto asset = s_EntityClassRegistry[handle];
			return InstantiateEntityClass(asset);
		}

		KG_WARN("No EntityClass is associated with provided handle!");
		return nullptr;
	}
	std::tuple<AssetHandle, Ref<Scenes::EntityClass>> AssetManager::GetEntityClass(const std::filesystem::path& filepath)
	{
		KG_ASSERT(Projects::ProjectService::GetActive(), "Attempt to use Project Field without active project!");

		std::filesystem::path EntityClassPath = filepath;

		if (filepath.is_absolute())
		{
			EntityClassPath = Utility::FileSystem::GetRelativePath(Projects::ProjectService::GetActiveAssetDirectory(), filepath);
		}

		for (auto& [assetHandle, asset] : s_EntityClassRegistry)
		{
			if (asset.Data.IntermediateLocation.compare(EntityClassPath) == 0)
			{
				return std::make_tuple(assetHandle, InstantiateEntityClass(asset));
			}
		}
		// Return empty EntityClass if EntityClass does not exist
		KG_WARN("No EntityClass Associated with provided handle. Returned new empty EntityClass");
		AssetHandle newHandle = CreateNewEntityClass(filepath.stem().string());
		return std::make_tuple(newHandle, GetEntityClass(newHandle));
	}

	Ref<Scenes::EntityClass> AssetManager::InstantiateEntityClass(const Assets::Asset& EntityClassAsset)
	{
		Ref<Scenes::EntityClass> newEntityClass = CreateRef<Scenes::EntityClass>();
		DeserializeEntityClass(newEntityClass, (Projects::ProjectService::GetActiveAssetDirectory() / EntityClassAsset.Data.IntermediateLocation).string());
		return newEntityClass;
	}


	void AssetManager::ClearEntityClassRegistry()
	{
		s_EntityClassRegistry.clear();
	}

	void AssetManager::CreateEntityClassFile(const std::string& EntityClassName, Assets::Asset& newAsset)
	{
		// Create Temporary EntityClass
		Ref<Scenes::EntityClass> temporaryEntityClass = CreateRef<Scenes::EntityClass>();
		temporaryEntityClass->SetName(EntityClassName);

		// Save Binary Intermediate into File
		std::string EntityClassPath = "EntityClass/" + EntityClassName + ".kgclass";
		std::filesystem::path intermediateFullPath = Projects::ProjectService::GetActiveAssetDirectory() / EntityClassPath;
		SerializeEntityClass(temporaryEntityClass, intermediateFullPath.string());

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::EntityClass;
		newAsset.Data.IntermediateLocation = EntityClassPath;
		Ref<Assets::EntityClassMetaData> metadata = CreateRef<Assets::EntityClassMetaData>();
		metadata->Name = EntityClassName;
		newAsset.Data.SpecificFileData = metadata;
	}
}
