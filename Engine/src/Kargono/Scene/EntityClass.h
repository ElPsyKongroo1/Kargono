#pragma once

#include "Kargono/Core/WrappedData.h"
#include "Kargono/Scripting/Scripting.h"

#include <vector>
#include <string>
#include <set>
#include <map>

namespace Kargono::Assets { class AssetManager; }

namespace Kargono
{
	struct EntityScripts
	{
		Scripting::Script* OnPhysicsCollisionStart{ nullptr };
		Assets::AssetHandle OnPhysicsCollisionStartHandle {0};
		Scripting::Script* OnPhysicsCollisionEnd{ nullptr };
		Assets::AssetHandle OnPhysicsCollisionEndHandle {0};
		Scripting::Script* OnCreate{ nullptr };
		Assets::AssetHandle OnCreateHandle {0};
		Scripting::Script* OnUpdate{ nullptr };
		Assets::AssetHandle OnUpdateHandle {0};
		std::set<Assets::AssetHandle> AllClassScripts {};
	};

	class EntityClass
	{
	public:
		std::map<std::string, WrappedVarType>& GetFields()
		{
			return m_FieldTypes;
		}
		WrappedVarType GetField(const std::string& fieldName)
		{
			if (!m_FieldTypes.contains(fieldName))
			{
				KG_CRITICAL("Could not get field type from class {}", fieldName);
				return WrappedVarType::None;
			}
			return m_FieldTypes.at(fieldName);
		}

		bool AddField(const std::string& fieldName, WrappedVarType fieldType)
		{
			if (m_FieldTypes.contains(fieldName))
			{
				KG_WARN("Attempt to add field to Entity Class that already exists");
				return false;
			}

			m_FieldTypes.insert_or_assign(fieldName, fieldType);
			return true;
		}

		bool DeleteField(const std::string& fieldName)
		{
			if (!m_FieldTypes.contains(fieldName))
			{
				KG_WARN("Attempt to delete field to Entity Class that does not exist");
				return false;
			}

			m_FieldTypes.erase(fieldName);
			return true;
		}

		EntityScripts& GetScripts()
		{
			return m_Scripts;
		}
		void SetName(const std::string& name)
		{
			m_Name = name;
		}

		std::string& GetName()
		{
			return m_Name;
		}
	private:
		std::string m_Name{};
		EntityScripts m_Scripts{};
		std::map<std::string, WrappedVarType> m_FieldTypes {};
	private:
		friend class Assets::AssetManager;
	};
}
