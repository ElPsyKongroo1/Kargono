#include "kgpch.h"

#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Renderer/RenderCommand.h"
#include "Kargono/Renderer/Shader.h"
#include "Kargono/Renderer/VertexArray.h"
#include "Kargono/Renderer/UniformBuffer.h"
#include "Kargono/Project/Project.h"
#include "Kargono/Scene/Scene.h"
#include "Kargono/Assets/AssetManager.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Kargono
{

	struct RendererData
	{
		static const uint32_t MaxQuads = 20000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;

		float LineWidth = 2.0f;

		Renderer::Statistics Stats;

		struct CameraData
		{
			glm::mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;

		// New Renderer
		// Maximum Vertex Buffer Size in Bytes
		static const uint32_t MaxTextureSlots = 32;
		static const uint32_t MaxIndicesBuffer = 1000;
		std::vector<Ref<Shader::DrawCallBuffer>> DrawCalls;
	};

	static RendererData s_Data;



	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::Init()
	{
		KG_CORE_ASSERT(Project::GetActive(), "No valid project is active while trying to initialize shaders!");
		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(RendererData::CameraData), 0);
	}
	void Renderer::Shutdown()
	{
	}
	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * transform;
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
	}
	void Renderer::BeginScene(const EditorCamera& camera)
	{
		s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
	}

	void Renderer::EndScene()
	{
		FlushBuffers();
	}

	float Renderer::GetLineWidth()
	{
		return s_Data.LineWidth;
	}
	void Renderer::SetLineWidth(float width)
	{
		s_Data.LineWidth = width;
	}

	void Renderer::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Statistics));
	}
	Renderer::Statistics Renderer::GetStats()
	{
		return s_Data.Stats;
	}

	void Renderer::FillTextureIndex(Shader::RendererInputSpec& inputSpec)
	{
		//if (s_Data.QuadIndexCount >= RendererData::MaxIndices) { NextBatch(); }
		KG_CORE_ASSERT(inputSpec.ShapeComponent->Texture, "Texture shader added, however, no texture is available in ShapeComponent.");
		auto& Textures = inputSpec.CurrentDrawBuffer->Textures;
		float textureIndex = -1.0f;

		uint32_t iteration = 0;
		for (auto& texture : Textures)
		{
			if (texture == inputSpec.ShapeComponent->Texture)
			{
				textureIndex = (float)iteration;
				break;
			}
			iteration++;
		}
		if (static_cast<int32_t>(textureIndex) == -1)
		{
			if (Textures.size() >= RendererData::MaxTextureSlots)
			{
				textureIndex = 0.0f;
				// TODO: NextBatch, Create a new DrawCallBuffer for the current shader and update Textures Ref
			}
			Textures.push_back(inputSpec.ShapeComponent->Texture);
			textureIndex = static_cast<float>(Textures.size() - 1);
		}

		std::size_t texIndexLocationInBuffer = inputSpec.Shader->GetInputLayout().FindElementByName("a_TexIndex").Offset;
		auto* pointerToID = inputSpec.Buffer.As<float>(texIndexLocationInBuffer);
		*pointerToID = textureIndex;
	}

	void Renderer::FillTextureCoordinate(Shader::RendererInputSpec& inputSpec, uint32_t iteration)
	{
		const glm::vec2& coordinates  = inputSpec.ShapeComponent->TextureCoordinates->at(iteration);

		std::size_t texCoordLocationInBuffer = inputSpec.Shader->GetInputLayout().FindElementByName("a_TexCoord").Offset;
		auto* pointerToID = inputSpec.Buffer.As <glm::vec2> (texCoordLocationInBuffer);
		*pointerToID = coordinates;
	}

	void Renderer::FillLocalPosition(Shader::RendererInputSpec& inputSpec, uint32_t iteration)
	{
		const glm::vec3& localPosition = inputSpec.ShapeComponent->Vertices->at(iteration);

		std::size_t localPositionLocationInBuffer = inputSpec.Shader->GetInputLayout().FindElementByName("a_LocalPosition").Offset;
		auto* pointerToID = inputSpec.Buffer.As <glm::vec3>(localPositionLocationInBuffer);
		*pointerToID = localPosition * 2.0f;
	}

	void Renderer::FillWorldPosition(Shader::RendererInputSpec& inputSpec, uint32_t iteration)
	{
		const glm::vec3& localPosition = inputSpec.ShapeComponent->Vertices->at(iteration);

		glm::vec3 worldPosition = inputSpec.TransformMatrix * glm::vec4(localPosition, 1.0f);

		std::size_t localPositionLocationInBuffer = inputSpec.Shader->GetInputLayout().FindElementByName("a_Position").Offset;
		auto* pointerToID = inputSpec.Buffer.As <glm::vec3>(localPositionLocationInBuffer);
		*pointerToID = worldPosition;
	}

	void Renderer::FillWorldPositionNoTransform(Shader::RendererInputSpec& inputSpec, uint32_t iteration)
	{
		const glm::vec3& localPosition = inputSpec.ShapeComponent->Vertices->at(iteration);

		std::size_t localPositionLocationInBuffer = inputSpec.Shader->GetInputLayout().FindElementByName("a_Position").Offset;
		auto* pointerToID = inputSpec.Buffer.As <glm::vec3>(localPositionLocationInBuffer);
		*pointerToID = localPosition;
	}

	void Renderer::FillIndicesData(Shader::RendererInputSpec& inputSpec)
	{
		// Upload Indices
		auto& drawCallBuffer = inputSpec.Shader->GetCurrentDrawCallBuffer();
		std::size_t currentBufferSize = (drawCallBuffer->VertexBufferIterator - drawCallBuffer->VertexBuffer.Data) / inputSpec.Shader->GetInputLayout().GetStride();
		for (auto& index : *(inputSpec.ShapeComponent->Indices))
		{
			drawCallBuffer->IndexBuffer.push_back(static_cast<uint32_t>(currentBufferSize) + index);
		}
	}

	void Renderer::SubmitDataToRenderer(Shader::RendererInputSpec& inputSpec)
	{
		if (!inputSpec.ShapeComponent->Vertices) { return; }

		// Manage current DrawCallBuffer
		auto& drawCallBuffer = inputSpec.Shader->GetCurrentDrawCallBuffer();
		if (!drawCallBuffer)
		{
			drawCallBuffer = CreateRef<Shader::DrawCallBuffer>();
			drawCallBuffer->VertexBuffer.Allocate(s_MaxVertexBufferSize);
			drawCallBuffer->VertexBufferIterator = drawCallBuffer->VertexBuffer.Data;
			if (inputSpec.Shader->GetSpecification().RenderType == Shape::RenderingType::DrawIndex)
			{
				drawCallBuffer->IndexBuffer.reserve(s_Data.MaxIndicesBuffer);
			}
			drawCallBuffer->Textures.reserve(s_Data.MaxTextureSlots);
			drawCallBuffer->Shader = inputSpec.Shader.get();
			s_Data.DrawCalls.emplace_back(drawCallBuffer);
		}

		std::size_t currentBufferSize = drawCallBuffer->VertexBufferIterator - drawCallBuffer->VertexBuffer.Data;
		std::size_t sizeOfNewDrawCallBuffer = inputSpec.Buffer.Size * inputSpec.ShapeComponent->Vertices->size() + currentBufferSize;
		if (sizeOfNewDrawCallBuffer >= s_MaxVertexBufferSize)
		{
			drawCallBuffer = CreateRef<Shader::DrawCallBuffer>();
			drawCallBuffer->VertexBuffer.Allocate(s_MaxVertexBufferSize);
			drawCallBuffer->VertexBufferIterator = drawCallBuffer->VertexBuffer.Data;
			if (inputSpec.Shader->GetSpecification().RenderType == Shape::RenderingType::DrawIndex)
			{
				drawCallBuffer->IndexBuffer.reserve(s_Data.MaxIndicesBuffer);
			}
			drawCallBuffer->Textures.reserve(s_Data.MaxTextureSlots);
			drawCallBuffer->Shader = inputSpec.Shader.get();
			s_Data.DrawCalls.emplace_back(drawCallBuffer);
		}

		inputSpec.CurrentDrawBuffer = drawCallBuffer;

		for (const auto& PerObjectFunction : inputSpec.Shader->GetFillDataObject())
		{
			PerObjectFunction(inputSpec);
		}

		for (uint32_t iteration {0}; iteration < inputSpec.ShapeComponent->Vertices->size(); iteration++)
		{
			for (const auto& PerVertexFunction : inputSpec.Shader->GetFillDataVertex())
			{
				PerVertexFunction(inputSpec, iteration);
			}
			memcpy_s(inputSpec.CurrentDrawBuffer->VertexBufferIterator, inputSpec.Buffer.Size, inputSpec.Buffer.Data, inputSpec.Buffer.Size);
			inputSpec.CurrentDrawBuffer->VertexBufferIterator += inputSpec.Buffer.Size;
			s_Data.Stats.VertexCount++;
		}

		
	}

	void Renderer::FillTextureUniform(Ref<Shader::DrawCallBuffer> buffer)
	{
		for (uint32_t i = 0; i < buffer->Textures.size(); i++) { buffer->Textures[i]->Bind(i); }
	}

	void Renderer::DrawBufferIndices(Ref<Shader::DrawCallBuffer> buffer)
	{
		RenderCommand::DrawIndexed(buffer->Shader->GetVertexArray(), buffer->IndexBuffer.data(), static_cast<uint32_t>(buffer->IndexBuffer.size()));
		s_Data.Stats.DrawCalls++;
	}

	void Renderer::DrawBufferLine(Ref<Shader::DrawCallBuffer> buffer)
	{
		RenderCommand::SetLineWidth(s_Data.LineWidth);
		RenderCommand::DrawLines(buffer->Shader->GetVertexArray(), static_cast<std::uint32_t>(buffer->VertexBufferIterator - buffer->VertexBuffer.Data) / buffer->Shader->GetInputLayout().GetStride());
		s_Data.Stats.DrawCalls++;
	}

	void Renderer::FlushBuffers()
	{
		auto& allBuffers = s_Data.DrawCalls;

		// Submit all Buffers to DrawCalls!
		for (auto buffer : allBuffers)
		{
			buffer->Shader->Bind();
			uint32_t dataSize = static_cast<uint32_t>(buffer->VertexBufferIterator - buffer->VertexBuffer.Data);
			buffer->Shader->GetVertexArray()->GetVertexBuffers().at(0)->SetData(buffer->VertexBuffer.Data, dataSize);

			// Submit Per Buffer Uniforms
			for (const auto& uniformFunction : buffer->Shader->GetSubmitUniforms())
			{
				uniformFunction(buffer);
			}

			// Final Draw Call Functions
			for (const auto& drawFunction : buffer->Shader->GetDrawFunctions())
			{
				drawFunction(buffer);
			}
			
		}

		// Clear current Buffers inside each shader!
		for (auto& buffer : allBuffers)
		{
			if (buffer->Shader->GetCurrentDrawCallBuffer())
			{
				buffer->Shader->SetCurrentDrawCallBufferNull();
			}
		}

		// Clear current Buffers in Renderer
		for (auto& buffer : allBuffers)
		{
			if (buffer->VertexBuffer)
			{
				buffer->VertexBuffer.Release();
			}
		}
		allBuffers.clear();




	}
	
}
