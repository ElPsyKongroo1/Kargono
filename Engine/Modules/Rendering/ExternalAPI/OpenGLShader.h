#pragma once

#include "Modules/Rendering/Shader.h"
#include "Kargono/Math/Math.h"

#ifdef KG_RENDERER_OPENGL

namespace API::RenderingAPI
{
	//============================================================
	// OpenGL Shader Class
	//============================================================
	// This class is the OpenGL representation of an active shader. This class is instantiated with
	//		precompiled binaries that represent the fragment, vertex, and optionally geometry shaders.
	//		These binaries are used to instantiate the OpenGL shader program and this class holds a
	//		reference to the OpenGL program with m_RendererID. Modifications to OpenGL uniforms are
	//		available here as well.
	class OpenGLShader : public Kargono::Rendering::Shader
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This constructor simply calls CreateProgram since most of the source code processing is done in the
		//		AssetManager. The CreateProgram call instantiates the shader program in OpenGL.
		OpenGLShader(std::string_view name, const std::unordered_map<Kargono::Rendering::GLenum, std::vector<uint32_t>>& shaderBinaries);
		// This destructor simply deletes the shader program in OpenGL
		virtual ~OpenGLShader();

		//==============================
		// Binding Functionality
		//==============================
		// This bind functionality allows this shader to be bound to the OpenGL context which allows subsequent
		//		render calls to use this shader
		virtual void Bind() const override;
		// This unbind functionality clears the shader from the OpenGL context and sets the OpenGL context shader
		//		section to null(or default, idk how OpenGL works internally)
		virtual void Unbind() const override;

		//==============================
		// Upload Uniforms
		//==============================
		// These functions allow uniforms to be uploaded to the GPU through the current shader program.
		//		Ex. Using the SetFloatUniform with name(position) and value(0.22f) will set the uniform
		//		position inside the current shader program to 0.22f. All subsequent uses of this shader
		//		will have that uniform set.
		virtual void SetIntUniform(const char* name, int value) override;
		virtual void SetIntArrayUniform(const char* name, int* values, uint32_t count) override;
		virtual void SetFloatUniform(const char* name, float value) override;
		virtual void SetFloat2Uniform(const char* name, const Kargono::Math::vec2& value) override;
		virtual void SetFloat3Uniform(const char* name, const Kargono::Math::vec3& value) override;
		virtual void SetFloat4Uniform(const char* name, const Kargono::Math::vec4& value) override;
		virtual void SetMat3Uniform(const char* name, const Kargono::Math::mat3& value) override;
		virtual void SetMat4Uniform(const char* name, const Kargono::Math::mat4& value) override;
	private:
		//==============================
		// Internal Functionality
		//==============================
		// This function takes the openGLSPIRV and creates the vertex shader, fragment shader,
		//		and links them together into a shader program. After the vertex and fragment shaders
		//		are linked they can be cleared. The shader program represents the active shader
		//		that exists in OpenGL and is identified by m_RendererID.
		void CreateProgram(const std::unordered_map<Kargono::Rendering::GLenum, std::vector<uint32_t>>& openGLSPIRV);
	private:
		// m_RendererID represents the OpenGL shader program associated with this object. Any OpenGL calls
		//		will use this ID
		uint32_t m_RendererID;
		// This name is present for debugging purposes
		std::string m_Name;
	};

}

#endif
