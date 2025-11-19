#pragma once

#include "Kargono/Core/UUID.h"
#include "Kargono/Math/MathAliases.h"
#include "Modules/Particles/Assets/EmitterConfig.h"

#include "Modules/Particles/Particle.h"

#include <vector>
#include <cstdint>

namespace Kargono::Scenes { class Scene; }

namespace Kargono::Particles
{
	struct EmitterInstance
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		EmitterInstance() = default;
		~EmitterInstance() = default;
	public:
		//==============================
		// Public Fields
		//==============================
		Math::vec3 m_Position;
		EmitterMotionType m_EmitterMotionType{ EmitterMotionType::NoMotion };
		Scenes::Scene* m_ParentScene{ nullptr };
		UUID m_ParentEntityID{ k_EmptyUUID };
		Assets::TAssetRef<EmitterConfig> m_Config;
		std::vector<Particle> m_Particles;
		size_t m_ParticleIndex;
		float m_ParticleSpawnAccumulator{ 0.0f };
		// Emitter lifetime information
		float m_StartTime;
		float m_EndTime;
	};
}