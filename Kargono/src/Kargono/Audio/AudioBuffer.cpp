#include "kgpch.h"
#include "AudioContext.h"
#include "AudioMisc.h"

void AudioBuffer::init(const char* fileLocation)
{

	// Create Buffers
	drwav_int16* pSampleData = drwav_open_file_and_read_pcm_frames_s16(fileLocation, &channels, &sampleRate, &totalPCMFrameCount, nullptr);
	if (pSampleData == NULL)
	{
		std::cerr << "Failed to load audio file\n";
		drwav_free(pSampleData, nullptr);
		return;
	}
	if (getTotalSamples() > drwav_uint64(std::numeric_limits<size_t>::max()))
	{
		std::cerr << "Too much data in file for 32bit addressed vector\n";
		drwav_free(pSampleData, nullptr);
		return;
	}
	pcmData.resize(size_t(getTotalSamples()));
	std::memcpy(pcmData.data(), pSampleData, pcmData.size() * 2 /*two bytes in s16*/);
	drwav_free(pSampleData, nullptr);

	alec(alGenBuffers(1, &bufferID));
	alec(alBufferData(bufferID, channels > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, pcmData.data(), pcmData.size() * 2 /*Two bytes per sample*/, sampleRate));

}
