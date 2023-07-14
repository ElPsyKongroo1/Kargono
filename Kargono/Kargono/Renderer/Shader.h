#pragma once

namespace Kargono
{
	class Shader
	{
	public:
		Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
		~Shader();

	public:
		void Bind() const;
		void Unbind() const;
	private:
		uint32_t m_RendererID;

	};

}