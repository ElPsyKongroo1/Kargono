#pragma once

#include "Kargono/Math/Math.h"

#include <string>
#include <filesystem>


namespace Kargono
{
	class Texture2D;
	class Font;

	class TextEngine
	{
	public:
		static void Init();
		static Ref<Font> InstantiateFontEditor(const std::filesystem::path& editorPath);
	};

	class Font
	{
	public:
		struct Character {
			Ref<Texture2D>	Texture;  // ID handle of the glyph texture
			Math::ivec2	Size;       // Size of glyph
			Math::ivec2	Bearing;    // Offset from baseline to left/top of glyph
			uint32_t	Advance;    // Offset to advance to next glyph
		};

		void PushTextData(std::string text, Math::vec3 translation, float scale, Math::vec3 color);
		Math::vec2 CalculateTextSize(const std::string& text);

		std::unordered_map<unsigned char, Character>& GetCharacters() { return m_Characters; }
	private:
		std::unordered_map<unsigned char, Character> m_Characters {};
	public:
		float m_AverageWidth{};
		float m_AverageHeight{};
	};
}
