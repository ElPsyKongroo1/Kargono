#pragma once

#include <cstdint>
#include <cstring>

namespace Kargono
{
	struct Buffer
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Buffer() = default;
		Buffer(const Buffer&) = default;
		Buffer(size_t size)
		{
			Allocate(size);
		}
	public:
		//==============================
		// Duplicate Buffer
		//==============================
		static Buffer Copy(Buffer other)
		{
			if (other)
			{
				Buffer result(other.m_Size);
				memcpy(result.m_Data, other.m_Data, other.m_Size);
				return result;
			}
			return {};
		}
		//==============================
		// Manage Heap
		//==============================
		void Allocate(size_t size)
		{
			Release();
			m_Data = new uint8_t[size];
			m_Size = size;
		}
		void Release()
		{
			delete[] m_Data;
			m_Data = nullptr;
			m_Size = 0;
		}
	public:
		//==============================
		// Set Data in Buffer
		//==============================
		void SetDataToByte(uint8_t byte)
		{
			if (m_Size == 0) { return; }
			memset(m_Data, byte, m_Size);
		}

		void SetString(const std::string& string)
		{
			if (m_Size < string.size())
			{
				Allocate(string.size());
			}
			SetDataToByte(0);
			memcpy(m_Data, string.data(), string.size());
		}

	public:
		//==============================
		// Retrieve Data from Buffer
		//==============================
		template<typename T>
		T* As()
		{
			return (T*)m_Data;
		}

		template<typename T>
		T* As(size_t offsetInBytes)
		{
			return (T*)(m_Data + offsetInBytes);
		}

		std::string GetString()
		{
			return { this->As<char>() };
		}

		operator bool() const
		{
			return (bool)m_Data;
		}

	public:
		//==============================
		// Public Fields
		//==============================
		uint8_t* m_Data{ nullptr };
		size_t m_Size{ 0 };
	};
}
