#pragma once

#include "Kargono.h"

namespace Kargono
{
	class LauncherApp : public Application
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		// The constructor simply calls the parent constructor and initializes
		//		the ImGui layer for window use.
		LauncherApp() = default;
		virtual ~LauncherApp() override = default;
	public:
		//=========================
		// LifeCycle Functions
		//=========================
		virtual void Init() override;
		virtual void Terminate() override;
		virtual void OnUpdate(Timestep ts) override;

	private:
		//=========================
		// Manage Projects
		//=========================
		void SelectProject();

	public:
		//=========================
		// Getters/Setters
		//=========================
		std::filesystem::path GetSelectedProject()
		{
			return m_SelectedProject;
		}
		void SetSelectedProject(const std::filesystem::path& path)
		{
			m_SelectedProject = path;
		}
	private:
		//=========================
		// Internal Fields
		//=========================
		std::filesystem::path m_SelectedProject{};
	};

}