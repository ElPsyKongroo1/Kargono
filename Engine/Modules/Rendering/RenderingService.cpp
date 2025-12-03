#include "kgpch.h"

#include "Modules/Rendering/RenderingService.h"
#include "Modules/Rendering/Assets/Shader.h"
#include "Modules/Rendering/Assets/Texture2D.h"
#include "Modules/Rendering/VertexArray.h"
#include "Modules/Rendering/UniformBuffer.h"
#include "Kargono/Projects/Project.h"
#include "Modules/Scenes/Assets/Scene.h"
#include "Modules/Rendering/Components/ShapeComponent.h"

namespace Kargono::Rendering
{
	constexpr uint32_t k_MaxQuads{ 20000 };
	constexpr uint32_t k_MaxVertices = k_MaxQuads * 4;
	constexpr uint32_t k_MaxIndices = k_MaxQuads * 6;
	constexpr uint32_t k_MaxTextureSlots = 32;
	constexpr uint32_t k_MaxIndicesBuffer = 1000;

	struct CameraData
	{
		Math::mat4 m_ViewProjection;
	};

	struct RendererData
	{
	public:
		float m_LineWidth = 4.0f;
		float m_PointWidth = 8.0f;
		Statistics m_Stats;
		CameraData m_CameraBuffer;
		UniformBuffer m_CameraUniformBuffer;
		std::vector<Ref<DrawCallBuffer>> m_DrawCalls;
	};

	static RendererData s_Data;

	void RenderingService::OnWindowResize(uint32_t width, uint32_t height)
	{
		RendererAPI::SetViewport(0, 0, width, height);
	}

	void RenderingService::Init()
	{
		s_Data.m_CameraUniformBuffer.RegisterBuffer(sizeof(CameraData), 0);
		KG_VERIFY(s_Data.m_CameraUniformBuffer.IsRegistered(), "Renderer Init")
	}
	void RenderingService::Shutdown()
	{
		s_Data.m_CameraUniformBuffer.DeregisterBuffer();
		s_Data.m_DrawCalls.clear();
	}
	void RenderingService::BeginScene(const Cameras::CameraProjection& camera, const Math::mat4& transform)
	{
		s_Data.m_CameraBuffer.m_ViewProjection = camera.GetProjection() * transform;
		s_Data.m_CameraUniformBuffer.SetData(&s_Data.m_CameraBuffer, sizeof(CameraData));
	}
	void RenderingService::BeginScene(const Cameras::PerspectiveCamera& camera)
	{
		s_Data.m_CameraBuffer.m_ViewProjection = camera.GetViewProjection();
		s_Data.m_CameraUniformBuffer.SetData(&s_Data.m_CameraBuffer, sizeof(CameraData));
	}

	void RenderingService::BeginScene(const Math::mat4 projection)
	{
		s_Data.m_CameraBuffer.m_ViewProjection = projection;
		s_Data.m_CameraUniformBuffer.SetData(&s_Data.m_CameraBuffer, sizeof(CameraData));
	}

	void RenderingService::EndScene()
	{
		FlushBuffers();
	}

	float RenderingService::GetLineWidth()
	{
		return s_Data.m_LineWidth;
	}
	void RenderingService::SetLineWidth(float width)
	{
		s_Data.m_LineWidth = width;
	}

	void RenderingService::ResetStats()
	{
		memset(&s_Data.m_Stats, 0, sizeof(Statistics));
	}
	Statistics RenderingService::GetStats()
	{
		return s_Data.m_Stats;
	}

	void RenderingService::FillTextureIndex(RendererInputSpec* inputSpec)
	{
		//if (s_Data.m_QuadIndexCount >= RendererData::MaxIndices) { NextBatch(); }
		KG_ASSERT(inputSpec->m_ShapeComponent->m_Texture, "Texture shader added, however, no texture is available in ShapeComponent.");
		auto& m_Textures = inputSpec->m_CurrentDrawBuffer->m_Textures;
		float textureIndex = -1.0f;

		uint32_t iteration = 0;
		for (auto& texture : m_Textures)
		{
			if (texture == inputSpec->m_ShapeComponent->m_Texture)
			{
				textureIndex = (float)iteration;
				break;
			}
			iteration++;
		}
		if (static_cast<int32_t>(textureIndex) == -1)
		{
			if (m_Textures.size() >= k_MaxTextureSlots)
			{
				textureIndex = 0.0f;
				// TODO: NextBatch, Create a new DrawCallBuffer for the current shader and update Textures Ref
			}
			m_Textures.push_back(inputSpec->m_ShapeComponent->m_Texture.GetAssetRef());
			textureIndex = static_cast<float>(m_Textures.size() - 1);
		}

		Shader::SetDataAtInputLocation<float>(textureIndex, 
			Utility::FileSystem::CRCFromString("a_TexIndex"),
			inputSpec->m_Buffer, inputSpec->m_Shader.GetAssetRef());
	}

	void RenderingService::FillTextureAtlas(RendererInputSpec* inputSpec)
	{
		//if (s_Data.m_QuadIndexCount >= RendererData::MaxIndices) { NextBatch(); }
		KG_ASSERT(inputSpec->m_ShapeComponent->m_Texture, "Texture shader added, however, no texture is available in ShapeComponent.");
		std::vector<Assets::AssetRef<Texture2D>>& m_Textures = inputSpec->m_CurrentDrawBuffer->m_Textures;
		m_Textures.clear();
		m_Textures.emplace_back(inputSpec->m_ShapeComponent->m_Texture.GetAssetRef());
	}

	void RenderingService::FillTextureCoordinate(RendererInputSpec* inputSpec, uint32_t iteration)
	{
		const Math::vec2& coordinates = inputSpec->m_ShapeComponent->m_TextureCoordinates->at(iteration);
		Shader::SetDataAtInputLocation<Math::vec2>(coordinates, 
			Utility::FileSystem::CRCFromString("a_TexCoord"),
			inputSpec->m_Buffer,
			inputSpec->m_Shader.GetAssetRef());
	}

	void RenderingService::FillLocalPosition(RendererInputSpec* inputSpec, uint32_t iteration)
	{
		const Math::vec3& localPosition = inputSpec->m_ShapeComponent->m_Vertices->at(iteration) * 2.0f;
		Shader::SetDataAtInputLocation<Math::vec3>(localPosition, 
			Utility::FileSystem::CRCFromString("a_LocalPosition"),
			inputSpec->m_Buffer, inputSpec->m_Shader.GetAssetRef());
	}

	void RenderingService::FillWorldPosition(RendererInputSpec* inputSpec, uint32_t iteration)
	{
		const Math::vec3& localPosition = inputSpec->m_ShapeComponent->m_Vertices->at(iteration);
		Math::vec3 worldPosition = inputSpec->m_TransformMatrix * Math::vec4(localPosition, 1.0f);
		Shader::SetDataAtInputLocation<Math::vec3>(worldPosition, 
			Utility::FileSystem::CRCFromString("a_Position"),
			inputSpec->m_Buffer, inputSpec->m_Shader.GetAssetRef());
	}


	void RenderingService::FillWorldPositionNoTransform(RendererInputSpec* inputSpec, uint32_t iteration)
	{
		const Math::vec3& localPosition = inputSpec->m_ShapeComponent->m_Vertices->at(iteration);

		Shader::SetDataAtInputLocation<Math::vec3>(localPosition, 
			Utility::FileSystem::CRCFromString("a_Position"),
			inputSpec->m_Buffer, inputSpec->m_Shader.GetAssetRef());
	}

	void RenderingService::FillVertexColor(RendererInputSpec* inputSpec, uint32_t iteration)
	{
		auto& colorVector = inputSpec->m_ShapeComponent->m_VertexColors;
		KG_ASSERT(iteration < static_cast<uint32_t>(colorVector->size()), "Invalid iteration inside FillVertexColor function");
		Shader::SetDataAtInputLocation<Math::vec4>(colorVector->at(iteration), 
			Utility::FileSystem::CRCFromString("a_Color"),
			inputSpec->m_Buffer, inputSpec->m_Shader.GetAssetRef());
	}

	void RenderingService::FillIndicesData(RendererInputSpec* inputSpec)
	{
		// Upload Indices
		Ref<DrawCallBuffer> drawCallBuffer = inputSpec->m_Shader->GetCurrentDrawCallBuffer();
		std::size_t currentBufferSize = (drawCallBuffer->m_VertexBufferIterator - drawCallBuffer->m_VertexBuffer.m_Data) / inputSpec->m_Shader->GetInputLayout().GetStride();
		for (auto& index : *(inputSpec->m_ShapeComponent->m_Indices))
		{
			drawCallBuffer->m_IndexBuffer.push_back(static_cast<uint32_t>(currentBufferSize) + index);
		}
	}

	void RenderingService::FillEntityID(Rendering::RendererInputSpec* inputSpec)
	{
		Shader::SetDataAtInputLocation<uint32_t>(inputSpec->m_Entity, 
			Utility::FileSystem::CRCFromString("a_EntityID"),
			inputSpec->m_Buffer, inputSpec->m_Shader.GetAssetRef());
	}

	void RenderingService::SubmitDataToRenderer(RendererInputSpec& inputSpec)
	{
		if (!inputSpec.m_ShapeComponent->m_Vertices || inputSpec.m_Shader->GetSpecification().m_RenderType == RenderingType::None) { return; }

		Ref<DrawCallBuffer> drawCallBuffer = inputSpec.m_Shader->GetCurrentDrawCallBuffer();

		// Create new DrawCallBuffer if one is not associated with active shader
		if (!drawCallBuffer)
		{
			drawCallBuffer = CreateRef<DrawCallBuffer>();
			drawCallBuffer->m_VertexBuffer.Allocate(k_MaxVertexBufferSize);
			drawCallBuffer->m_VertexBufferIterator = drawCallBuffer->m_VertexBuffer.m_Data;
			if (inputSpec.m_Shader->GetSpecification().m_RenderType == RenderingType::DrawIndex)
			{
				drawCallBuffer->m_IndexBuffer.reserve(k_MaxIndicesBuffer);
			}
			drawCallBuffer->m_Textures.reserve(k_MaxTextureSlots);
			drawCallBuffer->m_Shader = &inputSpec.m_Shader.GetAsset();
			s_Data.m_DrawCalls.emplace_back(drawCallBuffer);
			inputSpec.m_Shader->SetCurrentDrawCallBuffer(drawCallBuffer);
		}

		std::size_t currentBufferSize = drawCallBuffer->m_VertexBufferIterator - drawCallBuffer->m_VertexBuffer.m_Data;
		std::size_t sizeOfNewDrawCallBuffer = inputSpec.m_Buffer.m_Size * inputSpec.m_ShapeComponent->m_Vertices->size() + currentBufferSize;
		// Create new DrawCallBuffer if current buffer overflows
		if (sizeOfNewDrawCallBuffer >= k_MaxVertexBufferSize)
		{
			drawCallBuffer = CreateRef<DrawCallBuffer>();
			drawCallBuffer->m_VertexBuffer.Allocate(k_MaxVertexBufferSize);
			drawCallBuffer->m_VertexBufferIterator = drawCallBuffer->m_VertexBuffer.m_Data;
			if (inputSpec.m_Shader->GetSpecification().m_RenderType == RenderingType::DrawIndex)
			{
				drawCallBuffer->m_IndexBuffer.reserve(k_MaxIndicesBuffer);
			}
			drawCallBuffer->m_Textures.reserve(k_MaxTextureSlots);
			drawCallBuffer->m_Shader = &inputSpec.m_Shader.GetAsset();
			s_Data.m_DrawCalls.emplace_back(drawCallBuffer);
			inputSpec.m_Shader->SetCurrentDrawCallBuffer(drawCallBuffer);
		}

		inputSpec.m_CurrentDrawBuffer = drawCallBuffer;

		for (const auto& PerObjectFunction : inputSpec.m_Shader->GetFillDataObject())
		{
			PerObjectFunction(&inputSpec);
		}

		for (uint32_t iteration {0}; iteration < inputSpec.m_ShapeComponent->m_Vertices->size(); iteration++)
		{
			for (const auto& PerVertexFunction : inputSpec.m_Shader->GetFillDataVertex())
			{
				PerVertexFunction(&inputSpec, iteration);
			}
			memcpy(inputSpec.m_CurrentDrawBuffer->m_VertexBufferIterator, inputSpec.m_Buffer.m_Data, inputSpec.m_Buffer.m_Size);
			inputSpec.m_CurrentDrawBuffer->m_VertexBufferIterator += inputSpec.m_Buffer.m_Size;
			s_Data.m_Stats.m_VertexCount++;
		}

		
	}

	void RenderingService::FillTextureUniform(Ref<DrawCallBuffer> buffer)
	{
		for (uint32_t i = 0; i < buffer->m_Textures.size(); i++) 
		{ 
			buffer->m_Textures[i]->Bind(i); 
		}
	}

	void RenderingService::DrawBufferIndices(Ref<DrawCallBuffer> buffer)
	{
		RendererAPI::DrawIndexed(buffer->m_Shader->GetVertexArray(), buffer->m_IndexBuffer.data(), static_cast<uint32_t>(buffer->m_IndexBuffer.size()));
		s_Data.m_Stats.m_DrawCalls++;
	}

	void RenderingService::DrawBufferPoints(Ref<DrawCallBuffer> buffer)
	{
		RendererAPI::SetPointWidth(s_Data.m_PointWidth);
		RendererAPI::DrawPoints(buffer->m_Shader->GetVertexArray(), static_cast<std::uint32_t>(buffer->m_VertexBufferIterator - buffer->m_VertexBuffer.m_Data) / buffer->m_Shader->GetInputLayout().GetStride());
		s_Data.m_Stats.m_DrawCalls++;
	}

	void RenderingService::DrawBufferLine(Ref<DrawCallBuffer> buffer)
	{
		RendererAPI::SetLineWidth(s_Data.m_LineWidth);
		RendererAPI::DrawLines(buffer->m_Shader->GetVertexArray(), static_cast<std::uint32_t>(buffer->m_VertexBufferIterator - buffer->m_VertexBuffer.m_Data) / buffer->m_Shader->GetInputLayout().GetStride());
		s_Data.m_Stats.m_DrawCalls++;
	}

	void RenderingService::DrawBufferTriangles(Ref<DrawCallBuffer> buffer)
	{
		RendererAPI::DrawTriangles(buffer->m_Shader->GetVertexArray(), static_cast<std::uint32_t>(buffer->m_VertexBufferIterator - buffer->m_VertexBuffer.m_Data) / buffer->m_Shader->GetInputLayout().GetStride());
		s_Data.m_Stats.m_DrawCalls++;
	}

	void RenderingService::FlushBuffers()
	{
		auto& allBuffers = s_Data.m_DrawCalls;

		// Submit all Buffers to DrawCalls!
		for (auto buffer : allBuffers)
		{
			for (const auto& preDrawFunction : buffer->m_Shader->GetPreDrawBuffer())
			{
				preDrawFunction(buffer);
			}

			buffer->m_Shader->Bind();
			uint32_t dataSize = static_cast<uint32_t>(buffer->m_VertexBufferIterator - buffer->m_VertexBuffer.m_Data);
			buffer->m_Shader->GetVertexArray().GetVertexBuffers().at(0).SetData(buffer->m_VertexBuffer.m_Data, dataSize);

			// Submit Per Buffer Uniforms
			for (const auto& uniformFunction : buffer->m_Shader->GetSubmitUniforms())
			{
				uniformFunction(buffer);
			}

			// Final Draw Call Functions
			for (const auto& drawFunction : buffer->m_Shader->GetDrawFunctions())
			{
				drawFunction(buffer);
			}

			for (const auto& postDrawFunction : buffer->m_Shader->GetPostDrawBuffer())
			{
				postDrawFunction(buffer);
			}
			
		}

		// Clear current Buffers inside each shader!
		for (auto& buffer : allBuffers)
		{
			if (buffer->m_Shader->GetCurrentDrawCallBuffer())
			{
				buffer->m_Shader->ClearCurrentDrawCallBuffer();
			}
		}

		// Clear current Buffers in Renderer
		for (auto& buffer : allBuffers)
		{
			if (buffer->m_VertexBuffer)
			{
				buffer->m_VertexBuffer.Release();
			}
		}
		allBuffers.clear();
	}
	
}
