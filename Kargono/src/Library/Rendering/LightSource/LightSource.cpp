#include "Kargono/kgpch.h"
#include "LightSource.h"

/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * LightSource Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/


/*============================================================================================================================================================================================
 * Constructors
 *============================================================================================================================================================================================*/

LightSource::LightSource(glm::vec3& position, glm::vec3& color)
{
	this->position = position;
	this->sourceColor = color;
	this->ambientColor = color * 0.1f;
	this->diffuseColor = color;
	this->specularColor = color;
	parentObject = NONE;
	lightType = POINT;
	lightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
	attenuation = glm::vec3(1.0f, 0.045f, 0.0075f);
	outerCutOff = 20.0f;
	innerCutOff = 10.0f;
}

LightSource::LightSource()
{
	this->position = glm::vec3(0.0f, 0.0f, 0.0f);
	this->sourceColor = glm::vec3(1.0f, 1.0f, 1.0f);
	this->ambientColor = sourceColor * 0.1f;
	this->diffuseColor = sourceColor;
	this->specularColor = sourceColor;
	parentObject = NONE;
	lightType = POINT;
	lightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
	attenuation = glm::vec3(1.0f, 0.045f, 0.0075f);
	outerCutOff = 20.0f;
	innerCutOff = 10.0f;
}
LightSource::~LightSource() 
{

	this->position = glm::vec3();
	this->sourceColor = glm::vec3();
	this->ambientColor = glm::vec3();
	this->diffuseColor = glm::vec3();
	this->specularColor = glm::vec3();
	parentObject = ParentObject::NONE;
	lightType = LightType::NA;
	lightDirection = glm::vec3();
	attenuation = glm::vec3();
	outerCutOff = -1.0f;
	innerCutOff = -1.0f;
}

/*============================================================================================================================================================================================
  * External Functionality
  *============================================================================================================================================================================================*/


void LightSource::setAttenuation(AttenuationPreset preset)
{
	switch (preset)
	{
	case DIST_7:
		attenuation = glm::vec3(1.0f, 0.7f, 1.8f);
		break;
	case DIST_13:
		attenuation = glm::vec3(1.0f, 0.35f, 0.44f);
		break;
	case DIST_20:
		attenuation = glm::vec3(1.0f, 0.22f, 0.20f);
		break;
	case DIST_32:
		attenuation = glm::vec3(1.0f, 0.14f, 0.07f);
		break;
	case DIST_50:
		attenuation = glm::vec3(1.0f, 0.09f, 0.032f);
		break;
	case DIST_65:
		attenuation = glm::vec3(1.0f, 0.07f, 0.017f);
		break;
	case DIST_100:
		attenuation = glm::vec3(1.0f, 0.045f, 0.0075f);
		break;
	case DIST_160:
		attenuation = glm::vec3(1.0f, 0.027f, 0.0028f);
		break;
	case DIST_200:
		attenuation = glm::vec3(1.0f, 0.022f, 0.0019f);
		break;
	case DIST_325:
		attenuation = glm::vec3(1.0f, 0.014f, 0.0007f);
		break;
	case DIST_600:
		attenuation = glm::vec3(1.0f, 0.007f, 0.0002f);
		break;
	case DIST_3250:
		attenuation = glm::vec3(1.0f, 0.0014f, 0.000007f);
		break;
	default:
		std::cerr << "ERROR::LIGHTSOURCE::SET_ATTENUATION::INCORRECT_ENUM" << std::endl;
		throw std::runtime_error("Check Logs");
		break;
	}
}