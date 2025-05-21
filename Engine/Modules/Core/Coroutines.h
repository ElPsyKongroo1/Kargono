#pragma once

#include <coroutine>
#include <string>

struct Chat
{
	struct promise_type
	{
		std::string m_MessageOut{};
		std::string m_MessageIn{};

		void unhandled_exception() noexcept {}					// Handle Exception
		Chat get_return_object() { return Chat{ this }; }		// Coroutine Creation
		std::suspend_always initial_suspend() noexcept { return {}; }	// Startup
		std::suspend_always yield_value(std::string msg) noexcept
		{
			m_MessageOut = std::move(msg);
			return {};
		}

		auto await_transform(std::string) noexcept // This transform is chosen for the string case
		{
			struct awaiter
			{
				promise_type& m_PromiseType;
				constexpr bool await_ready() const noexcept { return true; };
				std::string await_resume() const noexcept { return std::move(m_PromiseType.m_MessageIn); }
				void await_suspend(std::coroutine_handle<>) const noexcept {}
			};

			return awaiter{ *this };
		}

		void return_value(std::string msg) noexcept { m_MessageOut = std::move(msg); }
		std::suspend_always final_suspend() noexcept { return {}; }

	};


	Chat(promise_type* type) {}
};

