#pragma once

#include "Kargono/Core/FileSystem.h"

#include "imgui.h"

#include <filesystem>


namespace Kargono
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
		ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
		bool                AutoScroll;  // Keep scrolling if already at the bottom.
	};

	class LogPanel
	{
	public:
		LogPanel();

		void OnImGuiRender();
		void LoadBuffer();
	private:
		ImGuiLog m_Log;
		std::filesystem::path m_LogPathRelative = "log/Kargono.log";
		Buffer m_Buffer;
	};
}
