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
		static int32_t GenerateRandomNumber(int32_t lowerBound, int32_t upperBound)
		{
			return std::uniform_int_distribution<int32_t>{lowerBound, upperBound}(randomGenerator);
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
