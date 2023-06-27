#pragma once
#include "../../../Library/Application/ResourceManager/ResourceManager.h"
#include "../BreakoutLevel/BreakoutLevel.h"

class BreakoutResourceManager : public ResourceManager
{
public:
	BreakoutLevel* currentLevel = nullptr;
public:

	BreakoutResourceManager() : ResourceManager()
	{

	}
	~BreakoutResourceManager()
	{
	}
public:
	void initializeResources() override;
private:
	void initializeInput() override;

};