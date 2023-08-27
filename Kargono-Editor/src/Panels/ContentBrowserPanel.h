#pragma once
#include <filesystem>
#include "Kargono/Renderer/Texture.h"

namespace Kargono
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();
	private:
		std::filesystem::path m_CurrentDirectory;
		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_GenericFileIcon;
	};
}
