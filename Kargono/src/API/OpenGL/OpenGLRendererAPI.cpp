#include "kgpch.h"

#include "API/OpenGL/OpenGLRendererAPI.h"

#include <glad/glad.h>

#include "Kargono/Renderer/RendererAPI.h"

namespace Kargono
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

	static GLenum StencilComparisonToGLenum(RendererAPI::StencilComparisonType comparisonType)
	{
		switch (comparisonType)
		{
		case RendererAPI::StencilComparisonType::NEVER: return GL_NEVER;
		case RendererAPI::StencilComparisonType::LESS: return GL_LESS;
		case RendererAPI::StencilComparisonType::LEQUAL: return GL_LEQUAL;
		case RendererAPI::StencilComparisonType::GREATER: return GL_GREATER;
		case RendererAPI::StencilComparisonType::GEQUAL: return GL_GEQUAL;
		case RendererAPI::StencilComparisonType::EQUAL: return GL_EQUAL;
		case RendererAPI::StencilComparisonType::NOTEQUAL: return GL_NOTEQUAL;
		case RendererAPI::StencilComparisonType::ALWAYS: return GL_ALWAYS;
		}
		KG_CORE_ASSERT(false, "Invalid enum provided in StencilComparisonToGLStencil");
		return 0;
	}

	static GLenum StencilOptionsToGLEnum(RendererAPI::StencilOptions option)
	{
		switch (option)
		{
		case RendererAPI::StencilOptions::KEEP: return GL_KEEP;
		case RendererAPI::StencilOptions::ZERO: return GL_ZERO;
		case RendererAPI::StencilOptions::REPLACE: return GL_REPLACE;
		case RendererAPI::StencilOptions::INCR: return GL_INCR;
		case RendererAPI::StencilOptions::INCR_WRAP: return GL_INCR_WRAP;
		case RendererAPI::StencilOptions::DECR: return GL_DECR;
		case RendererAPI::StencilOptions::DECR_WRAP: return GL_DECR_WRAP;
		case RendererAPI::StencilOptions::INVERT: return GL_INVERT;
		}
		KG_CORE_ASSERT(false, "Invalid enum provided in StencilOptionsToGLEnum");
		return 0;
	}

	void OpenGLRendererAPI::Init()
	{

	#ifdef KG_DEBUG
		// Enable Debug Output
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		// Connect callback function to debug error events
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);
		// Filter Source, Type, or Severity. Currently all notifications are displayed!
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	#endif

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_LINE_SMOOTH);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::ClearDepthBuffer()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}
	void OpenGLRendererAPI::SetDepthTesting(bool value)
	{
		value ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	}
	void OpenGLRendererAPI::SetStencilMask(uint32_t value)
	{
		glStencilMask(value);
	}
	void OpenGLRendererAPI::StencilTestFunc(RendererAPI::StencilComparisonType comparisonType, int32_t reference, uint32_t mask)
	{
		glStencilFunc(StencilComparisonToGLenum(comparisonType), reference, mask);
	}

	void OpenGLRendererAPI::StencilTestOptions(RendererAPI::StencilOptions sfail, RendererAPI::StencilOptions dfail, RendererAPI::StencilOptions sdpass)
	{
		glStencilOp(StencilOptionsToGLEnum(sfail), StencilOptionsToGLEnum(dfail), StencilOptionsToGLEnum(sdpass));
	}
	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray,uint32_t* indexPointer, uint32_t indexCount)
	{
		vertexArray->Bind();
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, indexPointer);
	}

	void OpenGLRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	void OpenGLRendererAPI::DrawTriangles(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	}

	void OpenGLRendererAPI::SetLineWidth(float width)
	{
		glLineWidth(width);
	}
}
