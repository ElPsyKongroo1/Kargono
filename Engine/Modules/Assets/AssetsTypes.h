#pragma once

#include "Kargono/Core/UUID.h"

#include <cstdint>
#include <array>

namespace Kargono::Assets
{
	//==============================
	// Core Assets Types
	//==============================
	// An AssetHandle is a unique identifier for an Asset.
	using AssetHandle = Kargono::UUID;
	constexpr uint64_t EmptyHandle{ 0 };

	// This enum provides a method to distinguish between different specific
	//		asset types in an Asset. The metadata struct will hold an AssetType.
	enum class AssetType
	{
		None = 0,
		Texture,
		Shader,
		Audio,
		Font,
		Scene,
		UserInterface,
		InputMap,
		Script,
		GameState,
		ProjectComponent,
		AIState,
		EmitterConfig,
		ProjectEnum,
		GlobalState,
		ColorPalette
	};

	constexpr std::array<AssetType, 15> s_AllAssetTypes
	{
		AssetType::AIState,
		AssetType::Audio,
		AssetType::ColorPalette,
		AssetType::EmitterConfig,
		AssetType::Font,
		AssetType::GameState,
		AssetType::GlobalState,
		AssetType::InputMap,
		AssetType::ProjectComponent,
		AssetType::ProjectEnum,
		AssetType::Scene,
		AssetType::Script,
		AssetType::Shader,
		AssetType::Texture,
		AssetType::UserInterface
	};
}

namespace Kargono::Utility
{
	//==============================
	// AssetType <-> String Conversions
	//==============================
	inline const char* AssetTypeToString(Assets::AssetType type)
	{
		switch (type)
		{
		case Assets::AssetType::Texture: return "Texture";
		case Assets::AssetType::Shader: return "Shader";
		case Assets::AssetType::Audio: return "Audio";
		case Assets::AssetType::Font: return "Font";
		case Assets::AssetType::Scene: return "Scene";
		case Assets::AssetType::UserInterface: return "UserInterface";
		case Assets::AssetType::InputMap: return "InputMap";
		case Assets::AssetType::Script: return "Script";
		case Assets::AssetType::GameState: return "GameState";
		case Assets::AssetType::ProjectComponent: return "ProjectComponent";
		case Assets::AssetType::AIState: return "AIState";
		case Assets::AssetType::EmitterConfig: return "EmitterConfig";
		case Assets::AssetType::ProjectEnum: return "ProjectEnum";
		case Assets::AssetType::GlobalState: return "GlobalState";
		case Assets::AssetType::ColorPalette: return "ColorPalette";
		case Assets::AssetType::None: return "None";
		}
		return "";
	}

	inline Assets::AssetType StringToAssetType(std::string_view type)
	{
		if (type == "Texture") { return Assets::AssetType::Texture; }
		if (type == "Shader") { return Assets::AssetType::Shader; }
		if (type == "Audio") { return Assets::AssetType::Audio; }
		if (type == "Font") { return Assets::AssetType::Font; }
		if (type == "Scene") { return Assets::AssetType::Scene; }
		if (type == "UserInterface") { return Assets::AssetType::UserInterface; }
		if (type == "InputMap") { return Assets::AssetType::InputMap; }
		if (type == "Script") { return Assets::AssetType::Script; }
		if (type == "GameState") { return Assets::AssetType::GameState; }
		if (type == "ProjectComponent") { return Assets::AssetType::ProjectComponent; }
		if (type == "AIState") { return Assets::AssetType::AIState; }
		if (type == "EmitterConfig") { return Assets::AssetType::EmitterConfig; }
		if (type == "ProjectEnum") { return Assets::AssetType::ProjectEnum; }
		if (type == "ColorPalette") { return Assets::AssetType::ColorPalette; }
		if (type == "GlobalState") { return Assets::AssetType::GlobalState; }
		if (type == "None") { return Assets::AssetType::None; }

		return Assets::AssetType::None;
	}
}