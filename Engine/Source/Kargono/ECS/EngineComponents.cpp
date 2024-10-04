#include "kgpch.h"

#include "Kargono/ECS/EngineComponents.h"
#include "Kargono/Assets/AssetService.h"

namespace Kargono::ECS
{
	bool ClassInstanceComponent::ChangeClass(Assets::AssetHandle classHandle)
	{
		Ref<Scenes::EntityClass> entityClassRef = Assets::AssetService::GetEntityClass(classHandle);
		if (!entityClassRef)
		{
			KG_WARN("Could not retrieve entity class reference in Components.h");
			return false;
		}
		ClassHandle = classHandle;
		ClassReference = entityClassRef;
		Fields.clear();
		for (auto& [name, type] : entityClassRef->GetFields())
		{
			Fields.push_back(Utility::WrappedVarTypeToWrappedVariable(type));
		}
		return true;
	}
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
