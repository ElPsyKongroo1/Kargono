#pragma once

#include "Kargono/Core/UUID.h"
#include "Kargono/Rendering/Shader.h"
#include "Kargono/RuntimeUI/Font.h"
#include "Kargono/Core/WrappedData.h"
#include "Kargono/Scripting/ScriptingCommon.h"

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

	static inline uint64_t EmptyHandle { 0 };

	// This enum provides a method to distinguish between different specific
	//		asset types in an Asset. The metadata struct will hold an AssetType.
	enum class AssetType
	{
		None = 0,
		Texture = 1,
		Shader = 2,
		Audio = 3,
		Font = 4,
		Scene = 5,
		UserInterface = 6,
		InputMode = 7,
		Script = 8,
		GameState = 9,
		EntityClass = 10
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
		std::filesystem::path IntermediateLocation;
		std::string CheckSum;
		Assets::AssetType Type = Assets::AssetType::None;
		Ref<void> SpecificFileData = nullptr;
	public:
		template <typename T>
		T* GetSpecificFileData()
		{
			return static_cast<T*>(SpecificFileData.get());
		}
	};

	//==============================
	// Asset Specific Metadata
	//==============================
	// These next few structs hold data that is specific to a unique asset
	//		type. These metadata structs are meant to be held inside the
	//		metadata struct in the SpecificFileData Ref<void> pointer.

	//==============================
	// Texture MetaData Struct
	//==============================
	// This metadata struct mostly holds loading information for the intermediate to load correctly.
	//		When the file is loaded from binary (Intermediate), it needs to know the
	//		image's width, height, and number of channels to interpret the binary
	//		correctly. The InitialFileLocation field is for debugging.
	struct TextureMetaData
	{
		int32_t Width, Height, Channels;
		std::filesystem::path InitialFileLocation;
	};

	//==============================
	// Audio MetaData Struct
	//==============================
	// This metadata struct mostly holds loading information for the intermediate to load correctly.
	//		The binary form of Audio is held in a PCM format. The channels, sample rate, TotalPcmFrameCount,
	//		and the TotalSize are used for loading the file correctly. The InitialFileLocation field is for debugging.
	struct AudioMetaData
	{
		uint32_t Channels, SampleRate;
		uint64_t TotalPcmFrameCount, TotalSize;
		std::filesystem::path InitialFileLocation;
	};

	//==============================
	// Shader MetaData Struct
	//==============================
	// This metadata struct holds information to correctly interpret the shader binary when creating the final Shader object
	//		in the selected rendering API. The ShaderSpec field holds the specification that was originally used to generate
	//		the shader through the shader builder. The UniformList holds the list of uniforms needed for this shader to
	//		run correctly. The InputBufferLayout is generated when a shader is created in the shader builder (the uniform list too)
	//		and holds the format of per vertex input. Ex: vec3(vertex position), vec3(world position), vec4(color), etc...
	struct ShaderMetaData
	{
		Rendering::ShaderSpecification ShaderSpec{};
		Rendering::UniformBufferList UniformList{};
		Rendering::InputBufferLayout InputLayout{};
	};

	//==============================
	// Font MetaData Struct
	//==============================
	// This metadata struct holds the information needed to correctly load a selected font
	//		from its intermediate(binary) form. Fonts are originally loaded from a .ttf file which
	//		holds most of this metadata itself. When the font is converted into an intermediate form,
	//		the information that defines each character is saved in the Characters vector. This
	//		information includes the character's height, width, etc... The BufferLocations describe 
	//		the exact location in the intermediate where each character image is stored. The index of the
	//		Characters and BufferLocations vectors correspond with each other. The Character object
	//		holds enough information to get the size of the image inside of the intermediate.
	//		The AverageWidth and AverageHeight fields are generated when loading the .ttf file and are
	//		used for text positioning in the user interface. The InitialFileLocation field is for
	//		debugging.
	struct FontMetaData
	{
		float AtlasWidth;
		float AtlasHeight;
		float LineHeight{};
		std::vector<std::pair<unsigned char, RuntimeUI::Character>> Characters{};
		std::filesystem::path InitialFileLocation;
	};

	//==============================
	// Scene MetaData Struct
	//==============================
	// This metadata struct is currently empty but keeps a consistent API when working with assets.
	//		There may be later additions to this struct.
	struct SceneMetaData
	{

	};

	//==============================
	// UI Object MetaData Struct
	//==============================
	// This metadata struct is currently empty but keeps a consistent API when working with assets.
	//		There may be later additions to this struct.
	struct UserInterfaceMetaData
	{

	};

	//==============================
	// Input Mode MetaData Struct
	//==============================
	// This metadata struct is currently empty but keeps a consistent API when working with assets.
	//		There may be later additions to this struct.
	struct InputModeMetaData
	{

	};

	struct GameStateMetaData
	{
		std::string Name{};
	};

	struct EntityClassMetaData
	{
		std::string Name{};
	};

	struct ScriptMetaData
	{
		std::string Name{};
		Scripting::ScriptType ScriptType {Scripting::ScriptType::None };
		std::string SectionLabel{};
		WrappedFuncType FunctionType{};
	};

	//==============================
	// Asset Struct
	//==============================
	// This struct represents an asset managed by the AssetManager class. This asset
	//		has an AssetHandle which uniquely identifies this asset and a metadata
	//		object which holds ancillary information about the asset such as the
	//		asset type, asset location, a checksum, and asset type specific metadata.
	struct Asset
	{
		AssetHandle Handle;
		Metadata Data;
	};
	
}

namespace Kargono::Utility
{
	//==============================
	// AssetType <-> String Conversions
	//==============================
	inline std::string AssetTypeToString(Assets::AssetType type)
	{
		switch (type)
		{
		case Assets::AssetType::Texture: return "Texture";
		case Assets::AssetType::Shader: return "Shader";
		case Assets::AssetType::Audio: return "Audio";
		case Assets::AssetType::Font: return "Font";
		case Assets::AssetType::Scene: return "Scene";
		case Assets::AssetType::UserInterface: return "UserInterface";
		case Assets::AssetType::InputMode: return "InputMode";
		case Assets::AssetType::Script: return "Script";
		case Assets::AssetType::GameState: return "GameState";
		case Assets::AssetType::EntityClass: return "EntityClass";
		case Assets::AssetType::None: return "None";
		}
		KG_ERROR("Unknown Type of AssetType.");
		return "";
	}

	inline Assets::AssetType StringToAssetType(std::string type)
	{
		if (type == "Texture") { return Assets::AssetType::Texture; }
		if (type == "Shader") { return Assets::AssetType::Shader; }
		if (type == "Audio") { return Assets::AssetType::Audio; }
		if (type == "Font") { return Assets::AssetType::Font; }
		if (type == "Scene") { return Assets::AssetType::Scene; }
		if (type == "UserInterface") { return Assets::AssetType::UserInterface; }
		if (type == "InputMode") { return Assets::AssetType::InputMode; }
		if (type == "Script") { return Assets::AssetType::Script; }
		if (type == "GameState") { return Assets::AssetType::GameState; }
		if (type == "EntityClass") { return Assets::AssetType::EntityClass; }
		if (type == "None") { return Assets::AssetType::None; }

		KG_ERROR("Unknown Type of AssetType String.");
		return Assets::AssetType::None;
	}
}
