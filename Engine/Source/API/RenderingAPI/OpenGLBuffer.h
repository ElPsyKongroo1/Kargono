#pragma once

#include "Kargono/Rendering/InputBuffer.h"

namespace API::RenderingAPI
{
	//============================================================
	// OpenGL VertexBuffer Class
	//============================================================
	// This class is the OpenGL Implementation of a Vertex Buffer in the Core Code.
	//		A VertexBuffer holds data specific to a particular vertex such as
	//		model location, vertex color, texture location, etc...
	class OpenGLVertexBuffer : public Kargono::Rendering::VertexBuffer
	{
	public:
		//=============================
		// Constructors and Destructors
		//=============================
		// Constructors instantiate the Vertex Buffer in OpenGL and bind
		//		it to the currently bound VAO (Vertex Array Object).
		//		This constructor assumes you are going to re-write to
		//		this data continuously. This is how the current renderer
		//		uses the dynamic shader pipeline!
		OpenGLVertexBuffer(uint32_t size);
		// This constructor assumes you are setting the data once, and using it repeatedly
		OpenGLVertexBuffer(float* vertices, uint32_t size);
		// Destructor removes buffer from OpenGL Context
		virtual ~OpenGLVertexBuffer() override;
	public:
		//==============================
		// Binding Functionality
		//==============================
		// Allows Vertex Buffers to be bound to various VAO's or for other debugging uses
		virtual void Bind() const override;
		virtual void Unbind() const override;
		//==============================
		// Update OpenGL Context
		//==============================
		// This function allows new vertices to be pushed into the OpenGL vertex buffer.
		virtual void SetData(const void* data, uint32_t size) override;
		//==============================
		// Getters/Setters
		//==============================
		virtual const Kargono::Rendering::InputBufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const Kargono::Rendering::InputBufferLayout& layout) override { m_Layout = layout; }
	private:
		// RendererID holds the numerical reference to the underlying OpenGL VertexBuffer
		//		Ex. A renderer ID of three would represent the third vertex buffer created
		//		in the open GL context. 
		uint32_t m_RendererID;
		// Layout dictates the pattern of data inside the buffer
		//		Ex: vec3, vec2, float, vec3, mat4 is a sample pattern
		//		that can represent concepts such as the vertex's world position,
		//		color, and transform.
		Kargono::Rendering::InputBufferLayout m_Layout;
	};

	//============================================================
	// OpenGL IndexBuffer Class
	//============================================================
	// This class is the OpenGL Implementation of an Index Buffer in the Core Code.
	//		The index buffer represents the ordered pattern of vertices that get displayed.
	//		Example. An object can have the vertices 0, 1, 2, 3, 4, 5.
	//		An index buffer could hold the numbers 2, 3, 1. The numbers in the index buffer
	//		represent OpenGL drawing a triangle starting at vertex 2, that going to vertex 3,
	//		and ending on vertex 1.
	class OpenGLIndexBuffer : public Kargono::Rendering::IndexBuffer
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// Constructors instantiate the Index Buffer in OpenGL and bind
		// it to the currently bound VAO (Vertex Array Object)
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
		// Destructor removes buffer from OpenGL Context
		virtual ~OpenGLIndexBuffer() override;
	public:
		//==============================
		// Binding Functionality
		//==============================
		// Allows Index Buffers to be bound to various VAO's or for other debugging uses
		virtual void Bind() const override;
		virtual void Unbind() const override;
		//==============================
		// Get the number of indices
		//==============================
		virtual uint32_t GetCount() const override { return m_Count; }
	private:
		// RendererID holds the numerical reference to the underlying OpenGL IndexBuffer
		//		Ex. A renderer ID of three would represent the third index buffer created
		//		in the open GL context. 
		uint32_t m_RendererID;
		// m_Count represents the size of the index buffer. (Number of Indices not memory)
		uint32_t m_Count;
	};
}

