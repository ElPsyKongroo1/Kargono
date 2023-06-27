#include "../Applications/DefaultApplication3D/DefaultApplication3D.h"
#include "../Applications/DefaultApplication2D/DefaultApplication2D.h"
#include "../Applications/Breakout/Breakout.h"
#include "../Library/Rendering/RendererState/RendererState.h"
#include "../Library/Library.h"
#include "../Library/Application/Audio/AudioContext.h"


/*============================================================================================================================================================================================
 * Entry Point of Projects
 *============================================================================================================================================================================================*/


int main()
{
	AudioContext* menuAudio = new AudioContext("Resources/Breakout/Sounds/Manoria-Cathedral.wav");
	menuAudio->stereoSource->play();
	Resources::applicationManager.CreateApplications();
	Resources::rendererManager.CreateDefaultRenderers();
	int Version[2]{ 4, 6 };
	
	int choice = 0;
	while (choice != 4) {
		if (!menuAudio) 
		{
			menuAudio = new AudioContext("Resources/Breakout/Sounds/Manoria-Cathedral.wav");
			menuAudio->stereoSource->play();
		}
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
				menuAudio->terminate();
				delete menuAudio;
				menuAudio = nullptr;
				// Run Default Application
				Resources::currentApplication = Resources::applicationManager.default3DApplication;
				Resources::currentApplication->renderer = Resources::rendererManager.Sample3DRenderer;
				DefaultApplication3D();
				Resources::currentApplication->renderer = nullptr;
				
				break;
			
			case 2:
				menuAudio->terminate();
				delete menuAudio;
				menuAudio = nullptr;
				// Run Default Application
				Resources::currentApplication = Resources::applicationManager.breakout;
				Resources::currentApplication->renderer = Resources::rendererManager.Sample2DRenderer;
				Application2D();
				Resources::currentApplication->renderer = nullptr;
				break;
			case 3:
				menuAudio->terminate();
				delete menuAudio;
				menuAudio = nullptr;
				// Run Default Application
				Resources::currentApplication = Resources::applicationManager.breakout;
				Resources::currentApplication->renderer = new RendererState("Breakout", Version,
					glm::vec2(Resources::currentApplication->width, Resources::currentApplication->height),
					glm::vec3(0.0f, 0.0f, 0.0f));
				Resources::currentGame = static_cast<GameApplication*>(Resources::currentApplication);
				Resources::currentGame->audioContext = new AudioContext("Resources/Breakout/Sounds/mechanist-theme.wav");
				BreakoutStart();
				delete Resources::currentGame->audioContext;
				delete Resources::currentApplication->renderer;
				Resources::currentApplication->renderer = nullptr;
				Resources::currentGame->renderer = nullptr;
				break;
			case 4:
				menuAudio->terminate();
				delete menuAudio;
				menuAudio = nullptr;
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