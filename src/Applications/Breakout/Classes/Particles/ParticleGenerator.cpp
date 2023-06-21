#include "ParticleGenerator.h"
#include "../../../Library/Library.h"

void ParticleGenerator::spawnParticles() 
{
	std::cout << "Number of Particles Beginning: " << allParticles.size(); //FIXME
	// Clear old Particles
	std::vector<GameParticle*> particlesToDelete;
	std::for_each(allParticles.rbegin(), allParticles.rend(), [&particlesToDelete](GameParticle* particle) mutable 
		{ 
			particle->lifeTime -= Resources::deltaTime;
			if (particle->lifeTime < 0)
			{
				particlesToDelete.push_back(particle);
			}
		});
	for (GameParticle* particle : particlesToDelete) 
	{
		auto iter = std::find(allParticles.rbegin(), allParticles.rend(), particle);
		if (iter != allParticles.rend())
		{
			allParticles.erase(std::next(iter).base());
		}
		auto iter2 = std::find(Resources::currentGame->renderer->objectRenderBuffer.rbegin(), Resources::currentGame->renderer->objectRenderBuffer.rend(), static_cast<Object*>(particle));
		if (iter2 != Resources::currentGame->renderer->objectRenderBuffer.rend())
		{
			Resources::currentGame->renderer->objectRenderBuffer.erase(std::next(iter2).base());
		}
	}
	std::for_each(particlesToDelete.begin(), particlesToDelete.end(), [](GameParticle* particle)
		{
			delete particle;
		});
	particlesToDelete.clear();
	std::cout << " Number of Particles after Clear: " << allParticles.size(); //FIXME
	if ((allParticles.size() + particleClusterSize - 1) < maxNumParticles && (secondsPassed + particleSpawnRate) < Resources::runtime)
	{
		secondsPassed = Resources::runtime;
		for (int i{ 0 }; i < particleClusterSize; i++)
		{
			Orientation orientation = { glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
				  *translation,
				  glm::vec3(10.0f, 10.0f, 0.0f) };
			ShapeRenderer* renderer{ new ShapeRenderer(orientation, Resources::currentGame->resourceManager->applicationMeshes.at(2), Resources::shaderManager.defaultShader) };
			GameParticle* particle{ new GameParticle(orientation, renderer, 1.0f, 0.4f) };
			allParticles.push_back(particle);
			Resources::currentGame->renderer->objectRenderBuffer.push_back(particle);
		}
	}
	std::cout << " Number of Particles after Addition: " << allParticles.size() << '\n'; //FIXME
}