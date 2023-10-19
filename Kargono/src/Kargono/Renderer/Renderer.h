#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Renderer/RendererAPI.h"
#include "Kargono/Renderer/EditorCamera.h"
#include "Kargono/Renderer/Camera.h"
#include "Kargono/Renderer/Shader.h"

namespace Kargono
{
	class Renderer
	{
	public:
		//============================================================
		// Renderer LifeTime Functions
		//============================================================
		static void Init();
		static void Shutdown();
		//============================================================
		// RendererAPI Connection Functions
		//============================================================
		static void OnWindowResize(uint32_t width, uint32_t height);
		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		//============================================================
		// Scene Rendering Submission LifeTime Functions
		//============================================================
		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void SubmitDataToRenderer(Shader::RendererInputSpec& inputSpec);
		static void EndScene();
	private:
		static void FlushBuffers();
	public:

		//============================================================
		// Set Line Width!
		//============================================================
		static float GetLineWidth();
		static void SetLineWidth(float width);


	private:

		// Note: The following functions are intended to be added to the
		// vectors inside the shader class when shaders are initialized

		// Set Shader as friend to allow function pointers below to be used
		friend Shader;

		//============================================================
		// Per Object Function Pointers for Filling Input Data
		//============================================================
		static void FillTextureIndex(Shader::RendererInputSpec& inputSpec);
		static void FillIndicesData(Shader::RendererInputSpec& inputSpec);

		//============================================================
		// Per Vertex Function Pointers for Filling Input Data
		//============================================================
		static void FillTextureCoordinate(Shader::RendererInputSpec& inputSpec, uint32_t iteration);
		static void FillLocalPosition(Shader::RendererInputSpec& inputSpec, uint32_t iteration);
		static void FillWorldPosition(Shader::RendererInputSpec& inputSpec, uint32_t iteration);
		static void FillWorldPositionNoTransform(Shader::RendererInputSpec& inputSpec, uint32_t iteration);
		static void FillVertexColor(Shader::RendererInputSpec& inputSpec, uint32_t iteration);
		
		//============================================================
		// Per DrawCallBuffer Function Pointers to fill Uniform Data
		//============================================================
		static void FillTextureUniform(Ref<Shader::DrawCallBuffer> buffer);

		//============================================================
		// Draw Call Function Pointers
		//============================================================
		static void DrawBufferLine(Ref<Shader::DrawCallBuffer> buffer);
		static void DrawBufferIndices(Ref<Shader::DrawCallBuffer> buffer);
		static void DrawBufferTriangles(Ref<Shader::DrawCallBuffer> buffer);

		// Specifies maximum size in bytes of DrawCallBuffers
		static const uint32_t s_MaxVertexBufferSize = 10000;
	public:

		//============================================================
		// Statistics Struct and Functions for Debugging
		//============================================================
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t VertexCount = 0;
		};
		static void ResetStats();
		static Statistics GetStats();
	};
}
