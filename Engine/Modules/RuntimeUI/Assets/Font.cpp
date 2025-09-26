#include "kgpch.h"

#include "Modules/RuntimeUI/Assets/Font.h"
#include "Modules/Assets/AssetService.h"
#include "Modules/Rendering/RenderingService.h"
#include "Modules/Core/Engine.h"
#include "Modules/Rendering/Shader.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Modules/Rendering/Texture.h"
#include "Kargono/Projects/Project.h"
#include "Modules/Rendering/Components/ShapeComponent.h"
#include "Modules/RuntimeUI/FontContext.h"


namespace Kargono::RuntimeUI
{
	void Font::OnRenderMultiLineText(std::string_view string, Math::vec3 translation, const glm::vec4& color, float scale, int maxLineWidth)
	{
		FontContext& fontContext{ FontService::GetActiveContext() };
		UNREFERENCED_PARAMETER(maxLineWidth);
		// Submit text color to the renderer buffer. The text will now be rendered with this color.
		fontContext.m_TextInputSpec.m_ShapeComponent->m_Texture = m_AtlasTexture;
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(color, 
			Utility::FileSystem::CRCFromString("a_Color"),
			fontContext.m_TextInputSpec.m_Buffer, fontContext.m_TextInputSpec.m_Shader);

		// Initialize the active location where text is being rendered
		double xLocation{ translation.x };
		double yLocation{ translation.y };

		// Iterate through each character in the string
		for (size_t characterIndex = 0; characterIndex < string.size(); characterIndex++)
		{
			// Get the active character from the string
			char character = string[characterIndex];
			Character glyph;

			// Handle specific character cases
			switch (character)
			{
			case '\n':
				// Move to next line
				xLocation = translation.x;
				yLocation -= scale * m_LineHeight;
				continue;
			case '\r':
				// Skip carriage return
				continue;
			case '\t':
				// Tab character
				character = ' ';
				break;
			}

			// Ensure the character exists in the font atlas
			if (m_Characters.contains(character))
			{
				glyph = m_Characters.at(character);
			}
			else
			{
				// TODO: Handle this case better
				glyph = m_Characters.at('?');
			}

			// Coordinates of the glyph in the texture atlas
			glm::vec2 texCoordMin(glyph.m_TexCoordinateMin);
			glm::vec2 texCoordMax(glyph.m_TexCoordinateMax);

			// Minimum and maximum bounds of the quad to be rendered 
			glm::vec2 quadMin(glyph.m_QuadMin);
			glm::vec2 quadMax(glyph.m_QuadMax);

			// Scale the rendering size of the glyph
			quadMin *= scale;
			quadMax *= scale;

			// Adjust the quad location based on the current location
			quadMin += glm::vec2(xLocation, yLocation);
			quadMax += glm::vec2(xLocation, yLocation);

			// Adjust the texture coordinates based on the texture atlas size
			float texelWidth = 1.0f / m_AtlasTexture->GetWidth();
			float texelHeight = 1.0f / m_AtlasTexture->GetHeight();
			texCoordMin *= glm::vec2(texelWidth, texelHeight);
			texCoordMax *= glm::vec2(texelWidth, texelHeight);

			// Submit the quad location data to the renderer
			fontContext.m_Vertices->clear();
			fontContext.m_Vertices->push_back({ quadMin.x, quadMax.y, translation.z });								// 0, 1
			fontContext.m_Vertices->push_back({ quadMin, translation.z });											// 0, 0
			fontContext.m_Vertices->push_back({ quadMax.x, quadMin.y, translation.z });								// 1, 0
			fontContext.m_Vertices->push_back({ quadMin.x, quadMax.y, translation.z });								// 0, 1
			fontContext.m_Vertices->push_back({ quadMax.x, quadMin.y, translation.z });								// 1, 0
			fontContext.m_Vertices->push_back({ quadMax, translation.z });											// 1, 1
			fontContext.m_TextInputSpec.m_ShapeComponent->m_Vertices = fontContext.m_Vertices;

			// Submit the texture coordinates data to the renderer
			fontContext.m_TexCoordinates->clear();
			fontContext.m_TexCoordinates->push_back({ texCoordMin.x, texCoordMax.y });			// 0, 1
			fontContext.m_TexCoordinates->push_back(texCoordMin);								// 0, 0
			fontContext.m_TexCoordinates->push_back({ texCoordMax.x, texCoordMin.y });			// 1, 0
			fontContext.m_TexCoordinates->push_back({ texCoordMin.x, texCoordMax.y });			// 0, 1
			fontContext.m_TexCoordinates->push_back({ texCoordMax.x, texCoordMin.y });			// 1, 0
			fontContext.m_TexCoordinates->push_back(texCoordMax);				
			fontContext.m_TextInputSpec.m_ShapeComponent->m_TextureCoordinates = fontContext.m_TexCoordinates;

			// TODO: Submit multiple glyphs at once for CPU optimization
			// Submit the glyph data to the renderer
			Rendering::RenderingService::SubmitDataToRenderer(fontContext.m_TextInputSpec);

			// Shift the location to the next character
			xLocation += scale * glyph.m_Advance;
		}
	}

	void Font::OnRenderSingleLineText(std::string_view string, Math::vec3 translation, const glm::vec4& color, float scale)
	{
		FontContext& fontContext{ FontService::GetActiveContext() };

		// Submit text color to the renderer buffer. The text will now be rendered with this color.
		fontContext.m_TextInputSpec.m_ShapeComponent->m_Texture = m_AtlasTexture;
		Rendering::Shader::SetDataAtInputLocation<Math::vec4>(color, 
			Utility::FileSystem::CRCFromString("a_Color"),
			fontContext.m_TextInputSpec.m_Buffer, fontContext.m_TextInputSpec.m_Shader);

		// Initialize the active location where text is being rendered
		double xLocation{ translation.x };
		double yLocation{ translation.y };

		// Iterate through each character in the string
		for (size_t characterIndex = 0; characterIndex < string.size(); characterIndex++)
		{
			// Get the active character from the string
			char character = string[characterIndex];
			Character glyph;

			// Handle specific character cases
			switch (character)
			{
			case '\n':
				// Skip newline
				continue;
			case '\r':
				// Skip carriage return
				continue;
			case '\t':
				// Tab character
				character = ' ';
				break;
			}

			// Ensure the character exists in the font atlas
			if (m_Characters.contains(character))
			{
				glyph = m_Characters.at(character);
			}
			else
			{
				// TODO: Handle this case better
				glyph = m_Characters.at('?');
			}

			// Coordinates of the glyph in the texture atlas
			glm::vec2 texCoordMin(glyph.m_TexCoordinateMin);
			glm::vec2 texCoordMax(glyph.m_TexCoordinateMax);

			// Minimum and maximum bounds of the quad to be rendered 
			glm::vec2 quadMin(glyph.m_QuadMin);
			glm::vec2 quadMax(glyph.m_QuadMax);

			// Scale the rendering size of the glyph
			quadMin *= scale;
			quadMax *= scale;

			// Adjust the quad location based on the current location
			quadMin += glm::vec2(xLocation, yLocation);
			quadMax += glm::vec2(xLocation, yLocation);

			// Adjust the texture coordinates based on the texture atlas size
			float texelWidth = 1.0f / m_AtlasTexture->GetWidth();
			float texelHeight = 1.0f / m_AtlasTexture->GetHeight();
			texCoordMin *= glm::vec2(texelWidth, texelHeight);
			texCoordMax *= glm::vec2(texelWidth, texelHeight);

			// Submit the quad location data to the renderer
			fontContext.m_Vertices->clear();
			fontContext.m_Vertices->push_back({ quadMin.x, quadMax.y, translation.z });								// 0, 1
			fontContext.m_Vertices->push_back({ quadMin, translation.z });											// 0, 0
			fontContext.m_Vertices->push_back({ quadMax.x, quadMin.y, translation.z });								// 1, 0
			fontContext.m_Vertices->push_back({ quadMin.x, quadMax.y, translation.z });								// 0, 1
			fontContext.m_Vertices->push_back({ quadMax.x, quadMin.y, translation.z });								// 1, 0
			fontContext.m_Vertices->push_back({ quadMax, translation.z });											// 1, 1
			fontContext.m_TextInputSpec.m_ShapeComponent->m_Vertices = fontContext.m_Vertices;

			// Submit the texture coordinates data to the renderer
			fontContext.m_TexCoordinates->clear();
			fontContext.m_TexCoordinates->push_back({ texCoordMin.x, texCoordMax.y });			// 0, 1
			fontContext.m_TexCoordinates->push_back(texCoordMin);								// 0, 0
			fontContext.m_TexCoordinates->push_back({ texCoordMax.x, texCoordMin.y });			// 1, 0
			fontContext.m_TexCoordinates->push_back({ texCoordMin.x, texCoordMax.y });			// 0, 1
			fontContext.m_TexCoordinates->push_back({ texCoordMax.x, texCoordMin.y });			// 1, 0
			fontContext.m_TexCoordinates->push_back(texCoordMax);
			fontContext.m_TextInputSpec.m_ShapeComponent->m_TextureCoordinates = fontContext.m_TexCoordinates;

			// TODO: Submit multiple glyphs at once for CPU optimization
			// Submit the glyph data to the renderer
			Rendering::RenderingService::SubmitDataToRenderer(fontContext.m_TextInputSpec);

			// Shift the location to the next character
			xLocation += scale * glyph.m_Advance;
		}
	}

	Math::vec2 Font::GetSingleLineTextSize(std::string_view text)
	{
		// Loop through all glyphs of provided text and generate the text's x and y extents
		Math::vec2 outputSize{ 0.0f };
		std::string::const_iterator currentCharacter;
		for (size_t characterIndex = 0; characterIndex < text.size(); characterIndex++)
		{
			// Early out if glyph does not exist
			if (!m_Characters.contains(text[characterIndex]))
			{
				continue;
			}
			Character glyph = m_Characters[text[characterIndex]];

			// Get the maximum y glyph size for the text
			float ySize{ glyph.m_QuadMax.y - glyph.m_QuadMin.y };
			if (ySize > outputSize.y)
			{ 
				outputSize.y = ySize; 
			}

			// Get the total x-axis length of the text
			outputSize.x += glyph.m_Advance;
		}

		// Magic number to center the text
		//outputSize.y /= 45.5f; // TODO: FIX THIS MAGIC NUMBER PLEASE
		return outputSize;
	}
	size_t Font::GetIndexFromMousePosition(std::string_view text, float textStartPoint, float mouseXPosition, float textScalingFactor)
	{
		// Loop through all glyphs of provided text and generate the text's x and y extents
		float accumulatedXPosition{ textStartPoint };
		std::string::const_iterator currentCharacter;
		for (size_t characterIndex = 0; characterIndex < text.size(); characterIndex++)
		{
			// Early out if glyph does not exist
			if (!m_Characters.contains(text[characterIndex]))
			{
				continue;
			}
			Character glyph = m_Characters[text[characterIndex]];

			// Get the total x-axis length of the text
			accumulatedXPosition += glyph.m_Advance * textScalingFactor;

			// Check if the (middle position of the this glyph) surpases the mouse position
			if (accumulatedXPosition - glyph.m_Advance * 0.5f * textScalingFactor > mouseXPosition)
			{
				return characterIndex;
			}
		}

		// Return the text's size if no position is identified
		return text.size();
	}
	void Font::GetMultiLineTextMetadata(std::string_view text, MultiLineTextDimensions& metadata, float scale, int maxLineWidth)
	{
		// Initialize the active location where text is being rendered
		float initialXLocation{ 0.0f };
		float initialYLocation{ 0.0f };
		float xLocation{ initialXLocation };
		float yLocation{ initialYLocation };
		float lineHeight{ 0.0f };
		bool wrapText{ maxLineWidth > 0 };
		size_t activeWordEnding{ 0 };
		int lastBreak{0};

		// Reset metadata
		metadata = {};

		// Iterate through each character in the string
		for (size_t characterIndex = 0; characterIndex < text.size(); characterIndex++)
		{
			// Check if we should evaluate the word this letter exists in
			if (wrapText && characterIndex >= activeWordEnding)
			{
				// Get active word's terminal location and word width
				size_t wordIndex{ characterIndex };
				double wordWidth{ 0.0f };
				while (wordIndex < text.size())
				{
					if (std::isspace(text[wordIndex]))
					{
						activeWordEnding = wordIndex;
						break;
					}
					wordWidth += m_Characters.at(text[wordIndex]).m_Advance;
					wordIndex++;
				}
				wordWidth *= scale;

				// Check if newline is appropriate for active word
				if (xLocation + wordWidth > initialXLocation + maxLineWidth)
				{
					metadata.m_LineBreaks.push_back({ lastBreak, (int)characterIndex });
					metadata.m_LineSize.push_back({ xLocation / scale, lineHeight });
					lastBreak = (int)characterIndex;
					lineHeight = 0.0f;
					xLocation = initialXLocation;
					yLocation -= scale * m_LineHeight;
				}
			}

			// Get the active character from the string
			char character = text[characterIndex];
			Character glyph;

			// Handle specific character cases
			switch (character)
			{
			case '\n':
				// Move to next line
				metadata.m_LineBreaks.push_back({ lastBreak, (int)characterIndex });
				metadata.m_LineSize.push_back({ xLocation / scale, lineHeight });
				lastBreak = (int)characterIndex;
				lineHeight = 0.0f;
				xLocation = initialXLocation;
				yLocation -= scale * m_LineHeight;
				continue;
			case '\r':
				// Skip carriage return
				continue;
			case '\t':
				// Tab character
				character = ' ';
				break;
			}

			// Ensure the character exists in the font atlas
			if (m_Characters.contains(character))
			{
				glyph = m_Characters.at(character);
			}
			else
			{
				// TODO: Handle this case better
				glyph = m_Characters.at('?');
			}

			// Get the line height from each glyph
			if (glyph.m_QuadMax.y - glyph.m_QuadMin.y > lineHeight)
			{
				lineHeight = glyph.m_QuadMax.y - glyph.m_QuadMin.y;
			}

			// Shift the location to the next character
			xLocation += scale * glyph.m_Advance;
		}

		// Get data for final line
		metadata.m_LineBreaks.push_back({ lastBreak , text.size()});
		metadata.m_LineSize.push_back({ xLocation / scale, lineHeight });
	}
}
