#pragma once

#include "Modules/Rendering/Module/RenderingModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"

#include "Modules/Assets/AssetsCommon.h"
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

	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(ShapeComponent* dst)
		{
			// Create the component in place
			std::construct_at<ShapeComponent>(dst);

			// Copy all fields
			dst->m_CurrentShape = m_CurrentShape;
			dst->m_Vertices = m_Vertices;
			dst->m_TextureCoordinates = m_TextureCoordinates;
			dst->m_Indices = m_Indices;
			dst->m_Shader = m_Shader;
			dst->m_ShaderSpecification = m_ShaderSpecification;
			dst->m_Texture = m_Texture;
			dst->m_TextureHandle = m_TextureHandle;
			dst->m_ShaderHandle = m_ShaderHandle;
			dst->m_ShaderData = Buffer::Copy(m_ShaderData);
			dst->m_VertexColors = m_VertexColors;
		}
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Public Fields
		//==============================
		Rendering::ShapeTypes m_CurrentShape{ Rendering::ShapeTypes::None };
		Ref<std::vector<Math::vec3>> m_Vertices{ nullptr };
		Ref<std::vector<Math::vec2>> m_TextureCoordinates{ nullptr };
		Ref<std::vector<uint32_t>> m_Indices{ nullptr };
		Ref<std::vector<Math::vec4>> m_VertexColors{ nullptr };
		Ref<Rendering::Shader> m_Shader;
		Rendering::ShaderSpecification m_ShaderSpecification{ Rendering::ColorInputType::None, Rendering::TextureInputType::None, false, true, true, Rendering::RenderingType::DrawIndex, false };
		Assets::AssetHandle m_ShaderHandle{ Assets::k_EmptyHandle };
		Ref<Rendering::Texture2D> m_Texture;
		Assets::AssetHandle m_TextureHandle{ Assets::k_EmptyHandle };
		Buffer m_ShaderData;
	};

	Register_Module_Type(ShapeComponent, ECSInternal::ComponentTag)
}