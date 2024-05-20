#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/Log.h"

#include <string>
#include <unordered_map>

namespace Kargono
{
	enum class InputDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	static uint32_t ShaderDataTypeSize(InputDataType type)
	{
		switch (type)
		{
		case InputDataType::Float:		return 4;
		case InputDataType::Float2:		return 4 * 2;
		case InputDataType::Float3:		return 4 * 3;
		case InputDataType::Float4:		return 4 * 4;
		case InputDataType::Mat3:		return 4 * 3 * 3;
		case InputDataType::Mat4:		return 4 * 4 * 4;
		case InputDataType::Int:		return 4;
		case InputDataType::Int2:		return 4 * 2;
		case InputDataType::Int3:		return 4 * 3;
		case InputDataType::Int4:		return 4 * 4;
		case InputDataType::Bool:		return 1;
		}
		KG_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	static InputDataType StringToInputDataType(std::string_view type)
	{
		if (type == "int")		{ return InputDataType::Int; }
		if (type == "float")	{ return InputDataType::Float; }
		if (type == "vec2")		{ return InputDataType::Float2; }
		if (type == "vec3")		{ return InputDataType::Float3; }
		if (type == "vec4")		{ return InputDataType::Float4; }
		if (type == "mat3")		{ return InputDataType::Mat3; }
		if (type == "mat4")		{ return InputDataType::Mat4; }
		if (type == "bool")		{ return InputDataType::Bool; }

		KG_CORE_ASSERT(false, "Unknown String trying to convert to InputDataType!");
		return InputDataType::None;
	}

	static std::string InputDataTypeToString(InputDataType type)
	{
		switch (type)
		{
		case InputDataType::Float:		return "float";
		case InputDataType::Float2:		return "vec2";
		case InputDataType::Float3:		return "vec3";
		case InputDataType::Float4:		return "vec4";
		case InputDataType::Mat3:		return "mat3";
		case InputDataType::Mat4:		return 	"mat4";
		case InputDataType::Int:		return "int";
		case InputDataType::Bool:		return "bool";
		}

		KG_CORE_ASSERT(false, "Unknown DataType to String Conversion");
		return "";
		
	}

	struct InputBufferElement
	{
	public:
		std::string Name;
		InputDataType Type;
		uint32_t Size;
		size_t Offset;
		bool Normalized = false;
	public:
		InputBufferElement() = default;

		InputBufferElement(InputDataType type, std::string_view name, bool normalized = false)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{
		}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
			case InputDataType::Float:		return 1;
			case InputDataType::Float2:		return 2;
			case InputDataType::Float3:		return 3;
			case InputDataType::Float4:		return 4;
			case InputDataType::Int:		return 1;
			case InputDataType::Int2:		return 2;
			case InputDataType::Int3:		return 3;
			case InputDataType::Int4:		return 4;
			case InputDataType::Mat3:		return 3; // 3* float3
			case InputDataType::Mat4:		return 4; // 4* float4
			case InputDataType::Bool:		return 1;
			}
			KG_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}

		operator bool() const
		{
			if (Name.empty()) { return false; }
			return true;
		}
	};

	class InputBufferLayout
	{
	public:
		InputBufferLayout() {}

		InputBufferLayout(std::initializer_list<InputBufferElement> elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		const std::vector<InputBufferElement>& GetElements() const { return m_Elements; }
		uint32_t GetStride() const { return m_Stride; }

		void AddBufferElement(const InputBufferElement& bufferElement)
		{
			m_Elements.push_back(bufferElement);
			CalculateOffsetsAndStride();
		}

		InputBufferElement FindElementByName(const std::string& name)
		{
			if (!m_ElementLocations.contains(name))
			{
				//KG_CORE_ERROR("Could not find element inside InputBufferLayout using name: {0}", name);
				return {};
			}

			return m_Elements.at(m_ElementLocations.at(name));
		}

		void Clear() { m_Elements.clear(); CalculateOffsetsAndStride(); }

		std::vector<InputBufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<InputBufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<InputBufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<InputBufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsAndStride ()
		{
			m_Stride = 0;
			m_ElementLocations.clear();
			if (m_Elements.empty()) { return; }

			size_t offset = 0;
			uint32_t iteration = 0;
			for (auto& element : m_Elements)
			{
				m_ElementLocations.insert({ element.Name, static_cast<std::uint32_t>(iteration) });
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
				iteration++;
			}
		}
	private:
		std::vector<InputBufferElement> m_Elements {};
		std::unordered_map<std::string, uint32_t> m_ElementLocations {};
		uint32_t m_Stride = 0;
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;
	public:

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void SetData(const void* data, uint32_t size) = 0;

		virtual const InputBufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const InputBufferLayout& layout) = 0;


		static Ref<VertexBuffer> Create(uint32_t size);
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size);
	};
	// Currently Kargono only supports 32-bit index buffers
	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;
	public:
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);
	};
}
