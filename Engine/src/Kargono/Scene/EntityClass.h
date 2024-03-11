#pragma once

#include "Kargono/Core/WrappedData.h"
#include "Kargono/Scripting/Scripting.h"

#include <vector>
#include <string>

namespace Kargono
{
	struct EntityFieldType
	{
	public:
		std::string Name{};
		WrappedVarType Type{};
	};

	struct EntityScripts
	{
		Scripting::Script* OnPhysicsCollisionStart{ nullptr };
		Scripting::Script* OnPhysicsCollisionEnd{ nullptr };
		Scripting::Script* OnCreate{ nullptr };
		Scripting::Script* OnUpdate{ nullptr };
	};

	class EntityClass
	{
	public:
		std::vector<EntityFieldType>& GetFieldTypes()
		{
			return m_FieldTypes;
		}

		EntityScripts& GetScripts()
		{
			return m_Scripts;
		}
		std::string& GetName()
		{
			return m_Name;
		}
	private:
		std::string m_Name{};
		std::vector<EntityFieldType> m_FieldTypes{};
		EntityScripts m_Scripts{};
	};
}
