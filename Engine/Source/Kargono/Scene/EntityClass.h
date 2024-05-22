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

	struct ClassField
	{
		std::string Name {};
		WrappedVarType Type {WrappedVarType::None};
	};

	class EntityClass
	{
	public:
		const std::vector<ClassField>& GetFields()
		{
			return m_FieldTypes;
		}

		int32_t GetFieldLocation(const std::string& fieldName) const
		{
			int32_t iteration{ 0 };
			for (auto& field : m_FieldTypes)
			{
				if (field.Name == fieldName)
				{
					return iteration;
				}
				iteration++;
			}

			KG_WARN("Could not get field location from class {}", fieldName);
			return -1;
		}

		WrappedVarType GetField(const std::string& fieldName) const
		{
			for (auto& field : m_FieldTypes)
			{
				if (field.Name == fieldName)
				{
					return field.Type;
				}
			}

			KG_WARN("Could not get field type from class {}", fieldName);
			return WrappedVarType::None;
		}

		bool ContainsField(const std::string& fieldName) const
		{
			for (auto& field : m_FieldTypes)
			{
				if (field.Name == fieldName)
				{
					return true;
				}
			}
			return false;
		}

		bool AddField(const std::string& fieldName, WrappedVarType fieldType)
		{
			for (auto& field : m_FieldTypes)
			{
				if (field.Name == fieldName)
				{
					KG_WARN("Attempt to add field to Entity Class that already exists");
					return false;
				}
			}

			m_FieldTypes.push_back({ fieldName, fieldType });
			return true;
		}

		bool DeleteField(const std::string& fieldName)
		{
			bool foundField = false;
			uint32_t fieldLocation{ 0 };
			uint32_t iteration{ 0 };
			for (auto& field : m_FieldTypes)
			{
				if (field.Name == fieldName)
				{
					foundField = true;
					fieldLocation = iteration;
					break;
				}
				iteration++;
			}

			if (!foundField)
			{
				KG_WARN("Attempt to delete field to Entity Class that does not exist");
				return false;
			}

			m_FieldTypes.erase(m_FieldTypes.begin() + fieldLocation);
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
		std::vector<ClassField> m_FieldTypes {};
	private:
		friend class Assets::AssetManager;
	};
}
