#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Renderer/InputBuffer.h"
#include "Kargono/Renderer/UniformBuffer.h"
#include "Kargono/Renderer/VertexArray.h"
#include "Kargono/Core/Buffer.h"
#include "Kargono/Renderer/Shape.h"

#include <glm/glm.hpp>

#include <string>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <functional>
#include <tuple>


namespace Kargono
{

	typedef unsigned int GLenum;

	class Texture2D;
	struct ShapeComponent;
	struct TransformComponent;
	class Shader
	{
	public:

		struct DrawCallBuffer
		{
			Buffer VertexBuffer {};
			uint8_t* VertexBufferIterator = VertexBuffer.Data;
			std::vector<uint32_t> IndexBuffer {};
			std::vector<Ref<Texture2D>> Textures {};
			Shader* Shader = nullptr;
		};

		// This struct allows input to be sent to the renderer
		struct RendererInputSpec
		{
			Ref<Shader> Shader;
			Ref<Texture2D> Texture;
			ShapeComponent* ShapeComponent;
			Ref<Shader::DrawCallBuffer> CurrentDrawBuffer;
			Buffer Buffer;
			uint32_t Entity;
			void* EntityRegistry;
			glm::mat4 TransformMatrix;
		};
	public:

		using ShaderSource = std::string;

		struct ShaderSpecification
		{
			// TODO: Note, ensure you update the serialization method after any changes!
			
			// Pixel Color Options
			bool AddFlatColor = false;
			bool AddTexture = false;

			// Structure Change Options
			bool AddCircleShape = false;

			// Other Options
			bool AddProjectionMatrix = false;
			bool AddEntityID = false;

			Shape::RenderingType RenderType = Shape::RenderingType::None;

			// Generates default relational operations for comparing the same class! https://en.cppreference.com/w/cpp/language/default_comparisons
			auto operator<=>(const ShaderSpecification&) const = default;
			// Default Copy Constructor
			ShaderSpecification(const ShaderSpecification&) = default;
			ShaderSpecification() = default;
			ShaderSpecification(bool flatColor, bool addTexture, bool addCircle, bool addProjection, bool addEntityID, Shape::RenderingType renderType)
				: AddFlatColor(flatColor), AddTexture(addTexture), AddCircleShape(addCircle), AddProjectionMatrix(addProjection), AddEntityID(addEntityID), RenderType(renderType)
			{}
		};

	public:
		Shader() = default;
		Shader(const ShaderSpecification& shader) : m_ShaderSpecification(shader) {}
	
	public:
		virtual ~Shader() = default;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
	public:
		static Ref<Shader> Create(const std::filesystem::path& filepath);
		static Ref<Shader> Create(const std::string& name, const Shader::ShaderSource& fullSource);
		static Ref<Shader> Create(const std::string& name, const Shader::ShaderSpecification shaderSpec);
		static Ref<Shader> Create(const std::string& name, const std::unordered_map<GLenum, std::vector<uint32_t>>& shaderBinaries);
	public:
		static std::tuple<Shader::ShaderSource, InputBufferLayout, UniformBufferList> BuildShader(const ShaderSpecification& shaderSpec);

		const Shader::ShaderSpecification& GetSpecification() const { return m_ShaderSpecification; }
		InputBufferLayout& GetInputLayout() { return m_InputBufferLayout; }
		const UniformBufferList& GetUniformList() const { return m_UniformBufferList; }
		const std::vector<std::function<void(RendererInputSpec& spec)>>& GetFillDataObject() const { return m_FillDataPerObject; }
		const std::vector<std::function<void(RendererInputSpec& spec, uint32_t iteration)>>& GetFillDataVertex() const { return m_FillDataPerVertex; }
		const std::vector<std::function<void(RendererInputSpec& spec)>>& GetFillDataObjectScene() const { return m_FillDataInScene; }
		const std::vector<std::function<void(Ref<Shader::DrawCallBuffer> buffer)>>& GetDrawFunctions() const { return m_DrawFunctions; }
		const std::vector<std::function<void(Ref<Shader::DrawCallBuffer> buffer)>>& GetSubmitUniforms() const { return m_SubmitUniforms; }
		Ref<DrawCallBuffer>& GetCurrentDrawCallBuffer() { return m_CurrentDrawCall; }
		void SetCurrentDrawCallBufferNull() { m_CurrentDrawCall = nullptr; }

		void SetSpecification(const Shader::ShaderSpecification& shaderSpec);
		void SetInputLayout(const InputBufferLayout& shaderInputLayout);
		void SetUniformList(const UniformBufferList& shaderUniformList) { m_UniformBufferList = shaderUniformList; }

		void SetVertexArray(Ref<VertexArray> newVertexArray) { m_VertexArray = newVertexArray; }
		Ref<VertexArray> GetVertexArray() { return m_VertexArray; }

	protected:
		Shader::ShaderSpecification m_ShaderSpecification {};
		InputBufferLayout m_InputBufferLayout{};
		UniformBufferList m_UniformBufferList{};
	private:
		void FillRenderFunctionList();

		// Renderer Specific Functionality
		std::vector<std::function<void(RendererInputSpec& spec)>> m_FillDataPerObject {};
		std::vector<std::function<void(RendererInputSpec& spec, uint32_t iteration)>> m_FillDataPerVertex {};
		std::vector<std::function<void(RendererInputSpec& spec)>> m_FillDataInScene {};
		std::vector<std::function<void(Ref<Shader::DrawCallBuffer> buffer)>> m_SubmitUniforms {};
		std::vector<std::function<void(Ref<Shader::DrawCallBuffer> buffer)>> m_DrawFunctions {};
		Ref<DrawCallBuffer> m_CurrentDrawCall = nullptr;
		Ref<VertexArray> m_VertexArray = nullptr;

	};
}
