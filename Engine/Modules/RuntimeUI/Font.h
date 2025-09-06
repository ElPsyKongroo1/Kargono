#pragma once

#include "Kargono/Math/Math.h"
#include "Modules/Rendering/RenderingService.h"

#include <string>
#include <string_view>
#include <filesystem>
#include <vector>

namespace Kargono::Rendering { class Texture2D; }

namespace Kargono::RuntimeUI
{
	class Font;

	struct MultiLineTextDimensions
	{
		std::vector<Math::vec2> m_LineSize;
		std::vector<Math::ivec2> m_LineBreaks;
	};

	class FontContext
	{
	public:
		//==============================
		// LifeCycle Functions
		//==============================
		void Init();
		void Terminate();

	public:
		//==============================
		// Create Unmanaged Font
		//==============================
		Ref<Font> InstantiateEditorFont(const std::filesystem::path& filepath);
		void SetID(uint32_t id);
	public:
		//==============================
		// Public Fields
		//==============================
		Rendering::RendererInputSpec m_TextInputSpec{};
		Ref<std::vector<Math::vec3>> m_Vertices;
		Ref<std::vector<Math::vec2>> m_TexCoordinates;
	};

	class FontService // TODO: REMOVE EWWWWWWW
	{
	public:
		//==============================
		// Getters/Setters
		//==============================
		static FontContext& GetActiveContext() { return s_Font; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline RuntimeUI::FontContext s_Font{};
	};

	struct Character
	{
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
		// Rendering
		//==============================
		void OnRenderMultiLineText(std::string_view string, Math::vec3 translation, const glm::vec4& color, float scale = 1.0f, int maxLineWidth = 0);
		void OnRenderSingleLineText(std::string_view string, Math::vec3 translation, const glm::vec4& color, float scale = 1.0f);
	public:
		//==============================
		// Query Values
		//==============================
		Math::vec2 GetSingleLineTextSize(std::string_view text);
		size_t GetIndexFromMousePosition(std::string_view text, float textStartPoint, float mouseXPosition, float textScalingFactor);
		void GetMultiLineTextMetadata(std::string_view text, MultiLineTextDimensions& metadata, float scale, int maxLineWidth = 0);
	public:
		//==============================
		// Public Fields
		//==============================
		Ref<Rendering::Texture2D> m_AtlasTexture = nullptr;
		float m_LineHeight {0.0f};
		std::unordered_map<unsigned char, Character> m_Characters{};
		float m_Ascender{ 0.0f };
		float m_Descender{ 0.0f };
	};
}
