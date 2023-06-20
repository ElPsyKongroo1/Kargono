#pragma once
#include "../../../../Library/Includes.h"
#include "../GameObject.h"

class ParticleGenerator
{
public:
    // level state
    std::vector<GameParticle*> allParticles;
    float maxNumParticles;
    int numParticlesPerSecond;
    long long  secondsPassed;
    
private:
    glm::vec3* translation;
    bool hasOwner;
public:

public:
    ParticleGenerator(float maxNumParticles, int numParticlesPerSecond) : 
        maxNumParticles{ maxNumParticles }, numParticlesPerSecond{ numParticlesPerSecond },
        translation{ new glm::vec3(0.0f, 0.0f, 0.0f) }, hasOwner{ false }, allParticles{std::vector<GameParticle*>()},
        secondsPassed{0}

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
        if (!hasOwner) 
        {
            delete translation;
            translation = nullptr;
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