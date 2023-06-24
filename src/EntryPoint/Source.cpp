#include "../Applications/DefaultApplication3D/DefaultApplication3D.h"
#include "../Applications/DefaultApplication2D/DefaultApplication2D.h"
#include "../Applications/Breakout/Breakout.h"
#include "../Library/Rendering/RendererState/RendererState.h"
#include "../Library/Library.h"
#include <AL/al.h>
#include <AL/alc.h>

/*============================================================================================================================================================================================
 * Entry Point of Projects
 *============================================================================================================================================================================================*/

struct ReadWavData 
{
	unsigned int channels = 0;
	unsigned int sampleRate = 0;
	drwav_uint64 totalPCMFrameCount = 0;
	std::vector<uint16_t> pcmData;
	drwav_uint64 getTotalSamples() { return totalPCMFrameCount * channels; }
};

#define OpenAL_ErrorCheck(message)\
{\
	ALenum error = alGetError();\
	if( error != AL_NO_ERROR)\
	{\
		std::cerr << "OpenAL Error: " << error << " with call for " << #message << '\n';\
	}\
}

#define alec(FUNCTION_CALL)\
FUNCTION_CALL;\
OpenAL_ErrorCheck(FUNCTION_CALL)

void playMusic() // This is just for testing! Will add actual system at some point
{
	// Find default audio device
	const ALCchar* defaultDeviceString = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
	ALCdevice* device = alcOpenDevice(defaultDeviceString);
	if (!device)
	{
		std::cerr << "Failed to get the default device for OpenAL\n";
		throw std::runtime_error("Check Logs");
	}
	std::cout << "OpenAL Device: " << alcGetString(device, ALC_DEVICE_SPECIFIER) << '\n';
	//OpenAL_ErrorCheck(device);

	// Create an OpenAL audio context from the device
	ALCcontext* context = alcCreateContext(device, nullptr);
	//OpenAL_ErrorCheck(context);

	// Activate this context so that OpenAL state modifications are applied to the context
	if (!alcMakeContextCurrent(context))
	{
		std::cerr << "Failed to make the OpenAL context the current context\n";
		throw std::runtime_error("Check Logs");
	}
	OpenAL_ErrorCheck("Make context current");

	// Create a listener in 3D space
	alec(alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f));
	alec(alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f));
	ALfloat forwardAndUpVectors[] =
	{
		1.0f, 0.0f, 0.0f,  // Forward Vectors
		0.0f, 1.0f, 0.0f   // Up Vectors
	};
	alec(alListenerfv(AL_ORIENTATION, forwardAndUpVectors));

	// Create Buffers


}


int main()
{
	playMusic();
	Resources::applicationManager.CreateApplications();
	Resources::rendererManager.CreateDefaultRenderers();
	int Version[2]{ 4, 6 };
	
	int choice = 0;
	while (choice != 4) {
		std::cout << std::endl;
		std::cout << "==============" << std::endl;
		std::cout << "1. 3D Renderer" << std::endl;
		std::cout << "2. 2D Renderer" << std::endl;
		std::cout << "3. Breakout" << std::endl;
		std::cout << "4. Exit" << std::endl;
		std::cout << "==============" << std::endl << std::endl;
		try {
			std::cin >> choice;
			if ((choice != 1) && (choice != 2) && (choice != 3)) {
				throw std::runtime_error("Invalid Input");
			}
			switch (choice) {
			case 1:
				// Run Default Application
				Resources::currentApplication = Resources::applicationManager.default3DApplication;
				Resources::currentApplication->renderer = Resources::rendererManager.Sample3DRenderer;
				DefaultApplication3D();
				Resources::currentApplication->renderer = nullptr;
				
				break;
			
			case 2:
				// Run Default Application
				Resources::currentApplication = Resources::applicationManager.breakout;
				Resources::currentApplication->renderer = Resources::rendererManager.Sample2DRenderer;
				Application2D();
				Resources::currentApplication->renderer = nullptr;
				break;
			case 3:
				// Run Default Application
				Resources::currentApplication = Resources::applicationManager.breakout;
				Resources::currentApplication->renderer = new RendererState("Breakout", Version,
					glm::vec2(Resources::currentApplication->width, Resources::currentApplication->height),
					glm::vec3(0.0f, 0.0f, 0.0f));
				Resources::currentGame = static_cast<GameApplication*>(Resources::currentApplication);
				BreakoutStart();
				delete Resources::currentApplication->renderer;
				Resources::currentApplication->renderer = nullptr;
				Resources::currentGame->renderer = nullptr;
				break;
			case 4:
				break;
			default:
				std::cout << "Something went wrong!" << std::endl;
				break;
			}
		}
		catch (std::runtime_error& excpt) {
			std::cin.clear(); //clear bad input flag
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //discard input
		}
	}
	
	Resources::rendererManager.DestroyDefaultRenderers();
	Resources::applicationManager.DestroyApplications();
}