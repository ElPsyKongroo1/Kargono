#include "kgpch.h"

#include "Windows/MainWindow/LogPanel.h"

#include "EditorApp.h"


//----------------------------------------------//
// ImGuiLog 
//----------------------------------------------//

static Kargono::EditorApp* s_EditorApp { nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{
	ImGuiLog::ImGuiLog()
	{
		AutoScroll = true;
		Clear();
	}

	void ImGuiLog::Clear()
	{
		Buf.clear();
		LineOffsets.clear();
		LineOffsets.push_back(0);
	}

	void ImGuiLog::AddLog(const char* line)
	{
		int old_size = Buf.size();
		Buf.append(line);
		for (int new_size = Buf.size(); old_size < new_size; old_size++)
			if (Buf[old_size] == '\n')
				LineOffsets.push_back(old_size + 1);
	}

	void ImGuiLog::Draw(const char* title, bool* p_open)
	{
		if (!ImGui::Begin(title, p_open))
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		// Options menu
		ImGui::SameLine();
		if (ImGui::BeginPopup("Options"))
		{
			ImGui::Checkbox("Auto-scroll", &AutoScroll);
			ImGui::EndPopup();
		}
		// Main window
		if (ImGui::Button("Options"))
			ImGui::OpenPopup("Options");
		ImGui::SameLine();
		bool clear = ImGui::Button("Clear");
		ImGui::SameLine();
		Filter.Draw("Filter", -100.0f);
		ImGui::Separator();

		if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
		{
			if (clear)
				Clear();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			const char* buf = Buf.begin();
			const char* buf_end = Buf.end();
			if (Filter.IsActive())
			{
				for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
				{
					const char* line_start = buf + LineOffsets[line_no];
					const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
					if (Filter.PassFilter(line_start, line_end))
						ImGui::TextUnformatted(line_start, line_end);
				}
			}
			else
			{
				ImGuiListClipper clipper;
				clipper.Begin(LineOffsets.Size);
				while (clipper.Step())
				{
					for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
					{
						const char* line_start = buf + LineOffsets[line_no];
						const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
						ImGui::TextUnformatted(line_start, line_end);
					}
				}
				clipper.End();
			}

			ImGui::PopStyleVar();

			if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);
		}
		ImGui::EndChild();
		EditorUI::EditorUIService::EndWindow();
	}

	LogPanel::LogPanel()
		: m_Log({})
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(LogPanel::OnKeyPressedEditor));
		LoadBuffer();
	}

	// Demonstrate creating a simple log window with basic filtering.
	void LogPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		// For the demo: add a debug button _BEFORE_ the normal log window contents
		// We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to the _same_ window.
		// Most of the contents of the window will be added by the log.Draw() call.
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_MainWindow->m_ShowLog);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (ImGui::Button("Reload"))
		{
			LoadBuffer();
			m_Log.Clear();
			m_Log.AddLog(m_Buffer.As<char>());

			// 
			std::string currentLine = {};
			char currentChar;
			for (unsigned int i = 0; i < m_Buffer.Size; i++)
			{
				currentChar = (char)*(m_Buffer.Data + i);
				currentLine = currentLine + currentChar;
				if (currentChar == '\n')
				{
					m_Log.AddLog(currentLine.c_str());
					currentLine.clear();
				}
			}
		}
		EditorUI::EditorUIService::EndWindow();

		// Actually call in the regular Log helper (which will Begin() into the same window as we just did)
		m_Log.Draw("Log");
	}

	bool LogPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}

	void LogPanel::LoadBuffer()
	{
		if (m_LogPathRelative.empty()) { KG_ERROR("Log File Path is empty!"); return; }

		m_Buffer = Utility::FileSystem::ReadFileBinary(m_LogPathRelative);
	}
}


