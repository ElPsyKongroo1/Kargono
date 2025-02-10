#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/Log.h"
#include "Kargono/Utility/FileSystem.h"

#include <string>
#include <unordered_map>

namespace Kargono::Rendering
{
	enum class InputDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
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


namespace Kargono::Rendering
{

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
			: Name(name), Type(type), Size(Utility::ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
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
			KG_ERROR("Unknown ShaderDataType!");
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

		InputBufferElement* FindElementByName(uint32_t nameHash)
		{
			if (!m_ElementLocations.contains(nameHash))
			{
				KG_WARN("Could not locate element inside InputBufferLayout")
				return {};
			}

			return &m_Elements.at(m_ElementLocations.at(nameHash));
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
			for (InputBufferElement& element : m_Elements)
			{
				m_ElementLocations.insert({ Utility::FileSystem::CRCFromString(element.Name.c_str()), static_cast<std::uint32_t>(iteration) });
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
				iteration++;
			}
		}
	private:
		std::vector<InputBufferElement> m_Elements {};
		std::unordered_map<uint32_t, uint32_t> m_ElementLocations {};
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
