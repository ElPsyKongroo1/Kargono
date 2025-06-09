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

	struct ProjectComponentMetaData
	{
		std::string Name{};
	};

	struct ProjectEnumMetaData
	{
		std::string Name{};
	};

	struct AssetInfo
	{
		AssetHandle m_Handle { Assets::EmptyHandle };
		Metadata Data;
	};
}
