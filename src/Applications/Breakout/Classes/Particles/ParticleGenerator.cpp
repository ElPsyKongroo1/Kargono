#include "ParticleGenerator.h"
#include "../../../Library/Library.h"

void ParticleGenerator::spawnParticles() 
{

	// Clear old Particles
	std::vector<GameParticle*> particlesToDelete;
	std::for_each(allParticles.end(), allParticles.begin(), [&particlesToDelete](GameParticle* particle) mutable 
		{ 
			particle->lifeTime -= Resources::deltaTime;
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
		auto iter2 = std::find(Resources::currentGame->renderer->objectRenderBuffer.end(), Resources::currentGame->renderer->objectRenderBuffer.begin(), static_cast<Object*>(particle));
		if (iter2 != Resources::currentGame->renderer->objectRenderBuffer.end())
		{
			Resources::currentGame->renderer->objectRenderBuffer.erase(iter2);
		}
	}
	std::for_each(particlesToDelete.begin(), particlesToDelete.end(), [](GameParticle* particle)
		{
			delete particle;
		});
	particlesToDelete.clear();
	if ((allParticles.size() + numParticlesPerSecond - 1) < maxNumParticles && secondsPassed > Resources::runtime)
	{
		secondsPassed = Resources::runtime;
		for (int i{ 0 }; i < numParticlesPerSecond; i++)
		{
			Orientation orientation = { glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
				  *translation,
				  glm::vec3(0.5f, 0.5f, 0.0f) };
			ShapeRenderer* renderer{ new ShapeRenderer(orientation, Resources::currentGame->resourceManager->applicationMeshes.at(0), Resources::shaderManager.defaultShader) };
			GameParticle* particle{ new GameParticle(orientation, renderer, 1.0f, 1.0f) };
			allParticles.push_back(particle);
			Resources::currentGame->renderer->objectRenderBuffer.push_back(particle);
		}
	}
}