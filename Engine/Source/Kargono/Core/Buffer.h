#pragma once

#include <stdint.h>
#include <cstring>

namespace Kargono
{
	//==============================
	// Buffer Class
	//==============================
	// Non-owning raw buffer struct
	struct Buffer
	{
		uint8_t* Data = nullptr;
		uint64_t Size = 0;

		// Does not allocate any heap data
		Buffer() = default;
		Buffer(const Buffer&) = default;

		// Need to call release when instantiated this way
		Buffer(uint64_t size)
		{
			Allocate(size);
		}

		static Buffer Copy(Buffer other)
		{
			if (other)
			{
				Buffer result(other.Size);
				memcpy(result.Data, other.Data, other.Size);
				return result;
			}
			return {};
			
		}

		void Allocate(uint64_t size)
		{
			Release();
			Data = new uint8_t[size];
			Size = size;
		}

		void Release()
		{
			delete[] Data;
			Data = nullptr;
			Size = 0;
		}

		void SetDataToByte(uint8_t byte)
		{
			if (Size == 0) { return; }
			memset(Data, byte, Size);
		}

		template<typename T>
		T* As()
		{
			return (T*)Data;
		}

		template<typename T>
		T* As(std::size_t offsetInBytes)
		{
			return (T*)(Data + offsetInBytes);
		}
		operator bool() const
		{
			return (bool)Data;
		}

		void SetString(const std::string& string)
		{
			if (Size < string.size())
			{
				Allocate(string.size());
			}
			SetDataToByte(0);
			memcpy(Data, string.data(), string.size());
		}

		std::string GetString()
		{
			return { this->As<char>() };
		}
	};
	//==============================
	// Scoped Buffer Class
	//==============================
	struct ScopedBuffer
	{

		ScopedBuffer(Buffer buffer)
			:m_Buffer(buffer)
		{
		}

		ScopedBuffer(uint64_t size)
			: m_Buffer(size)
		{
			
		}

		ScopedBuffer()
		{
			
		}

		~ScopedBuffer()
		{
			m_Buffer.Release();
		}

		uint8_t* Data() { return m_Buffer.Data; }
		uint64_t Size() { return m_Buffer.Size; }
		void Allocate(uint64_t size) { m_Buffer.Allocate(size); }

		template<typename T>
		T* As()
		{
			return m_Buffer.As<T>();
		}

		operator bool() const
		{
			return m_Buffer;
		}

	private:
		Buffer m_Buffer;
	};
}
