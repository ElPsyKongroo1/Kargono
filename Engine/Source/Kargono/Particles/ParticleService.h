#pragma once
#include "Kargono/Math/Math.h"
#include "Kargono/Core/Base.h"
#include "Kargono/Core/Timestep.h"

#include <vector>

namespace Kargono::ECS { struct TransformComponent; }

namespace Kargono::Particles
{
	struct EmitterConfig
	{
		float m_ParticleLifetime{ 1.0f };
		size_t m_SpawnRatePerSec{ 5 };
		size_t m_BufferSize{ 1000 };
		Math::vec4 m_ColorBegin{ 1.0f };
		Math::vec4 m_ColorEnd{ 1.0f };
		Math::vec3 m_SizeBegin{ 1.0f };
		Math::vec3 m_SizeEnd{ 1.0f };
	};

    struct Particle
    {
	public:
        // Particle transform 
        Math::vec3 m_Position;
		Math::vec3 m_Size;
		Math::vec3 m_Rotation;

		// Particle physics information
		Math::vec3 m_Velocity;

		bool active{ false };
	private:
        // Particle lifetime information
		float startTime;
		float endTime;
	private:
		friend class ParticleService;
    };

	struct EmitterInstance
	{
		Math::vec3 m_Position;
		EmitterConfig* m_Config;
		std::vector<Particle> m_Particles;
		size_t m_ParticleIndex;
		float m_ParticleSpawnAccumulator{ 0.0f };
	};

    class ParticleService
    {
    public:
        //==============================
        // Lifecycle Functions
        //==============================
        static void Init();
        static void Terminate();
    public:
        //==============================
        // On Event Functions
        //==============================
	    static void OnUpdate(Timestep ts);
		static void OnRender(const Math::mat4& viewProjection);

		//==============================
		// Manage Emitters
		//==============================
		static UUID AddEmitter(EmitterConfig* config, const Math::vec3 position);
		static bool RemoveEmitter(UUID emitterID);
		static void ClearEmitters();
		static std::unordered_map<UUID, EmitterInstance>& GetAllEmitters();
    };
}

