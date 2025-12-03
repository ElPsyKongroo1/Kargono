#pragma once

#include "Kargono/Core/Base.h"
#include "Modules/Rendering/RendererAPI.h"
#include "Modules/Cameras/PerspectiveCamera.h"
#include "Modules/Cameras/CameraProjection.h"
#include "Modules/Rendering/Assets/Shader.h"
#include "Kargono/Math/Math.h"

namespace Kargono::Rendering
{
	struct Statistics
	{
		uint32_t m_DrawCalls{ 0 };
		uint32_t m_VertexCount{ 0 };
	};

	constexpr inline uint32_t k_MaxVertexBufferSize{ 10000 };

	class RenderingService
	{
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		static void Init();
		static void Shutdown();
	public:
		//==============================
		// Manage Window
		//==============================
		static void OnWindowResize(uint32_t width, uint32_t height);
	public:
		//==============================
		// Scene Rendering
		//==============================
		static void BeginScene(const Cameras::CameraProjection& camera, const Math::mat4& viewMatrix);
		static void BeginScene(const Cameras::PerspectiveCamera& camera);
		static void BeginScene(const Math::mat4 viewProjection);
		static void SubmitDataToRenderer(RendererInputSpec& inputSpec);
		static void EndScene();
	private:
		// Helper(s)
		static void FlushBuffers();
	public:
		//==============================
		// Renderer Configuration
		//==============================
		static float GetLineWidth();
		static void SetLineWidth(float width);
	private:
		//==============================
		// Per Object Function Pointers for Filling Input Data
		//==============================
		static void FillTextureIndex(RendererInputSpec* inputSpec);
		static void FillTextureAtlas(RendererInputSpec* inputSpec);
		static void FillIndicesData(RendererInputSpec* inputSpec);
		static void FillEntityID(Rendering::RendererInputSpec* inputSpec);
	private:
		//==============================
		// Per Vertex Function Pointers for Filling Input Data
		//==============================
		static void FillTextureCoordinate(RendererInputSpec* inputSpec, uint32_t iteration);
		static void FillLocalPosition(RendererInputSpec* inputSpec, uint32_t iteration);
		static void FillWorldPosition(RendererInputSpec* inputSpec, uint32_t iteration);
		static void FillWorldPositionNoTransform(RendererInputSpec* inputSpec, uint32_t iteration);
		static void FillVertexColor(RendererInputSpec* inputSpec, uint32_t iteration);
	private:
		//==============================
		// Per DrawCallBuffer Function Pointers to fill Uniform Data
		//==============================
		static void FillTextureUniform(Ref<DrawCallBuffer> buffer);
	private:
		//==============================
		// Draw Call Functions
		//==============================
		static void DrawBufferPoints(Ref<DrawCallBuffer> buffer);
		static void DrawBufferLine(Ref<DrawCallBuffer> buffer);
		static void DrawBufferIndices(Ref<DrawCallBuffer> buffer);
		static void DrawBufferTriangles(Ref<DrawCallBuffer> buffer);
	public:
		//==============================
		// Debug Statistics
		//==============================
		static void ResetStats();
		static Statistics GetStats();
	private:
		//==============================
		// Friend Declarations
		//==============================
		friend Shader;
	};
}
