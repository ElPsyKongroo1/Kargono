#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Renderer/VertexArray.h"

namespace API::OpenGL
{
	//============================================================
	// OpenGL Vertex Array Class
	//============================================================
	// This class represents a vertex array inside the OpenGL context.
	//		A vertex array's existance in OpenGL is meant to bundle
	//		a number of VertexBuffers, an IndexBuffer, and preferences
	//		into one construct. There is a variable number of VertexBuffers
	//		but there is only one IndexBuffer. VertexBuffers are held in
	//		the OpenGL memory contiguously. A vertex array can be bound
	//		and all of the connected buffers get automatically connected.
	class OpenGLVertexArray : public Kargono::VertexArray
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This constructor simply instantiates the Vertex Array inside the OpenGL context
		//		and stores the reference inside m_RendererID
		OpenGLVertexArray();
		// This destructor simply clears the Vertex Array inside the OpenGL context
		virtual ~OpenGLVertexArray();
	public:
		//==============================
		// Binding Functionality
		//==============================
		// This bind function associates the OpenGL vertex array associated with this class with
		//		the current OpenGL context. Further default render calls will use this vertex array.
		virtual void Bind() const override;
		// This unbind function clears the vertex array slot in the OpenGL context.
		virtual void Unbind() const override;
		//==============================
		// Update OpenGL Context
		//==============================
		// This function adds another vertex buffer into the OpenGL vertex array associated with this class.
		//		This process requires binding both the vertex array and the vertexBuffer, adding all of the
		//		elements inside the vertexBuffer's layout individually to the vertex array, and adding the
		//		new vertexBuffer to m_VertexBuffers.
		virtual void AddVertexBuffer(const Kargono::Ref<Kargono::VertexBuffer>& vertexBuffer) override;
		// This function adds the provided index buffer to both the m_IndexBuffer variable and associates
		//		the indexBuffer with the underlying OpenGL vertex array
		virtual void SetIndexBuffer(const Kargono::Ref<Kargono::IndexBuffer>& indexBuffer) override;

		//==============================
		// Getters
		//==============================
		virtual const std::vector<Kargono::Ref<Kargono::VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		virtual const Kargono::Ref<Kargono::IndexBuffer>& GetIndexBuffer() const  override { return m_IndexBuffer; }
	private:
		// m_VertexBuffers holds the in-engine representations of the vertexBuffers associated with this vertex array
		std::vector<Kargono::Ref<Kargono::VertexBuffer>> m_VertexBuffers;
		// m_IndexBuffer holds the in-engine representation of the indexBuffer associated with this vertex array
		Kargono::Ref<Kargono::IndexBuffer> m_IndexBuffer;
		// m_VertexBufferIndex is a running total for the number of vertexBuffer elements associated with this
		//		vertex array. This value gets incremented for every element inside the layout of every vertexBuffer
		//		that gets added to this vertex array.
		uint32_t m_VertexBufferIndex = 0;
		// m_RendererID represents the OpenGL vertex array associated with this object. Any OpenGL calls
		//		will use this ID.
		uint32_t m_RendererID;
	};
}

