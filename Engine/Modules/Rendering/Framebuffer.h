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
		// Create Framebuffer
		//==============================
		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		FrameBuffer() = default;
		virtual ~Framebuffer() = default;
	public:
		//==============================
		// Interact With Renderer
		//==============================
		// Bind w/ context
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		// Display
		virtual void DisplayToDefaultFrameBuffer() = 0;
		// Configure buffer
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void SetAttachment(uint32_t attachmentIndex, int value) = 0;
		// Get pixel data
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;
	public:
		//==============================
		// Getters/Setters
		//==============================
		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
		virtual const FramebufferSpecification& GetSpecification() const = 0;

	};
}
