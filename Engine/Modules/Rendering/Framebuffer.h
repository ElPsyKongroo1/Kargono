#pragma once

#include "Kargono/Core/Base.h"

namespace Kargono::Rendering
{
	enum class FramebufferDataFormat
	{
		None = 0,
		// Color
		RGBA8,
		RED_INTEGER,
		// Depth/stencil
		DEPTH24STENCIL8,
		// Defaults
		Depth = DEPTH24STENCIL8
	};

	struct FramebufferDataSpecification
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		FramebufferDataSpecification() = default;
		FramebufferDataSpecification(FramebufferDataFormat format)
			: m_DataFormat(format) {}
	public:
		//==============================
		// Public Fields
		//==============================
		FramebufferDataFormat m_DataFormat{ FramebufferDataFormat::None };
		//TODO: filtering/wrap
	};

	struct FramebufferAttachmentSpecification
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(std::initializer_list<FramebufferDataSpecification> attachments)
			: m_Attachments(attachments) {}
	public:
		//==============================
		// Public Fields
		//==============================
		std::vector<FramebufferDataSpecification> m_Attachments;
	};

	struct FramebufferSpecification
	{
		uint32_t m_Width{ 0 };
		uint32_t m_Height{ 0 };
		FramebufferAttachmentSpecification m_Attachments;
		uint32_t m_Samples{ 1 };
		bool m_SwapChainTarget{ false }; // Are we rendering to the screen? false is no. true is yes!
	};

	class Framebuffer
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Framebuffer() = default;
		~Framebuffer()
		{
			if (m_Registered)
			{
				DeregisterBuffer();
			}
		}
	public:
		//==============================
		// Interact With Renderer
		//==============================
		// Register framebuffer w/ renderer
		void RegisterBuffer(const FramebufferSpecification& spec);
		void DeregisterBuffer();
		// Bind w/ OpenGL state machine
		void Bind();
		void Unbind();
		// Display
		void DisplayToDefaultFrameBuffer();
		// Configure buffer
		void Resize(uint32_t width, uint32_t height);
		void SetAttachment(uint32_t attachmentIndex, int value);
		// Get pixel data
		int ReadPixel(uint32_t attachmentIndex, int x, int y);
	private:
		//==============================
		// Internal Functionality
		//==============================
		void Invalidate();
	public:
		//==============================
		// Getters/Setters
		//==============================
		uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const;
		const FramebufferSpecification& GetSpecification() const { return m_Specification; }
	private:
		uint32_t m_RendererID{ 0 };
		bool m_Registered{ false };
		FramebufferSpecification m_Specification;
		std::vector<FramebufferDataSpecification> m_ColorAttachmentSpecifications;
		FramebufferDataSpecification m_DepthAttachmentSpecification{ FramebufferDataFormat::None };
		std::vector<uint32_t> m_ColorAttachmentIDs;
		uint32_t m_DepthAttachmentID{ 0 };
	};

	class FrameBufferService
	{
	public:
		static void Init();

	public:
		static unsigned int GetScreenSpaceQuadVAO();
		static unsigned int GetScreenSpaceQuadVBO();
		static unsigned int GetScreenSpaceShaderProgram();

	private:
		static inline unsigned int s_ScreenSpaceQuadVAO;
		static inline unsigned int s_ScreenSpaceQuadVBO;
		static inline unsigned int s_ScreenSpaceShaderProgram;
	};
}
