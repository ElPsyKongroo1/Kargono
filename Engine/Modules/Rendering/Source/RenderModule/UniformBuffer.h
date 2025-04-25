#pragma once

#include "Kargono/Core/Base.h"

#include <string>

namespace Kargono::Rendering
{

	enum class UniformDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool, Sampler2D
	};

	struct UniformElement
	{
	public:
		std::string Name;
		UniformDataType Type;
	public:
		UniformElement() = default;

		UniformElement(UniformDataType type, std::string_view name)
			: Name(name), Type(type)
		{
		}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
			case UniformDataType::Float:	return 1;
			case UniformDataType::Float2:	return 2;
			case UniformDataType::Float3:	return 3;
			case UniformDataType::Float4:	return 4;
			case UniformDataType::Sampler2D:return 4;
			case UniformDataType::Int:		return 1;
			case UniformDataType::Int2:		return 2;
			case UniformDataType::Int3:		return 3;
			case UniformDataType::Int4:		return 4;
			case UniformDataType::Mat3:		return 3; // 3* float3
			case UniformDataType::Mat4:		return 4; // 4* float4
			case UniformDataType::Bool:		return 1;
			}
			KG_ERROR("Unknown UniformDataType!");
			return 0;
		}
	};

	class UniformBufferList
	{
	public:
		UniformBufferList() {}

		UniformBufferList(std::initializer_list<UniformElement> elements)
			: m_Elements(elements)
		{
		}

		const std::vector<UniformElement>& GetElements() const { return m_Elements; }

		void AddBufferElement(const UniformElement& uniformElement)
		{
			m_Elements.push_back(uniformElement);
		}

		void Clear() { m_Elements.clear(); }

		std::vector<UniformElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<UniformElement>::iterator end() { return m_Elements.end(); }
		std::vector<UniformElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<UniformElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		std::vector<UniformElement> m_Elements {};
	};

	class UniformBuffer
	{
	public:
		virtual ~UniformBuffer() {}
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

		static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);
	};

}

namespace Kargono::Utility
{
	inline uint32_t UniformDataTypeSize(Rendering::UniformDataType type)
	{
		switch (type)
		{
		case Rendering::UniformDataType::Float:		return 4;
		case Rendering::UniformDataType::Float2:		return 4 * 2;
		case Rendering::UniformDataType::Float3:		return 4 * 3;
		case Rendering::UniformDataType::Float4:		return 4 * 4;
		case Rendering::UniformDataType::Mat3:			return 4 * 3 * 3;
		case Rendering::UniformDataType::Mat4:			return 4 * 4 * 4;
		case Rendering::UniformDataType::Int:			return 4;
		case Rendering::UniformDataType::Int2:			return 4 * 2;
		case Rendering::UniformDataType::Int3:			return 4 * 3;
		case Rendering::UniformDataType::Int4:			return 4 * 4;
		case Rendering::UniformDataType::Bool:			return 1;
		case Rendering::UniformDataType::Sampler2D:	return 4 * 4;
		}
		KG_ERROR("Unknown UniformDataType!");
		return 0;
	}

	inline Rendering::UniformDataType StringToUniformDataType(std::string_view type)
	{
		if (type == "int") { return Rendering::UniformDataType::Int; }
		if (type == "float") { return Rendering::UniformDataType::Float; }
		if (type == "vec2") { return Rendering::UniformDataType::Float2; }
		if (type == "vec3") { return Rendering::UniformDataType::Float3; }
		if (type == "vec4") { return Rendering::UniformDataType::Float4; }
		if (type == "mat3") { return Rendering::UniformDataType::Mat3; }
		if (type == "mat4") { return Rendering::UniformDataType::Mat4; }
		if (type == "sampler2D") { return Rendering::UniformDataType::Sampler2D; }
		if (type == "bool") { return Rendering::UniformDataType::Bool; }

		KG_ERROR("Unknown String trying to convert to UniformDataType!");
		return Rendering::UniformDataType::None;
	}

	inline const char* UniformDataTypeToString(Rendering::UniformDataType type)
	{
		switch (type)
		{
		case Rendering::UniformDataType::Float:		return "float";
		case Rendering::UniformDataType::Float2:		return "vec2";
		case Rendering::UniformDataType::Float3:		return "vec3";
		case Rendering::UniformDataType::Float4:		return "vec4";
		case Rendering::UniformDataType::Mat3:			return "mat3";
		case Rendering::UniformDataType::Mat4:			return "mat4";
		case Rendering::UniformDataType::Int:			return "int";
		case Rendering::UniformDataType::Bool:			return "bool";
		case Rendering::UniformDataType::Sampler2D:	return "sampler2D";
		}

		KG_ERROR("Unknown UniformDataType for conversion to string!");
		return "";
	}
}
