#pragma once
#include "../../Includes.h"

/*============================================================================================================================================================================================
  * LightSource Class
  *============================================================================================================================================================================================*/


class LightSource 
{
public:
	enum LightType
	{
		DIRECTIONAL = 0, POINT = 1, SPOT = 2, NA = 3
	};
	enum ParentObject
	{
		CAMERA, OBJECT, NONE
	};
	enum AttenuationPreset
	{
		DIST_7, DIST_13, DIST_20, DIST_32, DIST_50,
		DIST_65, DIST_100, DIST_160, DIST_200, DIST_325,
		DIST_600, DIST_3250
	};

public:
	LightType lightType;
	ParentObject parentObject;
	glm::vec3 attenuation;
	glm::vec3 lightDirection;
	glm::vec3 position;
	glm::vec3 sourceColor;
	glm::vec3 ambientColor;
	glm::vec3 diffuseColor;
	glm::vec3 specularColor;
	float outerCutOff, innerCutOff;
public:
	LightSource(glm::vec3& position, glm::vec3& color);
	LightSource();
	~LightSource();
public:
	void setAttenuation(AttenuationPreset preset);

};