#include "../DefaultApplication/DefaultApplication.h"
#include "../Application2D/Application2D.h"
#include "../Library/Application/Application.h"
#include "../Library/Library.h"

/*============================================================================================================================================================================================
 * Entry Point of Projects
 *============================================================================================================================================================================================*/


int main()
{
	Resources::applicationManager.CreateApplications();
	
	int choice = 0;
	while (choice != 3) {
		std::cout << std::endl;
		std::cout << "==============" << std::endl;
		std::cout << "1. 3D Renderer" << std::endl;
		std::cout << "2. 2D Renderer" << std::endl;
		std::cout << "3. Exit" << std::endl;
		std::cout << "==============" << std::endl << std::endl;
		try {
			std::cin >> choice;
			if ((choice != 1) && (choice != 2) && (choice != 3)) {
				throw std::runtime_error("Invalid Input");
			}
			switch (choice) {
			case 1:
				// Run Default Application
				Resources::currentApplication = &Resources::applicationManager.defaultApplication;
				DefaultApplication();
				break;
			case 2:
				// Run Default Application
				Resources::currentApplication = &Resources::applicationManager.application2D;
				Application2D();
				break;
			case 3:
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
	
	Resources::applicationManager.DestroyApplications();
}