#include "kgpch.h"

#include "Modules/Rendering/Shader.h"
#include "Modules/Rendering/RenderingService.h"
#include "Kargono/Scenes/Scene.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Modules/Rendering/Components/ShapeComponent.h"
#include "Modules/Rendering/ExternalAPI/OpenGLShader.h"

namespace Kargono::Rendering
{
	void RendererInputSpec::ClearData()
	{
		m_Shader = nullptr;
		m_Texture = nullptr;
		if (m_ShapeComponent)
		{
			m_ShapeComponent->m_Texture = nullptr;
			m_ShapeComponent->m_Shader = nullptr;
		}
		m_CurrentDrawBuffer = nullptr;
	}

	ShaderSpecification::ShaderSpecification(ColorInputType colorInput, TextureInputType textureInput,
		bool addCircle, bool addProjection, bool addEntityID,
		RenderingType renderType, bool drawOutline)
		: m_ColorInput(colorInput), m_TextureInput(textureInput), m_AddCircleShape(addCircle), 
		m_AddProjectionMatrix(addProjection), m_AddEntityID(addEntityID), 
		m_RenderType(renderType), m_DrawOutline(drawOutline)
	{
	}


	Shader::Shader(const ShaderSpecification& shader) : m_ShaderSpecification(shader) {}

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

		auto quadVertexBuffer = VertexBuffer::Create(k_MaxVertexBufferSize);
		quadVertexBuffer->SetLayout(m_InputBufferLayout);
		m_VertexArray->AddVertexBuffer(quadVertexBuffer);
	}



	void Shader::FillRenderFunctionList()
	{
		if (m_ShaderSpecification.m_RenderType == RenderingType::DrawLine)
		{
			m_DrawFunctions.push_back(RenderingService::DrawBufferLine);
		}
		if (m_ShaderSpecification.m_RenderType == RenderingType::DrawPoint)
		{
			m_DrawFunctions.push_back(RenderingService::DrawBufferPoints);
		}

		if (m_ShaderSpecification.m_RenderType == RenderingType::DrawLine ||
			m_ShaderSpecification.m_RenderType == RenderingType::DrawPoint ||
			m_ShaderSpecification.m_TextureInput == TextureInputType::TextTexture)
		{
			m_FillDataPerVertex.push_back(RenderingService::FillWorldPositionNoTransform);
		}
		else
		{
			m_FillDataPerVertex.push_back(RenderingService::FillWorldPosition);
		}

		if (m_ShaderSpecification.m_AddEntityID)
		{
			m_FillDataInScene.push_back(RenderingService::FillEntityID);
		}

		if (m_ShaderSpecification.m_TextureInput == TextureInputType::ColorTexture)
		{
			m_FillDataPerObject.push_back(RenderingService::FillTextureIndex);
			m_FillDataPerVertex.push_back(RenderingService::FillTextureCoordinate);
			m_SubmitUniforms.push_back(RenderingService::FillTextureUniform);
		}

		if (m_ShaderSpecification.m_TextureInput == TextureInputType::TextTexture)
		{

			m_FillDataPerObject.push_back(RenderingService::FillTextureAtlas);
			m_FillDataPerVertex.push_back(RenderingService::FillTextureCoordinate);
			m_SubmitUniforms.push_back(RenderingService::FillTextureUniform);
		}

		if (m_ShaderSpecification.m_ColorInput == ColorInputType::VertexColor)
		{
			m_FillDataPerVertex.push_back(RenderingService::FillVertexColor);
		}

		if (m_ShaderSpecification.m_AddCircleShape)
		{
			m_FillDataPerVertex.push_back(RenderingService::FillLocalPosition);
		}

		if (m_ShaderSpecification.m_RenderType == RenderingType::DrawIndex)
		{
			m_FillDataPerObject.push_back(RenderingService::FillIndicesData);
			m_DrawFunctions.push_back(RenderingService::DrawBufferIndices);
		}

		if (m_ShaderSpecification.m_RenderType == RenderingType::DrawTriangle)
		{
			m_DrawFunctions.push_back(RenderingService::DrawBufferTriangles);
		}

		static_assert(sizeof(ShaderSpecification) == sizeof(uint8_t) * 20);
	}
}
