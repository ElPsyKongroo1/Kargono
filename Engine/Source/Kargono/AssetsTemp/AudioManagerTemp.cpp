#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/AudioManagerTemp.h"

#include "API/Audio/OpenALAPI.h"
#include "API/Audio/drwavAPI.h"
namespace Kargono::Assets
{
	Ref<Audio::AudioBuffer> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath)
	{
		Assets::AudioMetaData metadata = *asset.Data.GetSpecificMetaData<Assets::AudioMetaData>();
		Buffer currentResource{};
		currentResource = Utility::FileSystem::ReadFileBinary(assetPath);
		Ref<Audio::AudioBuffer> newAudio = CreateRef<Audio::AudioBuffer>();
		CallAndCheckALError(alBufferData(newAudio->m_BufferID, metadata.Channels > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, currentResource.Data, static_cast<ALsizei>(currentResource.Size), metadata.SampleRate));
		currentResource.Release();
		return newAudio;
	}
}
