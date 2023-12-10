#pragma once

#include "Kargono/Core/UUID.h"
#include "Kargono/Renderer/Shader.h"
#include "Kargono/Text/TextEngine.h"

#include <filesystem>
#include <vector>
#include <utility>

namespace Kargono
{

	using AssetHandle = Kargono::UUID;

	namespace Assets
	{
		enum AssetType
		{
			None = 0,
			Texture = 1,
			Shader = 2,
			Audio = 3,
			Font = 4,
			Scene = 5,
			UIObject = 6
		};

		inline std::string AssetTypeToString(AssetType type)
		{
			switch (type)
			{
			case AssetType::Texture: return "Texture";
			case AssetType::Shader: return "Shader";
			case AssetType::Audio: return "Audio";
			case AssetType::Font: return "Font";
			case AssetType::Scene: return "Scene";
			case AssetType::UIObject: return "UIObject";
			case AssetType::None: return "None";
			}
			KG_CORE_ASSERT(false, "Unknown Type of AssetType.");
			return "";
		}

		inline AssetType StringToAssetType(std::string type)
		{
			if (type == "Texture") { return AssetType::Texture; }
			if (type == "Shader") { return AssetType::Shader; }
			if (type == "Audio") { return AssetType::Audio; }
			if (type == "Font") { return AssetType::Font; }
			if (type == "Scene") { return AssetType::Scene; }
			if (type == "UIObject") { return AssetType::UIObject; }
			if (type == "None") { return AssetType::None; }

			KG_CORE_ASSERT(false, "Unknown Type of AssetType String.");
			return AssetType::None;
		}

	}


	struct TextureMetaData
	{
		int32_t Width, Height, Channels;
		std::filesystem::path InitialFileLocation;
	};

	struct AudioMetaData
	{
		uint32_t Channels, SampleRate;
		uint64_t TotalPcmFrameCount, TotalSize;
		std::filesystem::path InitialFileLocation;
	};

	struct ShaderMetaData
	{
		Shader::ShaderSpecification ShaderSpec{};
		UniformBufferList UniformList{};
		InputBufferLayout InputLayout{};
	};

	struct FontMetaData
	{
		std::vector<uint64_t> BufferLocations{};
		std::vector<std::pair<unsigned char, Font::Character>> Characters{};
		float AverageWidth{ 0 };
		float AverageHeight{ 0 };
		std::filesystem::path InitialFileLocation;
	};

	struct SceneMetaData
	{
		
	};

	struct UIObjectMetaData
	{

	};

	struct Metadata
	{
		std::filesystem::path IntermediateLocation;
		std::string CheckSum;
		Assets::AssetType Type = Assets::AssetType::None;
		Ref<void> SpecificFileData = nullptr;
	};

	struct Asset
	{
		AssetHandle Handle;
		Metadata Data;
	};

}
