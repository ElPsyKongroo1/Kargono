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

	PseudoGenerator::PseudoGenerator(uint64_t seed)
	{
		m_Seed = seed;
		m_State = seed;
	}

	uint64_t PseudoRandomService::GenerateNumber(PseudoGenerator& gen)
	{
		gen.m_State = (gen.m_State * s_Multiplier) % s_Modulus;
		return gen.m_State;
	}

	float PseudoRandomService::GenerateFloatBounds(PseudoGenerator& gen, float lowerBound, float upperBound)
	{
		// Generate the pseudo-random number
		gen.m_State = (gen.m_State * s_Multiplier) % s_Modulus;

		// Normalize the result to a float in [0, 1)
		float normalized = static_cast<float>(gen.m_State) / static_cast<float>(s_Modulus);

		// Scale and shift to the desired bounds [lowerBound, upperBound]
		return lowerBound + normalized * (upperBound - lowerBound);
	}

	void PseudoRandomService::ResetState(PseudoGenerator& gen)
	{
		gen.m_State = gen.m_Seed;
	}
}
