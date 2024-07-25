#pragma once

#include "Kargono/Events/KeyEvent.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Kargono::Panels
{
	enum class BrowserFileType
	{
		None = 0, Directory = 1, Image = 2, Audio = 3,
		Binary = 4, Registry = 5, Scene = 6, ScriptProject = 7,
		Font = 8, UserInterface = 9, Input = 10
	};


	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	public:
		void UpdateCurrentDirectory(const std::filesystem::path& newPath);
		void RefreshCachedDirectoryEntries();
		void ResetPanelResources();
	private:
		std::string m_PanelName{ "Content Browser" };
		std::filesystem::path m_BaseDirectory {};
		std::filesystem::path m_CurrentDirectory {};
		std::filesystem::path m_LongestRecentPath {};
		std::vector<std::filesystem::path> m_CachedDirectoryEntries {};

		
	};
}
