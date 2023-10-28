#pragma once

#include "Kargono/Renderer/Texture.h"
#include "Kargono/Renderer/Shader.h"
#include "Kargono/Renderer/EditorCamera.h"

#include "glm/glm.hpp"

#include <string>


namespace Kargono
{
	class Text
	{
	public:
		struct Character {
			Ref<Texture2D>	Texture;  // ID handle of the glyph texture
			glm::ivec2	Size;       // Size of glyph
			glm::ivec2	Bearing;    // Offset from baseline to left/top of glyph
			uint32_t	Advance;    // Offset to advance to next glyph
		};

		void Init();
		void RenderText(const EditorCamera& camera , std::string text, float x, float y, float scale, glm::vec3 color);
	private:
		std::unordered_map<unsigned char, Character> m_Characters {};
	};
}
