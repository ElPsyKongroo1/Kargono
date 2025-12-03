#pragma once
#include "Modules/Particles/ParticleContext.h"
#include "Modules/Particles/Module/ParticlesModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/AssetReference.h"

namespace Kargono::Particles
{
	struct ParticleEmitter
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ParticleEmitter() = default;
		~ParticleEmitter() = default;
	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(ParticleEmitter* dst)
		{
			// Create the component in place
			std::construct_at<ParticleEmitter>(dst);

			// Copy over relevant data
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
		Assets::AssetRef<EmitterConfig> m_EmitterConfigRef{};
	};

	Register_Module_Type(ParticleEmitter, ECSInternal::ComponentTag)
}