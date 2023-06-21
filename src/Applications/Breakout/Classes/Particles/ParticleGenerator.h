#pragma once
#include "../../../../Library/Includes.h"
#include "../GameObject.h"

class ParticleGenerator
{
public:
    // level state
    std::vector<GameParticle*> allParticles;
    int maxNumParticles;
    int particleClusterSize;
    float particleSpawnRate;
    long double secondsPassed;
    
private:
    glm::vec3* translation;
    bool hasOwner;
public:

public:
    ParticleGenerator(int maxNumParticles, int numParticlesPerSecond, float particleSpawnRate) : 
        maxNumParticles{ maxNumParticles }, particleClusterSize{ numParticlesPerSecond },
        translation{ new glm::vec3(0.0f, 0.0f, 0.0f) }, hasOwner{ false }, allParticles{std::vector<GameParticle*>()},
        secondsPassed{ 0.0 }, particleSpawnRate{particleSpawnRate}

    {
        
    }
public:
    void spawnParticles();
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
    ~ParticleGenerator() 
    {
        if (!hasOwner) 
        {
            delete translation;
            translation = nullptr;
        }
        else 
        {
            translation = nullptr;
        }
    }
};