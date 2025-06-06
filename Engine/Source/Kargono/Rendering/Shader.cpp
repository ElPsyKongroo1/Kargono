#include "kgpch.h"

#include "Kargono/Rendering/Shader.h"
#include "Kargono/Rendering/RenderingService.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/Utility/FileSystem.h"

#include "API/RenderingAPI/OpenGLShader.h"

namespace Kargono::Rendering
{
	void RendererInputSpec::ClearData()
	{
		m_Shader = nullptr;
		m_Texture = nullptr;
		if (m_ShapeComponent)
		{
			m_ShapeComponent->Texture = nullptr;
			m_ShapeComponent->Shader = nullptr;
		}
		m_CurrentDrawBuffer = nullptr;
	}

	Ref<Shader> Shader::Create(const std::string& name, const std::unordered_map<GLenum, std::vector<uint32_t>>& shaderBinaries)
	{
#ifdef KG_RENDERER_OPENGL
		return  CreateRef<API::RenderingAPI::OpenGLShader>(name, shaderBinaries);
#endif
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

		auto quadVertexBuffer = VertexBuffer::Create(RenderingService::s_MaxVertexBufferSize);
		quadVertexBuffer->SetLayout(m_InputBufferLayout);
		m_VertexArray->AddVertexBuffer(quadVertexBuffer);
	}



	void Shader::FillRenderFunctionList()
	{
		if (m_ShaderSpecification.RenderType == RenderingType::DrawLine)
		{
			m_DrawFunctions.push_back(RenderingService::DrawBufferLine);
		}
		if (m_ShaderSpecification.RenderType == RenderingType::DrawPoint)
		{
			m_DrawFunctions.push_back(RenderingService::DrawBufferPoints);
		}

		if (m_ShaderSpecification.RenderType == RenderingType::DrawLine ||
			m_ShaderSpecification.RenderType == RenderingType::DrawPoint ||
			m_ShaderSpecification.TextureInput == TextureInputType::TextTexture)
		{
			m_FillDataPerVertex.push_back(RenderingService::FillWorldPositionNoTransform);
		}
		else
		{
			m_FillDataPerVertex.push_back(RenderingService::FillWorldPosition);
		}

		if (m_ShaderSpecification.AddEntityID)
		{
			m_FillDataInScene.push_back(RenderingService::FillEntityID);
		}

		if (m_ShaderSpecification.TextureInput == TextureInputType::ColorTexture)
		{
			m_FillDataPerObject.push_back(RenderingService::FillTextureIndex);
			m_FillDataPerVertex.push_back(RenderingService::FillTextureCoordinate);
			m_SubmitUniforms.push_back(RenderingService::FillTextureUniform);
		}

		if (m_ShaderSpecification.TextureInput == TextureInputType::TextTexture)
		{

			m_FillDataPerObject.push_back(RenderingService::FillTextureAtlas);
			m_FillDataPerVertex.push_back(RenderingService::FillTextureCoordinate);
			m_SubmitUniforms.push_back(RenderingService::FillTextureUniform);
		}

		if (m_ShaderSpecification.ColorInput == ColorInputType::VertexColor)
		{
			m_FillDataPerVertex.push_back(RenderingService::FillVertexColor);
		}

		if (m_ShaderSpecification.AddCircleShape)
		{
			m_FillDataPerVertex.push_back(RenderingService::FillLocalPosition);
		}

		if (m_ShaderSpecification.RenderType == RenderingType::DrawIndex)
		{
			m_FillDataPerObject.push_back(RenderingService::FillIndicesData);
			m_DrawFunctions.push_back(RenderingService::DrawBufferIndices);
		}

		if (m_ShaderSpecification.RenderType == RenderingType::DrawTriangle)
		{
			m_DrawFunctions.push_back(RenderingService::DrawBufferTriangles);
		}

		static_assert(sizeof(ShaderSpecification) == sizeof(uint8_t) * 20);
	}
}
