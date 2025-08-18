#pragma once
#include <string>
#include <algorithm>
#include <tuple>
#include <type_traits>

namespace Kargono::Utility
{
	// Helper to check if T is in a type list
	template<typename T, typename... Ts>
	struct is_one_of : std::disjunction<std::is_same<T, Ts>...> {};

	template<typename T, typename Tuple>
	struct tuple_contains;

	template<typename T, typename... Ts>
	struct tuple_contains<T, std::tuple<Ts...>> : is_one_of<T, Ts...> {};

	template<typename T, typename Tuple>
	inline constexpr bool tuple_contains_v = tuple_contains<T, Tuple>::value;

	// =============================
	// Check if any QueryType exists in Tuple
	// =============================
	template<typename Tuple, typename... QueryTypes>
	struct tuple_has_any;

	template<typename... TupleTypes, typename... QueryTypes>
	struct tuple_has_any<std::tuple<TupleTypes...>, QueryTypes...>
	{
		static constexpr bool value =
			(tuple_contains_v<QueryTypes, std::tuple<TupleTypes...>> || ...);
	};

	template<typename Tuple, typename... QueryTypes>
	inline constexpr bool tuple_has_any_v = tuple_has_any<Tuple, QueryTypes...>::value;

	class Operations
	{
	public:
		//==============================
		// Boolean Operations
		//==============================
		static void ToggleBoolean(bool& boolToToggle)
		{
			boolToToggle = !boolToToggle;
		}

		//==============================
		// String Operations
		//==============================
		constexpr static size_t GetStringLength(const char* string)
		{
			// TODO: Note this function is dangerous for non-null terminated strings
			size_t length{ 0 };
			while (string[length] != '\0') 
			{
				length++;
			}
			return length;
		}

		static void RemoveCharacterFromString(std::string& string, char character)
		{
			std::erase(string, character);
		}

		static void RemoveWhitespaceFromString(std::string& string) 
		{
			std::erase_if(string, [](unsigned char character)
			{
				return std::isspace(character); 
			});
		}

		static bool IsValidCString(const char* str)
		{
			// Check if the string is not NULL and has at least one character
			return str != nullptr && strlen(str) > 0 && str[strlen(str)] == '\0';
		}

		static std::string_view GetSubstringToCharacter(const std::string& inputString, char queryCharacter)
		{
			// Find the position of the first occurrence of queryCharacter
			size_t position = inputString.find(queryCharacter);

			// If the character is not found, return the entire string
			if (position == std::string::npos) 
			{
				return std::string_view(inputString);
			}

			// Otherwise, return a substring view up to the found position
			return std::string_view(inputString.data(), position);
		}

		//==============================
		// Buffer Operations
		//==============================
		static bool IsBufferZero(uint8_t* buffer, size_t size) 
		{
			KG_ASSERT(buffer != nullptr);

			for (size_t i = 0; i < size; ++i) 
			{
				if (buffer[i] != 0)
				{
					return false;
				}
			}
			return true;
		}

		//==============================
		// Integer Operations
		//==============================
		template <typename T>
		static T RoundDown(T valueToRound, T multipleOf)
		{
			// Avoid division by zero
			if (multipleOf == 0)
			{
				return valueToRound;
			}

			// Calculate the remainder
			T remainder = valueToRound % multipleOf;

			// If the remainder is not zero and the value is negative, adjust the value
			if (remainder != 0 && valueToRound < 0)
			{
				remainder += multipleOf;
			}

			return valueToRound - remainder;
		}
		template <typename T>
		static T RoundUp(T valueToRound, T multipleOf) {
			// Avoid division by zero
			if (multipleOf == 0)
			{
				return valueToRound;
			}
			T remainder = valueToRound % multipleOf;

			// If valueToRound is already a multiple of multipleOf
			if (remainder == 0)
			{
				return valueToRound; 
			}
			return valueToRound + (multipleOf - remainder);
		}
	};
}
