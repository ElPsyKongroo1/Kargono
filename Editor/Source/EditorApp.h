#pragma once
#include "Kargono.h"

#include "Windows/UserInterface/UIEditorPanel.h"
#include "Windows/MainWindow/MainWindow.h"

#include <filesystem>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>


namespace Kargono
{
	//============================================================
	// Editor App Class
	//============================================================
	// This class holds all of the editor application logic. The concept of
	//		layers is in its infancy currently. This class holds the logic for
	//		all of the different systems that the editor uses such as scene management,
	//		ImGui code for editor windows, and project management.
	class EditorApp : public Application
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		// The constructor simply calls the parent constructor and initializes
		//		the ImGui layer for window use.
		EditorApp(const std::filesystem::path& projectPath);
		virtual ~EditorApp() override = default;
		//=========================
		// LifeCycle Functions
		//=========================
		virtual void Init() override;
		virtual void Terminate() override;

		//=========================
		// On Event Functions
		//=========================
		virtual void OnUpdate(Timestep ts) override;
		virtual bool OnApplicationEvent(Events::Event* event) override;
		virtual bool OnNetworkEvent(Events::Event* event) override;
		virtual bool OnInputEvent(Events::Event* event) override;
		virtual bool OnPhysicsEvent(Events::Event* event) override;
		virtual bool OnSceneEvent(Events::Event* event) override;
		virtual bool OnEditorEvent(Events::Event* event) override;
		virtual bool OnAssetEvent(Events::Event* event) override;
		virtual bool OnLogEvent(Events::Event* event) override;
	private:
		// This function responds to application collision events.
		bool OnPhysicsCollisionStart(Events::PhysicsCollisionStart event);
		bool OnPhysicsCollisionEnd(Events::PhysicsCollisionEnd event);

	public:
		//=========================
		// Getters/Setters
		//=========================
		static EditorApp* GetCurrentApp() { return s_EditorApp; }
	public:
		//=========================
		// Scene/Project Management
		//=========================
		// These functions are called from different parts of the editor to allow smooth
		//		transitions between the current Project and other projects. Also allows
		//		the current project to save.
		bool OpenProject();
		void OpenProject(const std::filesystem::path& path);
		void SaveProject();
		
	public:
		//=========================
		// Public App Data
		//=========================
		// Windows
		Scope<Windows::MainWindow> m_MainWindow;

	private:
		//=========================
		// Private App Data
		//=========================
		// EditorApp Singleton
		static inline EditorApp* s_EditorApp{ nullptr };
		// Initialization Fields
		std::filesystem::path m_InitProjectPath {};

	};

}
