#include "kgpch.h"

#include "Kargono/Rendering/RenderingService.h"
#include "Kargono/Rendering/Shader.h"
#include "Kargono/Rendering/Texture.h"
#include "Kargono/Rendering/VertexArray.h"
#include "Kargono/Rendering/UniformBuffer.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Scenes/Scene.h"

namespace Kargono::Rendering
{
	struct RendererData
	{
		static const uint32_t MaxQuads = 20000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;

		float LineWidth = 4.0f;
		float PointWidth = 8.0f;

		RenderingService::Statistics Stats;

		struct CameraData
		{
			Math::mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;

		// New Renderer
		// Maximum Vertex Buffer Size in Bytes
		static const uint32_t MaxTextureSlots = 32;
		static const uint32_t MaxIndicesBuffer = 1000;
		std::vector<Ref<DrawCallBuffer>> DrawCalls;
	};

	static RendererData s_Data;



	void RenderingService::OnWindowResize(uint32_t width, uint32_t height)
	{
		RendererAPI::SetViewport(0, 0, width, height);
	}

	void RenderingService::Init()
	{
		KG_ASSERT(Projects::ProjectService::GetActive(), "No valid project is active while trying to initialize shaders!");
		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(RendererData::CameraData), 0);
		KG_VERIFY(s_Data.CameraUniformBuffer, "Renderer Init")
	}
	void RenderingService::Shutdown()
	{
		s_Data.CameraUniformBuffer.reset();
		s_Data.DrawCalls.clear();
	}
	void RenderingService::BeginScene(const Camera& camera, const Math::mat4& transform)
	{
		s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * transform;
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
	}
	void RenderingService::BeginScene(const EditorCamera& camera)
	{
		s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
	}

	void RenderingService::BeginScene(const Math::mat4 projection)
	{
		s_Data.CameraBuffer.ViewProjection = projection;
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
	}

	void RenderingService::EndScene()
	{
		FlushBuffers();
	}

	float RenderingService::GetLineWidth()
	{
		return s_Data.LineWidth;
	}
	void RenderingService::SetLineWidth(float width)
	{
		s_Data.LineWidth = width;
	}

	void RenderingService::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Statistics));
	}
	RenderingService::Statistics RenderingService::GetStats()
	{
		return s_Data.Stats;
	}

	void RenderingService::FillTextureIndex(RendererInputSpec& inputSpec)
	{
		//if (s_Data.QuadIndexCount >= RendererData::MaxIndices) { NextBatch(); }
		KG_ASSERT(inputSpec.m_ShapeComponent->Texture, "Texture shader added, however, no texture is available in ShapeComponent.");
		auto& m_Textures = inputSpec.m_CurrentDrawBuffer->m_Textures;
		float textureIndex = -1.0f;

		uint32_t iteration = 0;
		for (auto& texture : m_Textures)
		{
			if (texture == inputSpec.m_ShapeComponent->Texture)
			{
				textureIndex = (float)iteration;
				break;
			}
			iteration++;
		}
		if (static_cast<int32_t>(textureIndex) == -1)
		{
			if (m_Textures.size() >= RendererData::MaxTextureSlots)
			{
				textureIndex = 0.0f;
				// TODO: NextBatch, Create a new DrawCallBuffer for the current shader and update Textures Ref
			}
			m_Textures.push_back(inputSpec.m_ShapeComponent->Texture);
			textureIndex = static_cast<float>(m_Textures.size() - 1);
		}

		Shader::SetDataAtInputLocation<float>(textureIndex, "a_TexIndex", inputSpec.m_Buffer, inputSpec.m_Shader);
	}

	void RenderingService::FillTextureCoordinate(RendererInputSpec& inputSpec, uint32_t iteration)
	{
		const Math::vec2& coordinates = inputSpec.m_ShapeComponent->TextureCoordinates->at(iteration);
		Shader::SetDataAtInputLocation<Math::vec2>(coordinates, "a_TexCoord", inputSpec.m_Buffer, inputSpec.m_Shader);
	}

	void RenderingService::FillLocalPosition(RendererInputSpec& inputSpec, uint32_t iteration)
	{
		const Math::vec3& localPosition = inputSpec.m_ShapeComponent->Vertices->at(iteration) * 2.0f;
		Shader::SetDataAtInputLocation<Math::vec3>(localPosition, "a_LocalPosition", inputSpec.m_Buffer, inputSpec.m_Shader);
	}

	void RenderingService::FillWorldPosition(RendererInputSpec& inputSpec, uint32_t iteration)
	{
		const Math::vec3& localPosition = inputSpec.m_ShapeComponent->Vertices->at(iteration);
		Math::vec3 worldPosition = inputSpec.m_TransformMatrix * Math::vec4(localPosition, 1.0f);
		Shader::SetDataAtInputLocation<Math::vec3>(worldPosition, "a_Position", inputSpec.m_Buffer, inputSpec.m_Shader);
	}


	void RenderingService::FillWorldPositionNoTransform(RendererInputSpec& inputSpec, uint32_t iteration)
	{
		const Math::vec3& localPosition = inputSpec.m_ShapeComponent->Vertices->at(iteration);

		Shader::SetDataAtInputLocation<Math::vec3>(localPosition, "a_Position", inputSpec.m_Buffer, inputSpec.m_Shader);
	}

	void RenderingService::FillVertexColor(RendererInputSpec& inputSpec, uint32_t iteration)
	{
		auto& colorVector = inputSpec.m_ShapeComponent->VertexColors;
		KG_ASSERT(iteration < static_cast<uint32_t>(colorVector->size()), "Invalid iteration inside FillVertexColor function");
		Shader::SetDataAtInputLocation<Math::vec4>(colorVector->at(iteration), "a_Color", inputSpec.m_Buffer, inputSpec.m_Shader);
	}

	void RenderingService::FillIndicesData(RendererInputSpec& inputSpec)
	{
		// Upload Indices
		Ref<DrawCallBuffer> drawCallBuffer = inputSpec.m_Shader->GetCurrentDrawCallBuffer();
		std::size_t currentBufferSize = (drawCallBuffer->m_VertexBufferIterator - drawCallBuffer->m_VertexBuffer.Data) / inputSpec.m_Shader->GetInputLayout().GetStride();
		for (auto& index : *(inputSpec.m_ShapeComponent->Indices))
		{
			drawCallBuffer->m_IndexBuffer.push_back(static_cast<uint32_t>(currentBufferSize) + index);
		}
	}

	void RenderingService::FillEntityID(Rendering::RendererInputSpec& inputSpec)
	{
		Shader::SetDataAtInputLocation<uint32_t>(inputSpec.m_Entity, "a_EntityID", inputSpec.m_Buffer, inputSpec.m_Shader);
	}

	void RenderingService::SubmitDataToRenderer(RendererInputSpec& inputSpec)
	{
		if (!inputSpec.m_ShapeComponent->Vertices || inputSpec.m_Shader->GetSpecification().RenderType == RenderingType::None) { return; }

		Ref<DrawCallBuffer> drawCallBuffer = inputSpec.m_Shader->GetCurrentDrawCallBuffer();

		// Create new DrawCallBuffer if one is not associated with active shader
		if (!drawCallBuffer)
		{
			drawCallBuffer = CreateRef<DrawCallBuffer>();
			drawCallBuffer->m_VertexBuffer.Allocate(s_MaxVertexBufferSize);
			drawCallBuffer->m_VertexBufferIterator = drawCallBuffer->m_VertexBuffer.Data;
			if (inputSpec.m_Shader->GetSpecification().RenderType == RenderingType::DrawIndex)
			{
				drawCallBuffer->m_IndexBuffer.reserve(s_Data.MaxIndicesBuffer);
			}
			drawCallBuffer->m_Textures.reserve(s_Data.MaxTextureSlots);
			drawCallBuffer->m_Shader = inputSpec.m_Shader.get();
			s_Data.DrawCalls.emplace_back(drawCallBuffer);
			inputSpec.m_Shader->SetCurrentDrawCallBuffer(drawCallBuffer);
		}

		std::size_t currentBufferSize = drawCallBuffer->m_VertexBufferIterator - drawCallBuffer->m_VertexBuffer.Data;
		std::size_t sizeOfNewDrawCallBuffer = inputSpec.m_Buffer.Size * inputSpec.m_ShapeComponent->Vertices->size() + currentBufferSize;
		// Create new DrawCallBuffer if current buffer overflows
		if (sizeOfNewDrawCallBuffer >= s_MaxVertexBufferSize)
		{
			drawCallBuffer = CreateRef<DrawCallBuffer>();
			drawCallBuffer->m_VertexBuffer.Allocate(s_MaxVertexBufferSize);
			drawCallBuffer->m_VertexBufferIterator = drawCallBuffer->m_VertexBuffer.Data;
			if (inputSpec.m_Shader->GetSpecification().RenderType == RenderingType::DrawIndex)
			{
				drawCallBuffer->m_IndexBuffer.reserve(s_Data.MaxIndicesBuffer);
			}
			drawCallBuffer->m_Textures.reserve(s_Data.MaxTextureSlots);
			drawCallBuffer->m_Shader = inputSpec.m_Shader.get();
			s_Data.DrawCalls.emplace_back(drawCallBuffer);
			inputSpec.m_Shader->SetCurrentDrawCallBuffer(drawCallBuffer);
		}

		inputSpec.m_CurrentDrawBuffer = drawCallBuffer;

		for (const auto& PerObjectFunction : inputSpec.m_Shader->GetFillDataObject())
		{
			PerObjectFunction(inputSpec);
		}

		for (uint32_t iteration {0}; iteration < inputSpec.m_ShapeComponent->Vertices->size(); iteration++)
		{
			for (const auto& PerVertexFunction : inputSpec.m_Shader->GetFillDataVertex())
			{
				PerVertexFunction(inputSpec, iteration);
			}
			memcpy(inputSpec.m_CurrentDrawBuffer->m_VertexBufferIterator, inputSpec.m_Buffer.Data, inputSpec.m_Buffer.Size);
			inputSpec.m_CurrentDrawBuffer->m_VertexBufferIterator += inputSpec.m_Buffer.Size;
			s_Data.Stats.VertexCount++;
		}

		
	}

	void RenderingService::FillTextureUniform(Ref<DrawCallBuffer> buffer)
	{
		for (uint32_t i = 0; i < buffer->m_Textures.size(); i++) { buffer->m_Textures[i]->Bind(i); }
	}

	void RenderingService::DrawBufferIndices(Ref<DrawCallBuffer> buffer)
	{
		RendererAPI::DrawIndexed(buffer->m_Shader->GetVertexArray(), buffer->m_IndexBuffer.data(), static_cast<uint32_t>(buffer->m_IndexBuffer.size()));
		s_Data.Stats.DrawCalls++;
	}

	void RenderingService::DrawBufferPoints(Ref<DrawCallBuffer> buffer)
	{
		RendererAPI::SetPointWidth(s_Data.PointWidth);
		RendererAPI::DrawPoints(buffer->m_Shader->GetVertexArray(), static_cast<std::uint32_t>(buffer->m_VertexBufferIterator - buffer->m_VertexBuffer.Data) / buffer->m_Shader->GetInputLayout().GetStride());
		s_Data.Stats.DrawCalls++;
	}

	void RenderingService::DrawBufferLine(Ref<DrawCallBuffer> buffer)
	{
		RendererAPI::SetLineWidth(s_Data.LineWidth);
		RendererAPI::DrawLines(buffer->m_Shader->GetVertexArray(), static_cast<std::uint32_t>(buffer->m_VertexBufferIterator - buffer->m_VertexBuffer.Data) / buffer->m_Shader->GetInputLayout().GetStride());
		s_Data.Stats.DrawCalls++;
	}

	void RenderingService::DrawBufferTriangles(Ref<DrawCallBuffer> buffer)
	{
		RendererAPI::DrawTriangles(buffer->m_Shader->GetVertexArray(), static_cast<std::uint32_t>(buffer->m_VertexBufferIterator - buffer->m_VertexBuffer.Data) / buffer->m_Shader->GetInputLayout().GetStride());
		s_Data.Stats.DrawCalls++;
	}

	void RenderingService::FlushBuffers()
	{
		auto& allBuffers = s_Data.DrawCalls;

		// Submit all Buffers to DrawCalls!
		for (auto buffer : allBuffers)
		{
			for (const auto& preDrawFunction : buffer->m_Shader->GetPreDrawBuffer())
			{
				preDrawFunction(buffer);
			}

			buffer->m_Shader->Bind();
			uint32_t dataSize = static_cast<uint32_t>(buffer->m_VertexBufferIterator - buffer->m_VertexBuffer.Data);
			buffer->m_Shader->GetVertexArray()->GetVertexBuffers().at(0)->SetData(buffer->m_VertexBuffer.Data, dataSize);

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
