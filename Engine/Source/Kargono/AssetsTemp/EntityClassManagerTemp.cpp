#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/EntityClassManagerTemp.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/Scenes/Entity.h"
#include "Kargono/Scenes/EntityClass.h"

namespace Kargono::Assets
{
	bool EntityClassManager::DeserializeEntityClass(Ref<Scenes::EntityClass> EntityClass, const std::filesystem::path& filepath)
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

	Ref<Scenes::EntityClass> EntityClassManager::InstantiateAssetIntoMemory(Assets::Asset& asset)
	{
		Ref<Scenes::EntityClass> newEntityClass = CreateRef<Scenes::EntityClass>();
		DeserializeEntityClass(newEntityClass, (Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.IntermediateLocation).string());
		return newEntityClass;
	}

	static EntityClassManager s_EntityClassManager;

	Ref<Scenes::EntityClass> AssetServiceTemp::GetEntityClass(const AssetHandle& handle)
	{
		return s_EntityClassManager.GetAsset(handle);
	}
}
