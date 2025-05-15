#include "kgpch.h"

#include "Modules/Particles/ParticleService.h"

#include "Modules/Rendering/RenderingService.h"
#include "Modules/Assets/AssetService.h"
#include "Modules/Core/Engine.h"
#include "Kargono/Scenes/Scene.h"
#include "Modules/ECS/Entity.h"
#include "Modules/ECS/EngineComponents.h"
#include "Kargono/Math/Interpolation.h"
#include "Modules/Events/SceneEvent.h"

namespace Kargono::Particles
{
    bool ParticleContext::Init()
    {
		// Initialize rendering data for particles
		{
			m_ParticleRenderSpec.ClearData();
				// Create shader for UI background/quad rendering
			Rendering::ShaderSpecification shaderSpec{ Rendering::ColorInputType::FlatColor, Rendering::TextureInputType::None, false, true, false, Rendering::RenderingType::DrawIndex, false };
			auto [uuid, localShader] = Assets::AssetService::GetShader(shaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			// Default particle color is pure white
			Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 1.0f, 1.0f, 1.0f, 1.0f }, 
				Utility::FileSystem::CRCFromString("a_Color"), 
				localBuffer, 
				localShader);

			// Create basic shape component for UI quad rendering
			ECS::ShapeComponent* shapeComp = new ECS::ShapeComponent();
			shapeComp->CurrentShape = Rendering::ShapeTypes::Quad;
			shapeComp->Vertices = CreateRef<std::vector<Math::vec3>>(Rendering::Shape::s_Quad.GetIndexVertices());
			shapeComp->Indices = CreateRef<std::vector<uint32_t>>(Rendering::Shape::s_Quad.GetIndices());


			m_ParticleRenderSpec.m_Shader = localShader;
			m_ParticleRenderSpec.m_Buffer = localBuffer;
			m_ParticleRenderSpec.m_ShapeComponent = shapeComp;
		}
		
		return true;
    }

    bool ParticleContext::Terminate()
    {
		return true;
    }



    void ParticleContext::OnUpdate(Timestep ts)
    {
		// Get current time
		float currentTime{ EngineService::GetActiveEngine().GetThread().GetInApplicationTime() };

		size_t iteration{ 0 };
		for (auto& [uuid, emitter] : m_AllEmitters)
		{
			// TODO: If a parent entity exists, maybe just set to inactive

			// Set emitter location if a parent entity exists
			if (emitter.m_ParentScene)
			{
				ECS::Entity entity = emitter.m_ParentScene->GetEntityByUUID(emitter.m_ParentEntityID);
				ECS::TransformComponent entityTransform = entity.GetComponent<ECS::TransformComponent>();
				emitter.m_Position = entityTransform.Translation;
			}


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
					EngineService::GetActiveEngine().GetThread().SubmitFunction([this]()
					{
						for (UUID id : s_EmittersToRemove)
						{
							m_AllEmitters.erase(id);
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
				currentParticle.m_Position.x = emitter.m_Position.x + m_RandomGenerator.GenerateFloatBounds
				( 
					emitter.m_Config->m_SpawningBounds[0].x,
					emitter.m_Config->m_SpawningBounds[1].x
				);
				currentParticle.m_Position.y = emitter.m_Position.y + m_RandomGenerator.GenerateFloatBounds
				(
					emitter.m_Config->m_SpawningBounds[0].y,
					emitter.m_Config->m_SpawningBounds[1].y
				);
				currentParticle.m_Position.z = emitter.m_Position.z + m_RandomGenerator.GenerateFloatBounds
				(
					emitter.m_Config->m_SpawningBounds[0].z,
					emitter.m_Config->m_SpawningBounds[1].z
				);

				currentParticle.m_StartTime = currentTime;
				currentParticle.m_EndTime = currentTime + emitter.m_Config->m_ParticleLifetime;
				currentParticle.m_Size = emitter.m_Config->m_SizeBegin;

				//TODO: Generate random velocity TODO: CHANGE THIS
				currentParticle.m_Velocity.x = m_RandomGenerator.GenerateFloatBounds(-1.0f, 1.0f);
				currentParticle.m_Velocity.y = m_RandomGenerator.GenerateFloatBounds(-1.0f, 1.0f);
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

	void ParticleContext::OnRender(const Math::mat4& viewProjection)
	{
		// Start rendering context
		Rendering::RenderingService::BeginScene(viewProjection);

		// Get current time
		float currentTime{ EngineService::GetActiveEngine().GetThread().GetInApplicationTime() };

		for (auto& [uuid, emitter] : m_AllEmitters)
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
				m_ParticleRenderSpec.m_TransformMatrix = 
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
					Utility::FileSystem::CRCFromString("a_Color"),
					m_ParticleRenderSpec.m_Buffer, 
					m_ParticleRenderSpec.m_Shader
				);

				// Render the particle
				Rendering::RenderingService::SubmitDataToRenderer(m_ParticleRenderSpec);
			}
		}

		// End rendering context and submit rendering data to GPU
		Rendering::RenderingService::EndScene();
	}
	bool ParticleContext::OnSceneEvent(Events::Event* event)
	{
		if (event->GetEventType() == Events::EventType::ManageEntity)
		{
			Events::ManageEntity* manageEntity = (Events::ManageEntity*)event;
			if (manageEntity->GetAction() == Events::ManageEntityAction::Delete)
			{
				if (Scenes::SceneService::GetActiveScene().get() != manageEntity->GetSceneReference())
				{
					KG_WARN("Attempt to remove particle emitters from a scene that is not active");
					return false;
				}
				if (manageEntity->GetEntityID() == k_EmptyUUID)
				{
					KG_WARN("Attempt to remove particle emitters using an empty entity handle");
					return false;
				}

				// Search for entity to delete
				UUID emitterToRemove{ k_EmptyUUID };
				for (auto& [handle, emitter] : m_AllEmitters)
				{
					if (emitter.m_ParentEntityID == manageEntity->GetEntityID())
					{
						emitterToRemove = emitter.m_ParentEntityID;
						break;
					}
				}

				// Remove indicated entity
				if (emitterToRemove != k_EmptyUUID)
				{
					m_AllEmitters.erase(emitterToRemove);
					return true;
				}

			}
		}

		return false;
	}
	UUID ParticleContext::AddEmitter(EmitterConfig* config, const Math::vec3& position)
	{
		KG_ASSERT(config);
		KG_ASSERT(config->m_BufferSize > 0);

		// Get current time
		float currentTime{ EngineService::GetActiveEngine().GetThread().GetInApplicationTime() };

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
		auto [iter, success] = m_AllEmitters.insert_or_assign(returnID, newEmitterInstance);

		// Ensure insertion of emitter was successful
		if (!success)
		{
			return k_EmptyUUID;
		}

		return returnID;
	}

	void ParticleContext::AddEmitterByHandle(Assets::AssetHandle emitterHandle, const Math::vec3& position)
	{
		KG_ASSERT(emitterHandle != Assets::EmptyHandle);

		// Get emitter from asset service
		Ref<Particles::EmitterConfig> emitter = Assets::AssetService::GetEmitterConfig(emitterHandle);

		KG_ASSERT(emitter);

		// Call add emitter function
		AddEmitter(emitter.get(), position);
	}

	UUID ParticleContext::AddEmitter(EmitterConfig* config, Scenes::Scene* parentScene, UUID entityID)
	{
		KG_ASSERT(config);
		KG_ASSERT(config->m_BufferSize > 0);

		// Get current time
		float currentTime{ EngineService::GetActiveEngine().GetThread().GetInApplicationTime() };

		// Create emitter instance
		UUID returnID{};
		EmitterInstance newEmitterInstance;
		newEmitterInstance.m_Config = config;
		newEmitterInstance.m_Particles.resize(config->m_BufferSize);
		newEmitterInstance.m_ParentScene = parentScene;
		newEmitterInstance.m_ParentEntityID = entityID;
		newEmitterInstance.m_ParticleIndex = config->m_BufferSize - 1;
		newEmitterInstance.m_StartTime = currentTime;
		newEmitterInstance.m_EndTime = currentTime + config->m_EmitterLifetime;

		// Attempt to insert new emitter
		auto [iter, success] = m_AllEmitters.insert_or_assign(returnID, newEmitterInstance);

		// Ensure insertion of emitter was successful
		if (!success)
		{
			return k_EmptyUUID;
		}

		return returnID;
	}

	bool ParticleContext::RemoveEmitter(UUID emitterID)
	{
		// Attempt to erase emitter from context
		size_t erased = m_AllEmitters.erase(emitterID);

		// Return success of erase operation
		return erased > 0;
	}
	void ParticleContext::ClearEmitters()
	{
		m_AllEmitters.clear();
	}
	void ParticleContext::ClearSceneEmitters()
	{
		std::vector<UUID> emittersToRemove;

		// Get ID's for all scene emitters
		for (auto& [emitterHandle, emitterInstance] : m_AllEmitters)
		{
			if (emitterInstance.m_ParentScene)
			{
				emittersToRemove.emplace_back(emitterHandle);
			}
		}

		// Remove all of the scene emitters
		for (UUID emitterID : emittersToRemove)
		{
			m_AllEmitters.erase(emitterID);
		}
	}
	std::unordered_map<UUID, EmitterInstance>& ParticleContext::GetAllEmitters()
	{
		return m_AllEmitters;
	}
	void ParticleContext::LoadSceneEmitters(Ref<Scenes::Scene> scene)
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

			AddEmitter(particleComp.m_EmitterConfigRef.get(), scene.get(), entity.GetUUID());
		}
	}
}
