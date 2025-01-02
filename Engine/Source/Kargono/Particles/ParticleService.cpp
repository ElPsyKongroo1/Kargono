#include "kgpch.h"

#include "Kargono/Particles/ParticleService.h"

#include "Kargono/Rendering/Shader.h"

namespace Kargono::Particles
{
    struct ParticleContext
    {
        // All particles being managed
        std::vector<Particle> m_ParticlePool;
        Rendering::RendererInputSpec m_ParticleRenderSpec;
    };

    static Ref<ParticleContext> s_ParticlesContext {nullptr};

    void ParticleService::Init()
    {

    }

    void ParticleService::Terminate()
    {
        
    }

    void ParticleService::OnUpdate(Timestep ts)
    {
        // Render Particles
    }

    void ParticleService::AddParticle(const Particle& particle)
    {
    }

}