#pragma once

#include "Kargono/Core/UUID.h"
#include "Kargono/Renderer/Shader.h"

#include <filesystem>

namespace Kargono
{

	using AssetHandle = Kargono::UUID;

	namespace Assets
	{
		enum AssetType
		{
			None = 0,
			Texture = 1,
			Shader = 2
		};

		std::string AssetTypeToString(AssetType type);

		AssetType StringToAssetType(std::string type);

	}


	struct TextureMetaData
	{
		int32_t Width, Height, Channels;
		std::filesystem::path InitialFileLocation;
	};

	struct ShaderMetaData
	{
		Shader::ShaderSpecification ShaderSpec{};
		UniformBufferList UniformList{};
		InputBufferLayout InputLayout{};
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
