#include "kgpch.h"

#include "Kargono/Rendering/RenderingEngine.h"
#include "Kargono/Rendering/RenderCommand.h"
#include "Kargono/Rendering/Shader.h"
#include "Kargono/Rendering/VertexArray.h"
#include "Kargono/Rendering/UniformBuffer.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Scene/Scene.h"

namespace Kargono::Rendering
{
	struct RendererData
	{
		static const uint32_t MaxQuads = 20000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;

		float LineWidth = 2.0f;

		RenderingEngine::Statistics Stats;

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



	void RenderingEngine::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void RenderingEngine::Init()
	{
		KG_ASSERT(Projects::Project::GetActive(), "No valid project is active while trying to initialize shaders!");
		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(RendererData::CameraData), 0);
		KG_VERIFY(s_Data.CameraUniformBuffer, "Renderer Init")
	}
	void RenderingEngine::Shutdown()
	{
	}
	void RenderingEngine::BeginScene(const Camera& camera, const Math::mat4& transform)
	{
		s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * transform;
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
	}
	void RenderingEngine::BeginScene(const EditorCamera& camera)
	{
		s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
	}

	void RenderingEngine::BeginScene(const Math::mat4 projection)
	{
		s_Data.CameraBuffer.ViewProjection = projection;
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
	}

	void RenderingEngine::EndScene()
	{
		FlushBuffers();
	}

	float RenderingEngine::GetLineWidth()
	{
		return s_Data.LineWidth;
	}
	void RenderingEngine::SetLineWidth(float width)
	{
		s_Data.LineWidth = width;
	}

	void RenderingEngine::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Statistics));
	}
	RenderingEngine::Statistics RenderingEngine::GetStats()
	{
		return s_Data.Stats;
	}

	void RenderingEngine::FillTextureIndex(RendererInputSpec& inputSpec)
	{
		//if (s_Data.QuadIndexCount >= RendererData::MaxIndices) { NextBatch(); }
		KG_ASSERT(inputSpec.ShapeComponent->Texture, "Texture shader added, however, no texture is available in ShapeComponent.");
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

		Shader::SetDataAtInputLocation<float>(textureIndex, "a_TexIndex", inputSpec.Buffer, inputSpec.Shader);
	}

	void RenderingEngine::FillTextureCoordinate(RendererInputSpec& inputSpec, uint32_t iteration)
	{
		const Math::vec2& coordinates  = inputSpec.ShapeComponent->TextureCoordinates->at(iteration);
		Shader::SetDataAtInputLocation<Math::vec2>(coordinates, "a_TexCoord", inputSpec.Buffer, inputSpec.Shader);
	}

	void RenderingEngine::FillLocalPosition(RendererInputSpec& inputSpec, uint32_t iteration)
	{
		const Math::vec3& localPosition = inputSpec.ShapeComponent->Vertices->at(iteration) * 2.0f;
		Shader::SetDataAtInputLocation<Math::vec3>(localPosition, "a_LocalPosition", inputSpec.Buffer, inputSpec.Shader);
	}

	void RenderingEngine::FillWorldPosition(RendererInputSpec& inputSpec, uint32_t iteration)
	{
		const Math::vec3& localPosition = inputSpec.ShapeComponent->Vertices->at(iteration);
		Math::vec3 worldPosition = inputSpec.TransformMatrix * Math::vec4(localPosition, 1.0f);
		Shader::SetDataAtInputLocation<Math::vec3>(worldPosition, "a_Position", inputSpec.Buffer, inputSpec.Shader);
	}


	void RenderingEngine::FillWorldPositionNoTransform(RendererInputSpec& inputSpec, uint32_t iteration)
	{
		const Math::vec3& localPosition = inputSpec.ShapeComponent->Vertices->at(iteration);

		Shader::SetDataAtInputLocation<Math::vec3>(localPosition, "a_Position", inputSpec.Buffer, inputSpec.Shader);
	}

	void RenderingEngine::FillVertexColor(RendererInputSpec& inputSpec, uint32_t iteration)
	{
		auto& colorVector = inputSpec.ShapeComponent->VertexColors;
		KG_ASSERT(iteration < static_cast<uint32_t>(colorVector->size()), "Invalid iteration inside FillVertexColor function");
		Shader::SetDataAtInputLocation<Math::vec4>(colorVector->at(iteration), "a_Color", inputSpec.Buffer, inputSpec.Shader);
	}

	void RenderingEngine::FillIndicesData(RendererInputSpec& inputSpec)
	{
		// Upload Indices
		auto& drawCallBuffer = inputSpec.Shader->GetCurrentDrawCallBuffer();
		std::size_t currentBufferSize = (drawCallBuffer->VertexBufferIterator - drawCallBuffer->VertexBuffer.Data) / inputSpec.Shader->GetInputLayout().GetStride();
		for (auto& index : *(inputSpec.ShapeComponent->Indices))
		{
			drawCallBuffer->IndexBuffer.push_back(static_cast<uint32_t>(currentBufferSize) + index);
		}
	}

	void RenderingEngine::SubmitDataToRenderer(RendererInputSpec& inputSpec)
	{
		if (!inputSpec.ShapeComponent->Vertices || inputSpec.Shader->GetSpecification().RenderType == RenderingType::None) { return; }

		// Manage current DrawCallBuffer
		auto& drawCallBuffer = inputSpec.Shader->GetCurrentDrawCallBuffer();
		if (!drawCallBuffer)
		{
			drawCallBuffer = CreateRef<DrawCallBuffer>();
			drawCallBuffer->VertexBuffer.Allocate(s_MaxVertexBufferSize);
			drawCallBuffer->VertexBufferIterator = drawCallBuffer->VertexBuffer.Data;
			if (inputSpec.Shader->GetSpecification().RenderType == RenderingType::DrawIndex)
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
			drawCallBuffer = CreateRef<DrawCallBuffer>();
			drawCallBuffer->VertexBuffer.Allocate(s_MaxVertexBufferSize);
			drawCallBuffer->VertexBufferIterator = drawCallBuffer->VertexBuffer.Data;
			if (inputSpec.Shader->GetSpecification().RenderType == RenderingType::DrawIndex)
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

	void RenderingEngine::FillTextureUniform(Ref<DrawCallBuffer> buffer)
	{
		for (uint32_t i = 0; i < buffer->Textures.size(); i++) { buffer->Textures[i]->Bind(i); }
	}

	void RenderingEngine::DrawBufferIndices(Ref<DrawCallBuffer> buffer)
	{
		RenderCommand::DrawIndexed(buffer->Shader->GetVertexArray(), buffer->IndexBuffer.data(), static_cast<uint32_t>(buffer->IndexBuffer.size()));
		s_Data.Stats.DrawCalls++;
	}

	void RenderingEngine::DrawBufferLine(Ref<DrawCallBuffer> buffer)
	{
		RenderCommand::SetLineWidth(s_Data.LineWidth);
		RenderCommand::DrawLines(buffer->Shader->GetVertexArray(), static_cast<std::uint32_t>(buffer->VertexBufferIterator - buffer->VertexBuffer.Data) / buffer->Shader->GetInputLayout().GetStride());
		s_Data.Stats.DrawCalls++;
	}

	void RenderingEngine::DrawBufferTriangles(Ref<DrawCallBuffer> buffer)
	{
		RenderCommand::DrawTriangles(buffer->Shader->GetVertexArray(), static_cast<std::uint32_t>(buffer->VertexBufferIterator - buffer->VertexBuffer.Data) / buffer->Shader->GetInputLayout().GetStride());
		s_Data.Stats.DrawCalls++;
	}

	void RenderingEngine::FlushBuffers()
	{
		auto& allBuffers = s_Data.DrawCalls;

		// Submit all Buffers to DrawCalls!
		for (auto buffer : allBuffers)
		{
			for (const auto& preDrawFunction : buffer->Shader->GetPreDrawBuffer())
			{
				preDrawFunction(buffer);
			}

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

			for (const auto& postDrawFunction : buffer->Shader->GetPostDrawBuffer())
			{
				postDrawFunction(buffer);
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
