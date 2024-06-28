#pragma once

#include "Kargono.h"
#include "Kargono/Utility/FileSystem.h"

#include <string>

namespace Kargono::Panels
{

	// Usage:
	//  static ImGuiLog my_log;
	//  my_log.AddLog("Hello %d world\n", 123);
	//  my_log.Draw("title");
	struct ImGuiLog
	{
		ImGuiLog();

		void Clear();

		void AddLog(const char* line);

		void Draw(const char* title, bool* p_open = NULL);
	public:
		ImGuiTextBuffer     Buf;
		ImGuiTextFilter     Filter;
		ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
		bool                AutoScroll;  // Keep scrolling if already at the bottom.
	};

	class LogPanel
	{
	public:
		LogPanel();

		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
		void LoadBuffer();
	private:
		ImGuiLog m_Log;
		std::filesystem::path m_LogPathRelative = "log/Kargono.log";
		Buffer m_Buffer;
		std::string m_PanelName{ "Log" };
	};
}
