#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/TextureManagerTemp.h"

#include "API/ImageProcessing/stbAPI.h"

namespace Kargono::Assets
{
	Ref<Rendering::Texture2D> TextureManager::InstantiateAssetIntoMemory(Assets::Asset& asset)
	{
		Assets::TextureMetaData metadata = *static_cast<Assets::TextureMetaData*>(asset.Data.SpecificFileData.get());
		Buffer currentResource{};
		currentResource = Utility::FileSystem::ReadFileBinary(Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.IntermediateLocation);
		Ref<Rendering::Texture2D> newTexture = Rendering::Texture2D::Create(currentResource, metadata);

		currentResource.Release();
		return newTexture;
	}

	static TextureManager s_TextureManager;

	Ref<Rendering::Texture2D> AssetServiceTemp::GetTexture(const AssetHandle& handle)
	{
		return s_TextureManager.GetAsset(handle);
	}
}
