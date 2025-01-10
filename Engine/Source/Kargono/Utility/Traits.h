#pragma once

#if 0
//#include <Helena/Platform/Compiler.hpp>
#include <array>
#include <string_view>
#include <utility>

namespace Kargono::Traits::Internal
{
	namespace 
	{
		template <typename T>
		class NameOf final
		{
			template <std::size_t... Indexes>
			[[nodiscard]] static constexpr auto Substring(std::string_view str, std::index_sequence<Indexes...>) noexcept
			{
				return std::array{ str[Indexes]..., '\0' };
			}

			template <typename>
			[[nodiscard]] static constexpr auto ParseName() noexcept
			{
				constexpr auto prefix = std::string_view{ "ParseName<" };
				constexpr auto suffix = std::string_view{ ">(void)" };
				constexpr auto function = std::string_view{ __FUNCSIG__ };
				constexpr auto start = function.find(prefix) + prefix.size();
				constexpr auto end = function.rfind(suffix);
				static_assert(start < end);
				constexpr auto name = function.substr(start, (end - start));
				return Substring(name, std::make_index_sequence<name.size()>{});
			}

		public:
			static constexpr auto NameStorage = ParseName<T>();
		};
	}

	template <typename T>
	inline constexpr auto NameOf = Internal::NameOf<T>::NameStorage.data();

#define ENUM_FIELDNAME(type) #type
}

#endif
