#pragma once

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <regex>
#include <functional>
#include "imgui.h"

#include "Kargono/EditorUI/EditorUI.h"
namespace API::EditorUI
{
	class TextEditorSpec;


	//=========================
	// Supporting Text Editor Spec Classes
	//=========================
	enum class PaletteIndex
	{
		Default = 0,
		Keyword,
		Number,
		String,
		CharLiteral,
		Punctuation,
		Preprocessor,
		Identifier,
		KnownIdentifier,
		PreprocIdentifier,
		Comment,
		MultiLineComment,
		Background,
		Cursor,
		Selection,
		ErrorBackground,
		Breakpoint,
		LineNumber,
		CurrentLineFill,
		CurrentLineFillInactive,
		CurrentLineEdge,
		ErrorText,
		Max
	};

	enum class SelectionMode
	{
		Normal = 0,
		Word,
		Line
	};

	static inline int k_InvalidIndex{ -1 };

	struct Breakpoint
	{
		//=========================
		// Struct Fields
		//=========================
		int m_Line;
		bool m_Enabled;
		std::string m_Condition;

		//=========================
		// Constructors/Destructors
		//=========================
		Breakpoint(): m_Line(k_InvalidIndex), m_Enabled(false) {}
	};

	// Represents a character coordinate from the user's point of view,
	// i. e. consider an uniform grid (assuming fixed-width font) on the
	// screen as it is rendered, and each cell has its own coordinate, starting from 0.
	// Tabs are counted as [1..m_TabSize] count empty spaces, depending on
	// how many space is necessary to reach the next tab stop.
	// For example, coordinate (1, 5) represents the character 'B' in a line "\tABC", when m_TabSize = 4,
	// because it is rendered as "    ABC" on the screen.
	struct Coordinates
	{
		//=========================
		// Struct Fields
		//=========================
		int m_Line, m_Column;

		//=========================
		// Constructors/Destructors
		//=========================
		Coordinates() : m_Line(0), m_Column(0) {}
		Coordinates(int line, int column) : m_Line(line), m_Column(column)
		{
			assert(line >= 0);
			if (column < 0)
			{
				column = 0;
			}
		}

		//=========================
		// Operator Overloads
		//=========================
		bool operator ==(const Coordinates& o) const
		{
			return
				m_Line == o.m_Line &&
				m_Column == o.m_Column;
		}

		bool operator !=(const Coordinates& o) const
		{
			return
				m_Line != o.m_Line ||
				m_Column != o.m_Column;
		}

		bool operator <(const Coordinates& o) const
		{
			if (m_Line != o.m_Line)
				return m_Line < o.m_Line;
			return m_Column < o.m_Column;
		}

		bool operator >(const Coordinates& o) const
		{
			if (m_Line != o.m_Line)
				return m_Line > o.m_Line;
			return m_Column > o.m_Column;
		}

		bool operator <=(const Coordinates& o) const
		{
			if (m_Line != o.m_Line)
				return m_Line < o.m_Line;
			return m_Column <= o.m_Column;
		}

		bool operator >=(const Coordinates& o) const
		{
			if (m_Line != o.m_Line)
				return m_Line > o.m_Line;
			return m_Column >= o.m_Column;
		}
	};

	struct Identifier
	{
		Coordinates m_Location;
		std::string m_Declaration;
	};

	struct ErrorLocation
	{
		uint32_t m_Column;
		uint32_t m_Length;
	};

	struct ErrorMarker
	{
		std::string m_Description;
		std::vector<ErrorLocation> m_Locations;
	};

	using RegexList = std::vector<std::pair<std::regex, PaletteIndex>>;

	struct EditorState
	{
		Coordinates m_SelectionStart;
		Coordinates m_SelectionEnd;
		Coordinates m_CursorPosition;
	};

	class UndoRecord
	{
	public:
		//=========================
		// Constructors/Destructors
		//=========================
		UndoRecord() {}
		~UndoRecord() {}

		UndoRecord(
			const std::string& added,
			const Coordinates addedStart,
			const Coordinates addedEnd,

			const std::string& removed,
			const Coordinates removedStart,
			const Coordinates removedEnd,

			EditorState& before,
			EditorState& after);

		//=========================
		// Undo/Redo Action Functions
		//=========================
		void Undo(TextEditorSpec* aEditor);
		void Redo(TextEditorSpec* aEditor);

		//=========================
		// Struct Fields
		//=========================
		std::string m_Added;
		Coordinates m_AddedStart;
		Coordinates m_AddedEnd;

		std::string m_Removed;
		Coordinates m_RemovedStart;
		Coordinates m_RemovedEnd;

		EditorState m_Before;
		EditorState m_After;
	};

	using Identifiers = std::unordered_map<std::string, Identifier>;
	using Keywords = std::unordered_set<std::string>;
	using ErrorMarkers = std::map<int, ErrorMarker>;
	using Breakpoints = std::unordered_set<int>;
	using Palette = std::array<ImU32, (unsigned)PaletteIndex::Max>;
	using UndoBuffer = std::vector<UndoRecord>;

	struct Glyph
	{
		//=========================
		// Struct Fields
		//=========================
		uint8_t m_Char;
		PaletteIndex m_ColorIndex{ PaletteIndex::Default };
		bool m_Comment{true};
		bool m_MultiLineComment{true};
		bool m_Preprocessor{true};

		//=========================
		// Constructors/Destructors
		//=========================
		Glyph(uint8_t character, PaletteIndex colorIndex) : m_Char(character), m_ColorIndex(colorIndex),
			m_Comment(false), m_MultiLineComment(false), m_Preprocessor(false) {}
	};

	using Line = std::vector<Glyph>;
	using Lines = std::vector<Line>;

	using TokenRegexString = std::pair<std::string, PaletteIndex>;
	using TokenRegexStrings = std::vector<TokenRegexString>;
	typedef bool(*TokenizeCallback)(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end, PaletteIndex& paletteIndex);

	struct LanguageDefinition
	{
		//=========================
		// Struct Fields
		//=========================
		std::string m_Name;
		Keywords m_Keywords;
		Identifiers m_Identifiers;
		Identifiers m_PreprocIdentifiers;
		std::string m_CommentStart, m_CommentEnd, m_SingleLineComment;
		char m_PreprocChar;
		bool m_AutoIndentation;
		TokenizeCallback m_Tokenize;
		bool m_TokenizeScript{ false };
		TokenRegexStrings m_TokenRegexStrings;
		bool m_CaseSensitive;

		//=========================
		// Constructors/Destructors
		//=========================
		LanguageDefinition()
			: m_PreprocChar('#'), m_AutoIndentation(true), m_Tokenize(nullptr), m_CaseSensitive(true)
		{
		}
	};

	//=========================
	// Main Text Editor Spec Class
	//=========================
	class TextEditorSpec
	{
	public:
		//=========================
		// Constructors/Destructors
		//=========================
		TextEditorSpec();
		~TextEditorSpec();

		//=========================
		// On Event Functions
		//=========================
		void OnEditorUIRender(const char* aTitle, const ImVec2& aSize = ImVec2(), bool aBorder = false);

		//=========================
		// Interact with Cursor
		//=========================
		void MoveUp(int aAmount = 1, bool aSelect = false);
		void MoveDown(int aAmount = 1, bool aSelect = false);
		void MoveLeft(int aAmount = 1, bool aSelect = false, bool aWordMode = false);
		void MoveRight(int aAmount = 1, bool aSelect = false, bool aWordMode = false);
		void MoveTop(bool aSelect = false);
		void MoveBottom(bool aSelect = false);
		void MoveHome(bool aSelect = false);
		void MoveEnd(bool aSelect = false);
		void SetCursorPosition(const Coordinates& aPosition);

		//=========================
		// Select Text in Editor
		//=========================
		void SetSelectionStart(const Coordinates& aPosition);
		void SetSelectionEnd(const Coordinates& aPosition);
		void SetSelection(const Coordinates& aStart, const Coordinates& aEnd, SelectionMode aMode = SelectionMode::Normal);
		void SelectAll();
		void SelectWordUnderCursor();

		//=========================
		// Modify Text and Clipboard Functions
		//=========================
		void Copy();
		void Cut();
		void Paste();
		void Delete();
		void Undo(int aSteps = 1);
		void Redo(int aSteps = 1);
		void Save();
		void DuplicateLine();
		void ShiftTextUp();
		void ShiftTextDown();
		void InsertText(const std::string& aValue);
		void InsertText(const char* aValue);
		void SetText(const std::string& aText);
		void SetTextLines(const std::vector<std::string>& aLines);

		//=========================
		// Manage Errors and Breakpoints
		//=========================
		void SetErrorMarkers(const ErrorMarkers& aMarkers) { m_ErrorMarkers = aMarkers; }
		void ClearErrorMarkers() { m_ErrorMarkers.clear(); }
		void SetBreakpoints(const Breakpoints& aMarkers) { m_Breakpoints = aMarkers; }

		//=========================
		// Manage Text Editor Internal State
		//=========================
		void SetLanguageDefinition(const LanguageDefinition& aLanguageDef);
		void SetLanguageDefinitionByExtension(const std::string& extension);
		const LanguageDefinition& GetLanguageDefinition() const { return m_LanguageDefinition; }
		void SetPalette(const Palette& aValue);
		void SetTabSize(int aValue);
		void SetSaveCallback(std::function<void()> saveCallback);
		void CloseSuggestions();
		void ClearUndoBuffer();

		//=========================
		// Query Text Editor Internal State
		//=========================
		const Palette& GetPalette() const { return m_PaletteBase; }
		std::string GetText() const;
		std::vector<std::string> GetTextLines() const;
		std::string GetSelectedText() const;
		std::string GetCurrentLineText()const;
		int GetLineCount() const { return (int)m_Lines.size(); }
		Coordinates GetCursorPosition() const { return GetActualCursorCoordinates(); }
		int GetTabSize() const { return m_TabSize; }
		bool HasSelection() const;

		//=========================
		// Manage Text Editor Flags
		//=========================
		void SetColorizerEnable(bool aValue);
		void SetHandleMouseInputs(bool aValue) { m_HandleMouseInputs = aValue;}
		void SetHandleKeyboardInputs (bool aValue){ m_HandleKeyboardInputs = aValue;}
		void SetImGuiChildIgnored    (bool aValue){ m_IgnoreImGuiChild     = aValue;}
		void SetShowWhitespaces(bool aValue) { m_ShowWhitespaces = aValue; }
		void SetReadOnly(bool aValue);
		
		//=========================
		// Query Text Editor Flags
		//=========================
		bool IsOverwrite() const { return m_Overwrite; }
		bool IsReadOnly() const { return m_ReadOnly; }
		bool IsTextChanged() const { return m_TextChanged; }
		bool IsCursorPositionChanged() const { return m_CursorPositionChanged; }
		bool IsColorizerEnabled() const { return m_ColorizerEnabled; }
		bool IsHandleMouseInputsEnabled() const { return m_HandleKeyboardInputs; }
		bool IsHandleKeyboardInputsEnabled() const { return m_HandleKeyboardInputs; }
		bool IsImGuiChildIgnored() const { return m_IgnoreImGuiChild; }
		bool IsShowingWhitespaces() const { return m_ShowWhitespaces; }
		bool CanUndo() const;
		bool CanRedo() const;

	private:
		//=========================
		// Manage Rich Text Color
		//=========================
		void Colorize(int aFromLine = 0, int aCount = -1);
		void ColorizeRange(int aFromLine = 0, int aToLine = 0);
		void ColorizeInternal();
		ImU32 GetGlyphColor(const Glyph& aGlyph) const;

		//=========================
		// Process Rendering
		//=========================
		void OnEditorUIRenderInternal();

		//=========================
		// Process Inputs
		//=========================
		void HandleKeyboardInputs();
		void HandleMouseInputs();
		void Backspace();
		void BackspaceInternal();

		//=========================
		// Query Coordinates/Text
		//=========================
		std::string GetText(const Coordinates& aStart, const Coordinates& aEnd) const;
		float TextDistanceToLineStart(const Coordinates& aFrom) const;
		float TextDistanceToLineStartWithTab(const Coordinates& aFrom) const;
		std::string GetWordUnderCursor() const;
		std::string GetWordAt(const Coordinates& aCoords) const;
		Coordinates GetActualCursorCoordinates() const;
		Coordinates ScreenPosToCoordinates(const ImVec2& aPosition) const;
		bool IsOnWordBoundary(const Coordinates& aAt) const;
		int GetPageSize() const;
		Coordinates FindWordStart(const Coordinates& aFrom) const;
		Coordinates FindWordEnd(const Coordinates& aFrom) const;
		Coordinates FindNextWord(const Coordinates& aFrom) const;
		int GetCharacterIndex(const Coordinates& aCoordinates) const;
		int GetCharacterColumn(int aLine, int aIndex) const;
		int GetLineCharacterCount(int aLine) const;
		int GetLineMaxColumn(int aLine) const;


		//=========================
		// Manage Cursor
		//=========================
		void EnsureCursorVisible();
		void Advance(Coordinates& aCoordinates) const;
		Coordinates SanitizeCoordinates(const Coordinates& aValue) const;

		//=========================
		// Manage Text
		//=========================
		int InsertTextAt(Coordinates& aWhere, const char* aValue);
		void EnterCharacter(ImWchar aChar, bool aShift);
		void EnterCharacterInternal(ImWchar aChar, bool aShift);
		void DeleteSelection();
		void DeleteRange(const Coordinates& aStart, const Coordinates& aEnd);

		//=========================
		// Manage Lines
		//=========================
		void RemoveLine(int aStart, int aEnd);
		void RemoveLine(int aIndex);
		Line& InsertEmptyLine(int aIndex);
		Line& InsertLine(const Line& lineToInsert, int index);

		//=========================
		// Manage Suggestion Window
		//=========================
		void SetSuggestionsWindowEnabled(bool isEnabled);
		void RefreshSuggestionsContent();

		//=========================
		// Conduct Undo
		//=========================
		void AddUndo(UndoRecord& aValue);
		
	private:
		//=========================
		// Internal Data Fields
		//=========================
		// Text Data
		Lines m_Lines;

		// Style Members
		float m_LineSpacing;
		int m_TabSize;
		Palette m_PaletteBase;
		Palette m_Palette;
		float m_TextStart;                   // position (in pixels) where a code line starts relative to the left of the TextEditorSpec.
		int  m_LeftMargin;

		// Flags
		bool m_Overwrite;
		bool m_ReadOnly;
		bool m_WithinRender;
		bool m_ScrollToCursor;
		bool m_ScrollToTop;
		bool m_TextChanged;
		bool m_ColorizerEnabled;
		bool m_HandleKeyboardInputs;
		bool m_HandleMouseInputs;
		bool m_IgnoreImGuiChild;
		bool m_ShowWhitespaces;

		// Other
		bool m_CursorPositionChanged;
		int m_ColorRangeMin;
		int m_ColorRangeMax;
		SelectionMode m_SelectionMode;
		
		bool m_CloseTextSuggestions{false};
		bool m_OpenTextSuggestions{false};
		bool m_SuggestionsWindowEnabled{ false };
		std::string m_SuggestionTextBuffer {};
		Kargono::EditorUI::TreeSpec m_SuggestionTree{};
		std::function<void()> m_SaveCallback;

		LanguageDefinition m_LanguageDefinition;
		RegexList m_RegexList;

		bool m_CheckComments;
		Breakpoints m_Breakpoints;
		ErrorMarkers m_ErrorMarkers;
		ImVec2 m_CharAdvance;
		Coordinates m_InteractiveStart, m_InteractiveEnd;
		std::string m_LineBuffer;
		uint64_t m_StartTime;
		EditorState m_State;
		UndoBuffer m_UndoBuffer;
		int m_UndoIndex;

		float m_LastClick;

	private:
		//=========================
		// Friend Declarations
		//=========================
		friend class UndoRecord;
	};

	//=========================
	// Internal Service Class
	//=========================
	class TextEditorService
	{
	public:
		//=========================
		// Get Invalid State Example
		//=========================
		static Coordinates GetInvalidCoordinate()
		{
			return {k_InvalidIndex, k_InvalidIndex};
		}

		//=========================
		// Generate Different Language Definitions
		//=========================
		static const LanguageDefinition& GenerateCPlusPlus();
		static const LanguageDefinition& GenerateHLSL();
		static const LanguageDefinition& GenerateGLSL();
		static const LanguageDefinition& GenerateC();
		static const LanguageDefinition& GenerateSQL();
		static const LanguageDefinition& GenerateLua();
		static const LanguageDefinition& GenerateKargonoScript();

		//=========================
		// Generate Color Palettes
		//=========================
		static Palette& GetDefaultColorPalette();
		static Palette& GetCurrentColorPalette();
	};

}
