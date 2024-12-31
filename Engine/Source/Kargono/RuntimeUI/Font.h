#pragma once

#include "Kargono/Math/Math.h"

#include <string>
#include <string_view>
#include <filesystem>

namespace Kargono::Rendering { class Texture2D; }

//============================================================
// UI (User Interface) Namespace
//============================================================
// This namespace holds all of the classes that enable the usage of the runtime
//		user interface, accessory classes for the Editor user interface, and
//		the font class to enable text rendering.
namespace Kargono::RuntimeUI
{
	class Font;

	struct TextMetadata
	{
		std::vector<Math::vec2> m_LineSize;
		std::vector<Math::ivec2> m_LineBreaks;
	};


	//============================================================
	// Text Class
	//============================================================
	// This class provides static functions to manage the creation of an unmanaged
	//		font (Editor Font) and initialize the rendering pipeline for all text.

	class FontService
	{
	public:
		//==============================
		// LifeCycle Functions
		//==============================
		// This function initializes the TextRenderingSpec inside the TextEngine.cpp
		//		file that enables draw call data for the text to be submitted to the
		//		renderer.
		static void Init();
		static void Terminate();

		//==============================
		// Create Unmanaged Font
		//==============================
		// This function allows font to be initialized outside the management of the
		//		AssetManager. This font will persist inside the application until the
		//		application is closed. This is primarily useful for editor only assets.
		static Ref<Font> InstantiateEditorFont(const std::filesystem::path& filepath);

		static void SetID(uint32_t id);
	};

	//==============================
	// MSDFCharacter Struct
	//==============================
	// This struct holds all of the data necessary to render a specific character including
	//		its image and its dimensions.
	struct Character
	{
		// This is the size of the Glyph in pixels (Width/Height).
		Math::vec2	Size;
		Math::vec2 TexCoordinateMin;
		Math::vec2 TexCoordinateMax;
		Math::vec2 QuadMin;
		Math::vec2 QuadMax;
		float Advance;
	};

	class Font
	{
	public:
		//==============================
		// Font API
		//==============================
		void PushTextData(std::string_view string, Math::vec3 translation, const glm::vec4& color, float scale = 1.0f, int maxLineWidth = 0);
		Math::vec2 CalculateTextSize(const std::string& text);
		void CalculateTextMetadata(const std::string& text, TextMetadata& metadata, float scale, int maxLineWidth = 0);

	public:
		//==============================
		// Getters/Setters
		//==============================
		std::unordered_map<unsigned char, Character>& GetCharacters() { return m_Characters; }
		float GetLineHeight() const { return m_LineHeight; }
		void SetLineHeight(float height) { m_LineHeight = height; }

	public:
		//==============================
		// Internal Fields
		//==============================
		Ref<Rendering::Texture2D> m_AtlasTexture = nullptr;
		float m_LineHeight {0};
		std::unordered_map<unsigned char, Character> m_Characters{};

	};



}
