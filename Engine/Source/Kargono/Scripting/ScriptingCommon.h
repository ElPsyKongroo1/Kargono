#pragma once

namespace Kargono::Scripting
{
	//==============================
	// Script Type Definition
	//==============================
	enum class ScriptType
	{
		None = 0,
		Project = 1,
		Engine = 2
	};
}


namespace Kargono::Utility
{
	//==============================
	// Conversion ScriptType <-> String
	//==============================
	inline std::string ScriptTypeToString(Scripting::ScriptType type)
	{
		switch (type)
		{
		case Scripting::ScriptType::Project: return "Project";
		case Scripting::ScriptType::Engine: return "Engine";
		case Scripting::ScriptType::None: return "None";
		}
		KG_ERROR("Unknown enum type of ScriptType.");
		return "";
	}

	inline Scripting::ScriptType StringToScriptType(std::string type)
	{
		if (type == "Project") { return Scripting::ScriptType::Project; }
		if (type == "Engine") { return Scripting::ScriptType::Engine; }
		if (type == "None") { return Scripting::ScriptType::None; }

		KG_ERROR("Unknown std::string type of ScriptType.");
		return Scripting::ScriptType::None;
	}
}
