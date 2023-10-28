#include "kgpch.h"

#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Renderer/Texture.h"
#include "API/OpenGL/OpenGLTexture.h"

namespace Kargono
{

	Ref<Texture2D> Texture2D::Create(uint32_t rendererID, uint32_t width, uint32_t height)
	{
		return CreateRef<OpenGLTexture2D>(rendererID, width, height);
	}

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		return CreateRef<OpenGLTexture2D>(width, height);
	}
	Ref<Texture2D> Texture2D::Create(Buffer buffer, const TextureMetaData& metadata)
	{
		return CreateRef<OpenGLTexture2D>(buffer, metadata);
	}
	Ref<Kargono::Texture2D> Texture2D::CreateEditorTexture(const std::filesystem::path& path)
	{
		KG_CORE_ASSERT(path.is_absolute(), "Path provided to texture create function is not an absolute path!")
		return CreateRef<OpenGLTexture2D>(path.string());
	}
}
