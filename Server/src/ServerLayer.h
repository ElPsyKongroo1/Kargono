#pragma once

#include "Kargono.h"

#include <iostream>
#include <filesystem>

namespace Kargono
{

	//============================================================
	// Server Layer Class
	//============================================================
	
	class ServerLayer : public Application
	{
	public:
		//==========================
		// Constructor/Destructor
		//==========================

		ServerLayer();
		virtual ~ServerLayer() = default;
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
