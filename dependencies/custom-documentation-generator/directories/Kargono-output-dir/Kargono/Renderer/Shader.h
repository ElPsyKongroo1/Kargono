#pragma once

namespace Kargono
{
	class Shader
	{
/// @brief Public member functions
/// @brief Public member functions
	public:
/// @brief Constructor for Shader class
/// @param vertexSrc Source code of the vertex shader
/// @param fragmentSrc Source code of the fragment shader
		Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
/// @brief Destructor for Shader class
		~Shader();

	public:
/// @brief Bind the shader
		void Bind() const;
/// @brief Unbind the shader
		void Unbind() const;
/// @brief Private member variables
	private:
/// @brief ID of the shader renderer
		uint32_t m_RendererID;

	};

}