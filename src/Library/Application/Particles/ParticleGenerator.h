#pragma once
#include "../../Includes.h"
#include "../GameApplication/GameObject/GameObject.h"

class ParticleGenerator
{
public:
    struct SpawnPattern 
    {
        int numParticles;
        std::vector<glm::vec3> relativeTranslations;
        std::vector<glm::vec3> directions;
    };
public:
    // level state
    bool hasLifeTime;
    float lifeTime;
    std::vector<GameParticle*> allParticles;
    SpawnPattern spawnPattern;
    int maxNumParticles;
    int particleClusterSize;
    float particleSpawnRate;
    long double secondsPassed;
    
private:
    glm::vec3* translation;
    bool hasOwner;
public:

public:
    ParticleGenerator(int maxNumParticles, int numParticlesPerSecond, float particleSpawnRate, bool hasLifeTime, float lifeTime, glm::vec3& translation) : 
        maxNumParticles{ maxNumParticles }, particleClusterSize{ numParticlesPerSecond },
        translation{ new glm::vec3( translation) }, hasOwner{ false }, allParticles{std::vector<GameParticle*>()},
        secondsPassed{ 0.0 }, particleSpawnRate{ particleSpawnRate }, hasLifeTime{ hasLifeTime }, lifeTime{lifeTime}

    {
        // Initialize Spawn Pattern
        std::vector<glm::vec3> relativeTranslations{std::vector<glm::vec3>()};
        relativeTranslations.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

        std::vector<glm::vec3> relativeDirections{std::vector<glm::vec3>()};
        relativeDirections.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

        spawnPattern = SpawnPattern(1, relativeTranslations, relativeDirections);
    }
    ~ParticleGenerator();
public:
    void spawnParticles();
    void moveParticles();
    void setTranslation(const glm::vec3& translation) 
    {
        if (hasOwner) { return; }
        *(this->translation) = translation;
    }
    void setOwner(glm::vec3* translation) 
    {
        //std::cout << translation
        if (!hasOwner) 
        {
            delete this->translation;
            this->translation = nullptr;
            hasOwner = true;
        }
        this->translation = translation;
        
    }
};