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
		ServerApp(std::filesystem::path projectPath);
		virtual ~ServerApp() = default;
		//==========================
		// LifeCycle Functions
		//==========================

		virtual void Init() override;

		virtual void Terminate() override;

		virtual void OnUpdate(Timestep ts) override;

		//==========================
		// Manage Project
		//==========================

		bool OpenProject();

		void OpenProject(const std::filesystem::path& path);
	private:
		std::filesystem::path m_ProjectPath;
	};
}
