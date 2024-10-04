#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Rendering/InputBuffer.h"
#include "Kargono/Rendering/UniformBuffer.h"
#include "Kargono/Rendering/VertexArray.h"
#include "Kargono/Core/Buffer.h"
#include "Kargono/Rendering/Shape.h"
#include "Kargono/Math/Math.h"

#include <string>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <functional>
#include <tuple>

namespace Kargono::ECS
{
	struct ShapeComponent;
	struct TransformComponent;
}

namespace Kargono::Rendering
{
	typedef unsigned int GLenum;

	class Shader;
	class Texture2D;

	struct DrawCallBuffer
	{
		Buffer VertexBuffer{};
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
		ECS::ShapeComponent* ShapeComponent;
		Ref<DrawCallBuffer> CurrentDrawBuffer;
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


	using ShaderSource = std::string;

	struct ShaderSpecification
	{
		// TODO: Note, ensure you update the serialization method after any changes!

		// Pixel Color Options
		ColorInputType ColorInput = ColorInputType::None;
		TextureInputType TextureInput = TextureInputType::None;

		// Structure Change Options
		bool AddCircleShape = false;

		// Other Options
		bool AddProjectionMatrix = false;
		bool AddEntityID = false;

		// Rendering Options
		RenderingType RenderType = RenderingType::None;
		bool DrawOutline = false;

		// Generates default relational operations for comparing the same class! https://en.cppreference.com/w/cpp/language/default_comparisons
		auto operator<=>(const ShaderSpecification&) const = default;
		// Default Copy Constructor
		ShaderSpecification(const ShaderSpecification&) = default;
		ShaderSpecification() = default;
		ShaderSpecification(ColorInputType colorInput, TextureInputType textureInput, bool addCircle, bool addProjection, bool addEntityID, RenderingType renderType, bool drawOutline)
			: ColorInput(colorInput), TextureInput(textureInput), AddCircleShape(addCircle), AddProjectionMatrix(addProjection), AddEntityID(addEntityID), RenderType(renderType), DrawOutline(drawOutline)
		{}
	};

	class Shader
	{
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

		const ShaderSpecification& GetSpecification() const { return m_ShaderSpecification; }
		InputBufferLayout& GetInputLayout() { return m_InputBufferLayout; }
		const UniformBufferList& GetUniformList() const { return m_UniformBufferList; }
		const std::vector<std::function<void(RendererInputSpec& spec)>>& GetFillDataObject() const { return m_FillDataPerObject; }
		const std::vector<std::function<void(RendererInputSpec& spec, uint32_t iteration)>>& GetFillDataVertex() const { return m_FillDataPerVertex; }
		const std::vector<std::function<void(RendererInputSpec& spec)>>& GetFillDataObjectScene() const { return m_FillDataInScene; }
		const std::vector<std::function<void(Ref<DrawCallBuffer> buffer)>>& GetDrawFunctions() const { return m_DrawFunctions; }
		const std::vector<std::function<void(Ref<DrawCallBuffer> buffer)>>& GetSubmitUniforms() const { return m_SubmitUniforms; }

		const std::vector<std::function<void(Ref<DrawCallBuffer> buffer)>>& GetPreDrawBuffer() const { return m_PreDrawBuffer; }
		const std::vector<std::function<void(Ref<DrawCallBuffer> buffer)>>& GetPostDrawBuffer() const { return m_PostDrawBuffer; }

		Ref<DrawCallBuffer> GetCurrentDrawCallBuffer() { return m_CurrentDrawCall; }
		void SetCurrentDrawCallBuffer(Ref<DrawCallBuffer> buffer) { m_CurrentDrawCall = buffer; }
		void ClearCurrentDrawCallBuffer() { m_CurrentDrawCall = nullptr; }

		void SetSpecification(const ShaderSpecification& shaderSpec);
		void SetInputLayout(const InputBufferLayout& shaderInputLayout);
		void SetUniformList(const UniformBufferList& shaderUniformList) { m_UniformBufferList = shaderUniformList; }

		void SetVertexArray(Ref<VertexArray> newVertexArray) { m_VertexArray = newVertexArray; }
		Ref<VertexArray> GetVertexArray() { return m_VertexArray; }

	protected:
		ShaderSpecification m_ShaderSpecification {};
		InputBufferLayout m_InputBufferLayout{};
		UniformBufferList m_UniformBufferList{};
	private:
		void FillRenderFunctionList();

		// Renderer Specific Functionality
		std::vector<std::function<void(RendererInputSpec& spec)>> m_FillDataPerObject {};
		std::vector<std::function<void(RendererInputSpec& spec, uint32_t iteration)>> m_FillDataPerVertex {};
		std::vector<std::function<void(RendererInputSpec& spec)>> m_FillDataInScene {};
		std::vector<std::function<void(Ref<DrawCallBuffer> buffer)>> m_SubmitUniforms {};
		std::vector<std::function<void(Ref<DrawCallBuffer> buffer)>> m_DrawFunctions {};

		std::vector<std::function<void(Ref<DrawCallBuffer> buffer)>> m_PreDrawBuffer {};
		std::vector<std::function<void(Ref<DrawCallBuffer> buffer)>> m_PostDrawBuffer {};
		Ref<DrawCallBuffer> m_CurrentDrawCall = nullptr;
		Ref<VertexArray> m_VertexArray = nullptr;

	};
}

namespace Kargono::Utility
{
	static std::string ColorInputTypeToString(Rendering::ColorInputType colorInput)
	{
		switch (colorInput)
		{
		case Rendering::ColorInputType::None: return "None";
		case Rendering::ColorInputType::FlatColor: return "FlatColor";
		case Rendering::ColorInputType::VertexColor: return "VertexColor";
		}
		KG_ERROR("Unknown Data Type sent to ColorInputToString Function");
		return "None";
	}

	static Rendering::ColorInputType StringToColorInputType(std::string_view string)
	{
		if (string == "None") { return Rendering::ColorInputType::None; }
		if (string == "FlatColor") { return Rendering::ColorInputType::FlatColor; }
		if (string == "VertexColor") { return Rendering::ColorInputType::VertexColor; }

		KG_ERROR("Unknown Data Type sent to StringToRendering::ColorInputType Function");
		return Rendering::ColorInputType::None;
	}

	static std::string TextureInputTypeToString(Rendering::TextureInputType textureInput)
	{
		switch (textureInput)
		{
		case Rendering::TextureInputType::None: return "None";
		case Rendering::TextureInputType::ColorTexture: return "ColorTexture";
		case Rendering::TextureInputType::TextTexture: return "TextTexture";
		}
		KG_ERROR("Unknown Data Type sent to TextureInputToString Function");
		return "None";
	}

	static Rendering::TextureInputType StringToTextureInputType(std::string_view string)
	{
		if (string == "None") { return Rendering::TextureInputType::None; }
		if (string == "ColorTexture") { return Rendering::TextureInputType::ColorTexture; }
		if (string == "TextTexture") { return Rendering::TextureInputType::TextTexture; }

		KG_ERROR("Unknown Data Type sent to StringToTextureInputType Function");
		return Rendering::TextureInputType::None;
	}
}
