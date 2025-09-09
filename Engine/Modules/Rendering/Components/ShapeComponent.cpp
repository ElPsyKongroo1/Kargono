#include "kgpch.h"

#include "Modules/Rendering/Components/ShapeComponent.h"
#include "Modules/Assets/AssetService.h"
#include "Kargono/Core/Buffer.h"

namespace Kargono::Rendering
{
	ShapeComponent::ShapeComponent()
	{
		auto [handle, shader] = Assets::AssetService::GetShader(m_ShaderSpecification);
		m_ShaderHandle = handle;
		m_Shader = shader;
		Buffer textureBuffer{ 4 };
		textureBuffer.SetDataToByte(0xff);
		m_TextureHandle = Assets::AssetService::ImportNewTextureFromData(textureBuffer, 1, 1, 4);
		m_Texture = Assets::AssetService::GetTexture2D(m_TextureHandle);
		textureBuffer.Release();
		Buffer buffer(m_Shader->GetInputLayout().GetStride() * sizeof(uint8_t));
		m_ShaderData = buffer;
		m_ShaderData.SetDataToByte(0);
	}
	ShapeComponent::~ShapeComponent()
	{
		if (m_ShaderData)
		{
			m_ShaderData.Release();
		}
	}
}