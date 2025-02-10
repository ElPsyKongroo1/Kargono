#include <algorithm>
#include <chrono>
#include <string>
#include <regex>
#include <cmath>
#include <format>
#include <map>

#include "TextEditor.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h" // for imGui::GetCurrentWindow()
#include "Kargono/EditorUI/EditorUI.h"
#include "Kargono/Scripting/ScriptCompilerService.h"
#include "Kargono/Scripting/ScriptTokenizer.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/Core/Base.h"

namespace API::EditorUI
{

	template<class InputIt1, class InputIt2, class BinaryPredicate>
	bool equals(InputIt1 first1, InputIt1 last1,
		InputIt2 first2, InputIt2 last2, BinaryPredicate p)
	{
		for (; first1 != last1 && first2 != last2; ++first1, ++first2)
		{
			if (!p(*first1, *first2))
				return false;
		}
		return first1 == last1 && first2 == last2;
	}

	TextEditorSpec::TextEditorSpec()
		: m_LineSpacing(1.0f)
		, m_UndoIndex(0)
		, m_TabSize(4)
		, m_Overwrite(false)
		, m_ReadOnly(false)
		, m_WithinRender(false)
		, m_ScrollToCursor(false)
		, m_ScrollToTop(false)
		, m_TextChanged(false)
		, m_ColorizerEnabled(true)
		, m_TextStart(20.0f)
		, m_LeftMargin(10)
		, m_CursorPositionChanged(false)
		, m_ColorRangeMin(0)
		, m_ColorRangeMax(0)
		, m_SelectionMode(SelectionMode::Normal)
		, m_CheckComments(true)
		, m_LastClick(-1.0f)
		, m_HandleKeyboardInputs(true)
		, m_HandleMouseInputs(true)
		, m_IgnoreImGuiChild(false)
		, m_ShowWhitespaces(true)
		, m_StartTime(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
	{
		SetPalette(TextEditorService::GetDefaultColorPalette());
		SetLanguageDefinition(TextEditorService::GenerateHLSL());
		m_Lines.push_back(Line());

		m_SuggestionTree.m_Label = "Suggestions";
	}

	TextEditorSpec::~TextEditorSpec()
	{
	}

	void TextEditorSpec::SetLanguageDefinition(const LanguageDefinition & aLanguageDef)
	{
		m_LanguageDefinition = aLanguageDef;
		m_RegexList.clear();

		for (TokenRegexString& r : m_LanguageDefinition.m_TokenRegexStrings)
			m_RegexList.push_back(std::make_pair(std::regex(r.first, std::regex_constants::optimize), r.second));

		Colorize();
	}

	void TextEditorSpec::SetLanguageDefinitionByExtension(const std::string& extension)
	{
		if (extension == ".cpp")
		{
			SetLanguageDefinition(TextEditorService::GenerateCPlusPlus());
			SetSuggestionsWindowEnabled(false);
		}
		else if (extension == ".kgscript")
		{
			SetLanguageDefinition(TextEditorService::GenerateKargonoScript());
			SetSuggestionsWindowEnabled(true);
		}
		else
		{
			SetLanguageDefinition(TextEditorService::GenerateC());
			SetSuggestionsWindowEnabled(false);
		}
	}

	void TextEditorSpec::SetPalette(const Palette & aValue)
	{
		m_PaletteBase = aValue;
	}

	std::string TextEditorSpec::GetText(const Coordinates & aStart, const Coordinates & aEnd) const
	{
		std::string result;

		int lstart = aStart.m_Line;
		int lend = aEnd.m_Line;
		int istart = GetCharacterIndex(aStart);
		int iend = GetCharacterIndex(aEnd);
		size_t s = 0;

		for (size_t i = lstart; i < lend; i++)
			s += m_Lines[i].size();

		result.reserve(s + s / 8);

		while (istart < iend || lstart < lend)
		{
			if (lstart >= (int)m_Lines.size())
				break;

			const Line& line = m_Lines[lstart];
			if (istart < (int)line.size())
			{
				result += line[istart].m_Char;
				istart++;
			}
			else
			{
				istart = 0;
				++lstart;
				result += '\n';
			}
		}

		return result;
	}

	Coordinates TextEditorSpec::GetActualCursorCoordinates() const
	{
		return SanitizeCoordinates(m_State.m_CursorPosition);
	}

	Coordinates TextEditorSpec::SanitizeCoordinates(const Coordinates & aValue) const
	{
		// Ensure coordinates are within the bounds of the text
		int line = aValue.m_Line;
		int column = aValue.m_Column;

		// Check if current line is in an invalid state
		if (line >= (int)m_Lines.size())
		{
			// Clear the line differently depending on the size of the m_Lines buffer
			if (m_Lines.empty())
			{
				line = 0;
				column = 0;
			}
			else
			{
				line = (int)m_Lines.size() - 1;
				column = GetLineMaxColumn(line);
			}
			return Coordinates(line, column);
		}
		else
		{
			// Line location is ok, ensure column is valid
			column = m_Lines.empty() ? 0 : std::min(column, GetLineMaxColumn(line));
			return Coordinates(line, column);
		}
	}

	// https://en.wikipedia.org/wiki/UTF-8
	// We assume that the char is a standalone character (<128) or a leading byte of an UTF-8 code sequence (non-10xxxxxx code)
	static int UTF8CharLength(uint8_t c)
	{
		if ((c & 0xFE) == 0xFC)
			return 6;
		if ((c & 0xFC) == 0xF8)
			return 5;
		if ((c & 0xF8) == 0xF0)
			return 4;
		else if ((c & 0xF0) == 0xE0)
			return 3;
		else if ((c & 0xE0) == 0xC0)
			return 2;
		return 1;
	}

	// "Borrowed" from ImGui source
	static inline int ImTextCharToUtf8(char* buf, int buf_size, unsigned int c)
	{
		if (c < 0x80)
		{
			buf[0] = (char)c;
			return 1;
		}
		if (c < 0x800)
		{
			if (buf_size < 2) return 0;
			buf[0] = (char)(0xc0 + (c >> 6));
			buf[1] = (char)(0x80 + (c & 0x3f));
			return 2;
		}
		if (c >= 0xdc00 && c < 0xe000)
		{
			return 0;
		}
		if (c >= 0xd800 && c < 0xdc00)
		{
			if (buf_size < 4) return 0;
			buf[0] = (char)(0xf0 + (c >> 18));
			buf[1] = (char)(0x80 + ((c >> 12) & 0x3f));
			buf[2] = (char)(0x80 + ((c >> 6) & 0x3f));
			buf[3] = (char)(0x80 + ((c) & 0x3f));
			return 4;
		}
		//else if (c < 0x10000)
		{
			if (buf_size < 3) return 0;
			buf[0] = (char)(0xe0 + (c >> 12));
			buf[1] = (char)(0x80 + ((c >> 6) & 0x3f));
			buf[2] = (char)(0x80 + ((c) & 0x3f));
			return 3;
		}
	}

	void TextEditorSpec::Advance(Coordinates & aCoordinates) const
	{
		if (aCoordinates.m_Line < (int)m_Lines.size())
		{
			const Line& line = m_Lines[aCoordinates.m_Line];
			int cindex = GetCharacterIndex(aCoordinates);

			if (cindex + 1 < (int)line.size())
			{
				int delta = UTF8CharLength(line[cindex].m_Char);
				cindex = std::min(cindex + delta, (int)line.size() - 1);
			}
			else
			{
				++aCoordinates.m_Line;
				cindex = 0;
			}
			aCoordinates.m_Column = GetCharacterColumn(aCoordinates.m_Line, cindex);
		}
	}

	void TextEditorSpec::DeleteRange(const Coordinates & aStart, const Coordinates & aEnd)
	{
		assert(aEnd >= aStart);
		assert(!m_ReadOnly);

		//printf("D(%d.%d)-(%d.%d)\n", aStart.m_Line, aStart.m_Column, aEnd.m_Line, aEnd.m_Column);

		if (aEnd == aStart)
			return;

		int start = GetCharacterIndex(aStart);
		int end = GetCharacterIndex(aEnd);

		if (aStart.m_Line == aEnd.m_Line)
		{
			Line& line = m_Lines[aStart.m_Line];
			int n = GetLineMaxColumn(aStart.m_Line);
			if (aEnd.m_Column >= n)
				line.erase(line.begin() + start, line.end());
			else
				line.erase(line.begin() + start, line.begin() + end);
		}
		else
		{
			Line& firstLine = m_Lines[aStart.m_Line];
			Line& lastLine = m_Lines[aEnd.m_Line];

			firstLine.erase(firstLine.begin() + start, firstLine.end());
			lastLine.erase(lastLine.begin(), lastLine.begin() + end);

			if (aStart.m_Line < aEnd.m_Line)
				firstLine.insert(firstLine.end(), lastLine.begin(), lastLine.end());

			if (aStart.m_Line < aEnd.m_Line)
				RemoveLine(aStart.m_Line + 1, aEnd.m_Line + 1);
		}

		m_TextChanged = true;
	}

	int TextEditorSpec::InsertTextAt(Coordinates& /* inout */ aWhere, const char * aValue)
	{
		assert(!m_ReadOnly);

		int cindex = GetCharacterIndex(aWhere);
		int totalLines = 0;
		while (*aValue != '\0')
		{
			assert(!m_Lines.empty());

			if (*aValue == '\r')
			{
				// skip
				++aValue;
			}
			else if (*aValue == '\n')
			{
				if (cindex < (int)m_Lines[aWhere.m_Line].size())
				{
					Line& newLine = InsertEmptyLine(aWhere.m_Line + 1);
					Line& line = m_Lines[aWhere.m_Line];
					newLine.insert(newLine.begin(), line.begin() + cindex, line.end());
					line.erase(line.begin() + cindex, line.end());
				}
				else
				{
					InsertEmptyLine(aWhere.m_Line + 1);
				}
				++aWhere.m_Line;
				aWhere.m_Column = 0;
				cindex = 0;
				++totalLines;
				++aValue;
			}
			else
			{
				Line& line = m_Lines[aWhere.m_Line];
				int d = UTF8CharLength(*aValue);
				while (d-- > 0 && *aValue != '\0')
					line.insert(line.begin() + cindex++, Glyph(*aValue, PaletteIndex::Default));

				if (*aValue == '\t')
					aWhere.m_Column += m_TabSize;
				else
					++aWhere.m_Column;

				++aValue;
			}

			m_TextChanged = true;
		}

		return totalLines;
	}

	void TextEditorSpec::AddUndo(UndoRecord& aValue)
	{
		assert(!m_ReadOnly);

		m_UndoBuffer.resize((size_t)(m_UndoIndex + 1));
		m_UndoBuffer.back() = aValue;
		++m_UndoIndex;
	}

	Coordinates TextEditorSpec::ScreenPosToCoordinates(const ImVec2& aPosition) const
	{
		ImVec2 origin = ImGui::GetCursorScreenPos();
		ImVec2 local(aPosition.x - origin.x, aPosition.y - origin.y);

		int lineNo = std::max(0, (int)floor(local.y / m_CharAdvance.y));

		int columnCoord = 0;

		if (lineNo >= 0 && lineNo < (int)m_Lines.size())
		{
			const Line& line = m_Lines.at(lineNo);

			int columnIndex = 0;
			float columnX = 0.0f;

			while ((size_t)columnIndex < line.size())
			{
				float columnWidth = 0.0f;

				if (line[columnIndex].m_Char == '\t')
				{
					float spaceSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, " ").x;
					float oldX = columnX;
					float newColumnX = (1.0f + std::floor((1.0f + columnX) / (float(m_TabSize) * spaceSize))) * (float(m_TabSize) * spaceSize);
					columnWidth = newColumnX - oldX;
					if (m_TextStart + columnX + columnWidth * 0.5f > local.x)
						break;
					columnX = newColumnX;
					columnCoord = (columnCoord / m_TabSize) * m_TabSize + m_TabSize;
					columnIndex++;
				}
				else
				{
					char buf[7];
					int d = UTF8CharLength(line[columnIndex].m_Char);
					int i = 0;
					while (i < 6 && d-- > 0)
						buf[i++] = line[columnIndex++].m_Char;
					buf[i] = '\0';
					columnWidth = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, buf).x;
					if (m_TextStart + columnX + columnWidth * 0.5f > local.x)
						break;
					columnX += columnWidth;
					columnCoord++;
				}
			}
		}

		return SanitizeCoordinates(Coordinates(lineNo, columnCoord));
	}

	Coordinates TextEditorSpec::FindWordStart(const Coordinates & aFrom) const
	{
		Coordinates at = aFrom;
		if (at.m_Line >= (int)m_Lines.size())
			return at;

		const Line& line = m_Lines[at.m_Line];
		int cindex = GetCharacterIndex(at);

		if (cindex >= (int)line.size())
			return at;

		while (cindex > 0 && isspace(line[cindex].m_Char))
			--cindex;

		PaletteIndex cstart = (PaletteIndex)line[cindex].m_ColorIndex;
		while (cindex > 0)
		{
			uint8_t c = line[cindex].m_Char;
			if ((c & 0xC0) != 0x80)	// not UTF code sequence 10xxxxxx
			{
				if (c <= 32 && isspace(c))
				{
					cindex++;
					break;
				}
				if (cstart != (PaletteIndex)line[size_t(cindex - 1)].m_ColorIndex)
					break;
			}
			--cindex;
		}
		return Coordinates(at.m_Line, GetCharacterColumn(at.m_Line, cindex));
	}

	Coordinates TextEditorSpec::FindWordEnd(const Coordinates & aFrom) const
	{
		Coordinates at = aFrom;
		if (at.m_Line >= (int)m_Lines.size())
			return at;

		const Line& line = m_Lines[at.m_Line];
		int cindex = GetCharacterIndex(at);

		if (cindex >= (int)line.size())
			return at;

		bool prevspace = (bool)isspace(line[cindex].m_Char);
		PaletteIndex cstart = (PaletteIndex)line[cindex].m_ColorIndex;
		while (cindex < (int)line.size())
		{
			uint8_t c = line[cindex].m_Char;
			int d = UTF8CharLength(c);
			if (cstart != (PaletteIndex)line[cindex].m_ColorIndex)
				break;

			if (prevspace != !!isspace(c))
			{
				if (isspace(c))
					while (cindex < (int)line.size() && isspace(line[cindex].m_Char))
						++cindex;
				break;
			}
			cindex += d;
		}
		return Coordinates(aFrom.m_Line, GetCharacterColumn(aFrom.m_Line, cindex));
	}

	Coordinates TextEditorSpec::FindNextWord(const Coordinates & aFrom) const
	{
		Coordinates at = aFrom;
		if (at.m_Line >= (int)m_Lines.size())
			return at;

		// skip to the next non-word character
		int cindex = GetCharacterIndex(aFrom);
		bool isword = false;
		bool skip = false;
		if (cindex < (int)m_Lines[at.m_Line].size())
		{
			const Line& line = m_Lines[at.m_Line];
			isword = isalnum(line[cindex].m_Char);
			skip = isword;
		}

		while (!isword || skip)
		{
			if (at.m_Line >= m_Lines.size())
			{
				int l = std::max(0, (int) m_Lines.size() - 1);
				return Coordinates(l, GetLineMaxColumn(l));
			}

			const Line& line = m_Lines[at.m_Line];
			if (cindex < (int)line.size())
			{
				isword = isalnum(line[cindex].m_Char);

				if (isword && !skip)
					return Coordinates(at.m_Line, GetCharacterColumn(at.m_Line, cindex));

				if (!isword)
					skip = false;

				cindex++;
			}
			else
			{
				cindex = 0;
				++at.m_Line;
				skip = false;
				isword = false;
			}
		}

		return at;
	}

	int TextEditorSpec::GetCharacterIndex(const Coordinates& aCoordinates) const
	{
		if (aCoordinates.m_Line >= m_Lines.size())
			return -1;
		const Line& line = m_Lines[aCoordinates.m_Line];
		int c = 0;
		int i = 0;
		for (; i < line.size() && c < aCoordinates.m_Column;)
		{
			if (line[i].m_Char == '\t')
				c = (c / m_TabSize) * m_TabSize + m_TabSize;
			else
				++c;
			i += UTF8CharLength(line[i].m_Char);
		}
		return i;
	}

	int TextEditorSpec::GetCharacterColumn(int aLine, int aIndex) const
	{
		if (aLine >= m_Lines.size())
			return 0;
		const Line& line = m_Lines[aLine];
		int col = 0;
		int i = 0;
		while (i < aIndex && i < (int)line.size())
		{
			uint8_t c = line[i].m_Char;
			i += UTF8CharLength(c);
			if (c == '\t')
				col = (col / m_TabSize) * m_TabSize + m_TabSize;
			else
				col++;
		}
		return col;
	}

	int TextEditorSpec::GetLineCharacterCount(int aLine) const
	{
		if (aLine >= m_Lines.size())
			return 0;
		const Line& line = m_Lines[aLine];
		int c = 0;
		for (unsigned i = 0; i < line.size(); c++)
			i += UTF8CharLength(line[i].m_Char);
		return c;
	}

	int TextEditorSpec::GetLineMaxColumn(int aLine) const
	{
		if (aLine >= m_Lines.size())
			return 0;
		const Line& line = m_Lines[aLine];
		int col = 0;
		for (unsigned i = 0; i < line.size(); )
		{
			uint8_t c = line[i].m_Char;
			if (c == '\t')
				col = (col / m_TabSize) * m_TabSize + m_TabSize;
			else
				col++;
			i += UTF8CharLength(c);
		}
		return col;
	}

	bool TextEditorSpec::IsOnWordBoundary(const Coordinates & aAt) const
	{
		if (aAt.m_Line >= (int)m_Lines.size() || aAt.m_Column == 0)
			return true;

		const Line& line = m_Lines[aAt.m_Line];
		int cindex = GetCharacterIndex(aAt);
		if (cindex >= (int)line.size())
			return true;

		if (m_ColorizerEnabled)
			return line[cindex].m_ColorIndex != line[size_t(cindex - 1)].m_ColorIndex;

		return isspace(line[cindex].m_Char) != isspace(line[cindex - 1].m_Char);
	}

	void TextEditorSpec::RemoveLine(int aStart, int aEnd)
	{
		assert(!m_ReadOnly);
		assert(aEnd >= aStart);
		assert(m_Lines.size() > (size_t)(aEnd - aStart));

		ErrorMarkers etmp;
		for (auto& [location, marker] : m_ErrorMarkers)
		{
			ErrorMarkers::value_type e(location >= aStart ? location - 1 : location, marker);
			if (location >= aStart && location <= aEnd)
			{
				continue;
			}
			etmp.insert(e);
		}
		m_ErrorMarkers = std::move(etmp);

		Breakpoints btmp;
		for (int location : m_Breakpoints)
		{
			if (location >= aStart && location <= aEnd)
			{
				continue;
			}
			btmp.insert(location >= aStart ? location - 1 : location);
		}
		m_Breakpoints = std::move(btmp);

		m_Lines.erase(m_Lines.begin() + aStart, m_Lines.begin() + aEnd);
		assert(!m_Lines.empty());

		m_TextChanged = true;
	}

	void TextEditorSpec::RemoveLine(int aIndex)
	{
		assert(!m_ReadOnly);
		assert(m_Lines.size() > 1);

		ErrorMarkers etmp;
		for (auto& [location, marker] : m_ErrorMarkers)
		{
			ErrorMarkers::value_type e(location > aIndex ? location - 1 : location, marker);
			if (location - 1 == aIndex)
			{
				continue;
			}
			etmp.insert(e);
		}
		m_ErrorMarkers = std::move(etmp);

		Breakpoints btmp;
		for (int location : m_Breakpoints)
		{
			if (location == aIndex)
			{
				continue;
			}
			btmp.insert(location >= aIndex ? location - 1 : location);
		}
		m_Breakpoints = std::move(btmp);

		m_Lines.erase(m_Lines.begin() + aIndex);
		assert(!m_Lines.empty());

		m_TextChanged = true;
	}

	Line& TextEditorSpec::InsertEmptyLine(int aIndex)
	{
		assert(!m_ReadOnly);

		Line& result = *m_Lines.insert(m_Lines.begin() + aIndex, Line());

		ErrorMarkers etmp;
		for (auto& [location, marker] : m_ErrorMarkers)
		{
			etmp.insert(ErrorMarkers::value_type(location >= aIndex ? location + 1 : location, marker));
		}
		m_ErrorMarkers = std::move(etmp);

		Breakpoints btmp;
		for (int location : m_Breakpoints)
		{
			btmp.insert(location >= aIndex ? location + 1 : location);
		}
		m_Breakpoints = std::move(btmp);

		return result;
	}

	Line& EditorUI::TextEditorSpec::InsertLine(const Line& lineToInsert, int index)
	{
		assert(!m_ReadOnly);

		Line& result = *m_Lines.insert(m_Lines.begin() + index, lineToInsert);

		ErrorMarkers etmp;
		for (auto& [location, marker] : m_ErrorMarkers)
		{
			etmp.insert(ErrorMarkers::value_type(location >= index ? location + 1 : location, marker));
		}
		m_ErrorMarkers = std::move(etmp);

		Breakpoints btmp;
		for (int location : m_Breakpoints)
		{
			btmp.insert(location >= index ? location + 1 : location);
		}
		m_Breakpoints = std::move(btmp);
		m_TextChanged = true;
		return result;
	}

	void TextEditorSpec::SetSuggestionsWindowEnabled(bool isEnabled)
	{
		if (isEnabled)
		{
			m_SuggestionsWindowEnabled = true;
			m_CloseTextSuggestions = false;
			m_OpenTextSuggestions = false;
			m_SuggestionTextBuffer.clear();
		}
		else
		{
			m_SuggestionsWindowEnabled = false;
			m_CloseTextSuggestions = false;
			m_OpenTextSuggestions = false;
			m_SuggestionTextBuffer.clear();
		}
	}

	void TextEditorSpec::RefreshSuggestionsContent()
	{
		std::string text = GetText();

		// Insert ContextProbe into editor text, which will be passed into the GetSuggestions() function
		{
			Coordinates cursorCoordinate = GetCursorPosition();
			uint32_t positionToInsert{ 0 };
			for (std::size_t iteration{0}; iteration < cursorCoordinate.m_Line; iteration++)
			{
				positionToInsert += (uint32_t)m_Lines.at(iteration).size();
				// Additional insert for newline character
				positionToInsert++;
			}
			positionToInsert += GetCharacterIndex(cursorCoordinate);
			text.insert(positionToInsert, Kargono::Scripting::ContextProbe);
			text.erase(positionToInsert - m_SuggestionTextBuffer.size(), m_SuggestionTextBuffer.size());
		}

		// Get suggestions from script compiler
		std::vector<Kargono::Scripting::SuggestionSpec> allSuggestions = Kargono::Scripting::ScriptCompilerService::GetSuggestions(text, m_SuggestionTextBuffer);

		// Exit gracefully if no suggestions were generated
		if (allSuggestions.size() == 0)
		{
			// Close suggestions
			m_SuggestionTextBuffer.clear();
			m_CloseTextSuggestions = true;
			return;
		}
		m_OpenTextSuggestions = true;
		m_CloseTextSuggestions = false;
		m_SuggestionTree.ClearTree();
	
		// Move suggestions into user interface 
		for (Kargono::Scripting::SuggestionSpec& suggestion : allSuggestions)
		{
			Kargono::EditorUI::TreeEntry entry;
			entry.m_Label = suggestion.m_Label;
			entry.m_ProvidedData = Kargono::CreateRef<std::tuple<std::string, int16_t>>(
				suggestion.m_ReplacementText, 
				suggestion.m_ShiftValue
			);
			entry.m_IconHandle = suggestion.m_Icon;
			entry.m_OnDoubleLeftClick = [&](Kargono::EditorUI::TreeEntry& entry)
			{
				// Retrieve tuple
				auto& [replacementText, shiftValue] = (*(std::tuple<std::string, int16_t>*)(entry.m_ProvidedData.get()));

				// Remove Buffer Text and add text
				UndoRecord u;
				u.m_Before = m_State;

				Coordinates cursorPosition = GetCursorPosition();
				DeleteRange({ cursorPosition.m_Line, cursorPosition.m_Column - (int)m_SuggestionTextBuffer.size() }, cursorPosition);
				SetCursorPosition({ cursorPosition.m_Line,cursorPosition.m_Column - (int)m_SuggestionTextBuffer.size() });
				cursorPosition = GetCursorPosition();
				u.m_AddedStart = cursorPosition;
				u.m_Added = replacementText.c_str();
				InsertTextAt(cursorPosition, u.m_Added.c_str());
				cursorPosition.m_Column += shiftValue;
				SetCursorPosition(cursorPosition);

				u.m_AddedEnd = cursorPosition;
				u.m_After = m_State;
				AddUndo(u);

				// Open suggestions again
				m_SuggestionTextBuffer.clear();
				RefreshSuggestionsContent();
			};

			m_SuggestionTree.InsertEntry(entry);
		}
	
		// Select the first available option
		Kargono::EditorUI::TreePath selectPath {};
		selectPath.AddNode(0);
		m_SuggestionTree.m_SelectedEntry = selectPath;
	}

	void TextEditorSpec::EnterCharacter(ImWchar aChar, bool aShift)
	{
		static std::unordered_set<char> s_TriggerCharacters { '.',':','(' };
		static std::unordered_set<char> s_AllowedCharacters { '\"', '_'};

		// Currently only support ascii
		if (aChar > 255)
		{
			return;
		}

		bool isSuggestionsOpen = ImGui::IsPopupOpen("TextEditorSuggestions");

		if (m_SuggestionsWindowEnabled && isSuggestionsOpen && aChar == '\t')
		{
			Kargono::EditorUI::TreeEntry* entry = m_SuggestionTree.GetEntryFromPath(m_SuggestionTree.m_SelectedEntry);
			if (entry && entry->m_OnDoubleLeftClick)
			{
				entry->m_OnDoubleLeftClick(*entry);
			}
			Colorize(GetCursorPosition().m_Line, 3);
			EnsureCursorVisible();
			return;
		}

		EnterCharacterInternal(aChar, aShift);
		if (m_SuggestionsWindowEnabled)
		{
			if (!std::isalpha(aChar) && 
				!std::isdigit(aChar) && 
				!s_TriggerCharacters.contains((char)aChar) &&
				!s_AllowedCharacters.contains((char)aChar))
			{
				if (isSuggestionsOpen)
				{
					m_CloseTextSuggestions = true;
				}
				return;
			}
			else if (!isSuggestionsOpen)
			{
				m_SuggestionTextBuffer.clear();
			}

			if (s_TriggerCharacters.contains((char)aChar))
			{
				m_SuggestionTextBuffer.clear();
				RefreshSuggestionsContent();
			}
			else
			{
				m_SuggestionTextBuffer.push_back((char)aChar);
				RefreshSuggestionsContent();
			}
			
		}
	}

	std::string TextEditorSpec::GetWordUnderCursor() const
	{
		Coordinates c = GetCursorPosition();
		return GetWordAt(c);
	}

	std::string TextEditorSpec::GetWordAt(const Coordinates & aCoords) const
	{
		Coordinates start = FindWordStart(aCoords);
		Coordinates end = FindWordEnd(aCoords);

		std::string r;

		int istart = GetCharacterIndex(start);
		int iend = GetCharacterIndex(end);

		for (int it = istart; it < iend; ++it)
		{
			r.push_back(m_Lines[aCoords.m_Line][it].m_Char);
		}

		return r;
	}

	ImU32 TextEditorSpec::GetGlyphColor(const Glyph & aGlyph) const
	{
		if (!m_ColorizerEnabled)
			return m_Palette[(int)PaletteIndex::Default];
		if (aGlyph.m_Comment)
			return m_Palette[(int)PaletteIndex::Comment];
		if (aGlyph.m_MultiLineComment)
			return m_Palette[(int)PaletteIndex::MultiLineComment];
		ImU32 color = m_Palette[(int)aGlyph.m_ColorIndex];
		if (aGlyph.m_Preprocessor)
		{
			ImU32 ppcolor = m_Palette[(int)PaletteIndex::Preprocessor];
			const int c0 = ((ppcolor & 0xff) + (color & 0xff)) / 2;
			const int c1 = (((ppcolor >> 8) & 0xff) + ((color >> 8) & 0xff)) / 2;
			const int c2 = (((ppcolor >> 16) & 0xff) + ((color >> 16) & 0xff)) / 2;
			const int c3 = (((ppcolor >> 24) & 0xff) + ((color >> 24) & 0xff)) / 2;
			return ImU32(c0 | (c1 << 8) | (c2 << 16) | (c3 << 24));
		}
		return color;
	}

	void TextEditorSpec::HandleKeyboardInputs()
	{
		ImGuiIO& io = ImGui::GetIO();
		bool shift = io.KeyShift;
		bool ctrl = io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;
		bool alt = io.ConfigMacOSXBehaviors ? io.KeyCtrl : io.KeyAlt;

		if (ImGui::IsWindowFocused())
		{
			if (ImGui::IsWindowHovered())
				ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
			//ImGui::CaptureKeyboardFromApp(true);

			io.WantCaptureKeyboard = true;
			io.WantTextInput = true;
			if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
				CloseSuggestions();
			else if (!IsReadOnly() && ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)))
				Undo();
			else if (!IsReadOnly() && !ctrl && !shift && alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Backspace)))
				Undo();
			else if (!IsReadOnly() && ctrl && shift && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)))
				Redo();
			else if (ctrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)))
				Save();
			else if (!ctrl && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
				MoveUp(1, shift);
			else if (!ctrl && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
				MoveDown(1, shift);
			else if (!alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
				MoveLeft(1, shift, ctrl);
			else if (!alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
				MoveRight(1, shift, ctrl);
			else if (!alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_PageUp)))
				MoveUp(GetPageSize() - 4, shift);
			else if (!alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_PageDown)))
				MoveDown(GetPageSize() - 4, shift);
			else if (!alt && ctrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Home)))
				MoveTop(shift);
			else if (ctrl && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_End)))
				MoveBottom(shift);
			else if (!ctrl && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Home)))
				MoveHome(shift);
			else if (!ctrl && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_End)))
				MoveEnd(shift);
			else if (!IsReadOnly() && !ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
				Delete();
			else if (!IsReadOnly() && !ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Backspace)))
				Backspace();
			else if (!ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Insert)))
				m_Overwrite ^= true;
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Insert)))
				Copy();
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)))
				Copy();
			// TODO: STILL NEED TO FIX THIS SHIT
			//else if (alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
			//	ShiftTextUp();
			//else if (alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
			//	ShiftTextDown();
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_D)))
				DuplicateText();
			else if (!IsReadOnly() && !ctrl && shift && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Insert)))
				Paste();
			else if (!IsReadOnly() && ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V)))
				Paste();
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_X)))
				Cut();
			else if (!ctrl && shift && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
				Cut();
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_A)))
				SelectAll();
			else if (!IsReadOnly() && !ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
			{
				EnterCharacter('\n', false);
			}
			else if (!IsReadOnly() && !ctrl && !alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Tab)))
				EnterCharacter('\t', shift);

			if (!IsReadOnly() && !io.InputQueueCharacters.empty())
			{
				for (int i = 0; i < io.InputQueueCharacters.Size; i++)
				{
					ImWchar c = io.InputQueueCharacters[i];
					if (c != 0 && (c == '\n' || c >= 32))
						EnterCharacter(c, shift);
				}
				io.InputQueueCharacters.resize(0);
			}
		}
	}

	void TextEditorSpec::HandleMouseInputs()
	{
		ImGuiIO& io = ImGui::GetIO();
		bool shift = io.KeyShift;
		bool ctrl = io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;
		bool alt = io.ConfigMacOSXBehaviors ? io.KeyCtrl : io.KeyAlt;

		if (ImGui::IsWindowHovered())
		{
			if (!shift && !alt)
			{
				bool click = ImGui::IsMouseClicked(0);
				bool doubleClick = ImGui::IsMouseDoubleClicked(0);
				double t = ImGui::GetTime();
				bool tripleClick = click && !doubleClick && (m_LastClick != -1.0f && (t - m_LastClick) < io.MouseDoubleClickTime);

				/*
				Left mouse button triple click
				*/

				if (tripleClick)
				{
					if (!ctrl)
					{
						m_State.m_CursorPosition = m_InteractiveStart = m_InteractiveEnd = ScreenPosToCoordinates(ImGui::GetMousePos());
						m_SelectionMode = SelectionMode::Line;
						SetSelection(m_InteractiveStart, m_InteractiveEnd, m_SelectionMode);
					}

					m_LastClick = -1.0f;
				}

				/*
				Left mouse button double click
				*/

				else if (doubleClick)
				{
					if (!ctrl)
					{
						m_State.m_CursorPosition = m_InteractiveStart = m_InteractiveEnd = ScreenPosToCoordinates(ImGui::GetMousePos());
						if (m_SelectionMode == SelectionMode::Line)
							m_SelectionMode = SelectionMode::Normal;
						else
							m_SelectionMode = SelectionMode::Word;
						SetSelection(m_InteractiveStart, m_InteractiveEnd, m_SelectionMode);
					}

					m_LastClick = (float)ImGui::GetTime();
				}

				/*
				Left mouse button click
				*/
				else if (click)
				{
					if (m_SuggestionsWindowEnabled && ImGui::IsPopupOpen("TextEditorSuggestions"))
					{
						m_CloseTextSuggestions = true;
					}
					m_State.m_CursorPosition = m_InteractiveStart = m_InteractiveEnd = ScreenPosToCoordinates(ImGui::GetMousePos());
					
					if (ctrl)
						m_SelectionMode = SelectionMode::Word;
					else
						m_SelectionMode = SelectionMode::Normal;
					SetSelection(m_InteractiveStart, m_InteractiveEnd, m_SelectionMode);

					m_LastClick = (float)ImGui::GetTime();
				}
				// Mouse left button dragging (=> update selection)
				else if (ImGui::IsMouseDragging(0) && ImGui::IsMouseDown(0))
				{
					io.WantCaptureMouse = true;
					m_State.m_CursorPosition = m_InteractiveEnd = ScreenPosToCoordinates(ImGui::GetMousePos());
					SetSelection(m_InteractiveStart, m_InteractiveEnd, m_SelectionMode);
				}
			}
			else if (shift)
			{
				bool click = ImGui::IsMouseClicked(0);
				if (click)
				{
					if (m_SuggestionsWindowEnabled && ImGui::IsPopupOpen("TextEditorSuggestions"))
					{
						m_CloseTextSuggestions = true;
					}
					if (HasSelection())
					{
						if (m_State.m_SelectionStart == m_State.m_CursorPosition)
						{
							io.WantCaptureMouse = true;
							m_State.m_CursorPosition = ScreenPosToCoordinates(ImGui::GetMousePos());
							SetSelection(m_State.m_SelectionEnd, m_State.m_CursorPosition, m_SelectionMode);
						}
						else if (m_State.m_SelectionEnd == m_State.m_CursorPosition)
						{
							io.WantCaptureMouse = true;
							m_State.m_CursorPosition = ScreenPosToCoordinates(ImGui::GetMousePos());
							SetSelection(m_State.m_SelectionStart, m_State.m_CursorPosition, m_SelectionMode);
						}
						else
						{
							m_State.m_CursorPosition = m_InteractiveStart = m_InteractiveEnd = ScreenPosToCoordinates(ImGui::GetMousePos());
							if (ctrl)
								m_SelectionMode = SelectionMode::Word;
							else
								m_SelectionMode = SelectionMode::Normal;
							SetSelection(m_InteractiveStart, m_InteractiveEnd, m_SelectionMode);
						}
					}
					else
					{
						io.WantCaptureMouse = true;
						Coordinates oldCursorPosition{ GetCursorPosition() };
						m_State.m_CursorPosition = ScreenPosToCoordinates(ImGui::GetMousePos());
						SetSelection(oldCursorPosition, m_State.m_CursorPosition, m_SelectionMode);
					}

					
				}
			}
		}
	}

	void TextEditorSpec::OnEditorUIRenderInternal()
	{
		/* Compute m_CharAdvance regarding to scaled font size (Ctrl + mouse wheel)*/
		const float fontSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, "#", nullptr, nullptr).x;
		m_CharAdvance = ImVec2(fontSize, ImGui::GetTextLineHeightWithSpacing() * m_LineSpacing);

		/* Update palette with the current alpha from style */
		for (int i = 0; i < (int)PaletteIndex::Max; ++i)
		{
			ImVec4 color = ImGui::ColorConvertU32ToFloat4(m_PaletteBase[i]);
			color.w *= ImGui::GetStyle().Alpha;
			m_Palette[i] = ImGui::ColorConvertFloat4ToU32(color);
		}

		assert(m_LineBuffer.empty());

		ImVec2 contentSize = ImGui::GetWindowContentRegionMax();
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		float longest(m_TextStart);

		if (m_ScrollToTop)
		{
			m_ScrollToTop = false;
			ImGui::SetScrollY(0.f);
		}

		ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
		float scrollX = ImGui::GetScrollX();
		float scrollY = ImGui::GetScrollY();

		int lineNo = (int)floor(scrollY / m_CharAdvance.y);
		int globalLineMax = (int)m_Lines.size();
		int lineMax = std::max(0, std::min((int)m_Lines.size() - 1, lineNo + (int)floor((scrollY + contentSize.y) / m_CharAdvance.y)));

		// Deduce m_TextStart by evaluating m_Lines size (global lineMax) plus two spaces as text width
		char buf[16];
		snprintf(buf, 16, " %d ", globalLineMax);
		m_TextStart = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, buf, nullptr, nullptr).x + m_LeftMargin;

		if (!m_Lines.empty())
		{
			float spaceSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, " ", nullptr, nullptr).x;
			bool drawErrorTooltip = false;
			std::string tooltipMessage[2];
			while (lineNo <= lineMax)
			{
				ImVec2 lineStartScreenPos = ImVec2(cursorScreenPos.x, cursorScreenPos.y + lineNo * m_CharAdvance.y);
				ImVec2 textScreenPos = ImVec2(lineStartScreenPos.x + m_TextStart, lineStartScreenPos.y);

				Line& line = m_Lines[lineNo];
				longest = std::max(m_TextStart + TextDistanceToLineStart(Coordinates(lineNo, GetLineMaxColumn(lineNo))), longest);
				int columnNo = 0;
				Coordinates lineStartCoord(lineNo, 0);
				Coordinates lineEndCoord(lineNo, GetLineMaxColumn(lineNo));

				// Draw selection for the current line
				float sstart = -1.0f;
				float ssend = -1.0f;

				assert(m_State.m_SelectionStart <= m_State.m_SelectionEnd);
				if (m_State.m_SelectionStart <= lineEndCoord)
					sstart = m_State.m_SelectionStart > lineStartCoord ? TextDistanceToLineStart(m_State.m_SelectionStart) : 0.0f;
				if (m_State.m_SelectionEnd > lineStartCoord)
					ssend = TextDistanceToLineStart(m_State.m_SelectionEnd < lineEndCoord ? m_State.m_SelectionEnd : lineEndCoord);

				if (m_State.m_SelectionEnd.m_Line > lineNo)
					ssend += m_CharAdvance.x;

				if (sstart != -1 && ssend != -1 && sstart < ssend)
				{
					ImVec2 vstart(lineStartScreenPos.x + m_TextStart + sstart, lineStartScreenPos.y);
					ImVec2 vend(lineStartScreenPos.x + m_TextStart + ssend, lineStartScreenPos.y + m_CharAdvance.y);
					drawList->AddRectFilled(vstart, vend, m_Palette[(int)PaletteIndex::Selection]);
				}

				// Draw breakpoints
				ImVec2 start = ImVec2(lineStartScreenPos.x + scrollX, lineStartScreenPos.y);

				if (m_Breakpoints.count(lineNo + 1) != 0)
				{
					ImVec2 end = ImVec2(lineStartScreenPos.x + contentSize.x + 2.0f * scrollX, lineStartScreenPos.y + m_CharAdvance.y);
					drawList->AddRectFilled(start, end, m_Palette[(int)PaletteIndex::Breakpoint]);
				}

				// Draw error markers
				auto errorIt = m_ErrorMarkers.find(lineNo + 1);
				if (errorIt != m_ErrorMarkers.end())
				{
					ImVec2 end = ImVec2(lineStartScreenPos.x + contentSize.x + 2.0f * scrollX, lineStartScreenPos.y + m_CharAdvance.y);
					drawList->AddRectFilled(start, end, m_Palette[(int)PaletteIndex::ErrorBackground]);
					float width = 1.0f;
					for (ErrorLocation& location : errorIt->second.m_Locations)
					{
						float textStart = TextDistanceToLineStartWithTab({lineNo, (int)(location.m_Column)});
						float textEnd = TextDistanceToLineStartWithTab({lineNo, (int)(location.m_Column) + (int)(location.m_Length) });
						ImVec2 cstart(textScreenPos.x + textStart, lineStartScreenPos.y + m_CharAdvance.y);
						ImVec2 cend(textScreenPos.x + textEnd, lineStartScreenPos.y + m_CharAdvance.y + width);
						drawList->AddRectFilled(cstart, cend, m_Palette[(int)PaletteIndex::ErrorText]);
					}

					if (ImGui::IsMouseHoveringRect(lineStartScreenPos, end))
					{
						drawErrorTooltip = true;
						tooltipMessage[0] = std::format("Error(s) at line {}:", errorIt->first);
						tooltipMessage[1] = errorIt->second.m_Description;
					}
				}

				// Draw line number (right aligned)
				snprintf(buf, 16, "%d  ", lineNo + 1);

				float lineNoWidth = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, buf, nullptr, nullptr).x;
				drawList->AddText(ImVec2(lineStartScreenPos.x + m_TextStart - lineNoWidth, lineStartScreenPos.y),
					m_Palette[(int)(m_State.m_CursorPosition.m_Line == lineNo &&!HasSelection() ? PaletteIndex::KnownIdentifier : PaletteIndex::LineNumber)], buf);

				if (m_State.m_CursorPosition.m_Line == lineNo)
				{
					bool focused = ImGui::IsWindowFocused();

					// Render the cursor
					if (focused)
					{
						long long timeEnd = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
						unsigned long long elapsed = timeEnd - m_StartTime;
						if (elapsed > 400)
						{
							float width = 1.0f;
							int cindex = GetCharacterIndex(m_State.m_CursorPosition);
							float cx = TextDistanceToLineStart(m_State.m_CursorPosition);

							if (m_Overwrite && cindex < (int)line.size())
							{
								uint8_t c = line[cindex].m_Char;
								if (c == '\t')
								{
									float x = (1.0f + std::floor((1.0f + cx) / (float(m_TabSize) * spaceSize))) * (float(m_TabSize) * spaceSize);
									width = x - cx;
								}
								else
								{
									char buf2[2];
									buf2[0] = line[cindex].m_Char;
									buf2[1] = '\0';
									width = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, buf2).x;
								}
							}
							ImVec2 cstart(textScreenPos.x + cx, lineStartScreenPos.y);
							ImVec2 cend(textScreenPos.x + cx + width, lineStartScreenPos.y + m_CharAdvance.y);
							drawList->AddRectFilled(cstart, cend, m_Palette[(int)PaletteIndex::Cursor]);
							if (elapsed > 800)
								m_StartTime = timeEnd;
						}
					}
				}

				// Render colorized text
				ImU32 prevColor = line.empty() ? m_Palette[(int)PaletteIndex::Default] : GetGlyphColor(line[0]);
				ImVec2 bufferOffset;

				for (int i = 0; i < line.size();)
				{
					Glyph& glyph = line[i];
					ImU32 color = GetGlyphColor(glyph);

					if ((color != prevColor || glyph.m_Char == '\t' || glyph.m_Char == ' ') && !m_LineBuffer.empty())
					{
						const ImVec2 newOffset(textScreenPos.x + bufferOffset.x, textScreenPos.y + bufferOffset.y);
						drawList->AddText(newOffset, prevColor, m_LineBuffer.c_str());
						ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, m_LineBuffer.c_str(), nullptr, nullptr);
						bufferOffset.x += textSize.x;
						m_LineBuffer.clear();
					}
					prevColor = color;

					if (glyph.m_Char == '\t')
					{
						float oldX = bufferOffset.x;
						bufferOffset.x = (1.0f + std::floor((1.0f + bufferOffset.x) / (float(m_TabSize) * spaceSize))) * (float(m_TabSize) * spaceSize);
						++i;

						if (m_ShowWhitespaces)
						{
							const float s = ImGui::GetFontSize();
							const float x1 = textScreenPos.x + oldX + 1.0f;
							const float x2 = textScreenPos.x + bufferOffset.x - 1.0f;
							const float y = textScreenPos.y + bufferOffset.y + s * 0.5f;
							const ImVec2 p1(x1, y);
							const ImVec2 p2(x2, y);
							const ImVec2 p3(x2 - s * 0.2f, y - s * 0.2f);
							const ImVec2 p4(x2 - s * 0.2f, y + s * 0.2f);
							drawList->AddLine(p1, p2, 0x90909090);
							drawList->AddLine(p2, p3, 0x90909090);
							drawList->AddLine(p2, p4, 0x90909090);
						}
					}
					else if (glyph.m_Char == ' ')
					{
						if (m_ShowWhitespaces)
						{
							const float s = ImGui::GetFontSize();
							const float x = textScreenPos.x + bufferOffset.x + spaceSize * 0.5f;
							const float y = textScreenPos.y + bufferOffset.y + s * 0.5f;
							drawList->AddCircleFilled(ImVec2(x, y), 1.5f, 0x80808080, 4);
						}
						bufferOffset.x += spaceSize;
						i++;
					}
					else
					{
						int l = UTF8CharLength(glyph.m_Char);
						while (l-- > 0)
							m_LineBuffer.push_back(line[i++].m_Char);
					}
					++columnNo;
				}

				if (!m_LineBuffer.empty())
				{
					const ImVec2 newOffset(textScreenPos.x + bufferOffset.x, textScreenPos.y + bufferOffset.y);
					drawList->AddText(newOffset, prevColor, m_LineBuffer.c_str());
					m_LineBuffer.clear();
				}

				++lineNo;
			}

			// Draw Suggestions Window
			if (m_SuggestionsWindowEnabled)
			{
				if (m_OpenTextSuggestions)
				{
					ImGui::OpenPopup("TextEditorSuggestions");
					m_OpenTextSuggestions = false;
				}

				if (ImGui::IsPopupOpen("TextEditorSuggestions"))
				{
					ImVec2 lineStartScreenPos = ImVec2(cursorScreenPos.x, cursorScreenPos.y + GetCursorPosition().m_Line * m_CharAdvance.y);
					ImVec2 textScreenPos = ImVec2(lineStartScreenPos.x + m_TextStart, lineStartScreenPos.y);
					float textStart = TextDistanceToLineStartWithTab(GetCursorPosition());
					ImVec2 cursorPosition(textScreenPos.x + textStart, lineStartScreenPos.y + m_CharAdvance.y + 2.0f);
					ImGui::SetNextWindowPos(cursorPosition);
					ImGui::SetNextWindowSize(ImVec2(0, 200.0f));
				}

				if (ImGui::BeginPopup("TextEditorSuggestions", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing))
				{
					Kargono::EditorUI::EditorUIService::BringCurrentWindowToFront();

					ImGui::PopStyleVar();
					Kargono::EditorUI::EditorUIService::Tree(m_SuggestionTree);
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
					if (m_CloseTextSuggestions)
					{
						ImGui::CloseCurrentPopup();
						m_CloseTextSuggestions = false;
						m_SuggestionTextBuffer.clear();
					}
					ImGui::EndPopup();
				}
			}
			// Draw a tooltip on known identifiers/preprocessor symbols
			if (ImGui::IsWindowHovered())
			{
				std::string id = GetWordAt(ScreenPosToCoordinates(ImGui::GetMousePos()));
				if (!id.empty())
				{
					auto it = m_LanguageDefinition.m_Identifiers.find(id);
					if (it != m_LanguageDefinition.m_Identifiers.end() && !it->second.m_Declaration.empty())
					{
						ImGui::SetNextWindowSize({ 400.0f, 0.0f });
						ImGui::PushStyleColor(ImGuiCol_Text, Kargono::EditorUI::EditorUIService::s_HighlightColor1);
						ImGui::BeginTooltip();
						ImGui::TextWrapped(it->second.m_Declaration.c_str());
						ImGui::PopStyleColor();
						ImGui::EndTooltip();
					}
					else if (drawErrorTooltip)
					{
						ImGui::BeginTooltip();
						ImGui::PushStyleColor(ImGuiCol_Text, m_Palette[(int)PaletteIndex::ErrorText]);
						ImGui::TextUnformatted(tooltipMessage[0].c_str());
						ImGui::Separator();
						ImGui::TextUnformatted(tooltipMessage[1].c_str());
						ImGui::PopStyleColor();
						ImGui::EndTooltip();
					}
				}
				else if (drawErrorTooltip)
				{
					ImGui::BeginTooltip();
					ImGui::PushStyleColor(ImGuiCol_Text, m_Palette[(int)PaletteIndex::ErrorText]);
					ImGui::TextUnformatted(tooltipMessage[0].c_str());
					ImGui::Separator();
					ImGui::TextUnformatted(tooltipMessage[1].c_str());
					ImGui::PopStyleColor();
					ImGui::EndTooltip();
				}
			}
		}


		ImGui::Dummy(ImVec2((longest + 2), m_Lines.size() * m_CharAdvance.y));

		if (m_ScrollToCursor)
		{
			EnsureCursorVisible();
			ImGui::SetWindowFocus();
			m_ScrollToCursor = false;
		}
	}

	void TextEditorSpec::OnEditorUIRender(const char* aTitle, const ImVec2& aSize, bool aBorder)
	{
		// Check for errors
		m_WithinRender = true;
		m_TextChanged = false;
		m_CursorPositionChanged = false;

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(m_Palette[(int)PaletteIndex::Background]));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		if (!m_IgnoreImGuiChild)
			ImGui::BeginChild(aTitle, aSize, aBorder, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NoMove);

		if (m_HandleKeyboardInputs)
		{
			HandleKeyboardInputs();
			ImGui::PushAllowKeyboardFocus(true);
		}

		if (m_HandleMouseInputs)
			HandleMouseInputs();

		ColorizeInternal();
		OnEditorUIRenderInternal();

		if (m_HandleKeyboardInputs)
			ImGui::PopAllowKeyboardFocus();

		if (!m_IgnoreImGuiChild)
			ImGui::EndChild();

		ImGui::PopStyleVar();
		ImGui::PopStyleColor();

		m_WithinRender = false;
	}

	void TextEditorSpec::SetText(const std::string & aText)
	{
		m_Lines.clear();
		m_Lines.emplace_back(Line());
		for (char chr : aText)
		{
			if (chr == '\r')
			{
				// ignore the carriage return character
			}
			else if (chr == '\n')
				m_Lines.emplace_back(Line());
			else
			{
				m_Lines.back().emplace_back(Glyph(chr, PaletteIndex::Default));
			}
		}

		//m_TextChanged = true;
		m_ScrollToTop = true;

		// TODO: May have to add back undo buffer here
		//m_UndoBuffer.clear();
		//m_UndoIndex = 0;

		Colorize();
	}

	void TextEditorSpec::SetTextLines(const std::vector<std::string> & aLines)
	{
		m_Lines.clear();

		if (aLines.empty())
		{
			m_Lines.emplace_back(Line());
		}
		else
		{
			m_Lines.resize(aLines.size());

			for (size_t i = 0; i < aLines.size(); ++i)
			{
				const std::string & aLine = aLines[i];

				m_Lines[i].reserve(aLine.size());
				for (size_t j = 0; j < aLine.size(); ++j)
					m_Lines[i].emplace_back(Glyph(aLine[j], PaletteIndex::Default));
			}
		}

		m_TextChanged = true;
		m_ScrollToTop = true;

		// TODO: May have to add back undo buffer here
		//m_UndoBuffer.clear();
		//m_UndoIndex = 0;

		Colorize();
	}

	void TextEditorSpec::EnterCharacterInternal(ImWchar aChar, bool aShift)
	{
		assert(!m_ReadOnly);
		UndoRecord u;

		u.m_Before = m_State;

		if (HasSelection())
		{
			if (aChar == '\t' && m_State.m_SelectionStart.m_Line != m_State.m_SelectionEnd.m_Line)
			{

				Coordinates start = m_State.m_SelectionStart;
				Coordinates end = m_State.m_SelectionEnd;
				Coordinates originalEnd = end;

				if (start > end)
					std::swap(start, end);
				start.m_Column = 0;
				//			end.m_Column = end.m_Line < m_Lines.size() ? m_Lines[end.m_Line].size() : 0;
				if (end.m_Column == 0 && end.m_Line > 0)
					--end.m_Line;
				if (end.m_Line >= (int)m_Lines.size())
					end.m_Line = m_Lines.empty() ? 0 : (int)m_Lines.size() - 1;
				end.m_Column = GetLineMaxColumn(end.m_Line);

				//if (end.m_Column >= GetLineMaxColumn(end.m_Line))
				//	end.m_Column = GetLineMaxColumn(end.m_Line) - 1;

				u.m_RemovedStart = start;
				u.m_RemovedEnd = end;
				u.m_Removed = GetText(start, end);

				bool modified = false;

				for (int i = start.m_Line; i <= end.m_Line; i++)
				{
					Line& line = m_Lines[i];
					if (aShift)
					{
						if (!line.empty())
						{
							if (line.front().m_Char == '\t')
							{
								line.erase(line.begin());
								modified = true;
							}
							else
							{
								for (int j = 0; j < m_TabSize && !line.empty() && line.front().m_Char == ' '; j++)
								{
									line.erase(line.begin());
									modified = true;
								}
							}
						}
					}
					else
					{
						line.insert(line.begin(), Glyph('\t', PaletteIndex::Background));
						modified = true;
					}
				}

				if (modified)
				{
					start = Coordinates(start.m_Line, GetCharacterColumn(start.m_Line, 0));
					Coordinates rangeEnd;
					if (originalEnd.m_Column != 0)
					{
						end = Coordinates(end.m_Line, GetLineMaxColumn(end.m_Line));
						rangeEnd = end;
						u.m_Added = GetText(start, end);
					}
					else
					{
						end = Coordinates(originalEnd.m_Line, 0);
						rangeEnd = Coordinates(end.m_Line - 1, GetLineMaxColumn(end.m_Line - 1));
						u.m_Added = GetText(start, rangeEnd);
					}

					u.m_AddedStart = start;
					u.m_AddedEnd = rangeEnd;
					u.m_After = m_State;

					m_State.m_SelectionStart = start;
					m_State.m_SelectionEnd = end;
					AddUndo(u);

					m_TextChanged = true;

					EnsureCursorVisible();
				}

				return;
			} // c == '\t'
			else
			{
				u.m_Removed = GetSelectedText();
				u.m_RemovedStart = m_State.m_SelectionStart;
				u.m_RemovedEnd = m_State.m_SelectionEnd;
				DeleteSelection();
			}
		} // HasSelection

		Coordinates coord = GetActualCursorCoordinates();
		u.m_AddedStart = coord;

		assert(!m_Lines.empty());

		if (aChar == '\n')
		{
			// Insert new line in text editor
			InsertEmptyLine(coord.m_Line + 1);
			Line& line = m_Lines[coord.m_Line];
			Line& newLine = m_Lines[coord.m_Line + 1];

			if (m_LanguageDefinition.m_AutoIndentation)
				for (size_t it = 0; it < line.size() && isascii(line[it].m_Char) && isblank(line[it].m_Char); ++it)
					newLine.push_back(line[it]);

			const size_t whitespaceSize = newLine.size();
			int cindex = GetCharacterIndex(coord);
			newLine.insert(newLine.end(), line.begin() + cindex, line.end());
			line.erase(line.begin() + cindex, line.begin() + line.size());
			SetCursorPosition(Coordinates(coord.m_Line + 1, GetCharacterColumn(coord.m_Line + 1, (int)whitespaceSize)));
			u.m_Added = (char)aChar;
		}
		else
		{
			char buf[7];
			int e = ImTextCharToUtf8(buf, 7, aChar);
			if (e > 0)
			{
				buf[e] = '\0';
				Line& line = m_Lines[coord.m_Line];
				int cindex = GetCharacterIndex(coord);

				if (m_Overwrite && cindex < (int)line.size())
				{
					int d = UTF8CharLength(line[cindex].m_Char);

					u.m_RemovedStart = m_State.m_CursorPosition;
					u.m_RemovedEnd = Coordinates(coord.m_Line, GetCharacterColumn(coord.m_Line, cindex + d));

					while (d-- > 0 && cindex < (int)line.size())
					{
						u.m_Removed += line[cindex].m_Char;
						line.erase(line.begin() + cindex);
					}
				}

				for (char* p = buf; *p != '\0'; p++, ++cindex)
					line.insert(line.begin() + cindex, Glyph(*p, PaletteIndex::Default));
				u.m_Added = buf;

				SetCursorPosition(Coordinates(coord.m_Line, GetCharacterColumn(coord.m_Line, cindex)));
			}
			else
				return;
		}

		m_TextChanged = true;

		u.m_AddedEnd = GetActualCursorCoordinates();
		u.m_After = m_State;

		AddUndo(u);

		Colorize(coord.m_Line - 1, 3);
		EnsureCursorVisible();
	}

	void TextEditorSpec::Backspace()
	{
		BackspaceInternal();
		if (m_SuggestionsWindowEnabled && ImGui::IsPopupOpen("TextEditorSuggestions"))
		{
			if (m_SuggestionTextBuffer.size() > 1)
			{
				m_SuggestionTextBuffer.pop_back();
				RefreshSuggestionsContent();
			}
			else
			{
				m_CloseTextSuggestions = true;
			}
		}
	}

	void TextEditorSpec::SetReadOnly(bool aValue)
	{
		m_ReadOnly = aValue;
	}

	void TextEditorSpec::SetColorizerEnable(bool aValue)
	{
		m_ColorizerEnabled = aValue;
	}


	void TextEditorSpec::SetCursorPosition(const Coordinates & aPosition)
	{
		if (m_State.m_CursorPosition != aPosition)
		{
			m_State.m_CursorPosition = aPosition;
			m_CursorPositionChanged = true;
			EnsureCursorVisible();
		}
	}

	void TextEditorSpec::SetSelectionStart(const Coordinates & aPosition)
	{
		m_State.m_SelectionStart = SanitizeCoordinates(aPosition);
		if (m_State.m_SelectionStart > m_State.m_SelectionEnd)
			std::swap(m_State.m_SelectionStart, m_State.m_SelectionEnd);
	}

	void TextEditorSpec::SetSelectionEnd(const Coordinates & aPosition)
	{
		m_State.m_SelectionEnd = SanitizeCoordinates(aPosition);
		if (m_State.m_SelectionStart > m_State.m_SelectionEnd)
			std::swap(m_State.m_SelectionStart, m_State.m_SelectionEnd);
	}

	void TextEditorSpec::SetSelection(const Coordinates & aStart, const Coordinates & aEnd, SelectionMode aMode)
	{
		Coordinates oldSelStart = m_State.m_SelectionStart;
		Coordinates oldSelEnd = m_State.m_SelectionEnd;

		m_State.m_SelectionStart = SanitizeCoordinates(aStart);
		m_State.m_SelectionEnd = SanitizeCoordinates(aEnd);
		if (m_State.m_SelectionStart > m_State.m_SelectionEnd)
			std::swap(m_State.m_SelectionStart, m_State.m_SelectionEnd);

		switch (aMode)
		{
		case SelectionMode::Normal:
			break;
		case SelectionMode::Word:
		{
			m_State.m_SelectionStart = FindWordStart(m_State.m_SelectionStart);
			if (!IsOnWordBoundary(m_State.m_SelectionEnd))
				m_State.m_SelectionEnd = FindWordEnd(FindWordStart(m_State.m_SelectionEnd));
			break;
		}
		case SelectionMode::Line:
		{
			const int lineNo = m_State.m_SelectionEnd.m_Line;
			//const size_t lineSize = (size_t)lineNo < m_Lines.size() ? m_Lines[lineNo].size() : 0;
			m_State.m_SelectionStart = Coordinates(m_State.m_SelectionStart.m_Line, 0);
			m_State.m_SelectionEnd = Coordinates(lineNo, GetLineMaxColumn(lineNo));
			break;
		}
		default:
			break;
		}

		if (m_State.m_SelectionStart != oldSelStart ||
			m_State.m_SelectionEnd != oldSelEnd)
			m_CursorPositionChanged = true;
	}

	void EditorUI::TextEditorSpec::ClearSelection()
	{
		SetSelection({}, {}, SelectionMode::Normal);
	}

	void TextEditorSpec::SetTabSize(int aValue)
	{
		m_TabSize = std::max(0, std::min(32, aValue));
	}

	void TextEditorSpec::InsertText(const std::string & aValue)
	{
		InsertText(aValue.c_str());
	}

	void TextEditorSpec::InsertText(const char * aValue)
	{
		if (aValue == nullptr)
			return;

		Coordinates pos = GetActualCursorCoordinates();
		Coordinates start = std::min(pos, m_State.m_SelectionStart);
		int totalLines = pos.m_Line - start.m_Line;

		totalLines += InsertTextAt(pos, aValue);

		SetSelection(pos, pos);
		SetCursorPosition(pos);
		Colorize(start.m_Line - 1, totalLines + 2);
	}

	void TextEditorSpec::DeleteSelection()
	{
		assert(m_State.m_SelectionEnd >= m_State.m_SelectionStart);

		if (m_State.m_SelectionEnd == m_State.m_SelectionStart)
			return;

		DeleteRange(m_State.m_SelectionStart, m_State.m_SelectionEnd);

		SetSelection(m_State.m_SelectionStart, m_State.m_SelectionStart);
		SetCursorPosition(m_State.m_SelectionStart);
		Colorize(m_State.m_SelectionStart.m_Line, 1);
	}

	void TextEditorSpec::MoveUp(int aAmount, bool aSelect)
	{
		if (m_SuggestionsWindowEnabled && ImGui::IsPopupOpen("TextEditorSuggestions"))
		{
			m_SuggestionTree.MoveUp();
			return;
		}

		Coordinates oldPos = m_State.m_CursorPosition;
		m_State.m_CursorPosition.m_Line = std::max(0, m_State.m_CursorPosition.m_Line - aAmount);
		if (oldPos != m_State.m_CursorPosition)
		{
			if (aSelect)
			{
				if (oldPos == m_InteractiveStart)
					m_InteractiveStart = m_State.m_CursorPosition;
				else if (oldPos == m_InteractiveEnd)
					m_InteractiveEnd = m_State.m_CursorPosition;
				else
				{
					m_InteractiveStart = m_State.m_CursorPosition;
					m_InteractiveEnd = oldPos;
				}
			}
			else
				m_InteractiveStart = m_InteractiveEnd = m_State.m_CursorPosition;
			SetSelection(m_InteractiveStart, m_InteractiveEnd);

			EnsureCursorVisible();
		}
	}

	void TextEditorSpec::MoveDown(int aAmount, bool aSelect)
	{
		if (m_SuggestionsWindowEnabled && ImGui::IsPopupOpen("TextEditorSuggestions"))
		{
			m_SuggestionTree.MoveDown();
			return;
		}

		assert(m_State.m_CursorPosition.m_Column >= 0);
		Coordinates oldPos = m_State.m_CursorPosition;
		m_State.m_CursorPosition.m_Line = std::max(0, std::min((int)m_Lines.size() - 1, m_State.m_CursorPosition.m_Line + aAmount));

		if (m_State.m_CursorPosition != oldPos)
		{
			if (aSelect)
			{
				if (oldPos == m_InteractiveEnd)
					m_InteractiveEnd = m_State.m_CursorPosition;
				else if (oldPos == m_InteractiveStart)
					m_InteractiveStart = m_State.m_CursorPosition;
				else
				{
					m_InteractiveStart = oldPos;
					m_InteractiveEnd = m_State.m_CursorPosition;
				}
			}
			else
				m_InteractiveStart = m_InteractiveEnd = m_State.m_CursorPosition;
			SetSelection(m_InteractiveStart, m_InteractiveEnd);

			EnsureCursorVisible();
		}
	}

	static bool IsUTFSequence(char c)
	{
		return (c & 0xC0) == 0x80;
	}

	void TextEditorSpec::MoveLeft(int aAmount, bool aSelect, bool aWordMode)
	{
		if (m_SuggestionsWindowEnabled && ImGui::IsPopupOpen("TextEditorSuggestions"))
		{
			m_CloseTextSuggestions = true;
		}
		if (m_Lines.empty())
			return;

		Coordinates oldPos = m_State.m_CursorPosition;
		m_State.m_CursorPosition = GetActualCursorCoordinates();
		int line = m_State.m_CursorPosition.m_Line;
		int cindex = GetCharacterIndex(m_State.m_CursorPosition);

		while (aAmount-- > 0)
		{
			if (cindex == 0)
			{
				if (line > 0)
				{
					--line;
					if ((int)m_Lines.size() > line)
						cindex = (int)m_Lines[line].size();
					else
						cindex = 0;
				}
			}
			else
			{
				--cindex;
				if (cindex > 0)
				{
					if ((int)m_Lines.size() > line)
					{
						while (cindex > 0 && IsUTFSequence(m_Lines[line][cindex].m_Char))
							--cindex;
					}
				}
			}

			m_State.m_CursorPosition = Coordinates(line, GetCharacterColumn(line, cindex));
			if (aWordMode)
			{
				m_State.m_CursorPosition = FindWordStart(m_State.m_CursorPosition);
				cindex = GetCharacterIndex(m_State.m_CursorPosition);
			}
		}

		m_State.m_CursorPosition = Coordinates(line, GetCharacterColumn(line, cindex));

		assert(m_State.m_CursorPosition.m_Column >= 0);
		if (aSelect)
		{
			if (oldPos == m_InteractiveStart)
				m_InteractiveStart = m_State.m_CursorPosition;
			else if (oldPos == m_InteractiveEnd)
				m_InteractiveEnd = m_State.m_CursorPosition;
			else
			{
				m_InteractiveStart = m_State.m_CursorPosition;
				m_InteractiveEnd = oldPos;
			}
		}
		else
			m_InteractiveStart = m_InteractiveEnd = m_State.m_CursorPosition;
		SetSelection(m_InteractiveStart, m_InteractiveEnd, SelectionMode::Normal);
		EnsureCursorVisible();
	}

	void TextEditorSpec::MoveRight(int aAmount, bool aSelect, bool aWordMode)
	{
		// Reset suggestion window
		if (m_SuggestionsWindowEnabled && ImGui::IsPopupOpen("TextEditorSuggestions"))
		{
			m_CloseTextSuggestions = true;
		}
		// Exit early if text is empty or cursor position is invalid
		Coordinates oldCursorPosition = m_State.m_CursorPosition;
		if (m_Lines.empty() || oldCursorPosition.m_Line >= m_Lines.size())
			return;

		// Update cursor position
		int cindex = GetCharacterIndex(m_State.m_CursorPosition);
		while (aAmount-- > 0)
		{
			int lindex = m_State.m_CursorPosition.m_Line;
			Line& line = m_Lines[lindex];

			if (cindex >= line.size())
			{
				// Handle moving lines
				if (m_State.m_CursorPosition.m_Line < m_Lines.size() - 1)
				{
					// Transition to next line
					m_State.m_CursorPosition.m_Line = std::max(0, std::min((int)m_Lines.size() - 1, m_State.m_CursorPosition.m_Line + 1));
					m_State.m_CursorPosition.m_Column = 0;
				}
				else
				{
					// Handle being at end of document
					return;
				}
			}
			else
			{
				// Handle moving within the line
				cindex += UTF8CharLength(line[cindex].m_Char);
				m_State.m_CursorPosition = Coordinates(lindex, GetCharacterColumn(lindex, cindex));
				if (aWordMode)
				{
					Coordinates nextWord = FindWordEnd(m_State.m_CursorPosition);
					if (nextWord.m_Line > oldCursorPosition.m_Line)
					{
						if (oldCursorPosition.m_Column == GetLineMaxColumn(oldCursorPosition.m_Line))
						{
							// Handle moving position normally
							m_State.m_CursorPosition = nextWord;
						}
						else
						{
							// Handle end of line
							m_State.m_CursorPosition.m_Column = GetLineMaxColumn(oldCursorPosition.m_Line);
						}
					}
					else
					{
						// Handle moving position normally
						m_State.m_CursorPosition = nextWord;
					}
				}
			}
		}

		// Handle selection of text
		if (aSelect)
		{
			if (oldCursorPosition == m_InteractiveEnd)
			{
				m_InteractiveEnd = SanitizeCoordinates(m_State.m_CursorPosition);
				SetCursorPosition(m_InteractiveEnd);
			}
			else if (oldCursorPosition == m_InteractiveStart)
			{
				m_InteractiveStart = m_State.m_CursorPosition;
			}
			else
			{
				m_InteractiveStart = oldCursorPosition;
				m_InteractiveEnd = m_State.m_CursorPosition;
			}
		}
		else
		{
			// Reset selection
			m_InteractiveStart = m_InteractiveEnd = m_State.m_CursorPosition;
		}

		SetSelection(m_InteractiveStart, m_InteractiveEnd, SelectionMode::Normal);

		EnsureCursorVisible();
	}

	void TextEditorSpec::MoveTop(bool aSelect)
	{
		Coordinates oldPos = m_State.m_CursorPosition;
		SetCursorPosition(Coordinates(0, 0));

		if (m_State.m_CursorPosition != oldPos)
		{
			if (aSelect)
			{
				m_InteractiveEnd = oldPos;
				m_InteractiveStart = m_State.m_CursorPosition;
			}
			else
				m_InteractiveStart = m_InteractiveEnd = m_State.m_CursorPosition;
			SetSelection(m_InteractiveStart, m_InteractiveEnd);
		}
	}

	void TextEditorSpec::TextEditorSpec::MoveBottom(bool aSelect)
	{
		Coordinates oldPos = GetCursorPosition();
		Coordinates newPos = Coordinates((int)m_Lines.size() - 1, 0);
		SetCursorPosition(newPos);
		if (aSelect)
		{
			m_InteractiveStart = oldPos;
			m_InteractiveEnd = newPos;
		}
		else
			m_InteractiveStart = m_InteractiveEnd = newPos;
		SetSelection(m_InteractiveStart, m_InteractiveEnd);
	}

	void TextEditorSpec::MoveHome(bool aSelect)
	{
		Coordinates oldPos = m_State.m_CursorPosition;
		SetCursorPosition(Coordinates(m_State.m_CursorPosition.m_Line, 0));

		if (m_State.m_CursorPosition != oldPos)
		{
			if (aSelect)
			{
				if (oldPos == m_InteractiveStart)
					m_InteractiveStart = m_State.m_CursorPosition;
				else if (oldPos == m_InteractiveEnd)
					m_InteractiveEnd = m_State.m_CursorPosition;
				else
				{
					m_InteractiveStart = m_State.m_CursorPosition;
					m_InteractiveEnd = oldPos;
				}
			}
			else
				m_InteractiveStart = m_InteractiveEnd = m_State.m_CursorPosition;
			SetSelection(m_InteractiveStart, m_InteractiveEnd);
		}
	}

	void TextEditorSpec::MoveEnd(bool aSelect)
	{
		Coordinates oldPos = m_State.m_CursorPosition;
		SetCursorPosition(Coordinates(m_State.m_CursorPosition.m_Line, GetLineMaxColumn(oldPos.m_Line)));

		if (m_State.m_CursorPosition != oldPos)
		{
			if (aSelect)
			{
				if (oldPos == m_InteractiveEnd)
					m_InteractiveEnd = m_State.m_CursorPosition;
				else if (oldPos == m_InteractiveStart)
					m_InteractiveStart = m_State.m_CursorPosition;
				else
				{
					m_InteractiveStart = oldPos;
					m_InteractiveEnd = m_State.m_CursorPosition;
				}
			}
			else
				m_InteractiveStart = m_InteractiveEnd = m_State.m_CursorPosition;
			SetSelection(m_InteractiveStart, m_InteractiveEnd);
		}
	}

	void TextEditorSpec::Delete()
	{
		assert(!m_ReadOnly);

		if (m_Lines.empty())
			return;

		UndoRecord u;
		u.m_Before = m_State;

		if (HasSelection())
		{
			u.m_Removed = GetSelectedText();
			u.m_RemovedStart = m_State.m_SelectionStart;
			u.m_RemovedEnd = m_State.m_SelectionEnd;

			DeleteSelection();
		}
		else
		{
			Coordinates pos = GetActualCursorCoordinates();
			SetCursorPosition(pos);
			Line& line = m_Lines[pos.m_Line];

			if (pos.m_Column == GetLineMaxColumn(pos.m_Line))
			{
				if (pos.m_Line == (int)m_Lines.size() - 1)
					return;

				u.m_Removed = '\n';
				u.m_RemovedStart = u.m_RemovedEnd = GetActualCursorCoordinates();
				Advance(u.m_RemovedEnd);

				Line& nextLine = m_Lines[pos.m_Line + 1];
				line.insert(line.end(), nextLine.begin(), nextLine.end());
				RemoveLine(pos.m_Line + 1);
			}
			else
			{
				int cindex = GetCharacterIndex(pos);
				u.m_RemovedStart = u.m_RemovedEnd = GetActualCursorCoordinates();
				u.m_RemovedEnd.m_Column++;
				u.m_Removed = GetText(u.m_RemovedStart, u.m_RemovedEnd);

				int d = UTF8CharLength(line[cindex].m_Char);
				while (d-- > 0 && cindex < (int)line.size())
					line.erase(line.begin() + cindex);
			}

			m_TextChanged = true;

			Colorize(pos.m_Line, 1);
		}

		u.m_After = m_State;
		AddUndo(u);
	}

	void TextEditorSpec::BackspaceInternal()
	{
		assert(!m_ReadOnly);

		if (m_Lines.empty())
			return;

		UndoRecord u;
		u.m_Before = m_State;

		if (HasSelection())
		{
			u.m_Removed = GetSelectedText();
			u.m_RemovedStart = m_State.m_SelectionStart;
			u.m_RemovedEnd = m_State.m_SelectionEnd;

			DeleteSelection();
		}
		else
		{
			Coordinates pos = GetActualCursorCoordinates();
			SetCursorPosition(pos);

			if (m_State.m_CursorPosition.m_Column == 0)
			{
				if (m_State.m_CursorPosition.m_Line == 0)
					return;

				u.m_Removed = '\n';
				u.m_RemovedStart = u.m_RemovedEnd = Coordinates(pos.m_Line - 1, GetLineMaxColumn(pos.m_Line - 1));
				Advance(u.m_RemovedEnd);

				Line& line = m_Lines[m_State.m_CursorPosition.m_Line];
				Line& prevLine = m_Lines[m_State.m_CursorPosition.m_Line - 1];
				int prevSize = GetLineMaxColumn(m_State.m_CursorPosition.m_Line - 1);
				prevLine.insert(prevLine.end(), line.begin(), line.end());

				ErrorMarkers etmp;
				for (auto& [location, marker] : m_ErrorMarkers)
					etmp.insert(ErrorMarkers::value_type(location - 1 == m_State.m_CursorPosition.m_Line ? location - 1 : location, marker));
				m_ErrorMarkers = std::move(etmp);

				RemoveLine(m_State.m_CursorPosition.m_Line);
				--m_State.m_CursorPosition.m_Line;
				m_State.m_CursorPosition.m_Column = prevSize;
			}
			else
			{
				Line& line = m_Lines[m_State.m_CursorPosition.m_Line];
				int cindex = GetCharacterIndex(pos) - 1;
				int cend = cindex + 1;
				while (cindex > 0 && IsUTFSequence(line[cindex].m_Char))
					--cindex;

				u.m_RemovedStart = u.m_RemovedEnd = GetActualCursorCoordinates();
				--u.m_RemovedStart.m_Column;
				if (line[cindex].m_Char == '\t')
				{
					// If a character exists before this tab, check its location
					if (cindex > 0)
					{
						// Get previous character column location
						int previousCharacter = GetCharacterColumn(m_State.m_CursorPosition.m_Line, cindex - 1) + 1;


						if (m_State.m_CursorPosition.m_Column - previousCharacter >= m_TabSize)
						{
							// Move back one tab size if previous character is aligned with tabs
							m_State.m_CursorPosition.m_Column -= m_TabSize;
						}
						else
						{
							// Calculate the tab size based on the previous character's column location
							m_State.m_CursorPosition.m_Column -= m_TabSize - (previousCharacter % m_TabSize);
						}
					}
					else
					{
						// Simply move back one tab size
						m_State.m_CursorPosition.m_Column -= m_TabSize;
					}
				}
					
				else
					--m_State.m_CursorPosition.m_Column;

				while (cindex < line.size() && cend-- > cindex)
				{
					u.m_Removed += line[cindex].m_Char;
					line.erase(line.begin() + cindex);
				}
			}

			m_TextChanged = true;

			EnsureCursorVisible();
			Colorize(m_State.m_CursorPosition.m_Line, 1);
		}

		u.m_After = m_State;
		AddUndo(u);
	}

	void TextEditorSpec::SelectWordUnderCursor()
	{
		Coordinates c = GetCursorPosition();
		SetSelection(FindWordStart(c), FindWordEnd(c));
	}

	void TextEditorSpec::SelectAll()
	{
		SetSelection(Coordinates(0, 0), Coordinates((int)m_Lines.size(), 0));
	}

	bool TextEditorSpec::HasSelection() const
	{
		return m_State.m_SelectionEnd > m_State.m_SelectionStart;
	}

	void TextEditorSpec::SetSaveCallback(std::function<void()> saveCallback)
	{
		m_SaveCallback = saveCallback;
	}

	void TextEditorSpec::CloseSuggestions()
	{
		if (m_SuggestionsWindowEnabled && ImGui::IsPopupOpen("TextEditorSuggestions"))
		{
			m_CloseTextSuggestions = true;
		}
	}

	void EditorUI::TextEditorSpec::ClearUndoBuffer()
	{
		m_UndoBuffer.clear();
		m_UndoIndex = 0;
	}

	void EditorUI::TextEditorSpec::SetUndoBuffer(const UndoBuffer& newBuffer, int undoIndex)
	{
		m_UndoBuffer = newBuffer;
		m_UndoIndex = undoIndex;
	}

	UndoBuffer EditorUI::TextEditorSpec::GetUndoBuffer()
	{
		return m_UndoBuffer;
	}

	int EditorUI::TextEditorSpec::GetUndoIndex()
	{
		return m_UndoIndex;
	}

	void TextEditorSpec::Copy()
	{
		if (HasSelection())
		{
			ImGui::SetClipboardText(GetSelectedText().c_str());
		}
		else
		{
			if (!m_Lines.empty())
			{
				std::string str;
				Line& line = m_Lines[GetActualCursorCoordinates().m_Line];
				for (Glyph& g : line)
					str.push_back(g.m_Char);
				ImGui::SetClipboardText(str.c_str());
			}
		}
	}

	void TextEditorSpec::Cut()
	{
		if (IsReadOnly())
		{
			Copy();
		}
		else
		{
			if (HasSelection())
			{
				UndoRecord u;
				u.m_Before = m_State;
				u.m_Removed = GetSelectedText();
				u.m_RemovedStart = m_State.m_SelectionStart;
				u.m_RemovedEnd = m_State.m_SelectionEnd;

				Copy();
				DeleteSelection();

				u.m_After = m_State;
				AddUndo(u);
			}
		}
	}

	void TextEditorSpec::Paste()
	{
		if (m_SuggestionsWindowEnabled && ImGui::IsPopupOpen("TextEditorSuggestions"))
		{
			m_CloseTextSuggestions = true;
		}
		if (IsReadOnly())
			return;

		const char* clipText = ImGui::GetClipboardText();
		if (clipText != nullptr && strlen(clipText) > 0)
		{
			UndoRecord u;
			u.m_Before = m_State;

			if (HasSelection())
			{
				u.m_Removed = GetSelectedText();
				u.m_RemovedStart = m_State.m_SelectionStart;
				u.m_RemovedEnd = m_State.m_SelectionEnd;
				DeleteSelection();
			}

			u.m_Added = clipText;
			u.m_AddedStart = GetActualCursorCoordinates();

			InsertText(clipText);

			u.m_AddedEnd = GetActualCursorCoordinates();
			u.m_After = m_State;
			AddUndo(u);
		}
	}

	bool TextEditorSpec::CanUndo() const
	{
		return !m_ReadOnly && m_UndoIndex > 0;
	}

	bool TextEditorSpec::CanRedo() const
	{
		return !m_ReadOnly && m_UndoIndex < (int)m_UndoBuffer.size();
	}

	void TextEditorSpec::Undo(int aSteps)
	{
		if (m_SuggestionsWindowEnabled && ImGui::IsPopupOpen("TextEditorSuggestions"))
		{
			m_CloseTextSuggestions = true;
		}
		while (CanUndo() && aSteps-- > 0)
			m_UndoBuffer[--m_UndoIndex].Undo(this);
	}

	void TextEditorSpec::Redo(int aSteps)
	{
		if (m_SuggestionsWindowEnabled && ImGui::IsPopupOpen("TextEditorSuggestions"))
		{
			m_CloseTextSuggestions = true;
		}
		while (CanRedo() && aSteps-- > 0)
			m_UndoBuffer[m_UndoIndex++].Redo(this);
	}

	void TextEditorSpec::Save()
	{
		if (m_SaveCallback)
		{
			m_SaveCallback();
		}
	}

	void EditorUI::TextEditorSpec::DuplicateText()
	{
		// Get the index of the line that should be duplicated
		Coordinates currentCursorCoord = GetCursorPosition();
		int currentLineIndex = currentCursorCoord.m_Line;

		// Check for invalid index
		if (currentLineIndex == k_InvalidIndex)
		{
			KG_WARN("Failed to duplicate line in text editor. Invalid index found!");
			return;
		}

		// Ensure no out of bounds errors occur
		if (currentLineIndex >= m_Lines.size())
		{
			KG_WARN("Failed to duplicate line in text editor. Line index is greater than the line buffer's size!");
			return;
		}

		// Check how many lines we need to move
		bool moveMultipleLines = m_State.m_SelectionStart.m_Line != m_State.m_SelectionEnd.m_Line;

		if (moveMultipleLines)
		{
			// Save initial state of text editor
			UndoRecord u;
			u.m_Before = m_State;

			// Insert all lines into text editor
			Coordinates startCoordinate = { m_State.m_SelectionStart.m_Line, 0 };
			Coordinates endCoordinate = { m_State.m_SelectionEnd.m_Line + 1, 0 };

			// Move cursor down to beginning of new selection
			SetCursorPosition({ currentCursorCoord.m_Line, 0 });

			// Get current line text and duplicate it
			std::string duplicateText{ GetText(startCoordinate, endCoordinate) };
			InsertText(duplicateText);

			// Set selection to encompass newly created lines
			int selectionLineDistance = (endCoordinate.m_Line - startCoordinate.m_Line);
			SetSelection(
				{ startCoordinate.m_Line + selectionLineDistance, 0 }, 
				{ endCoordinate.m_Line + selectionLineDistance, 0 }, 
				SelectionMode::Normal
			);
			u.m_Added = duplicateText;
			u.m_AddedStart = startCoordinate;
			u.m_AddedEnd = endCoordinate;
			u.m_After = m_State;
			AddUndo(u);
		}
		else
		{
			// Get the line that should be duplicated
			UndoRecord u;
			u.m_Before = m_State;
			Line& currentLine = m_Lines[currentLineIndex];
			// Insert a line after the current cursor's line
			InsertLine(currentLine, currentLineIndex);

			// Move cursor down one line
			currentCursorCoord.m_Line++;
			SetCursorPosition(currentCursorCoord);

			u.m_Added = '\n';
			u.m_Added.append(GetCurrentLineText());
			u.m_AddedStart = Coordinates(currentCursorCoord.m_Line - 1, GetLineMaxColumn(currentCursorCoord.m_Line - 1));
			u.m_AddedEnd = Coordinates(currentCursorCoord.m_Line, GetLineMaxColumn(currentCursorCoord.m_Line));

			u.m_After = m_State;
			AddUndo(u);
		}

		
	}

	void EditorUI::TextEditorSpec::ShiftTextUp()
	{
		// Get the index of the line that should be duplicated
		Coordinates currentCursorCoord = GetCursorPosition();
		int currentLineIndex = currentCursorCoord.m_Line;

		// Check for invalid index
		if (currentLineIndex == k_InvalidIndex)
		{
			KG_WARN("Failed to duplicate line in text editor. Invalid index found!");
			return;
		}

		// Ensure no out of bounds errors occur
		if (currentLineIndex >= m_Lines.size())
		{
			KG_WARN("Failed to duplicate line in text editor. Line index is greater than the line buffer's size!");
			return;
		}

		// Ensure no out of bounds errors occur
		if (currentLineIndex == 0)
		{
			return;
		}

		// Get the line that should be duplicated
		UndoRecord u;
		u.m_Before = m_State;
		Line& currentLine = m_Lines[currentLineIndex];
		// Insert line above current location
		InsertLine(currentLine, currentLineIndex - 1);
		// Insert a line after the current cursor's line
		RemoveLine(currentLineIndex + 1);


		// Move cursor up one line
		currentCursorCoord.m_Line--;
		SetCursorPosition(currentCursorCoord);

		u.m_Removed = '\n';
		u.m_Removed.append(GetCurrentLineText());
		u.m_RemovedStart = Coordinates(currentCursorCoord.m_Line + 1, GetLineMaxColumn(currentCursorCoord.m_Line + 1));
		u.m_RemovedEnd = Coordinates(currentCursorCoord.m_Line + 2, GetLineMaxColumn(currentCursorCoord.m_Line + 2));

		u.m_Added = '\n';
		u.m_Added.append(GetCurrentLineText());
		u.m_AddedStart = Coordinates(currentCursorCoord.m_Line - 2, GetLineMaxColumn(currentCursorCoord.m_Line - 2));
		u.m_AddedEnd = Coordinates(currentCursorCoord.m_Line - 1, GetLineMaxColumn(currentCursorCoord.m_Line - 1));

		u.m_After = m_State;
		AddUndo(u);
	}

	void EditorUI::TextEditorSpec::ShiftTextDown()
	{
		// Get the index of the line that should be duplicated
		Coordinates currentCursorCoord = GetCursorPosition();
		int currentLineIndex = currentCursorCoord.m_Line;

		// Check for invalid index
		if (currentLineIndex == k_InvalidIndex)
		{
			KG_WARN("Failed to duplicate line in text editor. Invalid index found!");
			return;
		}

		// Ensure no out of bounds errors occur
		if (currentLineIndex >= m_Lines.size())
		{
			KG_WARN("Failed to duplicate line in text editor. Line index is greater than the line buffer's size!");
			return;
		}

		// Ensure no out of bounds errors occur
		if (currentLineIndex + 1 == m_Lines.size())
		{
			return;
		}

		// Get the line that should be duplicated
		UndoRecord u;
		u.m_Before = m_State;
		Line& currentLine = m_Lines[currentLineIndex];
		// Insert line above current location
		InsertLine(currentLine, currentLineIndex + 2);
		// Insert a line after the current cursor's line
		RemoveLine(currentLineIndex);


		// Move cursor up one line
		currentCursorCoord.m_Line++;
		SetCursorPosition(currentCursorCoord);

		/*u.m_Removed = GetCurrentLineText();
		u.m_RemovedStart = Coordinates(currentCursorCoord.m_Line - 1, GetLineMaxColumn(currentCursorCoord.m_Line - 1));
		u.m_RemovedEnd = Coordinates(currentCursorCoord.m_Line, GetLineMaxColumn(currentCursorCoord.m_Line));

		u.m_Added = GetCurrentLineText();
		u.m_AddedStart = Coordinates(currentCursorCoord.m_Line - 2, GetLineMaxColumn(currentCursorCoord.m_Line - 2));
		u.m_AddedEnd = Coordinates(currentCursorCoord.m_Line - 1, GetLineMaxColumn(currentCursorCoord.m_Line - 1));*/

		u.m_After = m_State;
		//AddUndo(u);
	}

	Palette& TextEditorService::GetDefaultColorPalette()
	{
		static Palette s_Palette
		{
			{
				0xffffffff,	// Default
				0xffffffff,	// Keyword	
				0xff00ff00,	// Number
				0xff7070e0,	// String
				0xff70a0e0, // Char literal
				0xffffffff, // Punctuation
				0xff408080,	// Preprocessor
				0xffaaaaaa, // Identifier
				0xff9bc64d, // Known identifier
				0xffc040a0, // Preproc identifier
				0xffffffff, // Comment (single line)
				0xffffffff, // Comment (multi line)
				0xffffffff, // Background
				0xffffffff, // Cursor
				0x80a06020, // Selection
				0x800020ff, // ErrorMarker
				0x40f08000, // Breakpoint
				0xff707000, // Line number
				0x40000000, // Current line fill
				0x40808080, // Current line fill (inactive)
				0x40a0a0a0, // Current line edge
			}
		};

		return s_Palette;
	}

	Palette& TextEditorService::GetCurrentColorPalette()
	{
		static Palette s_CurrentPalette;

		s_CurrentPalette = 
		{
			{
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_SecondaryTextColor),	// Default
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_HighlightColor3),	// Keyword	
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_HighlightColor2),	// Number
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_HighlightColor2),	// String
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_HighlightColor2), // Char literal
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_SecondaryTextColor), // Punctuation
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_DisabledColor),	// Preprocessor
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_PrimaryTextColor), // Identifier
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_HighlightColor1), // Known identifier
					0xffc040a0, // Preproc identifier
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_DisabledColor), // Comment (single line)
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_DisabledColor), // Comment (multi line)
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_BackgroundColor), // Background
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_PrimaryTextColor), // Cursor
					0x80a06020, // Selection
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_HighlightColor3_UltraThin), // Error Background
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_HighlightColor1), // Breakpoint
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_DisabledColor), // Line number
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_DarkBackgroundColor), // Current line fill
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_PureEmpty), // Current line fill (inactive)
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_PureEmpty), // Current line edge
					ImGui::ColorConvertFloat4ToU32(Kargono::EditorUI::EditorUIService::s_HighlightColor3), // Error Text
			}
		};

		return s_CurrentPalette;
	}




	std::string TextEditorSpec::GetText() const
	{
		int lastLine = (int)m_Lines.size() - 1;
		int lastLineLength = GetLineMaxColumn(lastLine);
		return GetText(Coordinates(), Coordinates(lastLine, lastLineLength));
	}

	std::vector<std::string> TextEditorSpec::GetTextLines() const
	{
		std::vector<std::string> result;

		result.reserve(m_Lines.size());

		for (const Line& line : m_Lines)
		{
			std::string text;

			text.resize(line.size());

			for (size_t i = 0; i < line.size(); ++i)
				text[i] = line[i].m_Char;

			result.emplace_back(std::move(text));
		}

		return result;
	}

	std::string TextEditorSpec::GetSelectedText() const
	{
		return GetText(m_State.m_SelectionStart, m_State.m_SelectionEnd);
	}

	std::string TextEditorSpec::GetCurrentLineText()const
	{
		int lineLength = GetLineMaxColumn(m_State.m_CursorPosition.m_Line);
		return GetText(
			Coordinates(m_State.m_CursorPosition.m_Line, 0),
			Coordinates(m_State.m_CursorPosition.m_Line, lineLength));
	}

	void TextEditorSpec::Colorize(int aFromLine, int aLines)
	{
		int toLine = aLines == -1 ? (int)m_Lines.size() : std::min((int)m_Lines.size(), aFromLine + aLines);
		m_ColorRangeMin = std::min(m_ColorRangeMin, aFromLine);
		m_ColorRangeMax = std::max(m_ColorRangeMax, toLine);
		m_ColorRangeMin = std::max(0, m_ColorRangeMin);
		m_ColorRangeMax = std::max(m_ColorRangeMin, m_ColorRangeMax);
		m_CheckComments = true;
	}

	void TextEditorSpec::ColorizeRange(int aFromLine, int aToLine)
	{
		if (m_Lines.empty() || aFromLine >= aToLine)
			return;

		std::string buffer;
		std::cmatch results;
		std::string id;

		int endLine = std::max(0, std::min((int)m_Lines.size(), aToLine));
		for (int i = aFromLine; i < endLine; ++i)
		{
			Line& line = m_Lines[i];

			if (line.empty())
				continue;

			buffer.resize(line.size());
			for (size_t j = 0; j < line.size(); ++j)
			{
				Glyph& col = line[j];
				buffer[j] = col.m_Char;
				col.m_ColorIndex = PaletteIndex::Default;
			}

			if (m_LanguageDefinition.m_TokenizeScript)
			{
				Kargono::Scripting::ScriptTokenizer tokenizer {};
				std::vector<Kargono::Scripting::ScriptToken> tokens = tokenizer.TokenizeString(buffer);
				PaletteIndex tokenColor = PaletteIndex::Default;
				for (Kargono::Scripting::ScriptToken& token : tokens)
				{
					switch (token.Type)
					{
					case Kargono::Scripting::ScriptTokenType::Semicolon:
					case Kargono::Scripting::ScriptTokenType::NamespaceResolver:
					case Kargono::Scripting::ScriptTokenType::OpenParentheses:
					case Kargono::Scripting::ScriptTokenType::CloseParentheses:
					case Kargono::Scripting::ScriptTokenType::OpenCurlyBrace:
					case Kargono::Scripting::ScriptTokenType::CloseCurlyBrace:
					case Kargono::Scripting::ScriptTokenType::Comma:
					case Kargono::Scripting::ScriptTokenType::AssignmentOperator:
					case Kargono::Scripting::ScriptTokenType::AdditionOperator:
					case Kargono::Scripting::ScriptTokenType::SubtractionOperator:
					case Kargono::Scripting::ScriptTokenType::MultiplicationOperator:
					case Kargono::Scripting::ScriptTokenType::DivisionOperator:
						tokenColor = PaletteIndex::Punctuation;
						break;
					case Kargono::Scripting::ScriptTokenType::Keyword:
						tokenColor = PaletteIndex::Keyword;
						break;
					case Kargono::Scripting::ScriptTokenType::StringLiteral:
						tokenColor = PaletteIndex::String;
						break;
					case Kargono::Scripting::ScriptTokenType::IntegerLiteral:
						tokenColor = PaletteIndex::Number;
						break;
					case Kargono::Scripting::ScriptTokenType::FloatLiteral:
						tokenColor = PaletteIndex::Number;
						break;
					case Kargono::Scripting::ScriptTokenType::CustomLiteral:
						tokenColor = PaletteIndex::Number;
						break;
					case Kargono::Scripting::ScriptTokenType::BooleanLiteral:
						tokenColor = PaletteIndex::String;
						break;
					case Kargono::Scripting::ScriptTokenType::SingleLineComment:
						tokenColor = PaletteIndex::Comment;
						break;
					case Kargono::Scripting::ScriptTokenType::MultiLineComment:
						tokenColor = PaletteIndex::MultiLineComment;
						break;
					case Kargono::Scripting::ScriptTokenType::PrimitiveType:
						if (m_LanguageDefinition.m_Identifiers.contains(token.Value))
						{
							tokenColor = PaletteIndex::KnownIdentifier;
						}
						break;
					case Kargono::Scripting::ScriptTokenType::Identifier:
					{
						if (m_LanguageDefinition.m_Identifiers.contains(token.Value))
						{
							tokenColor = PaletteIndex::KnownIdentifier;
						}
						else if (m_LanguageDefinition.m_PreprocIdentifiers.contains(token.Value))
						{
							tokenColor = PaletteIndex::PreprocIdentifier;
						}
						else
						{
							tokenColor = PaletteIndex::Identifier;
						}
						break;
					}
					default:
						tokenColor = PaletteIndex::Default;
						break;
					}
					for (size_t iteration = 0; iteration < token.Value.size(); ++iteration)
					{
						line[token.Column + iteration].m_ColorIndex = tokenColor;
					}
				}
			}
			else
			{
				const char* bufferBegin = &buffer.front();
				const char* bufferEnd = bufferBegin + buffer.size();

				const char* last = bufferEnd;

				for (const char* first = bufferBegin; first != last; )
				{
					const char* token_begin = nullptr;
					const char* token_end = nullptr;
					PaletteIndex token_color = PaletteIndex::Default;

					bool hasTokenizeResult = false;

					if (m_LanguageDefinition.m_Tokenize != nullptr)
					{
						if (m_LanguageDefinition.m_Tokenize(first, last, token_begin, token_end, token_color))
							hasTokenizeResult = true;
					}

					if (hasTokenizeResult == false)
					{

						for (auto& [regex, paletteIndex] : m_RegexList)
						{
							if (std::regex_search(first, last, results, regex, std::regex_constants::match_continuous))
							{
								hasTokenizeResult = true;

								auto& v = *results.begin();
								token_begin = v.first;
								token_end = v.second;
								token_color = paletteIndex;
								break;
							}
						}
					}

					if (hasTokenizeResult == false)
					{
						first++;
					}
					else
					{
						const size_t token_length = token_end - token_begin;

						if (token_color == PaletteIndex::Identifier)
						{
							id.assign(token_begin, token_end);

							if (!m_LanguageDefinition.m_CaseSensitive)
								std::transform(id.begin(), id.end(), id.begin(), ::toupper);

							if (!line[first - bufferBegin].m_Preprocessor)
							{
								if (m_LanguageDefinition.m_Keywords.count(id) != 0)
									token_color = PaletteIndex::Keyword;
								else if (m_LanguageDefinition.m_Identifiers.count(id) != 0)
									token_color = PaletteIndex::KnownIdentifier;
								else if (m_LanguageDefinition.m_PreprocIdentifiers.count(id) != 0)
									token_color = PaletteIndex::PreprocIdentifier;
							}
							else
							{
								if (m_LanguageDefinition.m_PreprocIdentifiers.count(id) != 0)
									token_color = PaletteIndex::PreprocIdentifier;
							}
						}

						for (size_t j = 0; j < token_length; ++j)
							line[(token_begin - bufferBegin) + j].m_ColorIndex = token_color;

						first = token_end;
					}
				}
			}

		

		}
	}

	void TextEditorSpec::ColorizeInternal()
	{
		if (m_Lines.empty() || !m_ColorizerEnabled)
			return;

		if (m_CheckComments)
		{
			std::size_t endLine = m_Lines.size();
			int endIndex = 0;
			std::size_t commentStartLine = endLine;
			int commentStartIndex = endIndex;
			bool withinString = false;
			bool withinSingleLineComment = false;
			bool withinPreproc = false;
			bool firstChar = true;			// there is no other non-whitespace characters in the line before
			bool concatenate = false;		// '\' on the very end of the line
			int currentLine = 0;
			int currentIndex = 0;
			while (currentLine < endLine || currentIndex < endIndex)
			{
				Line& line = m_Lines[currentLine];

				if (currentIndex == 0 && !concatenate)
				{
					withinSingleLineComment = false;
					withinPreproc = false;
					firstChar = true;
				}

				concatenate = false;

				if (!line.empty())
				{
					Glyph& g = line[currentIndex];
					uint8_t c = g.m_Char;

					if (c != m_LanguageDefinition.m_PreprocChar && !isspace(c))
						firstChar = false;

					if (currentIndex == (int)line.size() - 1 && line[line.size() - 1].m_Char == '\\')
						concatenate = true;

					bool inComment = (commentStartLine < currentLine || (commentStartLine == currentLine && commentStartIndex <= currentIndex));

					if (withinString)
					{
						line[currentIndex].m_MultiLineComment = inComment;

						if (c == '\"')
						{
							if (currentIndex + 1 < (int)line.size() && line[currentIndex + 1].m_Char == '\"')
							{
								currentIndex += 1;
								if (currentIndex < (int)line.size())
									line[currentIndex].m_MultiLineComment = inComment;
							}
							else
								withinString = false;
						}
						else if (c == '\\')
						{
							currentIndex += 1;
							if (currentIndex < (int)line.size())
								line[currentIndex].m_MultiLineComment = inComment;
						}
					}
					else
					{
						if (firstChar && c == m_LanguageDefinition.m_PreprocChar)
							withinPreproc = true;

						if (c == '\"')
						{
							withinString = true;
							line[currentIndex].m_MultiLineComment = inComment;
						}
						else
						{
							auto pred = [](const char& a, const Glyph& b) { return a == b.m_Char; };
							auto from = line.begin() + currentIndex;
							std::string& startStr = m_LanguageDefinition.m_CommentStart;
							std::string& singleStartStr = m_LanguageDefinition.m_SingleLineComment;

							if (singleStartStr.size() > 0 &&
								currentIndex + singleStartStr.size() <= line.size() &&
								equals(singleStartStr.begin(), singleStartStr.end(), from, from + singleStartStr.size(), pred))
							{
								withinSingleLineComment = true;
							}
							else if (!withinSingleLineComment && currentIndex + startStr.size() <= line.size() &&
								equals(startStr.begin(), startStr.end(), from, from + startStr.size(), pred))
							{
								commentStartLine = currentLine;
								commentStartIndex = currentIndex;
							}

							inComment = inComment = (commentStartLine < currentLine || (commentStartLine == currentLine && commentStartIndex <= currentIndex));

							line[currentIndex].m_MultiLineComment = inComment;
							line[currentIndex].m_Comment = withinSingleLineComment;

							std::string& endStr = m_LanguageDefinition.m_CommentEnd;
							if (currentIndex + 1 >= (int)endStr.size() &&
								equals(endStr.begin(), endStr.end(), from + 1 - endStr.size(), from + 1, pred))
							{
								commentStartIndex = endIndex;
								commentStartLine = endLine;
							}
						}
					}
					line[currentIndex].m_Preprocessor = withinPreproc;
					currentIndex += UTF8CharLength(c);
					if (currentIndex >= (int)line.size())
					{
						currentIndex = 0;
						++currentLine;
					}
				}
				else
				{
					currentIndex = 0;
					++currentLine;
				}
			}
			m_CheckComments = false;
		}

		if (m_ColorRangeMin < m_ColorRangeMax)
		{
			const int increment = (m_LanguageDefinition.m_Tokenize == nullptr) ? 10 : 10000;
			const int to = std::min(m_ColorRangeMin + increment, m_ColorRangeMax);
			ColorizeRange(m_ColorRangeMin, to);
			m_ColorRangeMin = to;

			if (m_ColorRangeMax == m_ColorRangeMin)
			{
				m_ColorRangeMin = std::numeric_limits<int>::max();
				m_ColorRangeMax = 0;
			}
			return;
		}
	}

	float TextEditorSpec::TextDistanceToLineStart(const Coordinates& aFrom) const
	{
		const Line& line = m_Lines[aFrom.m_Line];
		float distance = 0.0f;
		float spaceSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, " ", nullptr, nullptr).x;
		int colIndex = GetCharacterIndex(aFrom);
		for (size_t it = 0u; it < line.size() && it < colIndex; )
		{
			if (line[it].m_Char == '\t')
			{
				distance = (1.0f + std::floor((1.0f + distance) / (float(m_TabSize) * spaceSize))) * (float(m_TabSize) * spaceSize);
				++it;
			}
			else
			{
				int d = UTF8CharLength(line[it].m_Char);
				char tempCString[7];
				int i = 0;
				for (; i < 6 && d-- > 0 && it < (int)line.size(); i++, it++)
					tempCString[i] = line[it].m_Char;

				tempCString[i] = '\0';
				distance += ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, tempCString, nullptr, nullptr).x;
			}
		}

		return distance;
	}

	float TextEditorSpec::TextDistanceToLineStartWithTab(const Coordinates& aFrom) const
	{
		const Line& line = m_Lines[aFrom.m_Line];
		float distance = 0.0f;
		float spaceSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, " ", nullptr, nullptr).x;
		for (size_t it = 0u; it < line.size() && it < aFrom.m_Column; )
		{
			if (line[it].m_Char == '\t')
			{
				distance = (1.0f + std::floor((1.0f + distance) / (float(m_TabSize) * spaceSize))) * (float(m_TabSize) * spaceSize);
				++it;
			}
			else
			{
				int d = UTF8CharLength(line[it].m_Char);
				char tempCString[7];
				int i = 0;
				for (; i < 6 && d-- > 0 && it < (int)line.size(); i++, it++)
					tempCString[i] = line[it].m_Char;

				tempCString[i] = '\0';
				distance += ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, tempCString, nullptr, nullptr).x;
			}
		}

		return distance;
	}

	void TextEditorSpec::EnsureCursorVisible()
	{
		if (!m_WithinRender)
		{
			m_ScrollToCursor = true;
			return;
		}

		float scrollX = ImGui::GetScrollX();
		float scrollY = ImGui::GetScrollY();

		float height = ImGui::GetWindowHeight();
		float width = ImGui::GetWindowWidth();

		int top = 1 + (int)ceil(scrollY / m_CharAdvance.y);
		int bottom = (int)ceil((scrollY + height) / m_CharAdvance.y);

		int left = (int)ceil(scrollX / m_CharAdvance.x);
		int right = (int)ceil((scrollX + width) / m_CharAdvance.x);

		Coordinates pos = GetActualCursorCoordinates();
		float len = TextDistanceToLineStart(pos);

		if (pos.m_Line < top)
			ImGui::SetScrollY(std::max(0.0f, (pos.m_Line - 1) * m_CharAdvance.y));
		if (pos.m_Line > bottom - 4)
			ImGui::SetScrollY(std::max(0.0f, (pos.m_Line + 4) * m_CharAdvance.y - height));
		if (len + m_TextStart < left + 4)
			ImGui::SetScrollX(std::max(0.0f, len + m_TextStart - 4));
		if (len + m_TextStart > right - 4)
			ImGui::SetScrollX(std::max(0.0f, len + m_TextStart + 4 - width));
	}

	int TextEditorSpec::GetPageSize() const
	{
		float height = ImGui::GetWindowHeight() - 20.0f;
		return (int)floor(height / m_CharAdvance.y);
	}

	UndoRecord::UndoRecord(
		const std::string& aAdded,
		const Coordinates aAddedStart,
		const Coordinates aAddedEnd,
		const std::string& aRemoved,
		const Coordinates aRemovedStart,
		const Coordinates aRemovedEnd,
		EditorState& aBefore,
		EditorState& aAfter)
		: m_Added(aAdded)
		, m_AddedStart(aAddedStart)
		, m_AddedEnd(aAddedEnd)
		, m_Removed(aRemoved)
		, m_RemovedStart(aRemovedStart)
		, m_RemovedEnd(aRemovedEnd)
		, m_Before(aBefore)
		, m_After(aAfter)
	{
		assert(m_AddedStart <= m_AddedEnd);
		assert(m_RemovedStart <= m_RemovedEnd);
	}

	void UndoRecord::Undo(TextEditorSpec * aEditor)
	{
		if (!m_Added.empty())
		{
			aEditor->DeleteRange(m_AddedStart, m_AddedEnd);
			aEditor->Colorize(m_AddedStart.m_Line - 1, m_AddedEnd.m_Line - m_AddedStart.m_Line + 2);
		}

		if (!m_Removed.empty())
		{
			Coordinates start = m_RemovedStart;
			aEditor->InsertTextAt(start, m_Removed.c_str());
			aEditor->Colorize(m_RemovedStart.m_Line - 1, m_RemovedEnd.m_Line - m_RemovedStart.m_Line + 2);
		}

		aEditor->m_State = m_Before;
		aEditor->EnsureCursorVisible();

	}

	void UndoRecord::Redo(TextEditorSpec * aEditor)
	{
		if (!m_Removed.empty())
		{
			aEditor->DeleteRange(m_RemovedStart, m_RemovedEnd);
			aEditor->Colorize(m_RemovedStart.m_Line - 1, m_RemovedEnd.m_Line - m_RemovedStart.m_Line + 1);
		}

		if (!m_Added.empty())
		{
			Coordinates start = m_AddedStart;
			aEditor->InsertTextAt(start, m_Added.c_str());
			aEditor->Colorize(m_AddedStart.m_Line - 1, m_AddedEnd.m_Line - m_AddedStart.m_Line + 1);
		}

		aEditor->m_State = m_After;
		aEditor->EnsureCursorVisible();
	}

	static bool TokenizeCStyleString(const char * in_begin, const char * in_end, const char *& out_begin, const char *& out_end)
	{
		const char * p = in_begin;

		if (*p == '"')
		{
			p++;

			while (p < in_end)
			{
				// handle end of string
				if (*p == '"')
				{
					out_begin = in_begin;
					out_end = p + 1;
					return true;
				}

				// handle escape character for "
				if (*p == '\\' && p + 1 < in_end && p[1] == '"')
					p++;

				p++;
			}
		}

		return false;
	}

	static bool TokenizeCStyleCharacterLiteral(const char * in_begin, const char * in_end, const char *& out_begin, const char *& out_end)
	{
		const char * p = in_begin;

		if (*p == '\'')
		{
			p++;

			// handle escape characters
			if (p < in_end && *p == '\\')
				p++;

			if (p < in_end)
				p++;

			// handle end of character literal
			if (p < in_end && *p == '\'')
			{
				out_begin = in_begin;
				out_end = p + 1;
				return true;
			}
		}

		return false;
	}

	static bool TokenizeCStyleIdentifier(const char * in_begin, const char * in_end, const char *& out_begin, const char *& out_end)
	{
		const char * p = in_begin;

		if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_')
		{
			p++;

			while ((p < in_end) && ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_'))
				p++;

			out_begin = in_begin;
			out_end = p;
			return true;
		}

		return false;
	}

	static bool TokenizeCStyleNumber(const char * in_begin, const char * in_end, const char *& out_begin, const char *& out_end)
	{
		const char * p = in_begin;

		const bool startsWithNumber = *p >= '0' && *p <= '9';

		if (*p != '+' && *p != '-' && !startsWithNumber)
			return false;

		p++;

		bool hasNumber = startsWithNumber;

		while (p < in_end && (*p >= '0' && *p <= '9'))
		{
			hasNumber = true;

			p++;
		}

		if (hasNumber == false)
			return false;

		bool isFloat = false;
		bool isHex = false;
		bool isBinary = false;

		if (p < in_end)
		{
			if (*p == '.')
			{
				isFloat = true;

				p++;

				while (p < in_end && (*p >= '0' && *p <= '9'))
					p++;
			}
			else if (*p == 'x' || *p == 'X')
			{
				// hex formatted integer of the type 0xef80

				isHex = true;

				p++;

				while (p < in_end && ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F')))
					p++;
			}
			else if (*p == 'b' || *p == 'B')
			{
				// binary formatted integer of the type 0b01011101

				isBinary = true;

				p++;

				while (p < in_end && (*p >= '0' && *p <= '1'))
					p++;
			}
		}

		if (isHex == false && isBinary == false)
		{
			// floating point exponent
			if (p < in_end && (*p == 'e' || *p == 'E'))
			{
				isFloat = true;

				p++;

				if (p < in_end && (*p == '+' || *p == '-'))
					p++;

				bool hasDigits = false;

				while (p < in_end && (*p >= '0' && *p <= '9'))
				{
					hasDigits = true;

					p++;
				}

				if (hasDigits == false)
					return false;
			}

			// single precision floating point type
			if (p < in_end && *p == 'f')
				p++;
		}

		if (isFloat == false)
		{
			// integer size type
			while (p < in_end && (*p == 'u' || *p == 'U' || *p == 'l' || *p == 'L'))
				p++;
		}

		out_begin = in_begin;
		out_end = p;
		return true;
	}

	static bool TokenizeCStylePunctuation(const char * in_begin, const char * in_end, const char *& out_begin, const char *& out_end)
	{
		(void)in_end;

		switch (*in_begin)
		{
		case '[':
		case ']':
		case '{':
		case '}':
		case '!':
		case '%':
		case '^':
		case '&':
		case '*':
		case '(':
		case ')':
		case '-':
		case '+':
		case '=':
		case '~':
		case '|':
		case '<':
		case '>':
		case '?':
		case ':':
		case '/':
		case ';':
		case ',':
		case '.':
			out_begin = in_begin;
			out_end = in_begin + 1;
			return true;
		}

		return false;
	}

	const LanguageDefinition& TextEditorService::GenerateCPlusPlus()
	{
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited)
		{
			static const char* const cppKeywords[] = {
				"alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit", "atomic_noexcept", "auto", "bitand", "bitor", "bool", "break", "case", "catch", "char", "char16_t", "char32_t", "class",
				"compl", "concept", "const", "constexpr", "const_cast", "continue", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "float",
				"for", "friend", "goto", "if", "import", "inline", "int", "long", "module", "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private", "protected", "public",
				"register", "reinterpret_cast", "requires", "return", "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch", "synchronized", "template", "this", "thread_local",
				"throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
			};
			for (const char* const k : cppKeywords)
				langDef.m_Keywords.insert(k);

			static const char* const identifiers[] = {
				"abort", "abs", "acos", "asin", "atan", "atexit", "atof", "atoi", "atol", "ceil", "clock", "cosh", "ctime", "div", "exit", "fabs", "floor", "fmod", "getchar", "getenv", "isalnum", "isalpha", "isdigit", "isgraph",
				"ispunct", "isspace", "isupper", "kbhit", "log10", "log2", "log", "memcmp", "modf", "pow", "printf", "sprintf", "snprintf", "putchar", "putenv", "puts", "rand", "remove", "rename", "sinh", "sqrt", "srand", "strcat", "strcmp", "strerror", "time", "tolower", "toupper",
				"std", "string", "vector", "map", "unordered_map", "set", "unordered_set", "min", "max"
			};
			for (const char* const k : identifiers)
			{
				Identifier id;
				id.m_Declaration = "Built-in function";
				langDef.m_Identifiers.insert(std::make_pair(std::string(k), id));
			}

			langDef.m_Tokenize = [](const char * in_begin, const char * in_end, const char *& out_begin, const char *& out_end, PaletteIndex & paletteIndex) -> bool
			{
				paletteIndex = PaletteIndex::Max;

				while (in_begin < in_end && isascii(*in_begin) && isblank(*in_begin))
					in_begin++;

				if (in_begin == in_end)
				{
					out_begin = in_end;
					out_end = in_end;
					paletteIndex = PaletteIndex::Default;
				}
				else if (TokenizeCStyleString(in_begin, in_end, out_begin, out_end))
					paletteIndex = PaletteIndex::String;
				else if (TokenizeCStyleCharacterLiteral(in_begin, in_end, out_begin, out_end))
					paletteIndex = PaletteIndex::CharLiteral;
				else if (TokenizeCStyleIdentifier(in_begin, in_end, out_begin, out_end))
					paletteIndex = PaletteIndex::Identifier;
				else if (TokenizeCStyleNumber(in_begin, in_end, out_begin, out_end))
					paletteIndex = PaletteIndex::Number;
				else if (TokenizeCStylePunctuation(in_begin, in_end, out_begin, out_end))
					paletteIndex = PaletteIndex::Punctuation;

				return paletteIndex != PaletteIndex::Max;
			};

			langDef.m_CommentStart = "/*";
			langDef.m_CommentEnd = "*/";
			langDef.m_SingleLineComment = "//";

			langDef.m_CaseSensitive = true;
			langDef.m_AutoIndentation = true;

			langDef.m_Name = "C++";

			inited = true;
		}
		return langDef;
	}

	const LanguageDefinition& TextEditorService::GenerateHLSL()
	{
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited)
		{
			static const char* const keywords[] = {
				"AppendStructuredBuffer", "asm", "asm_fragment", "BlendState", "bool", "break", "Buffer", "ByteAddressBuffer", "case", "cbuffer", "centroid", "class", "column_major", "compile", "compile_fragment",
				"CompileShader", "const", "continue", "ComputeShader", "ConsumeStructuredBuffer", "default", "DepthStencilState", "DepthStencilView", "discard", "do", "double", "DomainShader", "dword", "else",
				"export", "extern", "false", "float", "for", "fxgroup", "GeometryShader", "groupshared", "half", "Hullshader", "if", "in", "inline", "inout", "InputPatch", "int", "interface", "line", "lineadj",
				"linear", "LineStream", "matrix", "min16float", "min10float", "min16int", "min12int", "min16uint", "namespace", "nointerpolation", "noperspective", "NULL", "out", "OutputPatch", "packoffset",
				"pass", "pixelfragment", "PixelShader", "point", "PointStream", "precise", "RasterizerState", "RenderTargetView", "return", "register", "row_major", "RWBuffer", "RWByteAddressBuffer", "RWStructuredBuffer",
				"RWTexture1D", "RWTexture1DArray", "RWTexture2D", "RWTexture2DArray", "RWTexture3D", "sample", "sampler", "SamplerState", "SamplerComparisonState", "shared", "snorm", "stateblock", "stateblock_state",
				"static", "string", "struct", "switch", "StructuredBuffer", "tbuffer", "technique", "technique10", "technique11", "texture", "Texture1D", "Texture1DArray", "Texture2D", "Texture2DArray", "Texture2DMS",
				"Texture2DMSArray", "Texture3D", "TextureCube", "TextureCubeArray", "true", "typedef", "triangle", "triangleadj", "TriangleStream", "uint", "uniform", "unorm", "unsigned", "vector", "vertexfragment",
				"VertexShader", "void", "volatile", "while",
				"bool1","bool2","bool3","bool4","double1","double2","double3","double4", "float1", "float2", "float3", "float4", "int1", "int2", "int3", "int4", "in", "out", "inout",
				"uint1", "uint2", "uint3", "uint4", "dword1", "dword2", "dword3", "dword4", "half1", "half2", "half3", "half4",
				"float1x1","float2x1","float3x1","float4x1","float1x2","float2x2","float3x2","float4x2",
				"float1x3","float2x3","float3x3","float4x3","float1x4","float2x4","float3x4","float4x4",
				"half1x1","half2x1","half3x1","half4x1","half1x2","half2x2","half3x2","half4x2",
				"half1x3","half2x3","half3x3","half4x3","half1x4","half2x4","half3x4","half4x4",
			};
			for (const char* const k : keywords)
				langDef.m_Keywords.insert(k);

			static const char* const identifiers[] = {
				"abort", "abs", "acos", "all", "AllMemoryBarrier", "AllMemoryBarrierWithGroupSync", "any", "asdouble", "asfloat", "asin", "asint", "asint", "asuint",
				"asuint", "atan", "atan2", "ceil", "CheckAccessFullyMapped", "clamp", "clip", "cos", "cosh", "countbits", "cross", "D3DCOLORtoUBYTE4", "ddx",
				"ddx_coarse", "ddx_fine", "ddy", "ddy_coarse", "ddy_fine", "degrees", "determinant", "DeviceMemoryBarrier", "DeviceMemoryBarrierWithGroupSync",
				"distance", "dot", "dst", "errorf", "EvaluateAttributeAtCentroid", "EvaluateAttributeAtSample", "EvaluateAttributeSnapped", "exp", "exp2",
				"f16tof32", "f32tof16", "faceforward", "firstbithigh", "firstbitlow", "floor", "fma", "fmod", "frac", "frexp", "fwidth", "GetRenderTargetSampleCount",
				"GetRenderTargetSamplePosition", "GroupMemoryBarrier", "GroupMemoryBarrierWithGroupSync", "InterlockedAdd", "InterlockedAnd", "InterlockedCompareExchange",
				"InterlockedCompareStore", "InterlockedExchange", "InterlockedMax", "InterlockedMin", "InterlockedOr", "InterlockedXor", "isfinite", "isinf", "isnan",
				"ldexp", "length", "lerp", "lit", "log", "log10", "log2", "mad", "max", "min", "modf", "msad4", "mul", "noise", "normalize", "pow", "printf",
				"Process2DQuadTessFactorsAvg", "Process2DQuadTessFactorsMax", "Process2DQuadTessFactorsMin", "ProcessIsolineTessFactors", "ProcessQuadTessFactorsAvg",
				"ProcessQuadTessFactorsMax", "ProcessQuadTessFactorsMin", "ProcessTriTessFactorsAvg", "ProcessTriTessFactorsMax", "ProcessTriTessFactorsMin",
				"radians", "rcp", "reflect", "refract", "reversebits", "round", "rsqrt", "saturate", "sign", "sin", "sincos", "sinh", "smoothstep", "sqrt", "step",
				"tan", "tanh", "tex1D", "tex1D", "tex1Dbias", "tex1Dgrad", "tex1Dlod", "tex1Dproj", "tex2D", "tex2D", "tex2Dbias", "tex2Dgrad", "tex2Dlod", "tex2Dproj",
				"tex3D", "tex3D", "tex3Dbias", "tex3Dgrad", "tex3Dlod", "tex3Dproj", "texCUBE", "texCUBE", "texCUBEbias", "texCUBEgrad", "texCUBElod", "texCUBEproj", "transpose", "trunc"
			};
			for (const char* const k : identifiers)
			{
				Identifier id;
				id.m_Declaration = "Built-in function";
				langDef.m_Identifiers.insert(std::make_pair(std::string(k), id));
			}

			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[ \\t]*#[ \\t]*[a-zA-Z_]+", PaletteIndex::Preprocessor));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("\\'\\\\?[^\\']\\'", PaletteIndex::CharLiteral));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[0-7]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

			langDef.m_CommentStart = "/*";
			langDef.m_CommentEnd = "*/";
			langDef.m_SingleLineComment = "//";

			langDef.m_CaseSensitive = true;
			langDef.m_AutoIndentation = true;

			langDef.m_Name = "HLSL";

			inited = true;
		}
		return langDef;
	}

	const LanguageDefinition& TextEditorService::GenerateGLSL()
	{
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited)
		{
			static const char* const keywords[] = {
				"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long", "register", "restrict", "return", "short",
				"signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary",
				"_Noreturn", "_Static_assert", "_Thread_local"
			};
			for (const char* const k : keywords)
				langDef.m_Keywords.insert(k);

			static const char* const identifiers[] = {
				"abort", "abs", "acos", "asin", "atan", "atexit", "atof", "atoi", "atol", "ceil", "clock", "cosh", "ctime", "div", "exit", "fabs", "floor", "fmod", "getchar", "getenv", "isalnum", "isalpha", "isdigit", "isgraph",
				"ispunct", "isspace", "isupper", "kbhit", "log10", "log2", "log", "memcmp", "modf", "pow", "putchar", "putenv", "puts", "rand", "remove", "rename", "sinh", "sqrt", "srand", "strcat", "strcmp", "strerror", "time", "tolower", "toupper"
			};
			for (const char* const k : identifiers)
			{
				Identifier id;
				id.m_Declaration = "Built-in function";
				langDef.m_Identifiers.insert(std::make_pair(std::string(k), id));
			}

			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[ \\t]*#[ \\t]*[a-zA-Z_]+", PaletteIndex::Preprocessor));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("\\'\\\\?[^\\']\\'", PaletteIndex::CharLiteral));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[0-7]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

			langDef.m_CommentStart = "/*";
			langDef.m_CommentEnd = "*/";
			langDef.m_SingleLineComment = "//";

			langDef.m_CaseSensitive = true;
			langDef.m_AutoIndentation = true;

			langDef.m_Name = "GLSL";

			inited = true;
		}
		return langDef;
	}

	const LanguageDefinition& TextEditorService::GenerateC()
	{
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited)
		{
			static const char* const keywords[] = {
				"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long", "register", "restrict", "return", "short",
				"signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary",
				"_Noreturn", "_Static_assert", "_Thread_local"
			};
			for (const char* const k : keywords)
				langDef.m_Keywords.insert(k);

			static const char* const identifiers[] = {
				"abort", "abs", "acos", "asin", "atan", "atexit", "atof", "atoi", "atol", "ceil", "clock", "cosh", "ctime", "div", "exit", "fabs", "floor", "fmod", "getchar", "getenv", "isalnum", "isalpha", "isdigit", "isgraph",
				"ispunct", "isspace", "isupper", "kbhit", "log10", "log2", "log", "memcmp", "modf", "pow", "putchar", "putenv", "puts", "rand", "remove", "rename", "sinh", "sqrt", "srand", "strcat", "strcmp", "strerror", "time", "tolower", "toupper"
			};
			for (const char* const k : identifiers)
			{
				Identifier id;
				id.m_Declaration = "Built-in function";
				langDef.m_Identifiers.insert(std::make_pair(std::string(k), id));
			}

			langDef.m_Tokenize = [](const char * in_begin, const char * in_end, const char *& out_begin, const char *& out_end, PaletteIndex & paletteIndex) -> bool
			{
				paletteIndex = PaletteIndex::Max;

				while (in_begin < in_end && isascii(*in_begin) && isblank(*in_begin))
					in_begin++;

				if (in_begin == in_end)
				{
					out_begin = in_end;
					out_end = in_end;
					paletteIndex = PaletteIndex::Default;
				}
				else if (TokenizeCStyleString(in_begin, in_end, out_begin, out_end))
					paletteIndex = PaletteIndex::String;
				else if (TokenizeCStyleCharacterLiteral(in_begin, in_end, out_begin, out_end))
					paletteIndex = PaletteIndex::CharLiteral;
				else if (TokenizeCStyleIdentifier(in_begin, in_end, out_begin, out_end))
					paletteIndex = PaletteIndex::Identifier;
				else if (TokenizeCStyleNumber(in_begin, in_end, out_begin, out_end))
					paletteIndex = PaletteIndex::Number;
				else if (TokenizeCStylePunctuation(in_begin, in_end, out_begin, out_end))
					paletteIndex = PaletteIndex::Punctuation;

				return paletteIndex != PaletteIndex::Max;
			};

			langDef.m_CommentStart = "/*";
			langDef.m_CommentEnd = "*/";
			langDef.m_SingleLineComment = "//";

			langDef.m_CaseSensitive = true;
			langDef.m_AutoIndentation = true;

			langDef.m_Name = "C";

			inited = true;
		}
		return langDef;
	}

	const LanguageDefinition& TextEditorService::GenerateSQL()
	{
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited)
		{
			static const char* const keywords[] = {
				"ADD", "EXCEPT", "PERCENT", "ALL", "EXEC", "PLAN", "ALTER", "EXECUTE", "PRECISION", "AND", "EXISTS", "PRIMARY", "ANY", "EXIT", "PRINT", "AS", "FETCH", "PROC", "ASC", "FILE", "PROCEDURE",
				"AUTHORIZATION", "FILLFACTOR", "PUBLIC", "BACKUP", "FOR", "RAISERROR", "BEGIN", "FOREIGN", "READ", "BETWEEN", "FREETEXT", "READTEXT", "BREAK", "FREETEXTTABLE", "RECONFIGURE",
				"BROWSE", "FROM", "REFERENCES", "BULK", "FULL", "REPLICATION", "BY", "FUNCTION", "RESTORE", "CASCADE", "GOTO", "RESTRICT", "CASE", "GRANT", "RETURN", "CHECK", "GROUP", "REVOKE",
				"CHECKPOINT", "HAVING", "RIGHT", "CLOSE", "HOLDLOCK", "ROLLBACK", "CLUSTERED", "IDENTITY", "ROWCOUNT", "COALESCE", "IDENTITY_INSERT", "ROWGUIDCOL", "COLLATE", "IDENTITYCOL", "RULE",
				"COLUMN", "IF", "SAVE", "COMMIT", "IN", "SCHEMA", "COMPUTE", "INDEX", "SELECT", "CONSTRAINT", "INNER", "SESSION_USER", "CONTAINS", "INSERT", "SET", "CONTAINSTABLE", "INTERSECT", "SETUSER",
				"CONTINUE", "INTO", "SHUTDOWN", "CONVERT", "IS", "SOME", "CREATE", "JOIN", "STATISTICS", "CROSS", "KEY", "SYSTEM_USER", "CURRENT", "KILL", "TABLE", "CURRENT_DATE", "LEFT", "TEXTSIZE",
				"CURRENT_TIME", "LIKE", "THEN", "CURRENT_TIMESTAMP", "LINENO", "TO", "CURRENT_USER", "LOAD", "TOP", "CURSOR", "NATIONAL", "TRAN", "DATABASE", "NOCHECK", "TRANSACTION",
				"DBCC", "NONCLUSTERED", "TRIGGER", "DEALLOCATE", "NOT", "TRUNCATE", "DECLARE", "NULL", "TSEQUAL", "DEFAULT", "NULLIF", "UNION", "DELETE", "OF", "UNIQUE", "DENY", "OFF", "UPDATE",
				"DESC", "OFFSETS", "UPDATETEXT", "DISK", "ON", "USE", "DISTINCT", "OPEN", "USER", "DISTRIBUTED", "OPENDATASOURCE", "VALUES", "DOUBLE", "OPENQUERY", "VARYING","DROP", "OPENROWSET", "VIEW",
				"DUMMY", "OPENXML", "WAITFOR", "DUMP", "OPTION", "WHEN", "ELSE", "OR", "WHERE", "END", "ORDER", "WHILE", "ERRLVL", "OUTER", "WITH", "ESCAPE", "OVER", "WRITETEXT"
			};

			for (const char* const k : keywords)
				langDef.m_Keywords.insert(k);

			static const char* const identifiers[] = {
				"ABS",  "ACOS",  "ADD_MONTHS",  "ASCII",  "ASCIISTR",  "ASIN",  "ATAN",  "ATAN2",  "AVG",  "BFILENAME",  "BIN_TO_NUM",  "BITAND",  "CARDINALITY",  "CASE",  "CAST",  "CEIL",
				"CHARTOROWID",  "CHR",  "COALESCE",  "COMPOSE",  "CONCAT",  "CONVERT",  "CORR",  "COS",  "COSH",  "COUNT",  "COVAR_POP",  "COVAR_SAMP",  "CUME_DIST",  "CURRENT_DATE",
				"CURRENT_TIMESTAMP",  "DBTIMEZONE",  "DECODE",  "DECOMPOSE",  "DENSE_RANK",  "DUMP",  "EMPTY_BLOB",  "EMPTY_CLOB",  "EXP",  "EXTRACT",  "FIRST_VALUE",  "FLOOR",  "FROM_TZ",  "GREATEST",
				"GROUP_ID",  "HEXTORAW",  "INITCAP",  "INSTR",  "INSTR2",  "INSTR4",  "INSTRB",  "INSTRC",  "LAG",  "LAST_DAY",  "LAST_VALUE",  "LEAD",  "LEAST",  "LENGTH",  "LENGTH2",  "LENGTH4",
				"LENGTHB",  "LENGTHC",  "LISTAGG",  "LN",  "LNNVL",  "LOCALTIMESTAMP",  "LOG",  "LOWER",  "LPAD",  "LTRIM",  "MAX",  "MEDIAN",  "MIN",  "MOD",  "MONTHS_BETWEEN",  "NANVL",  "NCHR",
				"NEW_TIME",  "NEXT_DAY",  "NTH_VALUE",  "NULLIF",  "NUMTODSINTERVAL",  "NUMTOYMINTERVAL",  "NVL",  "NVL2",  "POWER",  "RANK",  "RAWTOHEX",  "REGEXP_COUNT",  "REGEXP_INSTR",
				"REGEXP_REPLACE",  "REGEXP_SUBSTR",  "REMAINDER",  "REPLACE",  "ROUND",  "ROWNUM",  "RPAD",  "RTRIM",  "SESSIONTIMEZONE",  "SIGN",  "SIN",  "SINH",
				"SOUNDEX",  "SQRT",  "STDDEV",  "SUBSTR",  "SUM",  "SYS_CONTEXT",  "SYSDATE",  "SYSTIMESTAMP",  "TAN",  "TANH",  "TO_CHAR",  "TO_CLOB",  "TO_DATE",  "TO_DSINTERVAL",  "TO_LOB",
				"TO_MULTI_BYTE",  "TO_NCLOB",  "TO_NUMBER",  "TO_SINGLE_BYTE",  "TO_TIMESTAMP",  "TO_TIMESTAMP_TZ",  "TO_YMINTERVAL",  "TRANSLATE",  "TRIM",  "TRUNC", "TZ_OFFSET",  "UID",  "UPPER",
				"USER",  "USERENV",  "VAR_POP",  "VAR_SAMP",  "VARIANCE",  "VSIZE "
			};
			for (const char* const k : identifiers)
			{
				Identifier id;
				id.m_Declaration = "Built-in function";
				langDef.m_Identifiers.insert(std::make_pair(std::string(k), id));
			}

			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("\\\'[^\\\']*\\\'", PaletteIndex::String));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[0-7]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

			langDef.m_CommentStart = "/*";
			langDef.m_CommentEnd = "*/";
			langDef.m_SingleLineComment = "//";

			langDef.m_CaseSensitive = false;
			langDef.m_AutoIndentation = false;

			langDef.m_Name = "SQL";

			inited = true;
		}
		return langDef;
	}

	const LanguageDefinition& TextEditorService::GenerateLua()
	{
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited)
		{
			static const char* const keywords[] = {
				"and", "break", "do", "", "else", "elseif", "end", "false", "for", "function", "if", "in", "", "local", "nil", "not", "or", "repeat", "return", "then", "true", "until", "while"
			};

			for (const char* const k : keywords)
				langDef.m_Keywords.insert(k);

			static const char* const identifiers[] = {
				"assert", "collectgarbage", "dofile", "error", "getmetatable", "ipairs", "loadfile", "load", "loadstring",  "next",  "pairs",  "pcall",  "print",  "rawequal",  "rawlen",  "rawget",  "rawset",
				"select",  "setmetatable",  "tonumber",  "tostring",  "type",  "xpcall",  "_G",  "_VERSION","arshift", "band", "bnot", "bor", "bxor", "btest", "extract", "lrotate", "lshift", "replace",
				"rrotate", "rshift", "create", "resume", "running", "status", "wrap", "yield", "isyieldable", "debug","getuservalue", "gethook", "getinfo", "getlocal", "getregistry", "getmetatable",
				"getupvalue", "upvaluejoin", "upvalueid", "setuservalue", "sethook", "setlocal", "setmetatable", "setupvalue", "traceback", "close", "flush", "input", "lines", "open", "output", "popen",
				"read", "tmpfile", "type", "write", "close", "flush", "lines", "read", "seek", "setvbuf", "write", "__gc", "__tostring", "abs", "acos", "asin", "atan", "ceil", "cos", "deg", "exp", "tointeger",
				"floor", "fmod", "ult", "log", "max", "min", "modf", "rad", "random", "randomseed", "sin", "sqrt", "string", "tan", "type", "atan2", "cosh", "sinh", "tanh",
				"pow", "frexp", "ldexp", "log10", "pi", "huge", "maxinteger", "mininteger", "loadlib", "searchpath", "seeall", "preload", "cpath", "path", "searchers", "loaded", "module", "require", "clock",
				"date", "difftime", "execute", "exit", "getenv", "remove", "rename", "setlocale", "time", "tmpname", "byte", "char", "dump", "find", "format", "gmatch", "gsub", "len", "lower", "match", "rep",
				"reverse", "sub", "upper", "pack", "packsize", "unpack", "concat", "maxn", "insert", "pack", "unpack", "remove", "move", "sort", "offset", "codepoint", "char", "len", "codes", "charpattern",
				"coroutine", "table", "io", "os", "string", "utf8", "bit32", "math", "debug", "package"
			};
			for (const char* const k : identifiers)
			{
				Identifier id;
				id.m_Declaration = "Built-in function";
				langDef.m_Identifiers.insert(std::make_pair(std::string(k), id));
			}

			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", PaletteIndex::String));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("\\\'[^\\\']*\\\'", PaletteIndex::String));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", PaletteIndex::Number));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", PaletteIndex::Number));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", PaletteIndex::Identifier));
			langDef.m_TokenRegexStrings.push_back(std::make_pair<std::string, PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", PaletteIndex::Punctuation));

			langDef.m_CommentStart = "--[[";
			langDef.m_CommentEnd = "]]";
			langDef.m_SingleLineComment = "--";

			langDef.m_CaseSensitive = true;
			langDef.m_AutoIndentation = false;

			langDef.m_Name = "Lua";

			inited = true;
		}
		return langDef;
	}

	static void processMemberType(LanguageDefinition& langDef, Kargono::Ref<Kargono::Scripting::MemberType> member)
	{
		if (Kargono::Scripting::DataMember* dataMember = std::get_if<Kargono::Scripting::DataMember>(&member->Value))
		{
			Identifier id;
			id.m_Declaration = dataMember->Description;
			langDef.m_Identifiers.insert_or_assign(dataMember->Name, id);
			for (auto& [childName, childMember] : dataMember->Members)
			{
				processMemberType(langDef, childMember);
			}
		}
		if (Kargono::Scripting::FunctionNode* function = std::get_if<Kargono::Scripting::FunctionNode>(&member->Value))
		{
			Identifier id;
			id.m_Declaration = function->Description;
			langDef.m_Identifiers.insert_or_assign(function->Name.Value, id);
		}
	}

	void AlignText(std::stringstream& ss, const std::string& text)
	{
		// Format description to fit in tooltip
		std::size_t lineIterator{ 0 };
		std::size_t endWordLength{ 0 };
		constexpr std::size_t k_MaxWordLength{ 5 };
		for (std::size_t charIteration{ 0 }; charIteration < text.size(); charIteration++)
		{
			// Add character to output buffer
			ss << text.at(charIteration);

			// Check if we reached the end of a line
			if (lineIterator % 42 == 0 && lineIterator != 0)
			{
				if (text.at(charIteration) == ' ')
				{
					ss << "\n  ";
					endWordLength = 0;
				}
				else
				{
					endWordLength++;
					// Handle case where max word length has been reached
					if (endWordLength >= k_MaxWordLength)
					{
						// Terminal state
						if (charIteration + 1 >= text.size())
						{
							continue;
						}
						// Peak next character and decide if a hyphen is appropriate
						if (text.at(charIteration + 1) == ' ')
						{
							ss << "\n  ";
						}
						else
						{
							ss << "-\n  ";
						}
						endWordLength = 0;
					}
					// Allow characters to contine being drawn normally
					else
					{
						continue;
					}
				}
			}
			lineIterator++;
		}
	}

	const LanguageDefinition& TextEditorService::GenerateKargonoScript()
	{
		static bool inited = false;
		static LanguageDefinition langDef;
		if (!inited)
		{
			// Lazy loading KGScript language def
			if (!Kargono::Scripting::ScriptCompilerService::s_ActiveLanguageDefinition)
			{
				Kargono::Scripting::ScriptCompilerService::CreateKGScriptLanguageDefinition();
			}

			for (std::string& keyword : Kargono::Scripting::ScriptCompilerService::s_ActiveLanguageDefinition.Keywords)
			{
				langDef.m_Keywords.insert(keyword);
			}

			for (auto& [name, primitiveType] : Kargono::Scripting::ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes)
			{
				Identifier id;
				id.m_Declaration = primitiveType.Description;
				langDef.m_Identifiers.insert(std::make_pair(primitiveType.Name, id));
			}

			for (auto& [funcName, funcNode] : Kargono::Scripting::ScriptCompilerService::s_ActiveLanguageDefinition.FunctionDefinitions)
			{
				Identifier id;
				
				std::stringstream descriptionStringStream;
				if (!funcNode.Description.empty())
				{
					AlignText(descriptionStringStream, funcNode.Description);
				}

				// Create function description
				std::stringstream declarationOutput;
				declarationOutput << "Function Name: ";
				if (funcNode.Namespace)
				{
					declarationOutput << funcNode.Namespace.Value << "::";
				}
				declarationOutput << funcName << "\n";
				if (funcNode.ReturnType.Type == Kargono::Scripting::ScriptTokenType::None)
				{
					declarationOutput << "Return Type: void\n";
				}
				else
				{
					declarationOutput << "Return Type: " << funcNode.ReturnType.Value << "\n";
				}
				declarationOutput << "Parameters:";

				if (funcNode.Parameters.size() > 0)
				{
					declarationOutput << '\n';
					for (Kargono::Scripting::FunctionParameter parameter : funcNode.Parameters)
					{
						KG_ASSERT(parameter.AllTypes.size() > 0);
#if 0
						if (parameter.AllTypes.size() > 1)
						{
							declarationOutput << "  (";
							constexpr std::size_t k_MaxIterations{ 3 };
							for (std::size_t iteration{ 0 }; iteration < parameter.AllTypes.size(); iteration++)
							{
								declarationOutput << parameter.AllTypes.at(iteration).Value;

								if (iteration + 1 >= k_MaxIterations)
								{
									declarationOutput << "/...";
									break;
								}

								// Exit early if we are on the last parameter
								if (iteration + 1 >= parameter.AllTypes.size())
								{
									break;
								}

								declarationOutput << '/';
							}

							declarationOutput << ") " << parameter.Identifier.Value << "\n";
						}
#endif
						declarationOutput << "  " << parameter.AllTypes.at(0).Value << " " << parameter.Identifier.Value << "\n";
				
					}
				}
				else
				{
					declarationOutput << " none\n";
				}
				

				if (!funcNode.Description.empty())
				{
					declarationOutput << "Description: \n  ";
					declarationOutput << descriptionStringStream.str();
				}
				
				id.m_Declaration = declarationOutput.str();
				
				langDef.m_Identifiers.insert_or_assign(funcName, id);

				if (funcNode.Namespace)
				{
					if (Kargono::Scripting::ScriptCompilerService::s_ActiveLanguageDefinition.NamespaceDescriptions.contains(funcNode.Namespace.Value))
					{
						id.m_Declaration = Kargono::Scripting::ScriptCompilerService::s_ActiveLanguageDefinition.NamespaceDescriptions.at(funcNode.Namespace.Value);
					}
					else
					{
						id.m_Declaration = "Built-in Namespace";
					}
					langDef.m_Identifiers.insert_or_assign(funcNode.Namespace.Value, id);
				}
			}

			for (auto& [name, primitiveType] : Kargono::Scripting::ScriptCompilerService::s_ActiveLanguageDefinition.PrimitiveTypes)
			{
				for (auto& [memberName, member] : primitiveType.Members)
				{
					processMemberType(langDef, member);
				}
			}
			langDef.m_TokenizeScript = true;

			langDef.m_Tokenize = [](const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end, PaletteIndex& paletteIndex) -> bool
			{
				paletteIndex = PaletteIndex::Max;

				while (in_begin < in_end && isascii(*in_begin) && isblank(*in_begin))
					in_begin++;

				if (in_begin == in_end)
				{
					out_begin = in_end;
					out_end = in_end;
					paletteIndex = PaletteIndex::Default;
				}
				else if (TokenizeCStyleString(in_begin, in_end, out_begin, out_end))
					paletteIndex = PaletteIndex::String;
				else if (TokenizeCStyleCharacterLiteral(in_begin, in_end, out_begin, out_end))
					paletteIndex = PaletteIndex::CharLiteral;
				else if (TokenizeCStyleIdentifier(in_begin, in_end, out_begin, out_end))
					paletteIndex = PaletteIndex::Identifier;
				else if (TokenizeCStyleNumber(in_begin, in_end, out_begin, out_end))
					paletteIndex = PaletteIndex::Number;
				else if (TokenizeCStylePunctuation(in_begin, in_end, out_begin, out_end))
					paletteIndex = PaletteIndex::Punctuation;

				return paletteIndex != PaletteIndex::Max;
			};

			langDef.m_CommentStart = "/*";
			langDef.m_CommentEnd = "*/";
			langDef.m_SingleLineComment = "//";

			langDef.m_CaseSensitive = true;
			langDef.m_AutoIndentation = true;

			langDef.m_Name = "Kargono Script";

			inited = true;
		}
		return langDef;
	}
}
