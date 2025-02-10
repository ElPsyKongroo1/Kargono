#pragma once
#include <array>
#include <cstring>
#include <cstdio>
#include <charconv>
#include <string>

namespace Kargono
{
	template<std::size_t BufferSize>
	class FixedString
	{
		static_assert(BufferSize > 0, "Cannot instantiate fixed string with empty buffer");

	public:
		//==============================
		// Constructors/Destructors
		//==============================
		FixedString()
		{
			m_DataBuffer[0] = '\0';
		}

		FixedString(const char* newString)
		{
			if (!newString)
			{
				ClearString();
				return;
			}

			ReplaceBuffer(newString);
		}

		FixedString(char newChar)
		{
			// TODO: I realize that a buffer of size one would cause an issue, but like really thooooooo

			// Add char
			m_DataBuffer[0] = newChar;

			// Add null terminator
			m_DataBuffer[1] = '\0';
		}

	public:
		//==============================
		// Modify String
		//==============================

		void ClearString()
		{
			m_DataBuffer[0] = '\0';
			m_StringLength = 0;
		}

		void SetString(const char* newString)
		{
			ReplaceBuffer(newString);
		}

		
		template<typename... VariadicArgs>
		bool SetFormat(const char* formatString, VariadicArgs&&... args)
		{
			// Note that this function may truncate resulting formatted string if the buffer is too small
			// Note that snprintf deals with null termination automatically

			// Replace data inside buffer with formatted string and get resultant size of buffer
			
			int32_t newStringSize = std::snprintf(m_DataBuffer.data(), BufferSize, formatString, std::forward<VariadicArgs>(args)...);

			// If snprintf fails, return false
			if (newStringSize < 0) 
			{
				return false; 
			}

			// If success, update string length
			m_StringLength = newStringSize;
			return true;
		}

		template<typename... VariadicArgs>
		bool AppendFormat(const char* formatString, VariadicArgs&&... args)
		{
			// Note that this function may truncate resulting formatted string if the buffer is too small
			// Note that snprintf deals with null termination automatically

			// Replace data inside buffer with formatted string and get resultant size of buffer

			int32_t newStringSize = std::snprintf(m_DataBuffer.data() + m_StringLength, BufferSize - m_StringLength, formatString, std::forward<VariadicArgs>(args)...);

			// If snprintf fails, return false
			if (newStringSize < 0)
			{
				return false;
			}

			// If success, update string length
			m_StringLength = m_StringLength + newStringSize;
			return true;
		}


		bool Append(const char* appendString)
		{
			// Get size of newly appending c-string
			std::size_t appendStringLength = std::strlen(appendString);

			// Early out if new size of string exceeds buffer length
			if (m_StringLength + appendStringLength + 1 > BufferSize)
			{
				return false;
			}

			// Fill data (Overwrite current null terminator)
			std::strncpy(m_DataBuffer.data() + m_StringLength, appendString, appendStringLength);
			m_StringLength += appendStringLength;

			// Add new null terminator
			m_DataBuffer[m_StringLength] = '\0';
			return true;
		}

		template<typename IntegerType>
		bool AppendInteger(IntegerType appendInteger)
		{
			// Ensure only integer types are used with this function
			static_assert(std::is_integral<IntegerType>::value, "Can only append simple integer types");

			// Attempt to append the const char* version of the integer to the end of the current string
			std::to_chars_result result = std::to_chars(m_DataBuffer.data() + m_StringLength, m_DataBuffer.data() + (BufferSize - 1), appendInteger);
			
			// Exit if appending fails
			if (result.ec != std::errc())
			{
				return false;
			}

			// Set new length for string
			m_StringLength = result.ptr - m_DataBuffer.data();

			// Set the new null terminator for the string
			*(result.ptr) = '\0';
			return true;

		}

		// TODO: Maybe add append substring and pop back functions

	public:
		//==============================
		// Operator Overloads
		//==============================
		FixedString& operator=(const char* newString)
		{
			// TODO: Maybe alert when fails????
			ReplaceBuffer(newString);
			return *this;
		}

		FixedString& operator=(std::string_view newStringView)
		{
			// TODO: Maybe alert when fails????
			ReplaceBuffer(newStringView);
			return *this;
		}

		operator const char* () const 
		{
			return m_DataBuffer.data(); 
		}

		template <std::size_t OtherBufferSize>
		bool operator==(const FixedString<OtherBufferSize>& other) const {
			// Check if lengths are different
			if (m_StringLength != other.m_StringLength)
			{
				return false;
			}
			// Compare content up to m_Length
			return std::memcmp(m_DataBuffer.data(), other.m_DataBuffer.data(), m_StringLength) == 0;
		}

		std::string operator+(const char* otherCString)
		{
			std::string returnString{};
			returnString.reserve(m_StringLength + std::strlen(otherCString));
			// Add contents of m_DataBuffer up to m_StringLength
			returnString.append(m_DataBuffer.data(), m_StringLength);

			// Append the const char* string
			returnString.append(otherCString);

			return returnString;
		}

	public:
		//==============================
		// Getters/Setters
		//==============================
		const char* CString() const
		{
			return m_DataBuffer.data();
		}

		std::size_t StringLength() const
		{
			return m_StringLength;
		}

		bool IsEmpty() const
		{
			return m_StringLength == 0;
		}

	private:
		//==============================
		// Internal Functionality
		//==============================
		bool ReplaceBuffer(const char* newString)
		{
			// Get size of new string
			std::size_t newStringLength = std::strlen(newString);

			// Truncate provided string based on buffer size
			if (newStringLength + 1 > BufferSize)
			{
				// Set a new string length to fill buffer as much as possible
				// * Note, leaving space for null terminator
				newStringLength = BufferSize - 1;
			}

			// Fill data (Overwrite current null terminator)
			std::strncpy(m_DataBuffer.data(), newString, newStringLength);
			m_StringLength = newStringLength;

			// Add new null terminator
			m_DataBuffer[m_StringLength] = '\0';
			return true;
		}

		bool ReplaceBuffer(std::string_view newString)
		{
			// Get size of new string
			std::size_t newStringLength = newString.size();

			// Truncate provided string based on buffer size
			if (newStringLength + 1 > BufferSize)
			{
				// Set a new string length to fill buffer as much as possible
				// * Note, leaving space for null terminator
				newStringLength = BufferSize - 1;
			}

			// Fill data (Overwrite current null terminator)
			std::memcpy(m_DataBuffer.data(), newString.data(), newStringLength); // Note, this assumes a char size of 1 byte
			m_StringLength = newStringLength;

			// Add new null terminator
			m_DataBuffer[m_StringLength] = '\0';
			return true;
		}

	private:
		std::array<char, BufferSize> m_DataBuffer;
		std::size_t m_StringLength{0};
	};

	template <size_t N>
	std::string operator+(const char* leftCString, const FixedString<N>& rightFixedString)
	{
		std::string returnString;
		returnString.reserve(std::strlen(leftCString) + rightFixedString.StringLength());

		// Append the const char* string
		returnString.append(leftCString);

		// Append the contents of rightFixedString up to its length
		returnString.append(rightFixedString.CString(), rightFixedString.StringLength());

		return returnString;
	}

	using FixedString16 = FixedString<16>; // Generally for small status codes, short labels, etc...
	using FixedString32 = FixedString<32>; // Generally for status codes, small integers, etc...
	using FixedString64 = FixedString<64>; // Generally for usernames, uuid's, small formatted strings... 
	using FixedString256 = FixedString<256>; // Generally for usernames, short log messages, etc... 
	using FixedString1024 = FixedString<1024>; // Generally for long file paths, full log messages, etc...
	using FixedString8192 = FixedString<8192>; // Generally for http headers, small socket payloads, etc...
	using FixedString64KB = FixedString<64000>; // Generally for large large data streams or file io...
}

namespace std
{
	template<size_t BufferSize>
	struct hash<Kargono::FixedString<BufferSize>>
	{
		std::size_t operator()(const Kargono::FixedString<BufferSize>& fixedString) const
		{
			unsigned int hash = 5381;

			const char* characterIterator = fixedString;

			for (unsigned int i {0}; i < fixedString.StringLength(); ++characterIterator, ++i)
			{
				hash = ((hash << 5) + hash) + (*characterIterator);
			}

			return hash;
		}
	};
}

