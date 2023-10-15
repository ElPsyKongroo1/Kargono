#include "kgpch.h"

#include "Kargono/Assets/Asset.h"

namespace Kargono::Assets
{
	std::string AssetTypeToString(AssetType type)
	{
		switch (type)
		{
		case AssetType::Texture: return "Texture";
		case AssetType::Shader: return "Shader";
		case AssetType::None: return "None";
		}
		KG_CORE_ASSERT(false, "Unknown Type of AssetType.");
		return "";
	}

	AssetType StringToAssetType(std::string type)
	{
		if (type == "Texture") { return AssetType::Texture; }
		if (type == "Shader") { return AssetType::Shader; }
		if (type == "None") { return AssetType::None; }

		KG_CORE_ASSERT(false, "Unknown Type of AssetType String.");
		return AssetType::None;
	}
}
