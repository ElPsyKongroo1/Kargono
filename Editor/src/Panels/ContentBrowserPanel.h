#pragma once

#include <filesystem>

namespace Kargono
{
	class Texture2D;

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
	private:
		Ref<Texture2D> BrowserFileTypeToIcon(BrowserFileType type);
	private:

		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;
		
	};
}
