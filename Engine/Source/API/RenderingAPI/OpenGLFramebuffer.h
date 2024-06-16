#pragma once

#include "Kargono/Rendering/Framebuffer.h"

#ifdef KG_RENDERER_OPENGL

namespace API::RenderingAPI
{
	//============================================================
	// OpenGL FrameBuffer Class
	//============================================================
	// This class represents a framebuffer, which is a construct inside OpenGL
	//		that represents data that matches the dimensions of the OpenGL viewport.
	//		This data can include color buffers (RGBA), depth buffers (Variable Size, typically 24bytes),
	//		a stencil buffers(1 Byte), and others... An single framebuffer can hold multiple
	//		datasets(colorbuffers, depthbuffers, etc...).
	class OpenGLFramebuffer : public Kargono::Rendering::Framebuffer
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This constructor stores the provided spec, sorts the FramebufferAttachments
		//		into the locally held m_ColorAttachment and m_DepthAttachment, and
		//		calls the invalidate function to instantiate the framebuffer in OpenGL.
		OpenGLFramebuffer(const Kargono::Rendering::FramebufferSpecification& spec);
		// This destructor clears the framebuffer from the OpenGL context. This includes
		//		clearing the Framebuffer itself and clearing the color/depth buffers.
		virtual ~OpenGLFramebuffer();

		//==============================
		// Binding Functionality
		//==============================
		// Binds the framebuffer to the screen. This functionality allows the framebuffer to
		//		be created while unbound and displayed when bound.
		virtual void Bind() override;
		// This functions clears viewport and displays the default framebuffer that comes
		//		with the OpenGL context.
		virtual void Unbind() override;

		//==============================
		// External API Functionality
		//==============================
		// Function allows the framebuffer to be resized. This function changes the size in
		//		m_Specification and calls the Invalidate() method.
		virtual void Resize(uint32_t width, uint32_t height) override;
		// Function allows for a particular pixel to be read from the selected color buffer
		//		Ex: attachmentIndex(1) selects the second color buffer in m_ColorAttachments
		//		and x,y denote the pixel coordinate. The function returns the value of the pixel
		//		at the specified location.
		virtual int32_t ReadPixel(uint32_t attachmentIndex, int32_t x, int32_t y) override;
		// Function clears a specific color attachment with a specific clear value.
		//		Ex: An attachmentIndex(2) selects the second color attachment in m_ColorAttachmentIDs
		//		and clears its buffer to value(-1). 
		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

		//==============================
		// Getters/Setters
		//==============================
		// Function is used to get the specific color attachmentID for use in external functionality
		//		Ex. Currently the main use of this function is to get the color attachment for the
		//		main viewport buffer and present it as an image to ImGui to display in the editor
		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override
		{
			KG_ASSERT(index < m_ColorAttachmentIDs.size(), "Invalid attempted access");
			return m_ColorAttachmentIDs[index];
		}
		virtual const Kargono::Rendering::FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		//==============================
		// Internal Functionality
		//==============================
		// Invalidate is a function that gets called by many other public functions in this class.
		//		It takes the current m_Specification, m_ColorAttachmentSpecifications, and
		//		m_DepthAttachmentSpecification to generate the actual OpenGL color attachments, OpenGL depth attachment,
		//		and the final OpenGL Framebuffer. This function is fairly long, but manageable. Take a look at it yourself.
		void Invalidate();
	private:
		// m_RendererID represents the framebuffer instance in OpenGL.
		uint32_t m_RendererID = 0;
		// This specification describes the framebuffer attachments, width, height, multisample, etc...
		Kargono::Rendering::FramebufferSpecification m_Specification;
		// Color Specification that is obtained from m_Specification during object construction
		std::vector<Kargono::Rendering::FramebufferDataSpecification> m_ColorAttachmentSpecifications;
		// Depth specification that is obtained from m_Specification during object construction
		Kargono::Rendering::FramebufferDataSpecification m_DepthAttachmentSpecification = Kargono::Rendering::FramebufferDataFormat::None;
		// The next two variables hold references to the OpenGL context(s) of the color attachments and
		//		the depth buffer.
		std::vector<uint32_t> m_ColorAttachmentIDs; 
		uint32_t m_DepthAttachmentID = 0;
		
	};
}

#endif
