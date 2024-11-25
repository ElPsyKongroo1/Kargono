#include "kgpch.h"

#include "Kargono/ECS/EngineComponents.h"
#include "Kargono/Assets/AssetService.h"

namespace Kargono::ECS
{
	ShapeComponent::ShapeComponent()
	{
		auto [handle, shader] = Assets::AssetService::GetShader(ShaderSpecification);
		ShaderHandle = handle;
		Shader = shader;
		Buffer textureBuffer{ 4 };
		textureBuffer.SetDataToByte(0xff);
		TextureHandle = Assets::AssetService::ImportNewTextureFromData(textureBuffer, 1, 1, 4);
		Texture = Assets::AssetService::GetTexture2D(TextureHandle);
		textureBuffer.Release();
		Buffer buffer(Shader->GetInputLayout().GetStride() * sizeof(uint8_t));
		ShaderData = buffer;
		ShaderData.SetDataToByte(0);
	}
}
