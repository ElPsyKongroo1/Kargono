#pragma once

#include "Kargono/Core/Base.h"

#include <cassert>
#include <stdint.h>

#define NUM_OBJECTS 10

struct SampleObject
{
	float x;
	float y;
	float z;
};

struct PoolObject
{
	bool allocated;
	SampleObject object;
};

inline PoolObject object_pool[NUM_OBJECTS] = {0};

inline SampleObject* BorrowSampleObject()
{
	for (int i = 0; i < NUM_OBJECTS; i++)
	{
		if (!object_pool[i].allocated)
		{
			object_pool[i].allocated = true;
			return &(object_pool[i].object);
		}
	}

	return nullptr;
}

inline void ReturnSampleObject(SampleObject* obj)
{
	unsigned int i = ((uintptr_t)obj - (uintptr_t)object_pool) / sizeof(PoolObject);

	assert(&(object_pool[i].object) == obj);

	assert(object_pool[i].allocated);
	object_pool[i].allocated = false;
}

//inline void ReturnSampleObject(SampleObject* obj)
//{
//	for (int i = 0; i < NUM_OBJECTS; i++)
//	{
//		if (&(object_pool[i].object) == obj)
//		{
//			assert(object_pool[i].allocated);
//			object_pool[i].allocated = false;
//			return;
//		}
//	}
//
//	// Returned an object that does not exist in this memory pool
//	assert(false);
//}

inline void TestObjectPool()
{
	for (int i = 0; i < 15; i++)
	{
		SampleObject* obj = BorrowSampleObject();
		
		KG_INFO("Got vector @ address {:#x}", (uintptr_t)obj);

		ReturnSampleObject(obj);
	}
}
