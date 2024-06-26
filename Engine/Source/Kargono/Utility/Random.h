#pragma once
#include <random>
#include <chrono>

namespace Kargono::Utility
{
	class RandomService
	{
	public:
		//==============================
		// Service API
		//==============================
		template <typename T>
		static T GenerateRandomNumber(T lowerBound, T upperBound)
		{
			return std::uniform_int_distribution<T>{lowerBound, upperBound}(randomGenerator);
		}
	private:
		//==============================
		// Initializez Static Resources
		//==============================
		static std::mt19937 CreateRandomNumberGenerator();
	private:
		//==============================
		// Internal Fields
		//==============================
		static std::mt19937 randomGenerator;
	};
}
