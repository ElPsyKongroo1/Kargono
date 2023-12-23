#include "kgpch.h"

#include "API/OpenGL/OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace API::Utility
{
	static GLenum TextureTarget(bool multisampled)
	{
		return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
	}

	static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count)
	{
		glCreateTextures(TextureTarget(multisampled), count, outID);
	}

	static void BindTexture(bool multisampled, uint32_t id)
	{
		glBindTexture(TextureTarget(multisampled), id);
	}

	static void AttachColorTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index)
	{
		bool multisampled = samples > 1;
		if (multisampled)
		{
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
	}

	static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height)
	{
		bool multisampled = samples > 1;
		if (multisampled)
		{
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
		}
		else
		{
			glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
	}

	static bool IsDepthFormat(Kargono::FramebufferDataFormat format)
	{
		switch (format)
		{
		case Kargono::FramebufferDataFormat::DEPTH24STENCIL8: return true;
		default: break;
		}

		return false;
	}

	static GLenum KargonoTextureFormatToGL(Kargono::FramebufferDataFormat format)
	{
		switch (format)
		{
		case Kargono::FramebufferDataFormat::RGBA8: return		GL_RGBA8;
		case Kargono::FramebufferDataFormat::RED_INTEGER: return	GL_RED_INTEGER;
		}

		KG_CORE_ASSERT(false, "Invalid framebuffertextureformat in KargonoTextureFormatToGL")
			return 0;
	}
}

namespace API::OpenGL
{
	static const uint32_t s_MaxFramebufferSize = 8192;

	OpenGLFramebuffer::OpenGLFramebuffer(const Kargono::FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		// For Loop organizes specifications into color or depth attachments that are held
		//		locally
		for (auto spec : m_Specification.Attachments.Attachments)
		{
			if (!Utility::IsDepthFormat(spec.DataFormat))
			{
				m_ColorAttachmentSpecifications.emplace_back(spec);
			}
			else
			{
				m_DepthAttachmentSpecification = spec;
			}
		}
		// Call Invalidate to instantiate framebuffer with provided spec
		Invalidate();
	}
	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(static_cast<GLsizei>(m_ColorAttachmentIDs.size()), m_ColorAttachmentIDs.data());
		glDeleteTextures(1, &m_DepthAttachmentID);

	}
	void OpenGLFramebuffer::Invalidate()
	{
		// Clear previous framebuffer if one already exists for this object.
		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(static_cast<GLsizei>(m_ColorAttachmentIDs.size()), m_ColorAttachmentIDs.data());
			glDeleteTextures(1, &m_DepthAttachmentID);

			m_ColorAttachmentIDs.clear();
			m_DepthAttachmentID = 0;
		}

		// Instantiate FrameBuffer in OpenGL and Bind It
		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		bool multisample = m_Specification.Samples > 1;

		// Generate Color Attachments if specified
		// Attachments
		if (m_ColorAttachmentSpecifications.size())
		{
			m_ColorAttachmentIDs.resize(m_ColorAttachmentSpecifications.size());
			Utility::CreateTextures(multisample, m_ColorAttachmentIDs.data(), static_cast<uint32_t>(m_ColorAttachmentIDs.size()));

			for (size_t i = 0; i < m_ColorAttachmentIDs.size(); i++)
			{
				Utility::BindTexture(multisample, m_ColorAttachmentIDs[i]);
				switch (m_ColorAttachmentSpecifications[i].DataFormat)
				{
				case Kargono::FramebufferDataFormat::RGBA8:
					Utility::AttachColorTexture(m_ColorAttachmentIDs[i], m_Specification.Samples, GL_RGBA8, GL_RGBA,
					                          m_Specification.Width, m_Specification.Height, static_cast<int>(i));
					break;
				case Kargono::FramebufferDataFormat::RED_INTEGER:
					Utility::AttachColorTexture(m_ColorAttachmentIDs[i], m_Specification.Samples, GL_R32I, GL_RED_INTEGER,
						m_Specification.Width, m_Specification.Height, static_cast<int>(i));
					break;
				}
			}
		}
		// Generate Depth Attachments if specified
		if (m_DepthAttachmentSpecification.DataFormat != Kargono::FramebufferDataFormat::None)
		{
			Utility::CreateTextures(multisample, &m_DepthAttachmentID, 1);
			Utility::BindTexture(multisample, m_DepthAttachmentID);
			switch (m_DepthAttachmentSpecification.DataFormat)
			{
			case Kargono::FramebufferDataFormat::DEPTH24STENCIL8:
				Utility::AttachDepthTexture(m_DepthAttachmentID, m_Specification.Samples,
					GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT,
					m_Specification.Width, m_Specification.Height);

			}
		}

		// This section specifies the buffers to be drawn to during a render pass.
		if (m_ColorAttachmentIDs.size() > 1)
		{
			KG_CORE_ASSERT(m_ColorAttachmentIDs.size() <= 4, "Core error, we only support a maximum of 4 color attachments currently");
			GLenum buffers[4] = {
				GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3
			};
			glDrawBuffers(static_cast<GLsizei>(m_ColorAttachmentIDs.size()), buffers);
		}
		else if (m_ColorAttachmentIDs.empty())
		{
			// Only depth-pass
			glDrawBuffer(GL_NONE);
		}

		// Final check to ensure framebuffer is created successfully
		KG_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}
	void OpenGLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);

	}
	void OpenGLFramebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
		{
			KG_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;

		Invalidate();
	}
	int32_t OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int32_t x, int32_t y)
	{
		KG_CORE_ASSERT(attachmentIndex < m_ColorAttachmentIDs.size(), "Color attachment selection is out of bounds!");
		KG_CORE_ASSERT(m_ColorAttachmentSpecifications.at(attachmentIndex).DataFormat == Kargono::FramebufferDataFormat::RED_INTEGER, "Attempt to use ReadPixel on a buffer that is not RED_INTEGER");
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		int pixelData;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
		return pixelData;
	}
	void OpenGLFramebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
		KG_CORE_ASSERT(attachmentIndex < m_ColorAttachmentIDs.size(), "Color attachment selection is out of bounds!")

		auto& spec = m_ColorAttachmentSpecifications[attachmentIndex];

		glClearTexImage(m_ColorAttachmentIDs[attachmentIndex], 0,
			Utility::KargonoTextureFormatToGL(spec.DataFormat), GL_INT, &value);
	}
}
