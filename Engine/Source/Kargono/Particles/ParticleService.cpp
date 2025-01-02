#include "kgpch.h"

#include "Kargono/Particles/ParticleService.h"

#include "Kargono/Rendering/Shader.h"

namespace Kargono::Particles
{
    struct ParticlesContext
    {
        // All particles being managed
        std::vector<Particle> m_ParticlePool;
        Rendering::RendererInputSpec m_ParticleRenderSpec;
    };

	    void ParticlesService::OnUpdate(Timestep ts)
        {
            // Render Particles
        }

	    void ParticlesService::AddParticle(const Particle& particle)
        {
        }

}