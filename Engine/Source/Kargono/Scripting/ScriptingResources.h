#pragma once

namespace Kargono::Scripting
{
	enum class ScriptType
	{
		None = 0,
		Global = 1,
		Class = 2,
	};
}


namespace Kargono::Utility
{
	inline std::string ScriptTypeToString(Scripting::ScriptType type)
	{
		switch (type)
		{
		case Scripting::ScriptType::Global: return "Global";
		case Scripting::ScriptType::Class: return "Class";
		case Scripting::ScriptType::None: return "None";
		}
		KG_ASSERT(false, "Unknown enum type of ScriptType.");
		return "";
	}

	inline Scripting::ScriptType StringToScriptType(std::string type)
	{
		if (type == "Global") { return Scripting::ScriptType::Global; }
		if (type == "Class") { return Scripting::ScriptType::Class; }
		if (type == "None") { return Scripting::ScriptType::None; }

		KG_ASSERT(false, "Unknown std::string type of ScriptType.");
		return Scripting::ScriptType::None;
	}
}
