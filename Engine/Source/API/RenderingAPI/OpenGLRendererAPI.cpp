#include "kgpch.h"

#include "Kargono/Rendering/RendererAPI.h"

#include "API/Platform/gladAPI.h"

#include <string>

#ifdef KG_RENDERER_OPENGL

namespace Kargono::Utility
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
			KG_ERROR("[Severity: {0}][Source: {1}] [Type: {2}] Message: {3}", severityString, sourceString, typeString, message);
			return;
		}
		case GL_DEBUG_SEVERITY_MEDIUM:
		{
			severityString = "Medium";
			KG_CRITICAL("[Severity: {0}][Source: {1}] [Type: {2}] Message: {3}", severityString, sourceString, typeString, message);
			return;
		}
		case GL_DEBUG_SEVERITY_LOW:
		{
			severityString = "Low";
			KG_WARN("[Severity: {0}][Source: {1}] [Type: {2}] Message: {3}", severityString, sourceString, typeString, message);
			return;
		}
		case GL_DEBUG_SEVERITY_NOTIFICATION:
		{
			severityString = "Notification";
			KG_INFO("[Severity: {0}][Source: {1}] [Type: {2}] Message: {3}", severityString, sourceString, typeString, message);
			return;
		}
		}

		KG_ERROR("Unknown severity level!");
	}

	static GLenum StencilComparisonToGLenum(Kargono::Rendering::StencilComparisonType comparisonType)
	{
		switch (comparisonType)
		{
		case Kargono::Rendering::StencilComparisonType::NEVER: return GL_NEVER;
		case Kargono::Rendering::StencilComparisonType::LESS: return GL_LESS;
		case Kargono::Rendering::StencilComparisonType::LEQUAL: return GL_LEQUAL;
		case Kargono::Rendering::StencilComparisonType::GREATER: return GL_GREATER;
		case Kargono::Rendering::StencilComparisonType::GEQUAL: return GL_GEQUAL;
		case Kargono::Rendering::StencilComparisonType::EQUAL: return GL_EQUAL;
		case Kargono::Rendering::StencilComparisonType::NOTEQUAL: return GL_NOTEQUAL;
		case Kargono::Rendering::StencilComparisonType::ALWAYS: return GL_ALWAYS;
		}
		KG_ERROR("Invalid enum provided in StencilComparisonToGLStencil");
		return 0;
	}

	static GLenum StencilOptionsToGLEnum(Kargono::Rendering::StencilOptions option)
	{
		switch (option)
		{
		case Kargono::Rendering::StencilOptions::KEEP: return GL_KEEP;
		case Kargono::Rendering::StencilOptions::ZERO: return GL_ZERO;
		case Kargono::Rendering::StencilOptions::REPLACE: return GL_REPLACE;
		case Kargono::Rendering::StencilOptions::INCR: return GL_INCR;
		case Kargono::Rendering::StencilOptions::INCR_WRAP: return GL_INCR_WRAP;
		case Kargono::Rendering::StencilOptions::DECR: return GL_DECR;
		case Kargono::Rendering::StencilOptions::DECR_WRAP: return GL_DECR_WRAP;
		case Kargono::Rendering::StencilOptions::INVERT: return GL_INVERT;
		}
		KG_ERROR("Invalid enum provided in StencilOptionsToGLEnum");
		return 0;
	}
}

namespace Kargono::Rendering
{

	void RendererAPI::Init()
	{

	// Only Enable OpenGL logging if debug is enabled
	#ifdef KG_DEBUG
		// Enable Debug Output
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		// Connect callback function to debug error events
		glDebugMessageCallback(Utility::OpenGLMessageCallback, nullptr);
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

	void RendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void RendererAPI::ClearDepthBuffer()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void RendererAPI::SetClearColor(const Kargono::Math::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}
	void RendererAPI::SetDepthTesting(bool value)
	{
		value ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	}
	void RendererAPI::SetStencilMask(uint32_t value)
	{
		glStencilMask(value);
	}
	void RendererAPI::StencilTestFunc(Kargono::Rendering::StencilComparisonType comparisonType, int32_t reference, uint32_t mask)
	{
		glStencilFunc(Utility::StencilComparisonToGLenum(comparisonType), reference, mask);
	}

	void RendererAPI::StencilTestOptions(Kargono::Rendering::StencilOptions sfail, Kargono::Rendering::StencilOptions dfail, Kargono::Rendering::StencilOptions sdpass)
	{
		glStencilOp(Utility::StencilOptionsToGLEnum(sfail), Utility::StencilOptionsToGLEnum(dfail), Utility::StencilOptionsToGLEnum(sdpass));
	}
	void RendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
	void RendererAPI::DrawIndexed(const Kargono::Ref<Kargono::Rendering::VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

	void RendererAPI::DrawIndexed(const Kargono::Ref<Kargono::Rendering::VertexArray>& vertexArray,uint32_t* indexPointer, uint32_t indexCount)
	{
		vertexArray->Bind();
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, indexPointer);
	}

	void RendererAPI::DrawLines(const Kargono::Ref<Kargono::Rendering::VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	void RendererAPI::DrawTriangles(const Kargono::Ref<Kargono::Rendering::VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	}

	void RendererAPI::SetLineWidth(float width)
	{
		glLineWidth(width);
	}
}

#endif
