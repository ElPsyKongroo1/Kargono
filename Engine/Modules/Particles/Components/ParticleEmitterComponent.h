#pragma once
#include "Modules/Particles/ParticleContext.h"
#include "Modules/Particles/Module/ParticlesModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"
#include "Modules/Assets/AssetsCommon.h"

namespace Kargono::Particles
{
	struct ParticleEmitterComponent
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ParticleEmitterComponent() = default;
		~ParticleEmitterComponent() = default;
	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(ParticleEmitterComponent* dst)
		{
			// Create the component in place
			std::construct_at<ParticleEmitterComponent>(dst);

			// Copy over relevant data
			dst->m_EmitterConfigHandle = m_EmitterConfigHandle;
			dst->m_EmitterConfigRef = m_EmitterConfigRef;
		}

	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Public Fields
		//==============================
		Assets::AssetHandle m_EmitterConfigHandle{ Assets::k_EmptyHandle };
		Ref<Particles::EmitterConfig> m_EmitterConfigRef{ nullptr };
	};

	Register_Module_Type(ParticleEmitterComponent, ECSInternal::ComponentTag)
}