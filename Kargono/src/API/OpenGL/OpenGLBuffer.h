#pragma once

#include "Kargono\Renderer\InputBuffer.h"

namespace Kargono {

	// This class is the OpenGL Implementation of a Vertex Buffer in the Core Code
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		//============================================================
		// Constructors and Destructors
		//============================================================
		// Constructors instantiate the Vertex Buffer in OpenGL and bind
		// it to the currently bound VAO (Vertex Array Object)
		// Destructor removes buffer from OpenGL Context
		OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(float* vertices, uint32_t size);
		virtual ~OpenGLVertexBuffer() override;
	public:
		//============================================================
		// Binding Functionality
		//============================================================
		// Allows Vertex Buffers to be bound to various VAO's or for other debugging uses
		virtual void Bind() const override;
		virtual void Unbind() const override;
		//============================================================
		// Set or overwrite current vertices associated with Vertex Buffer
		//============================================================
		virtual void SetData(const void* data, uint32_t size) override;
		//============================================================
		// Getters/Setters
		//============================================================
		// Layout dictates the pattern of data inside the buffer
		virtual const InputBufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const InputBufferLayout& layout) override { m_Layout = layout; }
	private:
		uint32_t m_RendererID;
		InputBufferLayout m_Layout;
	};

	// This class is the OpenGL Implementation of an Index Buffer in the Core Code
	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		//============================================================
		// Constructors and Destructors
		//============================================================

		// Constructors instantiate the Index Buffer in OpenGL and bind
		// it to the currently bound VAO (Vertex Array Object)
		// Destructor removes buffer from OpenGL Context
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~OpenGLIndexBuffer() override;
	public:
		//============================================================
		// Binding Functionality
		//============================================================
		// Allows Index Buffers to be bound to various VAO's or for other debugging uses
		virtual void Bind() const override;
		virtual void Unbind() const override;
		//============================================================
		// Get the number of indices
		//============================================================
		virtual uint32_t GetCount() const override { return m_Count; }
	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};
}

