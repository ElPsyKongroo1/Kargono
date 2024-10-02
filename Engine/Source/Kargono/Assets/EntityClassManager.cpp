#include "kgpch.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Assets/EntityClassManager.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/Scenes/Entity.h"
#include "Kargono/Scenes/EntityClass.h"

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

	static void ClearClassReferenceFromScene(Ref<Scenes::Scene> scene, Ref<Scenes::EntityClass> entityClass, Assets::AssetHandle entityHandle)
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
	void EntityClassManager::SaveEntityClass(AssetHandle entityClassHandle, Ref<Scenes::EntityClass> entityClass, Ref<Scenes::Scene> editorScene)
	{
		if (!m_AssetRegistry.contains(entityClassHandle))
		{
			KG_ERROR("Attempt to save EntityClass that does not exist in registry");
			return;
		}
		// Store Field Location for Processing ClassInstanceComponent Data
		std::vector<Scenes::ClassField> oldFields = GetAsset(entityClassHandle)->GetFields();
		std::vector<Scenes::ClassField> newFields = entityClass->GetFields();

		Assets::Asset EntityClassAsset = m_AssetRegistry[entityClassHandle];
		SerializeAsset(entityClass, (Projects::ProjectService::GetActiveAssetDirectory() / EntityClassAsset.Data.FileLocation).string());

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
		Ref<Scenes::EntityClass> newEntityClass = GetAsset(entityClassHandle);
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();

		Utility::TransferClassInstanceFieldData(activeScene, newEntityClass, entityClassHandle, transferFieldDataMap);
		// Update Editor Scene if applicable
		if (editorScene)
		{
			Utility::TransferClassInstanceFieldData(editorScene, newEntityClass, entityClassHandle, transferFieldDataMap);
		}

		for (auto& [handle, asset] : AssetService::GetScriptRegistry())
		{
			const Ref<Scenes::Scene> scene = AssetService::GetScene(handle);
			if (!scene)
			{
				KG_WARN("Unable to load scene in SaveEntityClass");
				continue;
			}

			Utility::TransferClassInstanceFieldData(scene, newEntityClass, entityClassHandle, transferFieldDataMap);
			AssetService::SaveScene(handle, scene);
		}

		Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>(entityClassHandle, EntityClassAsset.Data.Type, Events::ManageAssetAction::Update);
		EngineService::SubmitToEventQueue(event);

	}

	void EntityClassManager::DeleteEntityClass(AssetHandle handle, Ref<Scenes::Scene> editorScene)
	{
		if (!m_AssetRegistry.contains(handle))
		{
			KG_WARN("Failed to delete EntityClass in AssetManager");
			return;
		}

		// Remove entity class references from all scenes
		Ref<Scenes::EntityClass> newEntityClass = GetAsset(handle);
		Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
		Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>(handle, AssetType::EntityClass, Events::ManageAssetAction::Delete);

		// Update Active Scene if applicable
		Utility::ClearClassReferenceFromScene(activeScene, newEntityClass, handle);
		// Update Editor Scene if applicable
		if (editorScene)
		{
			Utility::ClearClassReferenceFromScene(editorScene, newEntityClass, handle);
		}
		// Update all scenes in scene registry
		for (auto& [handle, asset] : AssetService::GetSceneRegistry())
		{
			const Ref<Scenes::Scene> scene = AssetService::GetScene(handle);
			if (!scene)
			{
				KG_WARN("Unable to load scene in SaveEntityClass");
				continue;
			}

			Utility::ClearClassReferenceFromScene(scene, newEntityClass, handle);
			AssetService::SaveScene(handle, scene);
		}


		Utility::FileSystem::DeleteSelectedFile(Projects::ProjectService::GetActiveAssetDirectory() /
			m_AssetRegistry.at(handle).Data.FileLocation);

		m_AssetRegistry.erase(handle);

		SerializeAssetRegistry();
		EngineService::SubmitToEventQueue(event);
	}

	void Assets::EntityClassManager::CreateAssetFileFromName(const std::string& name, Asset& asset, const std::filesystem::path& assetPath)
	{
		// Create Temporary EntityClass
		Ref<Scenes::EntityClass> temporaryEntityClass = CreateRef<Scenes::EntityClass>();
		temporaryEntityClass->SetName(name);

		// Save Binary into File
		SerializeAsset(temporaryEntityClass, assetPath.string());

		// Load data into In-Memory Metadata object
		Ref<Assets::EntityClassMetaData> metadata = CreateRef<Assets::EntityClassMetaData>();
		metadata->Name = name;
		asset.Data.SpecificFileData = metadata;
	}
	void Assets::EntityClassManager::SerializeAsset(Ref<Scenes::EntityClass> assetReference, const std::filesystem::path& assetPath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map

		out << YAML::Key << "Name" << YAML::Value << assetReference->GetName(); // Output State Name
		out << YAML::Key << "FieldTypes" << YAML::Value;
		out << YAML::BeginSeq; // Start Fields

		for (auto& [name, field] : assetReference->m_FieldTypes)
		{
			out << YAML::BeginMap; // Start Field

			out << YAML::Key << "Name" << YAML::Value << name; // Name/Map Key
			out << YAML::Key << "Type" << YAML::Value << Utility::WrappedVarTypeToString(field); // Field Type

			out << YAML::EndMap; // End Field
		}

		out << YAML::EndSeq; // End Fields

		out << YAML::Key << "OnPhysicsCollisionStart" << YAML::Value <<
			static_cast<uint64_t>(assetReference->m_Scripts.OnPhysicsCollisionStartHandle);
		out << YAML::Key << "OnPhysicsCollisionEnd" << YAML::Value <<
			static_cast<uint64_t>(assetReference->m_Scripts.OnPhysicsCollisionEndHandle);
		out << YAML::Key << "OnCreate" << YAML::Value <<
			static_cast<uint64_t>(assetReference->m_Scripts.OnCreateHandle);
		out << YAML::Key << "OnUpdate" << YAML::Value <<
			static_cast<uint64_t>(assetReference->m_Scripts.OnUpdateHandle);

		out << YAML::Key << "AllScripts" << YAML::Value;
		out << YAML::BeginSeq; // Start AllScripts

		for (auto& script : assetReference->m_Scripts.AllClassScripts)
		{
			out << YAML::Value << static_cast<uint64_t>(script); // Script ID
		}

		out << YAML::EndSeq; // End AllScripts

		out << YAML::EndMap; // End of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized EntityClass at {}", assetPath.string());
	}
	Ref<Scenes::EntityClass> EntityClassManager::DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath)
	{
		Ref<Scenes::EntityClass> newEntityClass = CreateRef<Scenes::EntityClass>();
		
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetPath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_WARN("Failed to load .kgui file '{0}'\n     {1}", assetPath, e.what());
			return nullptr;
		}

		newEntityClass->m_Name = data["Name"].as<std::string>();

		// Get Fields
		{
			auto fields = data["FieldTypes"];
			if (fields)
			{
				auto& newFieldsMap = newEntityClass->m_FieldTypes;
				for (auto field : fields)
				{
					std::string fieldName = field["Name"].as<std::string>();
					WrappedVarType fieldType = Utility::StringToWrappedVarType(field["Type"].as<std::string>());
					if (!newEntityClass->AddField(fieldName, fieldType))
					{
						KG_WARN("Unable to add field inside deserialize entity class function");
					}

				}
			}
		}

		// Get Function Slots
		{
			Scenes::EntityScripts& scripts = newEntityClass->m_Scripts;
			scripts.OnPhysicsCollisionStartHandle =
				static_cast<Assets::AssetHandle>(data["OnPhysicsCollisionStart"].as<uint64_t>());
			if (scripts.OnPhysicsCollisionStartHandle != Assets::EmptyHandle)
			{
				scripts.OnPhysicsCollisionStart =
					AssetService::GetScript(scripts.OnPhysicsCollisionStartHandle).get();
			}
			scripts.OnPhysicsCollisionEndHandle =
				static_cast<Assets::AssetHandle>(data["OnPhysicsCollisionEnd"].as<uint64_t>());
			if (scripts.OnPhysicsCollisionEndHandle != Assets::EmptyHandle)
			{
				scripts.OnPhysicsCollisionEnd =
					AssetService::GetScript(scripts.OnPhysicsCollisionEndHandle).get();
			}
			scripts.OnCreateHandle =
				static_cast<Assets::AssetHandle>(data["OnCreate"].as<uint64_t>());
			if (scripts.OnCreateHandle != Assets::EmptyHandle)
			{
				scripts.OnCreate =
					AssetService::GetScript(scripts.OnCreateHandle).get();
			}
			scripts.OnUpdateHandle =
				static_cast<Assets::AssetHandle>(data["OnUpdate"].as<uint64_t>());
			if (scripts.OnUpdateHandle != Assets::EmptyHandle)
			{
				scripts.OnUpdate =
					AssetService::GetScript(scripts.OnUpdateHandle).get();
			}
		}

		// Get Class Functions
		{
			auto allScripts = data["AllScripts"];
			if (allScripts)
			{
				std::set<Assets::AssetHandle>& classScripts = newEntityClass->m_Scripts.AllClassScripts;
				for (auto script : allScripts)
				{
					classScripts.insert(static_cast<Assets::AssetHandle>(script.as<uint64_t>()));
				}
			}
		}

		return newEntityClass;
	}
	void EntityClassManager::SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset)
	{
		Assets::EntityClassMetaData* metadata = currentAsset.Data.GetSpecificMetaData<EntityClassMetaData>();
		serializer << YAML::Key << "Name" << YAML::Value << metadata->Name;
	}
	void Assets::EntityClassManager::DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset)
	{
		Ref<Assets::EntityClassMetaData> EntityClassMetaData = CreateRef<Assets::EntityClassMetaData>();
		EntityClassMetaData->Name = metadataNode["Name"].as<std::string>();
		currentAsset.Data.SpecificFileData = EntityClassMetaData;
	}
}
