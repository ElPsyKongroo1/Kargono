#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/Buffer.h"

#include <charconv>
#include <system_error>

namespace Kargono::Utility
{
	class Conversions
	{
	public:
		template<typename T>
		static bool CharBufferToVariable(Buffer buffer, T& variable)
		{
			//std::string stringValue{buffer.As<char>()};
			auto [ptr, ec] = std::from_chars(buffer.As<char>(), buffer.As<char>() + buffer.Size, variable);

			if ((bool)ec)
			{
				if (ec == std::errc::invalid_argument)
				{
					KG_ERROR("Invalid Type provided to Buffer -> Variable conversion function");
					return false;
				}
				if (ec == std::errc::result_out_of_range)
				{
					KG_ERROR("Improper bounds checking for Buffer -> Variable conversion function");
					return false;
				}

				KG_ERROR("Unknown error in Buffer to Variable conversion function");
				return false;
			}
			return true;
		}

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
	};
}


