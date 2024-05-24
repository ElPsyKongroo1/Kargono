#pragma once

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

namespace Kargono::Utility
{
#pragma once
#define OpenAL_ErrorCheck(message)\
		{\
			ALenum error = alGetError();\
			if( error != AL_NO_ERROR)\
			{\
				KG_ERROR("OpenAL Error: {} with call for {}", error, #message);\
			}\
		}

#define alec(FUNCTION_CALL)\
		FUNCTION_CALL;\
		OpenAL_ErrorCheck(FUNCTION_CALL)
}
