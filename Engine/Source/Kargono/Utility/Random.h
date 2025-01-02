#pragma once
#include <random>
#include <chrono>
#include <cstdint>

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

	//==============================
	// PseudoGenerator Class
	//==============================
	class PseudoGenerator
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		PseudoGenerator(uint64_t seed);

	private:
		//==============================
		// Internal Fields to hold state
		//==============================
		uint64_t m_Seed{0};
		uint64_t m_State{0};
	private:
		friend class PseudoRandomService;
	};

	constexpr inline uint64_t s_Multiplier { 6364136223846793005ULL };
	constexpr inline uint64_t s_Modulus { std::numeric_limits<uint64_t>::max() };

	//==============================
	// PseudoRandomService Class
	//==============================
	class PseudoRandomService
	{
	public:
		//==============================
		// Service API
		//==============================
		static uint64_t GenerateNumber(PseudoGenerator& state);

		//==============================
		// Modify Generators
		//==============================
		static void ResetState(PseudoGenerator& state);
	};
}
