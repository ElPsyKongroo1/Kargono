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
		//==============================
		// Convert Float -> String
		//==============================
		static std::string FloatToString(float value, int precision = 2)
		{
			std::ostringstream out;
			out << std::fixed << std::setprecision(precision) << value;
			return out.str();
		}

		//==============================
		// Convert String -> Bool
		//==============================
		static bool StringToBool(std::string_view value)
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


