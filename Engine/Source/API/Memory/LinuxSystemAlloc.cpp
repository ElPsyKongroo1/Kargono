#include "kgpch.h"

#include "Kargono/Memory/SystemAlloc.h"

#if defined(KG_PLATFORM_LINUX)
namespace Kargono::Memory
{
	uint8_t* System::PageAlloc(size_t dataSize, size_t alignment)
	{
		KG_ERROR("Linux page alloc is unimplemented");
		return nullptr;
	}
}
#endif
