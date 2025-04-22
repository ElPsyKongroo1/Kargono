#pragma once

#include <optional>
#include <memory>

namespace Kargono
{
	//==============================
	// Engine Types
	//==============================
	// Definition of Scope, which is a wrapper for a Unique Pointer
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	constexpr Scope<T> CreateScope(T* pointer)
	{
		return std::unique_ptr<T>(pointer);
	}
	// Definition of Ref, which is a wrapper for a Shared Pointer
	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	constexpr Ref<T> CreateRef(T* pointer)
	{
		return std::shared_ptr<T>(pointer);
	}

	// Definition of Weak, which is a wrapper for a Weak Pointer
	template<typename T>
	using Weak = std::weak_ptr<T>;

	template<typename T>
	constexpr Weak<T> CreateWeak(Ref<T> reference)
	{
		std::weak_ptr<T> weakReference = reference;
		return weakReference;
	}

	template<typename T>
	constexpr Weak<T> CreateWeak(Scope<T> scope)
	{
		std::weak_ptr<T> weakReference = scope;
		return weakReference;
	}

	template<typename T>
	using Expected = std::optional<T>;

	template<typename T>
	using Optional = std::optional<T>;

}
