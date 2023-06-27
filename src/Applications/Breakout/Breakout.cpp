#include "Breakout.h"
#include "../../Library/Includes.h"

#include "../../Library/Library.h"
#include "../../Library/Rendering/Shaders/Shaders.h"
#include "../../Library/Rendering/Mesh/Meshes.h"
#include "../../Library/Rendering/Textures/Textures.h"
#include "../../Library/Application/Objects/Objects.h"
#include "../../Library/Application/ResourceManager/ResourceManager.h"
#include "../../Library/Application/GameApplication/GameObject/GameObject.h"
#include "../../Library/Application/GameApplication/GameLevel/GameLevel.h"
#include "../../Library/Application/Particles/ParticleGenerator.h"
#include "../../Library/Application/Audio/AudioContext.h"

void initializeRenderer();
void processBrickCollisions(GameBall* ball, std::vector<GameBrick*> bricks, GameLevel* level);
void processPaddleCollision(GameBall* ball, GamePaddle* paddle, GameLevel* level);

void BreakoutStart()
{
	// Initialize GLFW context, Meshes, Shaders, and Textures
	initializeRenderer();
	Resources::currentGame->resourceManager = new ResourceManager();
	Resources::currentGame->resourceManager->initializeResources();
    Resources::currentGame->recentInput = Resources::currentGame->resourceManager->localInputs.at(1);
    Resources::currentGame->currentInput = Resources::currentGame->recentInput;
    Resources::currentGame->audioContext->stereoSource->play();
    AudioBuffer* popSound = new AudioBuffer("Resources/Breakout/Sounds/pop-sound.wav");
    Resources::currentGame->audioContext->allAudioBuffers.push_back(popSound);
    AudioSource* popSource = new AudioSource(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
        1.0f, 1.0f, AL_FALSE, popSound);
    Resources::currentApplication->audioContext->allAudioSources.push_back(popSource);
    AudioSource* lowPopSource = new AudioSource(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
        0.5f, 0.4f, AL_FALSE, popSound);
    Resources::currentApplication->audioContext->allAudioSources.push_back(lowPopSource);
    
    
	GameLevel* currentLevel{ new GameLevel(25, 18) };
	currentLevel->Load("Resources/Breakout/Map/Level1.txt");
    Resources::currentGame->currentLevel = currentLevel;
	for (GameBrick* object : currentLevel->currentMapBricks)
	{
		if (!object) { continue; }
		Resources::currentApplication->renderer->objectRenderBuffer.push_back(object);
	}

	Orientation orientation = { glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
				  glm::vec3(0.0f, -260.0f, 0.0f),
				  glm::vec3(120.0f, 120.0f, 0.0f) };
	ShapeRenderer* renderer = { new ShapeRenderer(orientation,
		Resources::currentGame->resourceManager->localMeshes.at(3),
		Resources::currentApplication->renderer->defaultShader) };
	GamePaddle* paddle{ new GamePaddle(orientation, renderer, 300.0f, lowPopSource) };
	Resources::currentApplication->renderer->objectRenderBuffer.push_back(paddle);
    Resources::currentGame->paddle = paddle;

    Orientation orientationBall = { glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, -150.0f, 0.0f),
                  glm::vec3(15.0f, 15.0f, 0.0f) };
    ShapeRenderer* rendererBall = { new ShapeRenderer(orientation,
        Resources::currentGame->resourceManager->localMeshes.at(2),
        Resources::currentApplication->renderer->defaultShader) };
    GameBall* ball{ new GameBall(orientationBall, rendererBall, 400.0f, nullptr) };
    Resources::currentGame->ball = ball;
    Resources::currentApplication->renderer->objectRenderBuffer.push_back(ball);

    glm::vec3 defaultTranslation {glm::vec3(0.0f, 0.0f, 0.0f)};

    ParticleGenerator* ballParticleGen{ new ParticleGenerator(10, 1, 0.15f, false, 0.0f, defaultTranslation) };
    ballParticleGen->setOwner(&ball->orientation.translation);
    currentLevel->currentParticleGenerators.push_back(ballParticleGen);

    ParticleGenerator* paddleParticleGen{ new ParticleGenerator(10, 1, 0.15f, false, 0.0f, defaultTranslation ) };
    paddleParticleGen->setOwner(&paddle->orientation.translation);
    currentLevel->currentParticleGenerators.push_back(paddleParticleGen);
    

	// Main running Loop
	while (!glfwWindowShouldClose(Resources::currentApplication->renderer->window))
	{
        std::vector<ParticleGenerator*> generatorsToDelete;
        for (ParticleGenerator* pgenerator : currentLevel->currentParticleGenerators) 
        {
            pgenerator->spawnParticles();
            pgenerator->moveParticles();
            if (pgenerator->hasLifeTime) 
            {
                pgenerator->lifeTime -= Resources::deltaTime;
                if (pgenerator->lifeTime < 0.0f)
                {
                    generatorsToDelete.push_back(pgenerator);
                }
            }
        }
        std::for_each(generatorsToDelete.rbegin(), generatorsToDelete.rend(), [&currentLevel](ParticleGenerator* particleGen) mutable
            {
                auto iter = std::find(currentLevel->currentParticleGenerators.rbegin(), currentLevel->currentParticleGenerators.rend(), particleGen);
                if (iter != currentLevel->currentParticleGenerators.rend())
                {
                    currentLevel->currentParticleGenerators.erase(std::next(iter).base());
                }
                delete particleGen;
            });
        generatorsToDelete.clear();
        
        if (Resources::currentGame->State == GameApplication::GAME_ACTIVE)
        {
            ball->Move();
            processBrickCollisions(ball, currentLevel->currentMapBricks, currentLevel);
            processPaddleCollision(ball, paddle, currentLevel);
        }

        
        
		Resources::currentApplication->renderer->render();

	}


    Resources::currentGame->paddle = nullptr;
	delete paddle;
	paddle = nullptr;

    delete ball;
    ball = nullptr;

    for (ParticleGenerator* pgenerator : currentLevel->currentParticleGenerators)
    {
        if (pgenerator) { delete pgenerator; }
        pgenerator = nullptr;
    }

    delete currentLevel;
    currentLevel = nullptr;

    Resources::currentGame->State = GameApplication::GAME_MENU;
    Resources::currentApplication->currentInput = nullptr;
    Resources::currentApplication->recentInput = nullptr;
    Resources::currentGame->audioContext->terminate();

	delete Resources::currentGame->resourceManager;
	Resources::currentGame->resourceManager = nullptr;
	delete Resources::currentApplication->renderer->currentCamera;
	Resources::currentApplication->renderer->currentCamera = nullptr;
	Resources::currentApplication->renderer->close();
}


void initializeRenderer() 
{
    Resources::currentApplication->renderer->init();
	Resources::currentApplication->renderer->currentCamera = new GLCamera(glm::vec3(0.0f, 0.0f, 50.0f), glm::normalize(glm::vec3(0.0f, 0.0, -1.0f)), glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(-90.0f, 0.0f, 0.0f), 3.0f, GLCamera::ORTHOGRAPHIC,
		glm::vec2(-400.0f, 400.0f), glm::vec2(-300.0f, 300.0f), glm::vec2(0.1f, 100.0f), 45.0f,
		(float)Resources::currentApplication->renderer->screenDimension.x / (float)Resources::currentApplication->renderer->screenDimension.y,
		0.1f);

    // Make Application 3D
    Resources::currentApplication->renderer->currentCamera->orientation.cameraPosition = glm::vec3(0.0f, -400.0f, 600.0f);
    Resources::currentApplication->renderer->currentCamera->orientation.cameraFront = glm::normalize(glm::vec3(0.0f, 0.67f, -1.0f));
    Resources::currentApplication->renderer->currentCamera->projection = GLCamera::PERSPECTIVE;
    Resources::currentApplication->renderer->currentCamera->frustrumDimensions.depthDimension = glm::vec2(0.1f, 3000.0f);

    
    Resources::currentApplication->renderer->setDefaultValues(Resources::modelManager.simpleBackpack,
        Resources::meshManager.cubeMesh,
        Resources::shaderManager.defaultShader,
		Resources::currentApplication->renderer->currentCamera);
	
}
void calculateSlope() 
{
    
}


void processPaddleCollision(GameBall* ball, GamePaddle* paddle, GameLevel* level)
{
    if (glm::distance(paddle->orientation.translation, ball->orientation.translation) < (ball->radius + glm::length(paddle->currentDimensions)))
    {
        enum ballRelativeLocation { LEFT, BELOW, ABOVE, RIGHT };
        ballRelativeLocation currentRelativeLocation;
        // y=mx+b for line between ball and brick
        bool is_delta_x_0 = false;
        float delta_y = paddle->orientation.translation.y - ball->orientation.translation.y;
        float delta_x = paddle->orientation.translation.x - ball->orientation.translation.x;

        float slope;
        float y_intercept;
        float paddleSlopeCutOff = (paddle->currentDimensions.y * 0.85f) / paddle->currentDimensions.x;
        if (delta_x == 0)
        {
            is_delta_x_0 = true;
            slope = 1000000;
            y_intercept = ball->orientation.translation.y - (slope * ball->orientation.translation.x);
        }

        else
        {
            slope = delta_y / delta_x;
            y_intercept = ball->orientation.translation.y - (slope * ball->orientation.translation.x);
        }

        if (slope <= -paddleSlopeCutOff || slope >= paddleSlopeCutOff || is_delta_x_0)
        {
            if (ball->orientation.translation.y > paddle->orientation.translation.y)
            {
                currentRelativeLocation = ABOVE;
            }
            else
            {
                currentRelativeLocation = BELOW;
            }
        }
        else
        {
            if (ball->orientation.translation.x > paddle->orientation.translation.x)
            {
                currentRelativeLocation = RIGHT;
            }
            else
            {
                currentRelativeLocation = LEFT;
            }
        }
        glm::vec2 ballTranslation = glm::vec2(ball->orientation.translation);
        // Calculate Closest Point on Brick to Circle
        glm::vec2 closestPointOnBrickToBall = glm::vec2();
        
        switch (currentRelativeLocation)
        {
        case ABOVE:
            closestPointOnBrickToBall.y = paddle->orientation.translation.y + (0.5f * paddle->currentDimensions.y);
            closestPointOnBrickToBall.x = (closestPointOnBrickToBall.y - y_intercept) / slope;
            break;
        case BELOW:
            closestPointOnBrickToBall.y = paddle->orientation.translation.y - (0.5f * paddle->currentDimensions.y);
            closestPointOnBrickToBall.x = (closestPointOnBrickToBall.y - y_intercept) / slope;
            break;
        case LEFT:
            closestPointOnBrickToBall.x = paddle->orientation.translation.x - (0.5f * paddle->currentDimensions.x);
            closestPointOnBrickToBall.y = (slope * closestPointOnBrickToBall.x) + y_intercept;
            break;
        case RIGHT:
            closestPointOnBrickToBall.x = paddle->orientation.translation.x + (0.5f * paddle->currentDimensions.x);
            closestPointOnBrickToBall.y = (slope * closestPointOnBrickToBall.x) + y_intercept;
            break;
        default:
            std::cerr << "Invalid enumeration in Breakout.cpp::processBrickCollision::CalcClosestCircle\n";
            break;
        }
        if (glm::distance(closestPointOnBrickToBall, ballTranslation) < ball->radius)
        {
            float buffer = 0.5f;
            switch (currentRelativeLocation)
            {
            case ABOVE:
                ball->orientation.translation.y = paddle->orientation.translation.y +
                    (paddle->currentDimensions.y / 2) +
                    ball->radius + buffer;
                ball->direction.y = -ball->direction.y;
                break;
            case BELOW:
                ball->orientation.translation.y = paddle->orientation.translation.y -
                    (paddle->currentDimensions.y / 2) -
                    ball->radius - buffer;
                ball->direction.y = -ball->direction.y;
                break;
            case LEFT:
                ball->orientation.translation.x = paddle->orientation.translation.x -
                    (paddle->currentDimensions.x / 2) -
                    ball->radius - buffer;
                ball->direction.x = -ball->direction.x;
                break;
            case RIGHT:
                ball->orientation.translation.x = paddle->orientation.translation.x +
                    (paddle->currentDimensions.x / 2) +
                    ball->radius + buffer;
                ball->direction.x = -ball->direction.x;
                break;
            default:
                std::cerr << "Invalid enumeration in Breakout.cpp::processBrickCollision::ChangeCircleDirection\n";
                break;
            }
            ball->direction = glm::normalize(ball->direction + (paddle->direction * 0.0035f * paddle->currentSpeed));
            paddle->audioSource->play();
        }

    }
}


void processBrickCollisions(GameBall* ball, std::vector<GameBrick*> bricks, GameLevel* level) 
{
    for (GameBrick* brick : bricks) 
    {
        if (glm::distance(brick->orientation.translation, ball->orientation.translation) < (ball->radius + glm::length(brick->currentDimensions))) 
        {
            enum ballRelativeLocation{ LEFT, BELOW, ABOVE, RIGHT };
            ballRelativeLocation currentRelativeLocation;
            // y=mx+b for line between ball and brick
            bool is_delta_x_0 = false;
            float delta_y = brick->orientation.translation.y - ball->orientation.translation.y;
            float delta_x = brick->orientation.translation.x - ball->orientation.translation.x;
            
            float slope;
            float y_intercept;
            if (delta_x == 0) 
            {
                is_delta_x_0 = true;
                slope = 0;
                y_intercept = 0;
            }
            else 
            {
                slope = delta_y / delta_x;
                y_intercept = ball->orientation.translation.y - (slope * ball->orientation.translation.x);
            }

            if (slope <= -1 || slope >= 1 || is_delta_x_0)
            {
                if (ball->orientation.translation.y > brick->orientation.translation.y)
                {
                    currentRelativeLocation = ABOVE;
                }
                else 
                {
                    currentRelativeLocation = BELOW;
                }
            }
            else 
            {
                if (ball->orientation.translation.x > brick->orientation.translation.x)
                {
                    currentRelativeLocation = RIGHT;
                }
                else 
                {
                    currentRelativeLocation = LEFT;
                }
            }
            // Calculate Closest Point on Brick to Circle
            glm::vec2 closestPointOnBrickToBall = glm::vec2();
            switch (currentRelativeLocation) 
            {
            case ABOVE:
                closestPointOnBrickToBall.y = brick->orientation.translation.y + (0.5f * brick->currentDimensions.y);
                closestPointOnBrickToBall.x = (closestPointOnBrickToBall.y - y_intercept) / slope;
                break;
            case BELOW:
                closestPointOnBrickToBall.y = brick->orientation.translation.y - (0.5f * brick->currentDimensions.y);
                closestPointOnBrickToBall.x = (closestPointOnBrickToBall.y - y_intercept) / slope;
                break;
            case LEFT:
                closestPointOnBrickToBall.x = brick->orientation.translation.x - (0.5f * brick->currentDimensions.x);
                closestPointOnBrickToBall.y = (slope * closestPointOnBrickToBall.x) + y_intercept;
                break;
            case RIGHT:
                closestPointOnBrickToBall.x = brick->orientation.translation.x + (0.5f * brick->currentDimensions.x);
                closestPointOnBrickToBall.y = (slope * closestPointOnBrickToBall.x) + y_intercept;
                break;
            default:
                std::cerr << "Invalid enumeration in Breakout.cpp::processBrickCollision::CalcClosestCircle\n";
                break;
            }
            if (glm::distance(closestPointOnBrickToBall, glm::vec2(ball->orientation.translation)) < ball->radius) 
            {
                switch (currentRelativeLocation) 
                {
                case ABOVE:
                    ball->orientation.translation.y = brick->orientation.translation.y + 
                                                      (brick->currentDimensions.y / 2) + 
                                                      ball->radius + 0.1f;
                    ball->direction.y = -ball->direction.y;
                    break;
                case BELOW:
                    ball->orientation.translation.y = brick->orientation.translation.y -
                        (brick->currentDimensions.y / 2) -
                        ball->radius - 0.1f;
                    ball->direction.y = -ball->direction.y;
                    break;
                case LEFT:
                    ball->orientation.translation.x = brick->orientation.translation.x -
                        (brick->currentDimensions.x / 2) -
                        ball->radius - 0.1f;
                    ball->direction.x = -ball->direction.x;
                    break;
                case RIGHT:
                    ball->orientation.translation.x = brick->orientation.translation.x +
                        (brick->currentDimensions.x / 2) +
                        ball->radius + 0.1f;
                    ball->direction.x = -ball->direction.x;
                    break;
                default:
                    std::cerr << "Invalid enumeration in Breakout.cpp::processBrickCollision::ChangeCircleDirection\n";
                    break;
                }
                ParticleGenerator* pgenerator = new ParticleGenerator(20, 2, 0.5, true, 0.5f, brick->orientation.translation);

                std::vector<glm::vec3> relativeTranslations{std::vector<glm::vec3>()};
                relativeTranslations.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
                relativeTranslations.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
                relativeTranslations.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
                relativeTranslations.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

                std::vector<glm::vec3> relativeDirections{std::vector<glm::vec3>()};
                relativeDirections.push_back(glm::vec3(1.0f, 1.0f, 0.0f));
                relativeDirections.push_back(glm::vec3(1.0f, -1.0f, 0.0f));
                relativeDirections.push_back(glm::vec3(-1.0f, 1.0f, 0.0f));
                relativeDirections.push_back(glm::vec3(-1.0f, -1.0f, 0.0f));

                ParticleGenerator::SpawnPattern spawnPattern{4, relativeTranslations, relativeDirections};

                pgenerator->spawnPattern = spawnPattern;
                brick->audioSource->play();
                level->currentParticleGenerators.push_back(pgenerator);
                level->RemoveBrick(brick);
                
            }
            
        }
    }
   
    
}