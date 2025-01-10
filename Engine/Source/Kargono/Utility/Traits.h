#pragma once

#if 0

#include <array>
#include <string_view>
#include <utility>

namespace Kargono::Utility
{
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
		template <typename T>
		static constexpr auto NameStorage = ParseName<T>();
	};

	class EnumNameOf final
	{
		template <std::size_t... Indexes>
		[[nodiscard]] static constexpr auto Substring(std::string_view str, std::index_sequence<Indexes...>) noexcept
		{
			return std::array{ str[Indexes]..., '\0' };
		}

		template <auto Value>
		[[nodiscard]] static constexpr auto ParseName() noexcept {
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
		template <auto Value>
		static constexpr auto NameStorage = ParseName<Value>();
	};
	

	template <typename T>
	inline constexpr auto NameOf = Internal::NameOf::NameStorage<T>.data();

	template <auto Value>
	inline constexpr auto NameFieldOf = Internal::EnumNameOf::NameStorage<Value>.data();
}
#endif
