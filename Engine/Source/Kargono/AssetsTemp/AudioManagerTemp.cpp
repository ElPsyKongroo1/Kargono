#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/AudioManagerTemp.h"

#include "API/Audio/OpenALAPI.h"
#include "API/Audio/drwavAPI.h"
namespace Kargono::Assets
{
	Ref<Audio::AudioBuffer> AudioManager::InstantiateAssetIntoMemory(Assets::Asset& asset)
	{
		Assets::AudioMetaData metadata = *static_cast<Assets::AudioMetaData*>(asset.Data.SpecificFileData.get());
		Buffer currentResource{};
		currentResource = Utility::FileSystem::ReadFileBinary(Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.IntermediateLocation);
		Ref<Audio::AudioBuffer> newAudio = CreateRef<Audio::AudioBuffer>();
		CallAndCheckALError(alBufferData(newAudio->m_BufferID, metadata.Channels > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, currentResource.Data, static_cast<ALsizei>(currentResource.Size), metadata.SampleRate));
		currentResource.Release();
		return newAudio;
	}

	static AudioManager s_AudioManager;

	Ref<Audio::AudioBuffer> AssetServiceTemp::GetAudio(const AssetHandle& handle)
	{
		return s_AudioManager.GetAsset(handle);
	}
}
