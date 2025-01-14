#pragma once
#include "Kargono/Math/Math.h"
#include "Kargono/Math/Interpolation.h"
#include "Kargono/Core/Base.h"
#include "Kargono/Core/Timestep.h"
#include "Kargono/Utility/Traits.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Assets/Asset.h"

#include <vector>
#include <array>

namespace Kargono::ECS { struct TransformComponent; }
namespace Kargono::ECS { class Entity; }
namespace Kargono::Scenes { class Scene; }

namespace Kargono::Particles
{
	enum class EmitterMotionType
	{
		None = 0,
		NoMotion,
		FollowEntity
	};

	enum class EmitterLifecycle
	{
		None = 0,
		Immortal,
		FixedTime
	};

	struct EmitterConfig
	{
		// Emitter data
		EmitterLifecycle m_EmitterLifecycle{ EmitterLifecycle::Immortal };
		float m_EmitterLifetime{ 1.0f };
		size_t m_BufferSize{ 1000 };
		std::array<Math::vec3, 2> m_SpawningBounds{ Math::vec3(0.0f), Math::vec3(0.0f) };

		// Particle data
		bool m_UseGravity{ false };
		Math::vec3 m_GravityAcceleration{ Math::vec3(0.0f) };
		float m_ParticleLifetime{ 1.0f };
		size_t m_SpawnRatePerSec{ 5 };
		Math::InterpolationType m_ColorInterpolationType{ Math::InterpolationType::Linear };
		Math::vec4 m_ColorBegin{ 1.0f };
		Math::vec4 m_ColorEnd{ 1.0f };
		Math::InterpolationType m_SizeInterpolationType{ Math::InterpolationType::Linear };
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

		bool m_Active{ false };
	private:
        // Particle lifetime information
		float m_StartTime;
		float m_EndTime;
	private:
		friend class ParticleService;
    };

	struct EmitterInstance
	{
		Math::vec3 m_Position;
		EmitterMotionType m_EmitterMotionType{ EmitterMotionType::NoMotion };
		Scenes::Scene* m_ParentScene{ nullptr };
		UUID m_ParentEntityID{ k_EmptyUUID };
		EmitterConfig* m_Config;
		std::vector<Particle> m_Particles;
		size_t m_ParticleIndex;
		float m_ParticleSpawnAccumulator{ 0.0f };

		// Emitter lifetime information
		float m_StartTime;
		float m_EndTime;
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
		static bool OnSceneEvent(Events::Event* event);

		//==============================
		// Manage Emitters
		//==============================
		static UUID AddEmitter(EmitterConfig* config, const Math::vec3& position);
		static void AddEmitterByHandle(Assets::AssetHandle emitterHandle, const Math::vec3& position);
		static UUID AddEmitter(EmitterConfig* config, Scenes::Scene* parentScene, UUID entityID);
		static bool RemoveEmitter(UUID emitterID);
		static void ClearEmitters();
		static void ClearSceneEmitters();
		static std::unordered_map<UUID, EmitterInstance>& GetAllEmitters();

		static void LoadSceneEmitters(Ref<Scenes::Scene> scene);
    };
}


namespace Kargono::Utility
{
	inline std::string EmitterMotionTypeToString(Particles::EmitterMotionType motionType)
	{
		switch (motionType)
		{
		case Particles::EmitterMotionType::None: return "None";
		case Particles::EmitterMotionType::NoMotion: return "NoMotion";
		case Particles::EmitterMotionType::FollowEntity: return "FollowEntity";
		default: 
			KG_WARN("Invalid emitter motion type provided when attempting convert to a string");
			return "None";
		}
	}

	inline Particles::EmitterMotionType StringToEmitterMotionType(const std::string& str)
	{
		if (str == "None") return Particles::EmitterMotionType::None;
		if (str == "NoMotion") return Particles::EmitterMotionType::NoMotion;
		if (str == "FollowEntity") return Particles::EmitterMotionType::FollowEntity;
		KG_WARN("Invalid string provided when attempting to get emitter motion type");
		return Particles::EmitterMotionType::None;
	}

	inline std::string EmitterLifecycleToString(Particles::EmitterLifecycle lifecycle)
	{
		switch (lifecycle)
		{
		case Particles::EmitterLifecycle::None: return "None";
		case Particles::EmitterLifecycle::Immortal: return "Immortal";
		case Particles::EmitterLifecycle::FixedTime: return "FixedTime";
		default: 
			KG_WARN("Invalid emitter lifecycle provided when attempting convert to a string");
			return "None";
		}
	}

	inline Particles::EmitterLifecycle StringToEmitterLifecycle(const std::string& str)
	{
		if (str == "None") return Particles::EmitterLifecycle::None;
		if (str == "Immortal") return Particles::EmitterLifecycle::Immortal;
		if (str == "FixedTime") return Particles::EmitterLifecycle::FixedTime;

		KG_WARN("Invalid string provided when attempting to get emitter lifecycle");
		return Particles::EmitterLifecycle::None;
	}
}

