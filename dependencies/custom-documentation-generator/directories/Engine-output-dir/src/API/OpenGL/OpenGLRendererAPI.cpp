/// @brief Main header inclusion for Kargono game engine
#include "kgpch.h"

/// @brief OpenGL specific Renderer API inclusion for extension functions, constants and types
#include "API/OpenGL/OpenGLRendererAPI.h"

/// @brief Graphics Library Loader header for managing OpenGL contexts and loading OpenGL function pointers
#include <glad/glad.h>

/// @brief Kargono's Renderer API header inclusion
#include "Kargono/Renderer/RendererAPI.h"

/// @brief Namespaced encapsulation for OpenGL specific functionality
namespace API::OpenGL
{
	static void OpenGLMessageCallback(unsigned source, unsigned type, unsigned id,
		unsigned severity, int length, const char* message, const void* userParam)
	{
		std::string sourceString = "";
		std::string typeString = "";
		std::string severityString = "";

		switch (source)
		{
		case GL_DEBUG_SOURCE_API:             sourceString = "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceString = "Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceString = "Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceString = "Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     sourceString = "Application"; break;
		case GL_DEBUG_SOURCE_OTHER:           sourceString = "Other"; break;
		}

		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:               typeString = "Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeString = "Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeString = "Undefined Behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         typeString = "Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         typeString = "Performance"; break;
		case GL_DEBUG_TYPE_MARKER:              typeString = "Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          typeString = "Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           typeString = "Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER:               typeString = "Other"; break;
		}

		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:
		{
			severityString = "High";
			KG_CORE_CRITICAL("[Severity: {0}][Source: {1}] [Type: {2}] Message: {3}", severityString, sourceString, typeString, message);
			return;
		}
		case GL_DEBUG_SEVERITY_MEDIUM:
		{
			severityString = "Medium";
			KG_CORE_ERROR("[Severity: {0}][Source: {1}] [Type: {2}] Message: {3}", severityString, sourceString, typeString, message);
			return;
		}
		case GL_DEBUG_SEVERITY_LOW:
		{
			severityString = "Low";
			KG_CORE_WARN("[Severity: {0}][Source: {1}] [Type: {2}] Message: {3}", severityString, sourceString, typeString, message);
			return;
		}
		case GL_DEBUG_SEVERITY_NOTIFICATION:
		{
			severityString = "Notification";
			KG_CORE_TRACE("[Severity: {0}][Source: {1}] [Type: {2}] Message: {3}", severityString, sourceString, typeString, message);
			return;
		}
		}

		KG_CORE_ASSERT(false, "Unknown severity level!");
	}

/// @brief Converts Kargono's enum for stencil comparison type to OpenGL's GLenum
/// @param comparisonType Kargono's stencil comparison type enum
/// @return GLenum equivalent of Kargono's stencil comparison type enum
	static GLenum StencilComparisonToGLenum(Kargono::RendererAPI::StencilComparisonType comparisonType)
	{
		switch (comparisonType)
		{
		case Kargono::RendererAPI::StencilComparisonType::NEVER: return GL_NEVER;
		case Kargono::RendererAPI::StencilComparisonType::LESS: return GL_LESS;
		case Kargono::RendererAPI::StencilComparisonType::LEQUAL: return GL_LEQUAL;
		case Kargono::RendererAPI::StencilComparisonType::GREATER: return GL_GREATER;
		case Kargono::RendererAPI::StencilComparisonType::GEQUAL: return GL_GEQUAL;
		case Kargono::RendererAPI::StencilComparisonType::EQUAL: return GL_EQUAL;
		case Kargono::RendererAPI::StencilComparisonType::NOTEQUAL: return GL_NOTEQUAL;
		case Kargono::RendererAPI::StencilComparisonType::ALWAYS: return GL_ALWAYS;
		}
		KG_CORE_ASSERT(false, "Invalid enum provided in StencilComparisonToGLStencil");
		return 0;
	}

/// @brief Converts Kargono's enum for stencil option to OpenGL's GLenum
/// @param option Kargono's stencil option enum
/// @return GLenum equivalent of Kargono's stencil option enum
	static GLenum StencilOptionsToGLEnum(Kargono::RendererAPI::StencilOptions option)
	{
		switch (option)
		{
		case Kargono::RendererAPI::StencilOptions::KEEP: return GL_KEEP;
		case Kargono::RendererAPI::StencilOptions::ZERO: return GL_ZERO;
		case Kargono::RendererAPI::StencilOptions::REPLACE: return GL_REPLACE;
		case Kargono::RendererAPI::StencilOptions::INCR: return GL_INCR;
		case Kargono::RendererAPI::StencilOptions::INCR_WRAP: return GL_INCR_WRAP;
		case Kargono::RendererAPI::StencilOptions::DECR: return GL_DECR;
		case Kargono::RendererAPI::StencilOptions::DECR_WRAP: return GL_DECR_WRAP;
		case Kargono::RendererAPI::StencilOptions::INVERT: return GL_INVERT;
		}
		KG_CORE_ASSERT(false, "Invalid enum provided in StencilOptionsToGLEnum");
		return 0;
	}

/// @brief Initializes the OpenGL renderer API with some default settings and checks
	void OpenGLRendererAPI::Init()
	{

	// Only Enable OpenGL logging if debug is enabled
	#ifdef KG_DEBUG
		// Enable Debug Output
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		// Connect callback function to debug error events
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);
		// Filter Source, Type, or Severity. Currently all notifications are displayed!
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	#endif

		// Set up Initial OpenGL State Functionality
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_LINE_SMOOTH);
	}

/// @brief Set the viewport for openGL rendering
/// @param x The x position of the viewport
/// @param y The y position of the viewport
/// @param width The width of the viewport
/// @param height The height of the viewport
	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

/// @brief Clears the depth buffer, useful for depth testing
	void OpenGLRendererAPI::ClearDepthBuffer()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

/// @brief Specifies the background color of the window
/// @param color The color to set, composed of r,g,b,a values
	void OpenGLRendererAPI::SetClearColor(const Kargono::Math::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}
/// @brief Enable or Disable OpenGL's depth testing functionality
/// @param value if true, it enables depth testing otherwise it disables depth testing
	void OpenGLRendererAPI::SetDepthTesting(bool value)
	{
		value ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	}
/// @brief Sets mask that enables and disables writing to individual bits in the stencil planes
/// @param value the stencil mask value
	void OpenGLRendererAPI::SetStencilMask(uint32_t value)
	{
		glStencilMask(value);
	}
/// @brief Specifies stencil function, which is used to compare stencil value with reference value.
/// @param comparisonType The comparison type between stencil and reference value
/// @param reference The reference value for the stencil test
/// @param mask The mask that is ANDed with both the reference value and the stored stencil value when the test is done
	void OpenGLRendererAPI::StencilTestFunc(RendererAPI::StencilComparisonType comparisonType, int32_t reference, uint32_t mask)
	{
		glStencilFunc(StencilComparisonToGLenum(comparisonType), reference, mask);
	}

/// @brief Function to Specify stencil action based on stencil test results
/// @brief sfail Stencil action when the stencil test fails
/// @brief dfail Stencil action when the stencil test passes, but the depth test fails
/// @brief sdpass Stencil action when both the stencil test and the depth test pass
	void OpenGLRendererAPI::StencilTestOptions(RendererAPI::StencilOptions sfail, RendererAPI::StencilOptions dfail, RendererAPI::StencilOptions sdpass)
	{
		glStencilOp(StencilOptionsToGLEnum(sfail), StencilOptionsToGLEnum(dfail), StencilOptionsToGLEnum(sdpass));
	}
/// @brief Clears all the buffers
	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
/// @brief Draw primitives from an array, using indexing value
/// @param vertexArray The set of vertices to draw
/// @param indexCount The number of indices to draw
	void OpenGLRendererAPI::DrawIndexed(const Kargono::Ref<Kargono::VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

/// @brief Draw primitives from an array, using a pointer to the indices
/// @param vertexArray The set of vertices to draw
/// @param indexPointer The pointer to the indices in memory
/// @param indexCount The number of indices to draw
	void OpenGLRendererAPI::DrawIndexed(const Kargono::Ref<Kargono::VertexArray>& vertexArray,uint32_t* indexPointer, uint32_t indexCount)
	{
		vertexArray->Bind();
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, indexPointer);
	}

/// @brief Draw Line Strip primitives from an array
/// @param vertexArray The set of vertices to draw
/// @param vertexCount The number of vertices to draw
	void OpenGLRendererAPI::DrawLines(const Kargono::Ref<Kargono::VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

/// @brief Draw Triangle Strip primitives from an array
/// @param vertexArray The set of vertices to draw
/// @param vertexCount The number of vertices to draw
	void OpenGLRendererAPI::DrawTriangles(const Kargono::Ref<Kargono::VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	}

/// @brief Sets the line width in OpenGL
/// @param width The width of the line
	void OpenGLRendererAPI::SetLineWidth(float width)
	{
		glLineWidth(width);
	}
}
