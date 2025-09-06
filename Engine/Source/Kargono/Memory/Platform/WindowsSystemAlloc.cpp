#include "kgpch.h"

#if defined(KG_PLATFORM_WINDOWS)

#include "Kargono/Memory/SystemAlloc.h"
#include "Kargono/Memory/MemoryCommon.h"

#include <windows.h>
#include <memoryapi.h>

// TODO: AHHHH THIS NEEDS TO BE IN THE BUILD SYSTEM YAH?
#pragma comment ( lib, "onecore.lib" )

namespace Kargono::Memory
{
	uint8_t* System::PageAllocHelper(size_t size, size_t alignment)
	{
		KG_ASSERT(Kargono::Utility::IsPowerOfTwo(alignment));

		/* TODO: implement hugepage aware page alloc
			* ---
			*  see: https://learn.microsoft.com/en-us/windows/win32/memory/large-page-support
			*/

		uintptr_t aligned_size = Kargono::Utility::AlignForward(size, alignment);

		DWORD prot = PAGE_READWRITE;
		DWORD flags = MEM_RESERVE | MEM_COMMIT;
		LPVOID ptr = VirtualAlloc(nullptr, aligned_size, flags, prot);

		return (uint8_t*)ptr;
	}

	MirrorAllocResult System::MirrorAlloc(size_t size, size_t mirrors)
	{
		KG_ASSERT(Utility::IsAlignedTo(size, k_MemAlign4KIB));

		/* TODO: double-check error handling w.r.t. UnmapViewOfFile(), CloseHandle(), and VirtualFree()
			* ---
			*  see: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createfilemappinga
			*  see: https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc2
			*  see: https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-mapviewoffile3
			*  see: https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-unmapviewoffile
			*  see: https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
			*/

		MirrorAllocResult result{};

		result.m_Length = size;
		result.m_Capacity = size * mirrors;

		result.m_MemoryFileDesc = CreateFileMapping(INVALID_HANDLE_VALUE, 0,
			PAGE_READWRITE,
			(DWORD)(result.m_Length >> 32),
			(DWORD)(result.m_Length & 0xffffffff),
			0);

		if (result.m_MemoryFileDesc == INVALID_HANDLE_VALUE)
			goto error;

		result.m_Data = (uint8_t*)VirtualAlloc2(nullptr, nullptr, result.m_Capacity,
			MEM_RESERVE | MEM_RESERVE_PLACEHOLDER,
			PAGE_NOACCESS, nullptr, 0);

		if (result.m_Data == nullptr)
		{
			CloseHandle(result.m_MemoryFileDesc);
			goto error;
		}

		for (size_t i = 0; i < mirrors; i++)
		{
			uint8_t* base = result.m_Data + (result.m_Length * i);

			VirtualFree(base, result.m_Length, MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER);

			PVOID res = MapViewOfFile3(result.m_MemoryFileDesc, 0,
				base, 0, result.m_Length,
				MEM_REPLACE_PLACEHOLDER,
				PAGE_READWRITE,
				nullptr, 0);
			if (res == nullptr)
			{
				VirtualFree(result.m_Data, result.m_Capacity, MEM_RELEASE);
				CloseHandle(result.m_MemoryFileDesc);
				goto error;
			}
		}

		return result;

	error:
		result.m_MemoryFileDesc = INVALID_HANDLE_VALUE;
		result.m_Data = nullptr;
		result.m_Length = result.m_Capacity = 0;

		return result;
	}
}

#endif

