#include "Kargono/kgpch.h"
#include "ParticleGenerator.h"
#include "../../../Library/Library.h"

ParticleGenerator::~ParticleGenerator() 
{
	

	std::for_each(allParticles.begin(), allParticles.end(), [](GameParticle* particle) 
		{

			auto iter = std::find(Resources::currentGame->renderer->objectRenderBuffer.rbegin(), Resources::currentGame->renderer->objectRenderBuffer.rend(), static_cast<Object*>(particle));
			if (iter != Resources::currentGame->renderer->objectRenderBuffer.rend())
			{
				Resources::currentGame->renderer->objectRenderBuffer.erase(std::next(iter).base());
			}
			delete particle;

			particle = nullptr;
		});
	
	allParticles.clear();
	std::vector<GameParticle*> allParticles;
	maxNumParticles = -1;
	particleClusterSize = -1;
	particleSpawnRate = -1;
	secondsPassed = -1;

	if (hasOwner) 
	{
		translation = nullptr;
	}
	else 
	{
		delete translation;
		translation = nullptr;
	}
	hasOwner = false;
	
}

void ParticleGenerator::spawnParticles() 
{
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


	// Spawn Particles
	if ((allParticles.size() + particleClusterSize - 1) < maxNumParticles && (secondsPassed + particleSpawnRate) < Resources::runtime)
	{
		secondsPassed = Resources::runtime;
		for (int i{ 0 }; i < spawnPattern.numParticles; i++)
		{
			Orientation orientation = { glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
				  (*translation) + spawnPattern.relativeTranslations.at(i),
				  glm::vec3(4.0f, 4.0f, 0.0f) };
			ShapeRenderer* renderer{ new ShapeRenderer(orientation, Resources::currentGame->resourceManager->localMeshes.at(2), Resources::shaderManager.defaultShader) };
			glm::vec3 particleDirection {spawnPattern.directions.at(i)};
			GameParticle* particle{ new GameParticle(orientation, renderer, 100.0f, 0.4f, particleDirection, nullptr) };
			allParticles.push_back(particle);
			Resources::currentGame->renderer->objectRenderBuffer.push_back(particle);
		}
	}
}

void ParticleGenerator::moveParticles() 
{
	std::for_each(allParticles.begin(), allParticles.end(), [](GameParticle* particle){ particle->Move();});
}