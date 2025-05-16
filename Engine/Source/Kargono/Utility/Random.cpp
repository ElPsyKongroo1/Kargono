#include "kgpch.h"

#include "Kargono/Utility/Random.h"

namespace Kargono::Utility
{
	PseudoGenerator::PseudoGenerator(uint64_t seed)
	{
		m_Seed = seed;
		m_State = seed;
	}

	uint64_t PseudoGenerator::GenerateNumber()
	{
		m_State = (m_State * k_PseudoMultiplier) % k_PseudoModulus;
		return m_State;
	}

	float PseudoGenerator::GenerateFloatBounds(float lowerBound, float upperBound)
	{
		// Generate the pseudo-random number
		m_State = (m_State * k_PseudoMultiplier) % k_PseudoModulus;

		// Normalize the result to a float in [0, 1)
		float normalized = static_cast<float>(m_State) / static_cast<float>(k_PseudoModulus);

		// Scale and shift to the desired bounds [lowerBound, upperBound]
		return lowerBound + normalized * (upperBound - lowerBound);
	}

	void PseudoGenerator::ResetState()
	{
		m_State = m_Seed;
	}
	STLRandom::STLRandom()
	{
		std::random_device randomDevice{};

		// Create seed_seq with clock and 7 random numbers from std::random_device
		std::seed_seq seedSequence{
			static_cast<std::seed_seq::result_type>(std::chrono::steady_clock::now().time_since_epoch().count()),
				randomDevice(), randomDevice(), randomDevice(), randomDevice(), randomDevice(), randomDevice(), randomDevice() };

		m_RandomGenerator = std::mt19937{seedSequence};
	}
}
