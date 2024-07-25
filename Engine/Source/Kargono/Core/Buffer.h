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
		//==============================
		// Constructors/Destructors
		//==============================
		// Does not allocate any heap data
		Buffer() = default;
		Buffer(const Buffer&) = default;

		// Need to call release when instantiated this way
		Buffer(uint64_t size)
		{
			Allocate(size);
		}
		//==============================
		// Duplicate Buffer
		//==============================
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
		//==============================
		// Manage Heap
		//==============================
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
		//==============================
		// Set Data in Buffer
		//==============================
		void SetDataToByte(uint8_t byte)
		{
			if (Size == 0) { return; }
			memset(Data, byte, Size);
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

		//==============================
		// Retrieve Data from Buffer
		//==============================
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

		std::string GetString()
		{
			return { this->As<char>() };
		}

		operator bool() const
		{
			return (bool)Data;
		}
	};
}
