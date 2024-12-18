#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Rendering/RendererAPI.h"
#include "Kargono/Rendering/EditorPerspectiveCamera.h"
#include "Kargono/Rendering/Camera.h"
#include "Kargono/Rendering/Shader.h"
#include "Kargono/Math/Math.h"

namespace Kargono::Rendering
{
	class RenderingService
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

		//============================================================
		// Scene Rendering Submission LifeTime Functions
		//============================================================
		static void BeginScene(const Camera& camera, const Math::mat4& transform);
		static void BeginScene(const EditorPerspectiveCamera& camera);
		static void BeginScene(const Math::mat4 projection);
		static void SubmitDataToRenderer(RendererInputSpec& inputSpec);
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
		static void FillTextureIndex(RendererInputSpec& inputSpec);
		static void FillIndicesData(RendererInputSpec& inputSpec);
		static void FillEntityID(Rendering::RendererInputSpec& inputSpec);

		//============================================================
		// Per Vertex Function Pointers for Filling Input Data
		//============================================================
		static void FillTextureCoordinate(RendererInputSpec& inputSpec, uint32_t iteration);
		static void FillLocalPosition(RendererInputSpec& inputSpec, uint32_t iteration);
		static void FillWorldPosition(RendererInputSpec& inputSpec, uint32_t iteration);
		static void FillWorldPositionNoTransform(RendererInputSpec& inputSpec, uint32_t iteration);
		static void FillVertexColor(RendererInputSpec& inputSpec, uint32_t iteration);
		
		//============================================================
		// Per DrawCallBuffer Function Pointers to fill Uniform Data
		//============================================================
		static void FillTextureUniform(Ref<DrawCallBuffer> buffer);

		//============================================================
		// Pre Batch Render Functions
		//============================================================
		

		//============================================================
		// Post Batch Render Functions
		//============================================================
		

		//============================================================
		// Draw Call Functions
		//============================================================
		static void DrawBufferPoints(Ref<DrawCallBuffer> buffer);
		static void DrawBufferLine(Ref<DrawCallBuffer> buffer);
		static void DrawBufferIndices(Ref<DrawCallBuffer> buffer);
		static void DrawBufferTriangles(Ref<DrawCallBuffer> buffer);

		

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
