#pragma once

#include "Modules/Rendering/Shader.h"
#include "Modules/RuntimeUI/Font.h"
#include "Kargono/Core/WrappedData.h"
#include "Modules/Scripting/ScriptingCommon.h"
#include "Kargono/Core/UUID.h"
#include "Modules/Core/Module.h"
#include "Kargono/Core/FixedBufferString.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/Concepts/MetadataConcept.h"

#include <filesystem>
#include <vector>
#include <utility>
#include <cstdint>
#include <array>


namespace Kargono::Assets
{
	struct Metadata
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Metadata() = default;
		~Metadata() = default;
	public:
		//==============================
		// Getters/Setters
		//==============================
		template <MetadataConcept t_MetadataType>
		t_MetadataType* GetSpecificMetaData()
		{
			KG_ASSERT(m_SpecificMetaData);
			return static_cast<t_MetadataType*>(m_SpecificMetaData);
		}

		template <MetadataConcept t_MetadataType>
		void SetSpecificMetaData(t_MetadataType* newMetaData)
		{
			m_SpecificMetaData = static_cast<void*>(newMetaData);
		}
	public:
		//==============================
		// Public Fields
		//==============================
		AssetHandle m_Handle{ Assets::k_EmptyHandle };
		AssetIdentifier m_TypeIdentifier{ k_InvalidAssetIdentifier };
		FixedBufStr64 m_CheckSum;
		std::filesystem::path m_FileLocation;
		std::filesystem::path m_IntermediateLocation;
	private:
		//==============================
		// Internal Field(s)
		//==============================
		void* m_SpecificMetaData{ nullptr };
	};

	struct TextureMetaData
	{
		int32_t m_Width{0};
		int32_t m_Height{0};
		int32_t m_Channels{0};
	};

	struct ShaderMetaData
	{
		Rendering::ShaderSpecification m_ShaderSpec{};
		Rendering::UniformBufferList m_UniformList{};
		Rendering::InputBufferLayout m_InputLayout{};
	};

	struct FontMetaData
	{
		float m_AtlasWidth{ 0.0f };
		float m_AtlasHeight{ 0.0f };
		float m_LineHeight{ 0.0f };
		float m_Ascender{ 0.0f };
		float m_Descender{ 0.0f };
		std::vector<std::pair<unsigned char, RuntimeUI::Character>> m_Characters{};
	};

	struct GameStateMetaData
	{
		FixedBufStr16 m_Name{};
	};

	struct GlobalStateMetaData
	{
		FixedBufStr16 m_Name{};
	};

	struct ScriptMetaData
	{
		FixedBufStr16 m_Name{};
		Scripting::ScriptType m_ScriptType {Scripting::ScriptType::None };
		FixedBufStr16 m_SectionLabel{};
		WrappedFuncType m_FunctionType{};
		Scripting::ExplicitFuncType m_ExplicitFuncType{};
	};

	struct ProjectEnumMetaData
	{
		FixedBufStr16 m_Name{};
	};

}
