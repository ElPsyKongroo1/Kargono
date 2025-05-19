#pragma once
#include <random>
#include <chrono>
#include <cstdint>

namespace Kargono::Utility
{
	class STLRandom
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		STLRandom();
		~STLRandom() = default;
	public:
		//==============================
		// Generate Random
		//==============================
		int32_t GenerateRandomInteger(int32_t lowerBound, int32_t upperBound)
		{
			return std::uniform_int_distribution<int32_t>{lowerBound, upperBound}(m_RandomGenerator);
		}

		float GenerateRandomFloat(float lowerBound, float upperBound)
		{
			return std::uniform_real_distribution<float>{lowerBound, upperBound}(m_RandomGenerator);
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		std::mt19937 m_RandomGenerator;
	};


	class STLRandomService
	{
	public:
		static STLRandom& GetActiveRandom()
		{
			return m_Random;
		}
	private:
		static inline STLRandom m_Random;
	};

	constexpr uint64_t k_PseudoMultiplier{ 6364136223846793005ULL };
	constexpr uint64_t k_PseudoModulus{ std::numeric_limits<uint64_t>::max() };

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

		//==============================
		// Generate Numbers
		//==============================
		uint64_t GenerateNumber();
		float GenerateFloatBounds(float lowerBound, float upperBound);

		//==============================
		// Modify Generators
		//==============================
		void ResetState();
	private:
		//==============================
		// Internal Fields
		//==============================
		uint64_t m_Seed{0};
		uint64_t m_State{0};
	private:
		friend class PseudoRandomService;
	};
}
