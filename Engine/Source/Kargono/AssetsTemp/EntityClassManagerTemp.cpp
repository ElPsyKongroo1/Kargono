#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/EntityClassManagerTemp.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/Scenes/Entity.h"
#include "Kargono/Scenes/EntityClass.h"

namespace Kargono::Assets
{
	Ref<Scenes::EntityClass> EntityClassManager::InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath)
	{
		Ref<Scenes::EntityClass> newEntityClass = CreateRef<Scenes::EntityClass>();
		
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetPath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgui file '{0}'\n     {1}", assetPath, e.what());
			return nullptr;
		}

		KG_INFO("Deserializing entity class");

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
				std::set<Assets::AssetHandle>& classScripts = newEntityClass->m_Scripts.AllClassScripts;
				for (auto script : allScripts)
				{
					classScripts.insert(static_cast<Assets::AssetHandle>(script.as<uint64_t>()));
				}
			}
		}

		return newEntityClass;
	}
}
