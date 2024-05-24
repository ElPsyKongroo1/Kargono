#include "kgpch.h"

#include "Kargono/Renderer/Shader.h"
#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Scene/Scene.h"
#include "Kargono/Utility/FileSystem.h"

#include "API/RenderingAPI/OpenGLShader.h"

namespace Kargono
{
	Ref<Shader> Shader::Create(const std::string& name, const std::unordered_map<GLenum, std::vector<uint32_t>>& shaderBinaries)
	{
		return  CreateRef<API::OpenGL::OpenGLShader>(name, shaderBinaries);
	}

	void Shader::SetSpecification(const ShaderSpecification& shaderSpec)
	{
		m_ShaderSpecification = shaderSpec;
		FillRenderFunctionList();
	}

	void Shader::SetInputLayout(const InputBufferLayout& shaderInputLayout)
	{
		m_InputBufferLayout = shaderInputLayout;
		m_VertexArray = VertexArray::Create();

		auto quadVertexBuffer = VertexBuffer::Create(Renderer::s_MaxVertexBufferSize);
		quadVertexBuffer->SetLayout(m_InputBufferLayout);
		m_VertexArray->AddVertexBuffer(quadVertexBuffer);
	}



	void Shader::FillRenderFunctionList()
	{

		if (m_ShaderSpecification.RenderType == RenderingType::DrawLine)
		{
			m_DrawFunctions.push_back(Renderer::DrawBufferLine);
		}

		if (m_ShaderSpecification.RenderType == RenderingType::DrawLine ||
			m_ShaderSpecification.TextureInput == TextureInputType::TextTexture)
		{
			m_FillDataPerVertex.push_back(Renderer::FillWorldPositionNoTransform);
		}
		else
		{
			m_FillDataPerVertex.push_back(Renderer::FillWorldPosition);
		}

		if (m_ShaderSpecification.AddEntityID)
		{
			m_FillDataInScene.push_back(Scene::FillEntityID);
		}
		if (m_ShaderSpecification.TextureInput == TextureInputType::ColorTexture ||
			m_ShaderSpecification.TextureInput == TextureInputType::TextTexture)
		{
			m_FillDataPerObject.push_back( Renderer::FillTextureIndex);
			m_FillDataPerVertex.push_back(Renderer::FillTextureCoordinate);
			m_SubmitUniforms.push_back(Renderer::FillTextureUniform);
		}

		if (m_ShaderSpecification.ColorInput == ColorInputType::VertexColor)
		{
			m_FillDataPerVertex.push_back(Renderer::FillVertexColor);
		}

		if (m_ShaderSpecification.AddCircleShape)
		{
			m_FillDataPerVertex.push_back(Renderer::FillLocalPosition);
		}

		if (m_ShaderSpecification.RenderType == RenderingType::DrawIndex)
		{
			m_FillDataPerObject.push_back(Renderer::FillIndicesData);
			m_DrawFunctions.push_back(Renderer::DrawBufferIndices);
		}

		if (m_ShaderSpecification.RenderType == RenderingType::DrawTriangle)
		{
			m_DrawFunctions.push_back(Renderer::DrawBufferTriangles);
		}

		KG_ASSERT(sizeof(ShaderSpecification) == sizeof(uint8_t) * 20, "Please Update Render section in Shader Code! It looks like you updated the shaderspecification");
	}
}
