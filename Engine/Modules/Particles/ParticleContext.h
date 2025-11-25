#pragma once
#include "Kargono/Math/Math.h"
#include "Kargono/Math/Interpolation.h"
#include "Kargono/Core/Base.h"
#include "Kargono/Core/Timestep.h"
#include "Modules/Events/Event.h"
#include "Modules/Rendering/Assets/Shader.h"
#include "Kargono/Utility/Random.h"
#include "Modules/Particles/EmitterInstance.h"

#include <vector>
#include <array>
#include <unordered_map>

namespace Kargono { struct Transform; }
namespace Kargono::ECS { class Entity; }
namespace Kargono::Scenes { class Scene; }

namespace Kargono::Particles
{
	using EmitterMap = std::unordered_map<UUID, EmitterInstance>;

	class ParticleContext
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ParticleContext() = default;
		~ParticleContext() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		bool Init();
		bool Terminate();
		void OnUpdate(Timestep ts);
	public:
		//==============================
		// On Event Functions
		//==============================
		void OnRender(const Math::mat4& viewProjection);
		bool OnSceneEvent(Events::Event* event);

		//==============================
		// Manage Emitters
		//==============================
		UUID AddEmitter(Assets::AssetRef<EmitterConfig> config, const Math::vec3& position);
		// TODO: YOU SHOULD RETURN A UUID YEA?
		void AddEmitterByHandle(Assets::AssetHandle emitterHandle, const Math::vec3& position);
		UUID AddEmitter(Assets::AssetRef<EmitterConfig> config, Scenes::Scene* parentScene, UUID entityID);
		bool RemoveEmitter(UUID emitterID);
		void ClearEmitters();
		void ClearSceneEmitters();
		void LoadSceneEmitters(Assets::AssetRef<Scenes::Scene> scene);

		//==============================
		// Getters/Setters
		//==============================
		EmitterMap& GetAllEmitters();
	private:
		//==============================
		// Internal Fields
		//==============================
		// All emitters being managed
		EmitterMap m_AllEmitters;
		Rendering::RendererInputSpec m_ParticleRenderSpec;
		Utility::PseudoGenerator m_RandomGenerator{ 37427394 };
	};


	class ParticleService // TODO: REMOVE EWWWWWWW
	{
	public:
		//==============================
		// Create Particle Context
		//==============================
		static void CreateParticleContext()
		{
			// Initialize ParticleContext
			if (!s_ParticleContext)
			{
				s_ParticleContext = CreateRef<ParticleContext>();
			}

			// Verify init is successful
			KG_VERIFY(s_ParticleContext, "Particle Service System Initiated");
		}
		static void RemoveParticleContext()
		{
			// Clear ParticleContext
			s_ParticleContext.reset();
			s_ParticleContext = nullptr;

			// Verify terminate is successful
			KG_VERIFY(!s_ParticleContext, "Particle Service System Initiated");
		}
		//==============================
		// Getters/Setters
		//==============================
		static ParticleContext& GetActiveContext() { return *s_ParticleContext; }
		static bool IsContextActive() { return (bool)s_ParticleContext; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Ref<ParticleContext> s_ParticleContext{ nullptr };
	};
}

