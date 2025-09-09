#pragma once

#include "Modules/Rendering/Shader.h"
#include "Modules/RuntimeUI/Font.h"
#include "Kargono/Core/WrappedData.h"
#include "Modules/Scripting/ScriptingCommon.h"

#include "Modules/Assets/AssetsTypes.h"

#include <filesystem>
#include <vector>
#include <utility>


namespace Kargono::Assets
{
	//==============================
	// Core Assets Types
	//==============================
	// An AssetHandle is a unique identifier for an Asset.
	using AssetHandle = Kargono::UUID;

	constexpr uint64_t EmptyHandle { 0 };

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
		CustomComponent,
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
		AssetType::CustomComponent,
		AssetType::ProjectEnum,
		AssetType::Scene,
		AssetType::Script,
		AssetType::Shader,
		AssetType::Texture,
		AssetType::UserInterface
	};

	//==============================
	// Metadata Struct
	//==============================
	// This is the generic metadata struct that is directly held inside an asset.
	//		This struct holds generic information about an asset that is universally
	//		needed. The IntermediateLocation provides a relative path from the selected
	//		project's asset directory to the intermediate. As an aside, an intermediate
	//		is file that has been preprocessed and is directly ready to be loaded into
	//		the engine and used without the use of external tools. The checksum is generated
	//		differently based on the file type, but it uniquely identifies the file itself.
	//		The checksum is generally used to ensure duplicate files of the same type are
	//		not being stored. The AssetType allows an easy method to identify the specific
	//		asset type in a generic sense. The SpecificFileData holds a reference to data
	//		that is only needed by this specific type of asset. Ex: A texture might need
	//		to know its width and height, while an audio file might need to know its sample
	//		rate.
	struct Metadata
	{
	public:
		std::filesystem::path FileLocation;
		std::filesystem::path IntermediateLocation;
		std::string CheckSum;
		Assets::AssetType Type = Assets::AssetType::None;
		Ref<void> SpecificFileData { nullptr };
	public:
		template <typename T>
		T* GetSpecificMetaData()
		{
			return static_cast<T*>(SpecificFileData.get());
		}
	};

	struct TextureMetaData
	{
		int32_t Width, Height, Channels;
	};

	struct AudioMetaData
	{
		uint32_t Channels, SampleRate;
		uint64_t TotalPcmFrameCount, TotalSize;
	};

	struct ShaderMetaData
	{
		Rendering::ShaderSpecification ShaderSpec{};
		Rendering::UniformBufferList UniformList{};
		Rendering::InputBufferLayout InputLayout{};
	};

	struct FontMetaData
	{
		float AtlasWidth{ 0.0f };
		float AtlasHeight{ 0.0f };
		float LineHeight{ 0.0f };
		float Ascender{ 0.0f };
		float Descender{ 0.0f };
		std::vector<std::pair<unsigned char, RuntimeUI::Character>> Characters{};
	};

	struct SceneMetaData
	{

	};

	struct UserInterfaceMetaData
	{

	};

	struct InputMapMetaData
	{

	};

	struct ParticleEmitterConfigMetaData
	{

	};

	struct AIStateMetaData
	{

	};

	struct GameStateMetaData
	{
		std::string Name{};
	};

	struct GlobalStateMetaData
	{
		std::string Name{};
	};

	struct ColorPaletteMetaData
	{
		std::string Name{};
	};

	struct ScriptMetaData
	{
		std::string m_Name{};
		Scripting::ScriptType m_ScriptType {Scripting::ScriptType::None };
		std::string m_SectionLabel{};
		WrappedFuncType m_FunctionType{};
		Scripting::ExplicitFuncType m_ExplicitFuncType{};
	};

	struct CustomComponentMetaData
	{
		std::string Name{};
	};

	struct ProjectEnumMetaData
	{
		std::string Name{};
	};

	struct AssetInfo
	{
		AssetHandle m_Handle { Assets::k_EmptyHandle };
		Metadata Data;
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
		case Assets::AssetType::CustomComponent: return "CustomComponent";
		case Assets::AssetType::AIState: return "AIState";
		case Assets::AssetType::EmitterConfig: return "EmitterConfig";
		case Assets::AssetType::ProjectEnum: return "ProjectEnum";
		case Assets::AssetType::GlobalState: return "GlobalState";
		case Assets::AssetType::ColorPalette: return "ColorPalette";
		case Assets::AssetType::None: return "None";
		}
		KG_ERROR("Unknown Type of AssetType.");
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
		if (type == "CustomComponent") { return Assets::AssetType::CustomComponent; }
		if (type == "AIState") { return Assets::AssetType::AIState; }
		if (type == "EmitterConfig") { return Assets::AssetType::EmitterConfig; }
		if (type == "ProjectEnum") { return Assets::AssetType::ProjectEnum; }
		if (type == "ColorPalette") { return Assets::AssetType::ColorPalette; }
		if (type == "GlobalState") { return Assets::AssetType::GlobalState; }
		if (type == "None") { return Assets::AssetType::None; }

		KG_ERROR("Unknown Type of AssetType String.");
		return Assets::AssetType::None;
	}
}
