#include "kgpch.h"

#include "Kargono/Particles/ParticleService.h"

#include "Kargono/Rendering/Shader.h"
#include "Kargono/Rendering/RenderingService.h"
#include "Kargono/Assets/AssetService.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Utility/Random.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/ECS/Entity.h"
#include "Kargono/ECS/EngineComponents.h"
#include "Kargono/Math/Interpolation.h"

namespace Kargono::Particles
{
    struct ParticleContext
    {
        // All emitters being managed
		std::unordered_map<UUID, EmitterInstance> m_AllEmitters;
        Rendering::RendererInputSpec m_ParticleRenderSpec;
		Utility::PseudoGenerator m_RandomGenerator{ 37427394 };
    };

    static Ref<ParticleContext> s_ParticleContext {nullptr};

    void ParticleService::Init()
    {
		KG_ASSERT(!s_ParticleContext);

		// Initialize basic particle context
		s_ParticleContext = CreateRef<ParticleContext>();

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

		// Get current time
		float currentTime{ EngineService::GetActiveEngine().GetInApplicationTime() };

		size_t iteration{ 0 };
		for (auto& [uuid, emitter] : s_ParticleContext->m_AllEmitters)
		{
			// TODO: If a parent entity exists, maybe just set to inactive

			// Check if emitter should be removed
			if (emitter.m_Config->m_EmitterLifecycle == EmitterLifecycle::FixedTime && currentTime > emitter.m_EndTime)
			{
				// List of emitters to be removed
				static std::vector<UUID> s_EmittersToRemove;

				// Check if a list of emitters already exists
				if (s_EmittersToRemove.size() > 0)
				{
					// Add current emitter to list of emitters to remove
					s_EmittersToRemove.emplace_back(uuid);
				}
				else
				{
					// Submit job to main thread that removes indicated emitters from the s_ParticleContext
					s_EmittersToRemove.emplace_back(uuid);
					EngineService::SubmitToMainThread([]()
					{
						for (UUID id : s_EmittersToRemove)
						{
							s_ParticleContext->m_AllEmitters.erase(id);
						}
						s_EmittersToRemove.clear();
					});
				}

				// Continue on to next emitter
				continue;
			}

			// Spawn more particles
			float spawnThreshold{ 1.0f / (float)emitter.m_Config->m_SpawnRatePerSec };
			emitter.m_ParticleSpawnAccumulator += ts;
			while (emitter.m_ParticleSpawnAccumulator > spawnThreshold)
			{
				// Decriment accumulator
				emitter.m_ParticleSpawnAccumulator -= spawnThreshold;

				// Spawn a particle
				Particle& currentParticle = emitter.m_Particles[emitter.m_ParticleIndex];
				currentParticle.m_Active = true;

				// Set x,y,z position based on the bounds provide in the emitter's config
				currentParticle.m_Position.x = emitter.m_Position.x + Utility::PseudoRandomService::GenerateFloatBounds
				(
					s_ParticleContext->m_RandomGenerator, 
					emitter.m_Config->m_SpawningBounds[0].x,
					emitter.m_Config->m_SpawningBounds[1].x
				);
				currentParticle.m_Position.y = emitter.m_Position.y + Utility::PseudoRandomService::GenerateFloatBounds
				(
					s_ParticleContext->m_RandomGenerator,
					emitter.m_Config->m_SpawningBounds[0].y,
					emitter.m_Config->m_SpawningBounds[1].y
				);
				currentParticle.m_Position.z = emitter.m_Position.z + Utility::PseudoRandomService::GenerateFloatBounds
				(
					s_ParticleContext->m_RandomGenerator,
					emitter.m_Config->m_SpawningBounds[0].z,
					emitter.m_Config->m_SpawningBounds[1].z
				);

				currentParticle.m_StartTime = currentTime;
				currentParticle.m_EndTime = currentTime + emitter.m_Config->m_ParticleLifetime;
				currentParticle.m_Size = emitter.m_Config->m_SizeBegin;

				//TODO: Generate random velocity TODO: CHANGE THIS
				currentParticle.m_Velocity.x = Utility::PseudoRandomService::GenerateFloatBounds(s_ParticleContext->m_RandomGenerator, -1.0f, 1.0f);
				currentParticle.m_Velocity.y = Utility::PseudoRandomService::GenerateFloatBounds(s_ParticleContext->m_RandomGenerator, -1.0f, 1.0f);
				currentParticle.m_Velocity.z = 0.0f;

				// Move iterator down
				emitter.m_ParticleIndex = --emitter.m_ParticleIndex % emitter.m_Particles.size();
			}

			// Manage particles
			for (Particles::Particle& particle : emitter.m_Particles)
			{
				// Check if particle is active
				if (!particle.m_Active)
				{
					continue;
				}

				// Check particle lifetime
				if (currentTime > particle.m_EndTime)
				{
					particle.m_Active = false;
					continue;
				}

				// Adjust velocity based on gravity if being used
				if (emitter.m_Config->m_UseGravity)
				{
					particle.m_Velocity += emitter.m_Config->m_GravityAcceleration;
				}

				// Move particle based on velocity
				particle.m_Position += particle.m_Velocity * (float)ts;
			}

			iteration++;

		}

    }

	void ParticleService::OnRender(const Math::mat4& viewProjection)
	{
		KG_ASSERT(s_ParticleContext);

		// Reset the rendering context
		Rendering::RendererAPI::ClearDepthBuffer(); // TODO: PLEASE REMOVE THIS AHHHHHHHHH

		// Start rendering context
		Rendering::RenderingService::BeginScene(viewProjection);

		// Get current time
		float currentTime{ EngineService::GetActiveEngine().GetInApplicationTime() };

		for (auto& [uuid, emitter] : s_ParticleContext->m_AllEmitters)
		{
			// Get color/size interpolation functions
			Math::EaseVec3Function sizeEaseFunc
			{
				Math::Interpolation::GetEasingFunctionVec3(emitter.m_Config->m_SizeInterpolationType)
			};

			Math::EaseVec4Function colorEaseFunc
			{
				Math::Interpolation::GetEasingFunctionVec4(emitter.m_Config->m_ColorInterpolationType)
			};

			for (Particles::Particle& particle : emitter.m_Particles)
			{
				// Check if particle is active
				if (!particle.m_Active)
				{
					continue;
				}

				// Calculate lerp progress value
				float progress{ (currentTime - particle.m_StartTime) / (particle.m_EndTime - particle.m_StartTime) };

				// Calculate size interpolation
				Math::vec3 size = sizeEaseFunc
				(
					emitter.m_Config->m_SizeBegin, 
					emitter.m_Config->m_SizeEnd, progress
				);

				// Create background rendering data
				s_ParticleContext->m_ParticleRenderSpec.m_TransformMatrix = 
					glm::translate(Math::mat4(1.0f), particle.m_Position) * 
					glm::scale(glm::mat4(1.0f), { size.x, size.y, size.z });

				// Submit a color based on the current interpolation
				Rendering::Shader::SetDataAtInputLocation<Math::vec4>
				(
					colorEaseFunc
					(
						emitter.m_Config->m_ColorBegin, 
						emitter.m_Config->m_ColorEnd, 
						progress
					),
					"a_Color", 
					s_ParticleContext->m_ParticleRenderSpec.m_Buffer, 
					s_ParticleContext->m_ParticleRenderSpec.m_Shader
				);

				// Render the particle
				Rendering::RenderingService::SubmitDataToRenderer(s_ParticleContext->m_ParticleRenderSpec);
			}
		}

		// End rendering context and submit rendering data to GPU
		Rendering::RenderingService::EndScene();
	}
	UUID ParticleService::AddEmitter(EmitterConfig* config, const Math::vec3 position)
	{
		KG_ASSERT(s_ParticleContext);
		KG_ASSERT(config);
		KG_ASSERT(config->m_BufferSize > 0);

		// Get current time
		float currentTime{ EngineService::GetActiveEngine().GetInApplicationTime() };

		// Create emitter instance
		UUID returnID{};
		EmitterInstance newEmitterInstance;
		newEmitterInstance.m_Config = config;
		newEmitterInstance.m_Particles.resize(config->m_BufferSize);
		newEmitterInstance.m_Position = position;
		newEmitterInstance.m_ParticleIndex = config->m_BufferSize - 1;
		newEmitterInstance.m_StartTime = currentTime;
		newEmitterInstance.m_EndTime = currentTime + config->m_EmitterLifetime;

		// Attempt to insert new emitter
		auto [iter, success] = s_ParticleContext->m_AllEmitters.insert_or_assign(returnID, newEmitterInstance);

		// Ensure insertion of emitter was successful
		if (!success)
		{
			return k_EmptyUUID;
		}

		return returnID;
	}
	bool ParticleService::RemoveEmitter(UUID emitterID)
	{
		KG_ASSERT(s_ParticleContext);

		// Attempt to erase emitter from context
		size_t erased = s_ParticleContext->m_AllEmitters.erase(emitterID);

		// Return success of erase operation
		return erased > 0;
	}
	void ParticleService::ClearEmitters()
	{
		s_ParticleContext->m_AllEmitters.clear();
	}
	std::unordered_map<UUID, EmitterInstance>& ParticleService::GetAllEmitters()
	{
		return s_ParticleContext->m_AllEmitters;
	}
	void ParticleService::LoadSceneEmitters(Ref<Scenes::Scene> scene)
	{
		for (entt::entity enttID : scene->GetAllEntitiesWith<ECS::ParticleEmitterComponent>())
		{
			ECS::Entity entity{ scene->GetEntityByEnttID(enttID) };
			ECS::ParticleEmitterComponent particleComp = entity.GetComponent<ECS::ParticleEmitterComponent>();
			ECS::TransformComponent transform = entity.GetComponent<ECS::TransformComponent>();
			if (particleComp.m_EmitterConfigHandle == Assets::EmptyHandle)
			{
				continue;
			}

			AddEmitter(particleComp.m_EmitterConfigRef.get(), transform.Translation);
		}
	}
}
