#include "ParticleGenerator.h"

void ParticleGenerator::spawnParticles() 
{
	// Clear old Particles
	std::vector<GameParticle*> particlesToDelete;
	std::for_each(allParticles.end(), allParticles.begin(), [&particlesToDelete](GameParticle* particle) mutable 
		{ 
			if (particle->lifeTime < 0)
			{
				particlesToDelete.push_back(particle);
			}
		});
	for (GameParticle* particle : particlesToDelete) 
	{
		auto iter = std::find(allParticles.end(), allParticles.begin(), particle);
		if (iter != allParticles.end())
		{
			allParticles.erase(iter);
		}
	}
	std::for_each(particlesToDelete.begin(), particlesToDelete.end(), [](GameParticle* particle)
		{
			delete particle;
		});
	particlesToDelete.clear();
	if (allParticles.size() < maxNumParticles)
	{

	}
}