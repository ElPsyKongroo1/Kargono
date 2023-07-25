#pragma once
#include <string>
#include <glm/glm.hpp>

namespace Kargono
{
/// @class Shader
	class Shader
	{
	public:
/// @brief Constructor for the Shader class
		Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
/// @brief Destructor for the Shader class
		~Shader();

	public:
/// @brief Binds the shader for rendering
		void Bind() const;
/// @brief Unbinds the shader
		void Unbind() const;

/// @brief Uploads a 4x4 matrix uniform to the shader
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
	private:
		uint32_t m_RendererID;

	};

}