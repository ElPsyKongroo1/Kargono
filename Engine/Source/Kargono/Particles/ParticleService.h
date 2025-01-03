#include "Kargono/Math/Math.h"
#include "Kargono/Core/Base.h"
#include "Kargono/Core/Timestep.h"

#include <vector>
#include <chrono>

namespace Kargono::Particles
{
    struct Particle
    {
	public:
        // Particle position data
        Math::vec3 m_Position;
		Math::vec2 m_Size;
        float m_Rotation;

	private:
        // Particle lifetime information
		std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
		std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
	private:
		friend class ParticleService;
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
        // Submit Particles
        //==============================
	    static void AddParticle(const Particle& particle, float lifeTime);
		static void AddParticleByLocation(Math::vec3 particleLocation, float lifeTime);
    };
}

