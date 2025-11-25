#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/Log.h"
#include "Modules/FileSystem/FileSystem.h"

#include <string>
#include <unordered_map>

namespace Kargono::Rendering
{
	enum class InputDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	struct InputBufferElement
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		InputBufferElement() = default;
		InputBufferElement(InputDataType type, std::string_view name, bool normalized = false);
	public:
		//==============================
		// Getters/Setters
		//==============================
		uint32_t GetComponentCount() const;
	public:
		//==============================
		// Operator Overloads
		//==============================
		operator bool() const;
	public:
		//==============================
		// Public Fields
		//==============================
		std::string m_Name{};
		InputDataType m_Type{};
		uint32_t m_Size{};
		size_t m_Offset{};
		bool m_Normalized{ false };
	};

	class InputBufferLayout
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		InputBufferLayout() = default;
		InputBufferLayout(std::initializer_list<InputBufferElement> elements);
	public:
		//==============================
		// Interact With Layout
		//==============================
		InputBufferElement* FindElementByHash(uint32_t nameHash);
		void AddBufferElement(const InputBufferElement& bufferElement);
		void Clear() { m_Elements.clear(); CalculateOffsetsAndStride(); }
	private:
		// Helper(s)
		void CalculateOffsetsAndStride();
	public:
		//==============================
		// Getters/Setters
		//==============================
		const std::vector<InputBufferElement>& GetElements() const { return m_Elements; }
		uint32_t GetStride() const { return m_Stride; }
	public:
		//==============================
		// Iterators
		//==============================
		std::vector<InputBufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<InputBufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<InputBufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<InputBufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		//==============================
		// Internal Fields
		//==============================
		std::vector<InputBufferElement> m_Elements {};
		std::unordered_map<uint32_t, uint32_t> m_ElementLocations {};
		uint32_t m_Stride = 0;
	};

	class VertexBuffer
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		VertexBuffer() = default;
		~VertexBuffer()
		{
			if (m_Registered)
			{
				DeregisterBuffer();
			}
		}
	public:
		//==============================
		// Interact With Renderer
		//==============================
		// Register w/ renderer
		void RegisterBuffer(uint32_t size);
		void RegisterBuffer(float* vertices, uint32_t size);
		void DeregisterBuffer();
		// Binding with OpenGL state machine
		void Bind() const;
		void Unbind() const;
		// Set data in renderer
		void SetData(const void* data, uint32_t size);
	public:
		//==============================
		// Getters/Setters
		//==============================
		const InputBufferLayout& GetLayout() const { return m_Layout; }
		void SetLayout(const InputBufferLayout& layout) { m_Layout = layout; }
	private:
		bool m_Registered{ false };
		uint32_t m_RendererID;
		InputBufferLayout m_Layout;
	};

	class IndexBuffer
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		IndexBuffer() = default;
		~IndexBuffer()
		{
			if (m_Registered)
			{
				DeregisterBuffer();
			}
		}
	public:
		//==============================
		// Interact With Renderer
		//==============================
		// Register w/ OpenGL
		void RegisterBuffer(uint32_t* indices, uint32_t count);
		void DeregisterBuffer();
		// Bind to OpenGL state machine
		void Bind() const;
		void Unbind() const;
	public:
		//==============================
		// Getters/Setters
		//==============================
		uint32_t GetCount() const { return m_Count; };
	private:
		bool m_Registered{ false };
		uint32_t m_RendererID;
		uint32_t m_Count;

	};
}

namespace Kargono::Utility
{
	inline uint32_t ShaderDataTypeSize(Rendering::InputDataType type)
	{
		switch (type)
		{
		case Rendering::InputDataType::Float:		return 4;
		case Rendering::InputDataType::Float2:		return 4 * 2;
		case Rendering::InputDataType::Float3:		return 4 * 3;
		case Rendering::InputDataType::Float4:		return 4 * 4;
		case Rendering::InputDataType::Mat3:		return 4 * 3 * 3;
		case Rendering::InputDataType::Mat4:		return 4 * 4 * 4;
		case Rendering::InputDataType::Int:		return 4;
		case Rendering::InputDataType::Int2:		return 4 * 2;
		case Rendering::InputDataType::Int3:		return 4 * 3;
		case Rendering::InputDataType::Int4:		return 4 * 4;
		case Rendering::InputDataType::Bool:		return 1;
		}
		KG_ERROR("Unknown ShaderDataType!");
		return 0;
	}

	inline Rendering::InputDataType StringToInputDataType(std::string_view type)
	{
		if (type == "int") { return Rendering::InputDataType::Int; }
		if (type == "float") { return Rendering::InputDataType::Float; }
		if (type == "vec2") { return Rendering::InputDataType::Float2; }
		if (type == "vec3") { return Rendering::InputDataType::Float3; }
		if (type == "vec4") { return Rendering::InputDataType::Float4; }
		if (type == "mat3") { return Rendering::InputDataType::Mat3; }
		if (type == "mat4") { return Rendering::InputDataType::Mat4; }
		if (type == "bool") { return Rendering::InputDataType::Bool; }

		KG_ERROR("Unknown String trying to convert to InputDataType!");
		return Rendering::InputDataType::None;
	}

	inline const char* InputDataTypeToString(Rendering::InputDataType type)
	{
		switch (type)
		{
		case Rendering::InputDataType::Float:		return "float";
		case Rendering::InputDataType::Float2:		return "vec2";
		case Rendering::InputDataType::Float3:		return "vec3";
		case Rendering::InputDataType::Float4:		return "vec4";
		case Rendering::InputDataType::Mat3:		return "mat3";
		case Rendering::InputDataType::Mat4:		return 	"mat4";
		case Rendering::InputDataType::Int:		return "int";
		case Rendering::InputDataType::Bool:		return "bool";
		}

		KG_ERROR("Unknown DataType to String Conversion");
		return "";

	}
}