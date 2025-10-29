#include "kgpch.h"

#if defined(KG_PLATFORM_LINUX)
#include "Kargono/Memory/SystemAlloc.h"
#include "Kargono/Memory/MemoryCommon.h"

# define _GNU_SOURCE 1
# include <sys/mman.h>
# include <unistd.h>

namespace Kargono::Memory
{ 
	uint8_t* System::PageAllocHelper(size_t size, size_t alignment)
	{
		KG_ASSERT(Utility::IsPowerOfTwo(alignment));
		KG_ASSERT(k_MemAlign4KIB <= alignment);

		/* hugepage aware page alloc:
			*  1. map a memory region with PROT_NONE pages (overallocating
			*     by up to (alignment - 1) bytes)
			*
			*  2. map a new memory region with PROT_READ|PROT_WRITE pages
			*     from within the previous memory region, ensuring that we
			*     allocate on a (huge)page-aligned address. this removes
			*     the previous PROT_NONE mapping
			*
			*  3. advise that the new memory region is to be backed by
			*     huge pages if possible (i.e. if alignment is sufficient)
			*
			*  4. unmap any remaining PROT_NONE pages preceeding our new
			*     memory region
			*/

		int prot = PROT_READ | PROT_WRITE;
		int flags = MAP_PRIVATE | MAP_ANONYMOUS;

		size_t aligned_size = Utility::AlignForward(size, alignment);
		void* unaligned_ptr = mmap(nullptr, aligned_size, PROT_NONE, flags, -1, 0);
		if (unaligned_ptr == MAP_FAILED)
		{
			return nullptr;
		}

		uintptr_t aligned_base = Utility::AlignForward((uintptr_t)unaligned_ptr, alignment);
		void* aligned_ptr = mmap((void*)aligned_base, size, prot, flags | MAP_FIXED, -1, 0);
		if (aligned_ptr == MAP_FAILED)
		{
			return nullptr;
		}

		if (Utility::IsAlignedTo(aligned_base, k_MemAlign2MIB))
		{
			madvise(aligned_ptr, size, MADV_HUGEPAGE);
		}

		munmap(unaligned_ptr, aligned_base - (uintptr_t)unaligned_ptr);

		return (uint8_t*)aligned_ptr;
	}

	MirrorAllocResult System::MirrorAlloc(size_t size, size_t mirrors)
	{
		KG_ASSERT(Utility::IsAlignedTo(size, k_MemAlign4KIB));

		/* mirrored alloc:
			*  1. create a memory-backed file to represent the shared
			*     buffer to be mirrored, and truncate it to the correct
			*     size
			*
			*  2. map a memory region with PROT_NONE pages, large enough
			*     to contain every mirrored region
			*
			*  3. create each mirrored region, at offset (i * size) in the
			*     large memory region, and set it to map the same view of
			*     the shared buffer
			*/

		int prot = PROT_READ | PROT_WRITE;
		int buffer_flags = MAP_PRIVATE | MAP_ANONYMOUS;
		int mirror_falgs = MAP_SHARED | MAP_FIXED;

		MirrorAllocResult result{};

		result.m_Length = size;
		result.m_Capacity = size * mirrors;

		result.m_MemoryFileDesc = memfd_create("linux mirror alloc", MFD_CLOEXEC);
		if (result.m_MemoryFileDesc < 0)
			goto error;

		ftruncate(result.m_MemoryFileDesc, result.m_Length);

		result.m_Data = (uint8_t*)mmap(nullptr, result.m_Capacity, PROT_NONE, buffer_flags, -1, 0);
		if (result.m_Data == MAP_FAILED)
		{
			close(result.m_MemoryFileDesc);
			goto error;
		}

		for (size_t i = 0; i < mirrors; i++)
		{
			uint8_t* base = result.m_Data + (result.m_Length * i);
			void* res = mmap(base, result.m_Length, prot, mirror_flags, result.m_MemoryFileDesc, 0);
			if (res == MAP_FAILED)
			{
				unmap(result.m_Data, result.m_Capacity);
				close(result.m_MemoryFileDesc);
				goto error;
			}
		}

		return result;

	error:
		result.m_MemoryFileDesc = -1;
		result.m_Data = nullptr;
		result.m_Length = result.m_Capacity = 0;

		return result;
	}
}

#endif
