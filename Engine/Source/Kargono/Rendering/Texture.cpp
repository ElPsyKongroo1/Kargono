#include "kgpch.h"

#include "Kargono/Rendering/RenderingEngine.h"
#include "Kargono/Rendering/Texture.h"

#include "API/RenderingAPI/OpenGLTexture.h"

namespace Kargono::Rendering
{
	Ref<Texture2D> Texture2D::Create(const TextureSpecification& spec)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLTexture2D>(spec);
#endif
	}
	Ref<Texture2D> Texture2D::Create(uint32_t rendererID, uint32_t width, uint32_t height)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLTexture2D>(rendererID, width, height);
#endif
	}
	Ref<Texture2D> Texture2D::Create(Buffer buffer, const Assets::TextureMetaData& metadata)
	{
#ifdef KG_RENDERER_OPENGL
		return CreateRef<API::RenderingAPI::OpenGLTexture2D>(buffer, metadata);
#endif
	}
	Ref<Texture2D> Texture2D::CreateEditorTexture(const std::filesystem::path& path)
	{
			KG_ASSERT(path.is_absolute(), "Path provided to texture create function is not an absolute path!")
#ifdef KG_RENDERER_OPENGL
			return CreateRef<API::RenderingAPI::OpenGLTexture2D>(path.string());
#endif
	}
}
