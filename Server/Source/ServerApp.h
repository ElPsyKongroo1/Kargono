#pragma once

#include "Kargono.h"

#include <filesystem>

namespace Kargono
{

	//============================================================
	// Server Layer Class
	//============================================================
	
	class ServerApp : public Application
	{
	public:
		//==========================
		// Constructor/Destructor
		//==========================

		ServerApp();
		virtual ~ServerApp() = default;
		//==========================
		// LifeCycle Functions
		//==========================

		virtual void OnAttach() override;

		bool OpenProject();

		void OpenProject(const std::filesystem::path& path);

		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override;

	};
}
