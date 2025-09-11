#pragma once
#include "Modules/Particles/ParticleService.h"
#include "Modules/Assets/Asset.h"

#include "Modules/Particles/Module/ParticlesModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"

namespace Kargono::Particles
{
	struct ParticleEmitterComponent
	{
		//==============================
		// Constructors/Destructors
		//==============================
		ParticleEmitterComponent() = default;
		~ParticleEmitterComponent() = default;

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
		// Public Fields
		//==============================
		Assets::AssetHandle m_EmitterConfigHandle{ Assets::k_EmptyHandle };
		Ref<Particles::EmitterConfig> m_EmitterConfigRef{ nullptr };
	};

	Register_Module_Type(ParticleEmitterComponent, ECSInternal::ComponentTag)
}