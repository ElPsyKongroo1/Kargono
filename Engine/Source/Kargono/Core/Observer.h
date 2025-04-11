#pragma once

#include "Kargono/Core/BitField.h"

#include <concepts>
#include <array>
#include <functional>
#include <limits.h>

namespace Kargono
{
	template <typename Func, typename... Args>
	concept Callable = requires(Func func, Args&&... args) 
	{
		// Checks if t() can be called with the argument list and returns void
		{ func(std::forward<Args>(args)...)} -> std::same_as<void>;  
	};


	//constexpr size_t k_InvalidObserverIndex{ std::numeric_limits<size_t>().max() };

	template<typename Func, typename Context>
	class Observer
	{
	public:
		Context m_Context{};
		Func m_CallbackFunc{ nullptr };
	};

	template<Observer ObserverType, size_t MaxObservers = 1> 
	class Subject
	{
	public:
		template<typename... Args>
		void Notify(Args&&... args)
		{
			for (size_t index{ 0 }; index < m_ObserverCount; index++)
			{
				Observer<Func, Context>& observer = m_Observers[index];
				observer.m_CallbackFunc(observer.m_Context, std::forward<Args>(args)...);
			}
		}

	public:
		void AddObserver(Observer<Func, Context> newObserver)
		{
			// Ensure observer count is not at or above capacity
			if (m_ObserverCount >= MaxObservers)
			{
				return;
			}
			
			// Add the observer
			m_Observers[m_ObserverCount] = newObserver;
			m_ObserverCount++;
		}
	private:
		std::array<Observer<Func, Context>, MaxObservers> m_Observers{};
		size_t m_ObserverCount{0};
	};
}


template <typename T>
struct FnRef;

template <typename Return, typename... Params>
struct FnRef<Return(Params...)> 
{
	constexpr FnRef(const FnRef&) = default;
	constexpr auto operator=(const FnRef&)->FnRef & = default;

	template <cpts::InvokableReturns<Return, Params...> T>
	FORCEINLINE constexpr FnRef(T&& other [[clang::lifetimebound]] )
		:
		data{ &other },
		fn{ [](void* data, Params... params) -> Return {
			return std::invoke(*static_cast<std::decay_t<T>*>(data), std::forward<Params>(params)...);
		} } {}

		FORCEINLINE constexpr auto operator()(Params... params) const -> Return 
		{
			return fn(data, std::forward<Params>(params)...);
		}

private:
	void* data;
	Return(*fn)(void*, Params...);
};
