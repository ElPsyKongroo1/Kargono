#include "kgpch.h"

#include "Kargono/Particles/ParticleService.h"

#include "Kargono/Rendering/Shader.h"
#include "Kargono/Rendering/RenderingService.h"
#include "Kargono/Assets/AssetService.h"
#include "Kargono/Core/Engine.h"

namespace Kargono::Particles
{
    struct ParticleContext
    {
        // All particles being managed
		std::vector<Particle> m_ParticlePool;
        Rendering::RendererInputSpec m_ParticleRenderSpec;
    };

    static Ref<ParticleContext> s_ParticleContext {nullptr};

    void ParticleService::Init()
    {
		KG_ASSERT(!s_ParticleContext);

		// Initialize basic particle context
		s_ParticleContext = CreateRef<ParticleContext>();
		s_ParticleContext->m_ParticlePool.reserve(1000); // TODO: Maybe rethink this

		// Initialize rendering data for particles
		{
			s_ParticleContext->m_ParticleRenderSpec.ClearData();
				// Create shader for UI background/quad rendering
			Rendering::ShaderSpecification shaderSpec{ Rendering::ColorInputType::FlatColor, Rendering::TextureInputType::None, false, true, false, Rendering::RenderingType::DrawIndex, false };
			auto [uuid, localShader] = Assets::AssetService::GetShader(shaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			// Default particle color is pure white
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 1.0f, 1.0f, 1.0f, 1.0f }, "a_Color", localBuffer, localShader);

			// Create basic shape component for UI quad rendering
			ECS::ShapeComponent* shapeComp = new ECS::ShapeComponent();
			shapeComp->CurrentShape = Rendering::ShapeTypes::Quad;
			shapeComp->Vertices = CreateRef<std::vector<Math::vec3>>(Rendering::Shape::s_Quad.GetIndexVertices());
			shapeComp->Indices = CreateRef<std::vector<uint32_t>>(Rendering::Shape::s_Quad.GetIndices());


			s_ParticleContext->m_ParticleRenderSpec.m_Shader = localShader;
			s_ParticleContext->m_ParticleRenderSpec.m_Buffer = localBuffer;
			s_ParticleContext->m_ParticleRenderSpec.m_ShapeComponent = shapeComp;
		}

		//{
			//AddParticle({}, 1.0f);
		//}
		
		KG_VERIFY(s_ParticleContext, "Particle System Init");
    }

    void ParticleService::Terminate()
    {
		KG_ASSERT(s_ParticleContext);

		// Clear input spec data
		s_ParticleContext->m_ParticleRenderSpec.ClearData();

		// Terminate Static Variables
		s_ParticleContext.reset();

		// Verify Termination
		KG_VERIFY(!s_ParticleContext, "Particle System Terminate");
    }

    void ParticleService::OnUpdate(Timestep ts)
    {
		KG_ASSERT(s_ParticleContext);
		std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
		// TODO: Find data structure that works best for this use case
		/*for (Particles::Particle& particle : s_ParticleContext->m_ParticlePool)
		{
			if (currentTime > particle.endTime)
			{

			}
		}*/

		// Delete indicated particles
    }

	void ParticleService::OnRender(const Math::mat4& viewProjection)
	{
		KG_ASSERT(s_ParticleContext);

		// Reset the rendering context
		Rendering::RendererAPI::ClearDepthBuffer(); // TODO: PLEASE REMOVE THIS AHHHHHHHHH

		// Start rendering context
		Rendering::RenderingService::BeginScene(viewProjection);

		// Render all particles
		for (Particles::Particle& particle : s_ParticleContext->m_ParticlePool)
		{
			// Create background rendering data
			s_ParticleContext->m_ParticleRenderSpec.m_TransformMatrix = glm::translate(Math::mat4(1.0f), particle.m_Position);

			// Submit particle to the GPU
			Rendering::RenderingService::SubmitDataToRenderer(s_ParticleContext->m_ParticleRenderSpec);
		}

		// End rendering context and submit rendering data to GPU
		Rendering::RenderingService::EndScene();
	}

    void ParticleService::AddParticle(const Particle& particle, float lifeTime)
    {
		// Add the particle...
		s_ParticleContext->m_ParticlePool.push_back(particle);
    }

	void ParticleService::AddParticleByLocation(Math::vec3 particleLocation, float lifeTime)
	{
		// Create the new particle
		Particle newParticle;
		newParticle.m_Position = particleLocation;
		
		// Set start and end times of particle's lifecycle
		newParticle.startTime = EngineService::GetActiveEngine().GetInApplicationTime();
		newParticle.endTime = newParticle.startTime + lifeTime;

		// Add particle to buffer
		s_ParticleContext->m_ParticlePool.push_back(newParticle);
	}

}
