#pragma once

#include "Kargono/Renderer/Texture.h"

#include <filesystem>

namespace Kargono
{
	enum class BrowserFileType;

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();
	private:
		Ref<Texture2D> BrowserFileTypeToIcon(BrowserFileType type);
	private:

		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;
		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_GenericFileIcon;
		Ref<Texture2D> m_BackIcon;
		Ref<Texture2D> m_AudioIcon;
		Ref<Texture2D> m_ImageIcon;
		Ref<Texture2D> m_BinaryIcon;
		Ref<Texture2D> m_SceneIcon;
		Ref<Texture2D> m_RegistryIcon;
		Ref<Texture2D> m_ScriptProjectIcon;
	};
}
