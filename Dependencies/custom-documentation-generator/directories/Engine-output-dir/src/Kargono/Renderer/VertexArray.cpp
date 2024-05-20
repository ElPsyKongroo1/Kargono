#include "kgpch.h"

#include "Kargono/Renderer/VertexArray.h"
#include "Kargono/Renderer/Renderer.h"
#include "API/OpenGL/OpenGLVertexArray.h"

namespace Kargono {

	Ref<VertexArray> VertexArray::Create()
	{
		return CreateRef<API::OpenGL::OpenGLVertexArray>();
	}


}
