#pragma once

#include "Modules/Rendering/Shader.h"
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
		FixedBufStr64 m_Name;
		AssetHandle m_Handle{ Assets::k_EmptyHandle };
		AssetIdentifier m_TypeIdentifier{ k_InvalidAssetIdentifier };
		FixedBufStr64 m_CheckSum;
		std::filesystem::path m_FileLocation;
		std::filesystem::path m_IntermediateLocation;
		bool m_IsHidden{ false };
	private:
		//==============================
		// Internal Field(s)
		//==============================
		void* m_SpecificMetaData{ nullptr };
	};

	struct ShaderMetaData
	{
		Rendering::ShaderSpecification m_ShaderSpec{};
		Rendering::UniformBufferList m_UniformList{};
		Rendering::InputBufferLayout m_InputLayout{};
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
}
