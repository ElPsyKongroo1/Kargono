#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Renderer/InputBuffer.h"
#include "Kargono/Renderer/UniformBuffer.h"
#include "Kargono/Renderer/VertexArray.h"
#include "Kargono/Core/Buffer.h"
#include "Kargono/Renderer/Shape.h"
#include "Kargono/Math/Math.h"

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
			Math::mat4 TransformMatrix;
			Math::mat4 ObjectOutlineMatrix;
		};

		// This struct specifies the type of color input used by a shader
		// For example, flat color only sends one color for each object while vertex color
		// sends a color for each vertex.
		enum class ColorInputType
		{
			None = 0, FlatColor, VertexColor
		};

		enum class TextureInputType
		{
			None = 0, ColorTexture, TextTexture
		};

		static std::string ColorInputTypeToString(Shader::ColorInputType colorInput)
		{
			switch (colorInput)
			{
			case Shader::ColorInputType::None: return "None";
			case Shader::ColorInputType::FlatColor: return "FlatColor";
			case Shader::ColorInputType::VertexColor: return "VertexColor";
			}
			KG_CORE_ASSERT(false, "Unknown Data Type sent to ColorInputToString Function");
			return "None";
		}

		static Shader::ColorInputType StringToColorInputType(std::string_view string)
		{
			if (string == "None") { return Shader::ColorInputType::None; }
			if (string == "FlatColor") { return Shader::ColorInputType::FlatColor; }
			if (string == "VertexColor") { return Shader::ColorInputType::VertexColor; }

			KG_CORE_ASSERT(false, "Unknown Data Type sent to StringToColorInputType Function");
			return Shader::ColorInputType::None;
		}

		static std::string TextureInputTypeToString(Shader::TextureInputType textureInput)
		{
			switch (textureInput)
			{
			case Shader::TextureInputType::None: return "None";
			case Shader::TextureInputType::ColorTexture: return "ColorTexture";
			case Shader::TextureInputType::TextTexture: return "TextTexture";
			}
			KG_CORE_ASSERT(false, "Unknown Data Type sent to TextureInputToString Function");
			return "None";
		}

		static Shader::TextureInputType StringToTextureInputType(std::string_view string)
		{
			if (string == "None") { return Shader::TextureInputType::None; }
			if (string == "ColorTexture") { return Shader::TextureInputType::ColorTexture; }
			if (string == "TextTexture") { return Shader::TextureInputType::TextTexture; }

			KG_CORE_ASSERT(false, "Unknown Data Type sent to StringToTextureInputType Function");
			return Shader::TextureInputType::None;
		}

	public:

		using ShaderSource = std::string;

		struct ShaderSpecification
		{
			// TODO: Note, ensure you update the serialization method after any changes!
			
			// Pixel Color Options
			Shader::ColorInputType ColorInput = Shader::ColorInputType::None;
			Shader::TextureInputType TextureInput = Shader::TextureInputType::None;

			// Structure Change Options
			bool AddCircleShape = false;

			// Other Options
			bool AddProjectionMatrix = false;
			bool AddEntityID = false;

			// Rendering Options
			Shape::RenderingType RenderType = Shape::RenderingType::None;
			bool DrawOutline = false;

			// Generates default relational operations for comparing the same class! https://en.cppreference.com/w/cpp/language/default_comparisons
			auto operator<=>(const ShaderSpecification&) const = default;
			// Default Copy Constructor
			ShaderSpecification(const ShaderSpecification&) = default;
			ShaderSpecification() = default;
			ShaderSpecification(Shader::ColorInputType colorInput, Shader::TextureInputType textureInput, bool addCircle, bool addProjection, bool addEntityID, Shape::RenderingType renderType, bool drawOutline)
				: ColorInput(colorInput), TextureInput(textureInput), AddCircleShape(addCircle), AddProjectionMatrix(addProjection), AddEntityID(addEntityID), RenderType(renderType), DrawOutline(drawOutline)
			{}
		};

	public:
		Shader() = default;
		Shader(const ShaderSpecification& shader) : m_ShaderSpecification(shader) {}
	
	public:
		virtual ~Shader() = default;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetMat3Uniform(const std::string& name, const Math::mat3& value) = 0;
		virtual void SetMat4Uniform(const std::string& name, const Math::mat4& value) = 0;
		virtual void SetFloatUniform(const std::string& name, float value) = 0;
		virtual void SetFloat2Uniform(const std::string& name, const Math::vec2& value) = 0;
		virtual void SetFloat3Uniform(const std::string& name, const Math::vec3& value) = 0;
		virtual void SetFloat4Uniform(const std::string& name, const Math::vec4& value) = 0;
		virtual void SetIntUniform(const std::string& name, int value) = 0;
		virtual void SetIntArrayUniform(const std::string& name, int* values, uint32_t count) = 0;
	public:
		static Ref<Shader> Create(const std::string& name, const std::unordered_map<GLenum, std::vector<uint32_t>>& shaderBinaries);
	public:
		template<typename T>
		static T* GetInputLocation(const std::string& inputName, Buffer inputBuffer, Ref<Shader> shader)
		{
			std::size_t inputLocation = shader->GetInputLayout().FindElementByName(inputName).Offset;
			return inputBuffer.As<T>(inputLocation);
		}
		template<typename T>
		static void SetDataAtInputLocation(const T& value , const std::string& inputName, Buffer inputBuffer, Ref<Shader> shader)
		{
			std::size_t inputLocation = shader->GetInputLayout().FindElementByName(inputName).Offset;
			T* inputPointer = inputBuffer.As<T>(inputLocation);
			*inputPointer = value;
		}
		static std::tuple<Shader::ShaderSource, InputBufferLayout, UniformBufferList> BuildShader(const ShaderSpecification& shaderSpec);

		const Shader::ShaderSpecification& GetSpecification() const { return m_ShaderSpecification; }
		InputBufferLayout& GetInputLayout() { return m_InputBufferLayout; }
		const UniformBufferList& GetUniformList() const { return m_UniformBufferList; }
		const std::vector<std::function<void(RendererInputSpec& spec)>>& GetFillDataObject() const { return m_FillDataPerObject; }
		const std::vector<std::function<void(RendererInputSpec& spec, uint32_t iteration)>>& GetFillDataVertex() const { return m_FillDataPerVertex; }
		const std::vector<std::function<void(RendererInputSpec& spec)>>& GetFillDataObjectScene() const { return m_FillDataInScene; }
		const std::vector<std::function<void(Ref<Shader::DrawCallBuffer> buffer)>>& GetDrawFunctions() const { return m_DrawFunctions; }
		const std::vector<std::function<void(Ref<Shader::DrawCallBuffer> buffer)>>& GetSubmitUniforms() const { return m_SubmitUniforms; }

		const std::vector<std::function<void(Ref<Shader::DrawCallBuffer> buffer)>>& GetPreDrawBuffer() const { return m_PreDrawBuffer; }
		const std::vector<std::function<void(Ref<Shader::DrawCallBuffer> buffer)>>& GetPostDrawBuffer() const { return m_PostDrawBuffer; }

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

		std::vector<std::function<void(Ref<Shader::DrawCallBuffer> buffer)>> m_PreDrawBuffer {};
		std::vector<std::function<void(Ref<Shader::DrawCallBuffer> buffer)>> m_PostDrawBuffer {};
		Ref<DrawCallBuffer> m_CurrentDrawCall = nullptr;
		Ref<VertexArray> m_VertexArray = nullptr;

	};
}
