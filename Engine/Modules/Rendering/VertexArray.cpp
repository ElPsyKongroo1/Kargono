#include "kgpch.h"

#include "Modules/Rendering/VertexArray.h"
#include "Modules/Rendering/RenderingService.h"

#include "Modules/Rendering/ExternalAPI/OpenGLVertexArray.h"

namespace Kargono::Rendering
{

	Ref<VertexArray> VertexArray::Create()
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLVertexArray>();
#endif
	}


}
