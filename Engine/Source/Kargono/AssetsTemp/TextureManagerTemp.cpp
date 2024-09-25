#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/TextureManagerTemp.h"

#include "API/ImageProcessing/stbAPI.h"

namespace Kargono::Assets
{
	Ref<Rendering::Texture2D> TextureManager::InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath)
	{
		Assets::TextureMetaData metadata = *asset.Data.GetSpecificMetaData<TextureMetaData>();
		Buffer currentResource = Utility::FileSystem::ReadFileBinary(assetPath);
		Ref<Rendering::Texture2D> newTexture = Rendering::Texture2D::Create(currentResource, metadata);
		currentResource.Release();
		return newTexture;
	}
}
