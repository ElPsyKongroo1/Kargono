#pragma once
#include "Kargono/Core.h"
#include "Kargono/Log.h"

namespace Kargono
{
/// @brief Enumeration defining different shader data types
	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

/// @brief Gets the size (in bytes) of a given ShaderDataType
/// @param type The ShaderDataType to get the size of
/// @return The size (in bytes) of the given ShaderDataType
	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:		return 4;
		case ShaderDataType::Float2:	return 4 * 2;
		case ShaderDataType::Float3:	return 4 * 3;
		case ShaderDataType::Float4:	return 4 * 4;
		case ShaderDataType::Mat3:		return 4 * 3 * 3;
		case ShaderDataType::Mat4:		return 4 * 4 * 4;
		case ShaderDataType::Int:		return 4;
		case ShaderDataType::Int2:		return 4 * 2;
		case ShaderDataType::Int3:		return 4 * 3;
		case ShaderDataType::Int4:		return 4 * 4;
		case ShaderDataType::Bool:		return 1;
		}
		KG_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

/// @brief Structure defining a buffer element
	struct BufferElement
	{
	public:
/// @brief Name of the buffer element
/// @details The Name member variable stores the name of the buffer element as a std::string
		std::string Name;
/// @brief Type of the buffer element
/// @details The Type member variable stores the type of the buffer element as a ShaderDataType
		ShaderDataType Type;
/// @brief Size (in bytes) of the buffer element
/// @details The Size member variable stores the size (in bytes) of the buffer element as a uint32_t
		uint32_t Size;
/// @brief Offset (in bytes) of the buffer element
/// @details The Offset member variable stores the offset (in bytes) of the buffer element as a uint32_t
		uint32_t Offset;
/// @brief Normalized flag of the buffer element
/// @details The Normalized member variable stores the normalized flag of the buffer element as a bool
		bool Normalized;
	public:
/// @brief Default constructor for the BufferElement struct
		BufferElement() {}

/// @brief Constructor for the BufferElement struct
/// @param type The type of the buffer element
/// @param name The name of the buffer element
/// @param normalized The normalized flag of the buffer element (default: false)
		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{
		}

/// @brief Gets the number of components in the buffer element
/// @return The number of components in the buffer element
		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
			case ShaderDataType::Float:		return 1;
			case ShaderDataType::Float2:	return 2;
			case ShaderDataType::Float3:	return 3;
			case ShaderDataType::Float4:	return 4;
			case ShaderDataType::Int:		return 1;
			case ShaderDataType::Int2:		return 2;
			case ShaderDataType::Int3:		return 3;
			case ShaderDataType::Int4:		return 4;
			case ShaderDataType::Mat3:		return 3 * 3;
			case ShaderDataType::Mat4:		return 4 * 4;
			case ShaderDataType::Bool:		return 1;
			}
			KG_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	};

/// @brief Class defining a buffer layout for vertex buffers
	class BufferLayout
	{
	public:
/// @brief Default constructor for the BufferLayout class
		BufferLayout() {}

/// @brief Constructor for the BufferLayout class
/// @param elements The initializer list of BufferElements that make up the layout
		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

/// @brief Gets the elements of the buffer layout
/// @return The elements of the buffer layout as a const std::vector<BufferElement>&
		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }
/// @brief Gets the stride (in bytes) of the buffer layout
/// @return The stride (in bytes) of the buffer layout
		inline uint32_t GetStride() const { return m_Stride; }

/// @brief Gets an iterator to the beginning of the elements in the buffer layout
/// @return An iterator to the beginning of the elements in the buffer layout
		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
/// @brief Gets an iterator to the end of the elements in the buffer layout
/// @return An iterator to the end of the elements in the buffer layout
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
/// @brief Gets a const iterator to the beginning of the elements in the buffer layout
/// @return A const iterator to the beginning of the elements in the buffer layout
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
/// @brief Gets a const iterator to the end of the elements in the buffer layout
/// @return A const iterator to the end of the elements in the buffer layout
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
/// @brief Calculates the offsets and stride of the buffer layout
		void CalculateOffsetsAndStride ()
		{
			uint32_t offset = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

/// @brief Abstract class for a vertex buffer
	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}
	public:

/// @brief Binds the vertex buffer
/// @brief Binds the index buffer
		virtual void Bind() const = 0;
/// @brief Unbinds the vertex buffer
/// @brief Unbinds the index buffer
		virtual void Unbind() const = 0;
/// @brief Gets the layout of the vertex buffer
/// @return The layout of the vertex buffer as a const BufferLayout&
		virtual const BufferLayout& GetLayout() const = 0;
/// @brief Sets the layout of the vertex buffer
/// @param layout The layout of the vertex buffer
		virtual void SetLayout(const BufferLayout& layout) = 0;

/// @brief Creates a new vertex buffer
/// @param vertices The vertices as a float pointer
/// @param size The size (in bytes) of the vertex buffer
/// @return A pointer to the newly created vertex buffer
		static VertexBuffer* Create(float* vertices, uint32_t size);
	};

/// @brief Abstract class for an index buffer
	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}
	public:
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

/// @brief Gets the number of indices in the index buffer
/// @return The number of indices in the index buffer
		virtual uint32_t GetCount() const = 0;

/// @brief Creates a new index buffer
/// @param indices The indices as a uint32_t pointer
/// @param size The size (in bytes) of the index buffer
/// @return A pointer to the newly created index buffer
		static IndexBuffer* Create(uint32_t* indices, uint32_t size);
	};
}
