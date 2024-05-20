#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Renderer/RendererAPI.h"
#include "Kargono/Renderer/VertexArray.h"
#include "Kargono/Math/Math.h"

namespace API::OpenGL
{
	//============================================================
	// OpenGL Renderer Class
	//============================================================
	// This class does not hold any data, but helps initialize the default OpenGL settings required
	//		by the renderer and provides a list of functions that update the OpenGL context.
	class OpenGLRendererAPI : public Kargono::RendererAPI
	{
	public:
		//==============================
		// Object LifeCycle Functions
		//==============================
		// This function sets up OpenGL debugging with the logging system and enables initial OpenGL state
		//		functionality such as blending, stencil testing, etc...
		virtual void Init() override;

		//==============================
		// Update OpenGL State Functionality
		//==============================
		// Function simply updates the OpenGL viewport. Note that the GLFW window also needs to be resized if
		//		the viewport is meant to match the window size.
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		// Update the clear color when the Clear() function is called. Ex: A red clear color will display red
		//		for the entire screen after clear is called.
		virtual void SetClearColor(const Kargono::Math::vec4& color) override;
		// This function clears the color buffer and sets the replacement color to the value that was set in the SetClearColor() function.
		//		This function affects the color attachment(s) inside the currently bound framebuffer.
		virtual void Clear() override;
		// Function clears the depth buffer of the currently attached Framebuffer's depth attachment
		virtual void ClearDepthBuffer() override;
		// Enable or Disable Depth Testing to allow all newly rendered objects to be drawn straight to the color
		//		buffer. Essentially depth testing allows objects written previously to not be overwritten if
		//		the new object is behind the old object. This is the default behavior of depth testing, but it
		//		can be modified.
		virtual void SetDepthTesting(bool value) override;
		// Function allows a bit mask to be set in the OpenGL context that filters/enables/disables writing to the
		//		stencil buffer itself. Ex. a value of 13(1101) will allow stencil values associated with the first, second,
		//		and last digit in the binary version of 13 to be updated. When a stencil test passes, only the digits that are masked with
		//		a 1 are eligible to be updated for that particular pixel. Note that this function is meant to dictate if the stencil test passes,
		//		which means a pixel is written to the screen if the stencil test passes along with other tests such as the depth test if enabled.
		virtual void SetStencilMask(uint32_t value) override;
		// This function updates the internal OpenGL stencil test function that controls when to pass or fail a stencil test.
		//		The stencil test applies the given mask to both the stencil buffer and the reference value and compares them
		//		with the function denoted by comparisonType. Ex. We can pass if the stencil buffer value at that location equals
		//		the reference value. Note that this function is only meant to dictate how the stencil buffer should be updated
		//		depending on the results of the stencil test. This does not affect the rendering of the pixel!
		virtual void StencilTestFunc(RendererAPI::StencilComparisonType comparisonType, int32_t reference, uint32_t mask) override;
		// This function denotes the actions that occur when a stencil test is passed or failed or if the depth test fails. The functions denotes
		//		by the enums can modify the stencil buffer in numerous different ways. Ex: You can set the stencil test to StencilOptions::Replace
		//		to update the stencil buffer with the ref value from StencilTestFunc() when the stencil test fails.
		virtual void StencilTestOptions(RendererAPI::StencilOptions sfail, RendererAPI::StencilOptions dfail, RendererAPI::StencilOptions sdpass) override;
		// This function sets the line width when a render call is made to the DrawLines() function.
		virtual void SetLineWidth(float width) override;

		//==============================
		// OpenGL Render Calls
		//==============================
		// All of the functions below make a render request to the OpenGL context for different types of rendering procedures.

		// The DrawIndexed() functions use the indices provided (or currently bound to OpenGL) to render the vertices
		//		provided by index. Ex: A Vertex array that holds vertex 0,1,2,3,4,5,6 can be drawn using the indices
		//		0, 5, 6   1, 2, 0  6, 4 ,2; which will draw three separate triangles using the given vertices.

		// This DrawIndexed() uses the currently bound indexArray in the OpenGL context
		virtual void DrawIndexed(const Kargono::Ref<Kargono::VertexArray>& vertexArray, uint32_t indexCount = 0) override;
		// This DrawIndexed() uses the provided index buffer to draw.
		virtual void DrawIndexed(const Kargono::Ref<Kargono::VertexArray>& vertexArray, uint32_t* indexPointer, uint32_t indexCount) override;
		// This function simply connects each pair of vertices as an individual line.
		//		Ex: A vertex buffer with vertices 0, 1, 2, 3, 4, 5 will draw the following lines:
		//		{0, 1} {2, 3} {4, 5}
		virtual void DrawLines(const Kargono::Ref<Kargono::VertexArray>& vertexArray, uint32_t vertexCount) override;
		// This function simply connects trios of vertices to form a triangle.
		//		Ex: A vertex buffer with vertices 0, 1, 2, 3, 4, 5 will draw the following triangles:
		//		{0, 1, 2} {3, 4, 5}
		virtual void DrawTriangles(const Kargono::Ref<Kargono::VertexArray>& vertexArray, uint32_t vertexCount) override;
	};
}
