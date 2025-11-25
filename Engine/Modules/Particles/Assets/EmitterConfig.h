#pragma once

#include "Kargono/Math/Interpolation.h"
#include "Kargono/Math/MathAliases.h"
#include "Kargono/Core/Base.h"
#include "Modules/Particles/Module/ParticlesModule.h"
#include "Modules/Assets/Module/AssetTag.h"

#include <array>
#include <cstdint>

namespace Kargono::Particles
{
	enum class EmitterMotionType
	{
		None = 0,
		NoMotion,
		FollowEntity
	};

	enum class EmitterLifecycle
	{
		None = 0,
		Immortal,
		FixedTime
	};

	struct EmitterConfig
	{
	public:
		//==============================
		// Asset Config Info
		//==============================
		constexpr static FixedBufStr32 GetAssetName()
		{
			return "Particle Emitter Config";
		}

		constexpr static Assets::AssetFlags GetAssetFlags()
		{
			Assets::AssetFlags flags{};
			flags.SetFlag(Assets::AssetFlag::HasAssetCache);
			flags.SetFlag(Assets::AssetFlag::RequireUniqueName);
			return flags;
		}

		constexpr static FixedBufStr16 GetFileExtension()
		{
			return ".kgparticle";
		}
		static void CreateFromName(Assets::Metadata<EmitterConfig>& metadata);
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Validation
		//==============================
		void ValidateDelete(Assets::Metadata<EmitterConfig>& metadata);
	public:
		//==============================
		// Public Fields
		//==============================
		// Emitter data
		EmitterLifecycle m_EmitterLifecycle{ EmitterLifecycle::Immortal };
		float m_EmitterLifetime{ 1.0f };
		size_t m_BufferSize{ 1000 };
		std::array<Math::vec3, 2> m_SpawningBounds{ Math::vec3(0.0f), Math::vec3(0.0f) };
		// Particle data
		bool m_UseGravity{ false };
		Math::vec3 m_GravityAcceleration{ Math::vec3(0.0f) };
		float m_ParticleLifetime{ 1.0f };
		size_t m_SpawnRatePerSec{ 5 };
		Math::InterpolationType m_ColorInterpolationType{ Math::InterpolationType::Linear };
		Math::vec4 m_ColorBegin{ 1.0f };
		Math::vec4 m_ColorEnd{ 1.0f };
		Math::InterpolationType m_SizeInterpolationType{ Math::InterpolationType::Linear };
		Math::vec3 m_SizeBegin{ 1.0f };
		Math::vec3 m_SizeEnd{ 1.0f };
	};

	Register_Module_Type(EmitterConfig, Assets::AssetTag)
}

namespace Kargono::Utility
{
	inline const char* EmitterMotionTypeToString(Particles::EmitterMotionType motionType)
	{
		switch (motionType)
		{
		case Particles::EmitterMotionType::None: return "None";
		case Particles::EmitterMotionType::NoMotion: return "NoMotion";
		case Particles::EmitterMotionType::FollowEntity: return "FollowEntity";
		default:
			KG_WARN("Invalid emitter motion type provided when attempting convert to a string");
			return "None";
		}
	}

	inline Particles::EmitterMotionType StringToEmitterMotionType(std::string_view str)
	{
		if (str == "None") return Particles::EmitterMotionType::None;
		if (str == "NoMotion") return Particles::EmitterMotionType::NoMotion;
		if (str == "FollowEntity") return Particles::EmitterMotionType::FollowEntity;
		KG_WARN("Invalid string provided when attempting to get emitter motion type");
		return Particles::EmitterMotionType::None;
	}

	inline const char* EmitterLifecycleToString(Particles::EmitterLifecycle lifecycle)
	{
		switch (lifecycle)
		{
		case Particles::EmitterLifecycle::None: return "None";
		case Particles::EmitterLifecycle::Immortal: return "Immortal";
		case Particles::EmitterLifecycle::FixedTime: return "FixedTime";
		default:
			KG_WARN("Invalid emitter lifecycle provided when attempting convert to a string");
			return "None";
		}
	}

	inline Particles::EmitterLifecycle StringToEmitterLifecycle(std::string_view str)
	{
		if (str == "None") return Particles::EmitterLifecycle::None;
		if (str == "Immortal") return Particles::EmitterLifecycle::Immortal;
		if (str == "FixedTime") return Particles::EmitterLifecycle::FixedTime;

		KG_WARN("Invalid string provided when attempting to get emitter lifecycle");
		return Particles::EmitterLifecycle::None;
	}
}