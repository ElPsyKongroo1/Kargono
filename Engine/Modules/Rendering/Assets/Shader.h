#pragma once

#include "Kargono/Core/Base.h"
#include "Modules/Rendering/InputBuffer.h"
#include "Modules/Rendering/UniformBuffer.h"
#include "Modules/Rendering/VertexArray.h"
#include "Modules/Rendering/Shape.h"
#include "Kargono/Core/Buffer.h"
#include "Kargono/Math/Math.h"
#include "Modules/Rendering/Module/RenderingModule.h"
#include "Modules/Assets/Module/AssetTag.h"
#include "Modules/Assets/AssetReference.h"

#include <string>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <functional>
#include <tuple>

namespace Kargono::Rendering
{
	struct ShapeComponent;
}
namespace Kargono 
{
	struct Transform;
}

namespace Kargono::Rendering
{
	typedef unsigned int GLenum;

	class Shader;
	class Texture2D;

	struct DrawCallBuffer
	{
		Buffer m_VertexBuffer{};
		uint8_t* m_VertexBufferIterator{ m_VertexBuffer.m_Data };
		std::vector<uint32_t> m_IndexBuffer {};
		std::vector<Assets::AssetRef<Texture2D>> m_Textures {};
		Shader* m_Shader{ nullptr };
	};

	struct RendererInputSpec
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		RendererInputSpec() = default;
		~RendererInputSpec() = default;
	public:
		//==============================
		// Clean Up
		//==============================
		void ClearData();
	public:
		//==============================
		// Public Fields
		//==============================
		Assets::TAssetRef<Shader> m_Shader;
		Assets::TAssetRef<Texture2D> m_Texture;
		Rendering::ShapeComponent* m_ShapeComponent{ nullptr }; // TODO: The shape component is a memory leak...
		Ref<DrawCallBuffer> m_CurrentDrawBuffer;
		Buffer m_Buffer;
		uint32_t m_Entity{ (uint32_t)-1};
		void* m_EntityRegistry;
		Math::mat4 m_TransformMatrix;
		Math::mat4 m_ObjectOutlineMatrix;
	};

	enum class ColorInputType
	{
		None = 0, 
		FlatColor, 
		VertexColor
	};

	enum class TextureInputType
	{
		None = 0, 
		ColorTexture, 
		TextTexture
	};

	using ShaderSource = std::string;

	struct ShaderSpecification
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ShaderSpecification(const ShaderSpecification&) = default;
		ShaderSpecification() = default;
		ShaderSpecification(ColorInputType colorInput, TextureInputType textureInput, 
			bool addCircle, bool addProjection, bool addEntityID, 
			RenderingType renderType, bool drawOutline);
	public:
		//==============================
		// Operator Overload(s)
		//==============================
		auto operator<=>(const ShaderSpecification&) const = default;
	public:
		//==============================
		// Public Fields
		//==============================
		// Pixel Color Options
		ColorInputType m_ColorInput{ ColorInputType::None };
		TextureInputType m_TextureInput{ TextureInputType::None };
		// Structure Change Options
		bool m_AddCircleShape{ false };
		// Other Options
		bool m_AddProjectionMatrix{ false };
		bool m_AddEntityID{ false };
		// Rendering Options
		RenderingType m_RenderType{ RenderingType::None };
		bool m_DrawOutline{ false };
	};

	struct ShaderMetaData
	{
	public:
		//==============================
		// Constructor(s)/Destructor(s)
		//==============================
		ShaderMetaData() = default;
		~ShaderMetaData() = default;
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Public Fields
		//==============================
		ShaderSpecification m_ShaderSpec{};
		UniformBufferList m_UniformList{};
		InputBufferLayout m_InputLayout{};
	};

	class Shader
	{
	public:
		//==============================
		// Interact with Input Locations
		//==============================
		template<typename T>
		static T* GetInputLocation(uint32_t inputNameHash, Buffer inputBuffer, Assets::AssetRef<Shader> shader)
		{
			InputBufferElement* currentInputBufferElement = shader->GetInputLayout().FindElementByName(inputNameHash);

			if (!currentInputBufferElement)
			{
				KG_WARN("Input name hash did not retrieve an InputBufferElement");
				return nullptr;
			}
			std::size_t inputLocation = currentInputBufferElement->Offset;
			return inputBuffer.As<T>(inputLocation);
		}
		template<typename T>
		static void SetDataAtInputLocation(const T& value, uint32_t inputNameHash, Buffer inputBuffer, Assets::AssetRef<Shader> shader)
		{
			InputBufferElement* currentInputBufferElement = shader->GetInputLayout().FindElementByName(inputNameHash);

			if (!currentInputBufferElement)
			{
				KG_WARN("Input name hash did not retrieve an InputBufferElement");
				return;
			}

			std::size_t inputLocation = currentInputBufferElement->Offset;
			T* inputPointer = inputBuffer.As<T>(inputLocation);
			*inputPointer = value;
		}
	public:
		//==============================
		// Metaprogramming Info
		//==============================
		using Metadata = ShaderMetaData;
		using Spec = ShaderSpecification;

		constexpr static std::array<FixedBufStr16, 3> k_IntermediateExtensions
		{ 
			".kgshadersrc", 
			".kgshaderfrag", 
			".kgshadervert"
		};
	public:
		//==============================
		// Asset Config Info
		//==============================
		constexpr static FixedBufStr32 GetAssetName()
		{
			return "Shader";
		}

		constexpr static Assets::AssetFlags GetAssetFlags()
		{
			Assets::AssetFlags flags{};
			flags.SetFlag(Assets::AssetFlag::HasAssetCache);
			flags.ClearFlag(Assets::AssetFlag::RequireUniqueName);
			return flags;
		}

		constexpr static std::span<const FixedBufStr16> GetIntermediateExtensions()
		{
			return std::span(k_IntermediateExtensions.data(), k_IntermediateExtensions.size());
		}
		
		static Utility::SHA256Hash GetHashFromSpec(const ShaderSpecification& spec);
		static bool GetAssetFromSpec(Assets::Metadata& metadata, const ShaderSpecification& querySpec);
		static void CreateFromSpec(Assets::Metadata& metadata, const ShaderSpecification& spec);

	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Shader() = default;
		~Shader()
		{
			if (m_Registered)
			{
				DeregisterShader();
			}
		}
		Shader(const ShaderSpecification& shader) : m_ShaderSpecification(shader) {}
	public:
		//==============================
		// Interact with Renderer
		//==============================
		// Register shader w/ renderer
		void RegisterShader(const std::unordered_map<GLenum, std::vector<uint32_t>>& openGLSPIRV);
		void DeregisterShader();
		// Binding with OpenGL state machine
		void Bind() const;
		void Unbind() const;
		// Modify underlying uniform
		void SetMat3Uniform(const char* name, const Math::mat3& value);
		void SetMat4Uniform(const char* name, const Math::mat4& value);
		void SetFloatUniform(const char* name, float value);
		void SetFloat2Uniform(const char* name, const Math::vec2& value);
		void SetFloat3Uniform(const char* name, const Math::vec3& value);
		void SetFloat4Uniform(const char* name, const Math::vec4& value);
		void SetIntUniform(const char* name, int value);
		void SetIntArrayUniform(const char* name, int* values, uint32_t count);
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Getters/Setters
		//==============================
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
	private:
		//==============================
		// Internal Fields
		//==============================
		std::string m_Name;
		bool m_Registered{ false };
		// Renderer Specific Functionality
		std::vector<std::function<void(RendererInputSpec& spec)>> m_FillDataPerObject {};
		std::vector<std::function<void(RendererInputSpec& spec, uint32_t iteration)>> m_FillDataPerVertex {};
		std::vector<std::function<void(RendererInputSpec& spec)>> m_FillDataInScene {};
		std::vector<std::function<void(Ref<DrawCallBuffer> buffer)>> m_SubmitUniforms {};
		std::vector<std::function<void(Ref<DrawCallBuffer> buffer)>> m_DrawFunctions {};
		// Pre and post draw call functionality
		std::vector<std::function<void(Ref<DrawCallBuffer> buffer)>> m_PreDrawBuffer {};
		std::vector<std::function<void(Ref<DrawCallBuffer> buffer)>> m_PostDrawBuffer {};
		Ref<DrawCallBuffer> m_CurrentDrawCall{ nullptr };
		Ref<VertexArray> m_VertexArray{ nullptr };
		// Underlying OpenGL hook
		uint32_t m_RendererID;
	};

	Register_Module_Type(Shader, Assets::AssetTag)
}

namespace Kargono::Utility
{
	inline const char* ColorInputTypeToString(Rendering::ColorInputType colorInput)
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

	inline Rendering::ColorInputType StringToColorInputType(std::string_view string)
	{
		if (string == "None") { return Rendering::ColorInputType::None; }
		if (string == "FlatColor") { return Rendering::ColorInputType::FlatColor; }
		if (string == "VertexColor") { return Rendering::ColorInputType::VertexColor; }

		KG_ERROR("Unknown Data Type sent to StringToRendering::ColorInputType Function");
		return Rendering::ColorInputType::None;
	}

	inline const char* TextureInputTypeToString(Rendering::TextureInputType textureInput)
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

	inline Rendering::TextureInputType StringToTextureInputType(std::string_view string)
	{
		if (string == "None") { return Rendering::TextureInputType::None; }
		if (string == "ColorTexture") { return Rendering::TextureInputType::ColorTexture; }
		if (string == "TextTexture") { return Rendering::TextureInputType::TextTexture; }

		KG_ERROR("Unknown Data Type sent to StringToTextureInputType Function");
		return Rendering::TextureInputType::None;
	}
}
