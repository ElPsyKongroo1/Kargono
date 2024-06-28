#include "kgpch.h"

#include "Kargono/Utility/Random.h"

namespace Kargono::Utility
{
	std::mt19937 RandomService::randomGenerator{ CreateRandomNumberGenerator() };
	std::mt19937 RandomService::CreateRandomNumberGenerator()
	{
		std::random_device randomDevice{};

		// Create seed_seq with clock and 7 random numbers from std::random_device
		std::seed_seq seedSequence{
			static_cast<std::seed_seq::result_type>(std::chrono::steady_clock::now().time_since_epoch().count()),
				randomDevice(), randomDevice(), randomDevice(), randomDevice(), randomDevice(), randomDevice(), randomDevice() };

		return std::mt19937{ seedSequence };
	}
}
