#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/Buffer.h"
#include "Kargono/Math/Math.h"

#include <charconv>
#include <system_error>

namespace Kargono::Utility
{
	//==============================
	// Convert Types Class
	//==============================
	class Conversions
	{
	public:
		//==============================
		// Convert Buffer -> Variable (Ex: uint64_t, float, and int)
		//==============================
		template<typename T>
		static bool CharBufferToVariable(Buffer buffer, T& variable)
		{
			//std::string stringValue{buffer.As<char>()};
			auto [ptr, ec] = std::from_chars(buffer.As<char>(), buffer.As<char>() + buffer.Size, variable);

			if ((bool)ec)
			{
				if (ec == std::errc::invalid_argument)
				{
					KG_WARN("Invalid Type provided to Buffer -> Variable conversion function");
					return false;
				}
				if (ec == std::errc::result_out_of_range)
				{
					KG_WARN("Improper bounds checking for Buffer -> Variable conversion function");
					return false;
				}

				KG_WARN("Unknown error in Buffer to Variable conversion function");
				return false;
			}
			return true;
		}

		template<>
		static bool CharBufferToVariable(Buffer buffer, Math::vec3& variable)
		{
			// Get the string from the buffer
			const std::string stringBuffer = buffer.GetString();

			// Create a string stream from the string buffer
			std::istringstream ss(stringBuffer);

			// Extract the floats
			if (!(ss >> variable.x >> variable.y >> variable.z)) 
			{
				KG_WARN("Unable to parse buffer as Math::vec3");
				return false;
			}

			return true; 

		}
		//==============================
		// Convert String -> Bool
		//==============================
		static bool StringToBool(const std::string& value)
		{
			if (value == "True")
			{
				return true;
			}
			else if (value == "False")
			{
				return false;
			}
			KG_ERROR("Conversion from string to bool failed");
			return false;
		}
		//==============================
		// Convert RValue -> LValue
		// (Here is a link that explains rvalues and lvalues if needed: https://www.geeksforgeeks.org/lvalues-references-and-rvalues-references-in-c-with-examples/ )
		//==============================
		template<typename T>
		T& RValueToLValue(T&& value) 
		{
			return value;
		}
	};
}


