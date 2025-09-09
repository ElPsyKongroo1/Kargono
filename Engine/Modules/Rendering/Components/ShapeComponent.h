#pragma once

#include "Modules/Rendering/Module/RenderingModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"

#include "Modules/Assets/Asset.h"
#include "Kargono/Core/Buffer.h"

#include <vector>

namespace Kargono::Rendering
{
	struct ShapeComponent
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ShapeComponent();
		~ShapeComponent();

		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(ShapeComponent* dst)
		{
			ShapeComponent* destination = (ShapeComponent*)dst;
			destination->m_CurrentShape = m_CurrentShape;
			destination->m_Vertices = m_Vertices;
			destination->m_TextureCoordinates = m_TextureCoordinates;
			destination->m_Indices = m_Indices;
			destination->m_Shader = m_Shader;
			destination->m_ShaderSpecification = m_ShaderSpecification;
			destination->m_Texture = m_Texture;
			destination->m_TextureHandle = m_TextureHandle;
			destination->m_ShaderHandle = m_ShaderHandle;
			destination->m_ShaderData = Buffer::Copy(m_ShaderData);
			destination->m_VertexColors = m_VertexColors;
		}

	public:
		//==============================
		// Public Fields
		//==============================
		Rendering::ShapeTypes m_CurrentShape{ Rendering::ShapeTypes::None };
		Ref<std::vector<Math::vec3>> m_Vertices{};
		Ref<std::vector<Math::vec2>> m_TextureCoordinates{};
		Ref<std::vector<uint32_t>> m_Indices{};
		Ref<std::vector<Math::vec4>> m_VertexColors{};
		Ref<Rendering::Shader> m_Shader;
		Rendering::ShaderSpecification m_ShaderSpecification{ Rendering::ColorInputType::None, Rendering::TextureInputType::None, false, true, true, Rendering::RenderingType::DrawIndex, false };
		Assets::AssetHandle m_ShaderHandle{ Assets::EmptyHandle };
		Ref<Rendering::Texture2D> m_Texture;
		Assets::AssetHandle m_TextureHandle{ Assets::EmptyHandle };
		Buffer m_ShaderData;
	};

	Register_Module_Type(ShapeComponent, ECSInternal::ComponentTag)
}