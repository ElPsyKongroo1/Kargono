#pragma once


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

		void AddLog(const char* fmt, ...) IM_FMTARGS(2);

		void Draw(const char* title, bool* p_open = NULL);
	public:
		ImGuiTextBuffer     Buf;
		ImGuiTextFilter     Filter;
		ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
		bool                AutoScroll;  // Keep scrolling if already at the bottom.
	};

	class ConsolePanel
	{
	public:
		ConsolePanel();

		void OnImGuiRender();
	private:
		ImGuiLog m_Log;
	};
}
