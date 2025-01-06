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
		Math::vec4 m_ColorBegin;
		Math::vec4 m_ColorEnd;
		Math::vec4 m_SizeBegin;
		Math::vec4 m_SizeEnd;
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
	private:
        // Particle lifetime information
		float startTime;
		float endTime;
	private:
		friend class ParticleService;
    };

	struct EmitterInstance
	{
		ECS::TransformComponent* m_ParentTransform;
		EmitterConfig* m_Config;
		std::vector<Particle> m_Particles;
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
    };
}

