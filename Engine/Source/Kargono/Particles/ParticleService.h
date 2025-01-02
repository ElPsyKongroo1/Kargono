#include "Kargono/Math/Math.h"
#include "Kargono/Core/Base.h"
#include "Kargono/Core/Timestep.h"

#include <vector>
#include <chrono>

namespace Kargono::Particles
{
    struct Particle
    {
        // Particle position data
        Math::vec3 m_Position;
        Math::vec3 m_Velocity;
        float m_Rotation;

        // Particle color interpolated from start to end of lifetime
        Math::vec4 m_ColorStart;
        Math::vec4 m_ColorEnd;

        // Particle width interpolated from start to end of lifetime
        float m_SizeStart;
        float m_SizeEnd;

        // Particle lifetime information
		std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
		std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
    };

    class ParticlesService
    {
    public:
        //==============================
        // On Event Functions
        //==============================
	    static void OnUpdate(Timestep ts);

        //==============================
        // Submit Particles
        //==============================
	    static void AddParticle(const Particle& particle);
    public:
        static inline Ref<ParticlesContext> s_ParticlesContext {nullptr};
    };
}

