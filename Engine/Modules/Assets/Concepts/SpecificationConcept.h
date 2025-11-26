#pragma once

namespace Kargono::Assets
{
	// Requirements of a specification
	template <typename t_SpecType>
	concept SpecificationConcept = true; // No requirements for now

	// Check if an asset type has a specification
	template <typename t_AssetType>
	concept HasSpecification = requires
	{
		typename t_AssetType::Spec;
	} && SpecificationConcept<typename t_AssetType::Spec>;
}

namespace Kargono::Detail
{
	template<typename t_AssetType>
	consteval auto GetSpecType()
	{
		// Retrieve specification type or std::monostate if none exists
		if constexpr (Assets::HasSpecification<t_AssetType>)
		{
			return std::type_identity<typename t_AssetType::Spec>{};
		}
		else
		{
			return std::type_identity<std::monostate>{};
		}
	}
}
namespace Kargono::Assets
{
	// Specification type. 
	// Use this type when you need to refer to the spec type directly
	template<typename t_AssetType>
	using Specification_t = typename decltype(Detail::GetSpecType<t_AssetType>())::type;
}