#pragma once
#include <filesystem>


namespace Kargono
{
	class TextEditorPanel
	{
	public:
		TextEditorPanel();

		void OnEditorUIRender();

		void OpenFile(const std::filesystem::path& path);

	};
}
