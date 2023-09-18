#include "kgpch.h"

#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Renderer/Texture.h"
#include "API/OpenGL/OpenGLTexture.h"

namespace Kargono {
	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			KG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;


		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(width, height);

		}
		KG_CORE_ASSERT(false, "RendererAPI:: Unknown RendererAPI!");
		return nullptr;
	}
	Ref<Kargono::Texture2D> Texture2D::Create(const std::filesystem::path& path)
	{
		KG_CORE_ASSERT(path.is_absolute(), "Path provided to texture create function is not an absolute path!")

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			KG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;


		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(path.string());

		}
		KG_CORE_ASSERT(false, "RendererAPI:: Unknown RendererAPI!");
		return nullptr;
	}
}
