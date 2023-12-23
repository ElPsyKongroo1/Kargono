#pragma once

#include "Kargono/Math/Math.h"

#include <string>
#include <filesystem>

namespace Kargono { class Texture2D; }
namespace Kargono::Assets { class AssetManager; }

//============================================================
// UI (User Interface) Namespace
//============================================================
// This namespace holds all of the classes that enable the usage of the runtime
//		user interface, accessory classes for the Editor user interface, and
//		the font class to enable text rendering.
namespace Kargono::UI
{
	class Font;

	//============================================================
	// Text Engine Class
	//============================================================
	// This class provides static functions to manage the creation of an unmanaged
	//		font (Editor Font) and initialize the rendering pipeline for all text.
	class TextEngine
	{
	public:
		//==============================
		// LifeCycle Functions
		//==============================
		// This function initializes the TextRenderingSpec inside the TextEngine.cpp
		//		file that enables draw call data for the text to be submitted to the
		//		renderer.
		static void Init();

		//==============================
		// Create Unmanaged Font
		//==============================
		// This function allows font to be initialized outside the management of the
		//		AssetManager. This font will persist inside the application until the
		//		application is closed. This is primarily useful for editor only assets.
		static Ref<Font> InstantiateFontEditor(const std::filesystem::path& editorPath);
	};

	//==============================
	// Character Struct
	//==============================
	// This struct holds all of the data necessary to render a specific character including
	//		its image and its dimensions.
	struct Character
	{
		// Reference to the Glyph (Image of Character). This is just an image...
		Ref<Texture2D>	Texture;
		// This is the size of the Glyph in pixels (Width/Height).
		Math::ivec2	Size;
		// Offset from baseline to left/top of glyph
		Math::ivec2	Bearing;
		// Offset to advance to next glyph (Space required to advance to next character position).
		uint32_t Advance;    
	};

	//============================================================
	// Font Class
	//============================================================
	// This class represents an entire font. Think of Times New Roman in Microsoft Word. The font
	//		holds a series of characters which intern holds the image of the character (Glyph) and
	//		its dimensions. The average width and height are used to center text in the RuntimeUI.
	class Font
	{
	public:
		//==============================
		// External Functionality
		//==============================
		// This function submits the rendering data for a line of text to the renderer. The parameters
		//		specify the text, its location on the screen, its scale, and its color.
		void PushTextData(std::string text, Math::vec3 translation, float scale, Math::vec3 color);
		// This function calculates the text size of a full string of text. The resultant value can be
		//		used to center the text on the screen.
		Math::vec2 CalculateTextSize(const std::string& text);

		//==============================
		// Getters/Setters
		//==============================
		std::unordered_map<unsigned char, Character>& GetCharacters() { return m_Characters; }
		float GetAverageWidth() const { return m_AverageWidth; }
		float GetAverageHeight() const { return m_AverageHeight; }
	private:
		// m_Characters holds a map to all of the characters that exist in the current
		//		font. These characters hold all the data necessary to render itself.
		std::unordered_map<unsigned char, Character> m_Characters {};
		// m_AverageWidth and m_AverageHeight hold the average width and height of all the characters
		//		in the font.
		float m_AverageWidth{};
		float m_AverageHeight{};

		friend class Assets::AssetManager;
		friend class TextEngine;
	};
}
