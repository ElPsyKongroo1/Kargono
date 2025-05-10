#include "kgpch.h"

#include "Kargono/Memory/MemoryCommon.h"
#include "Kargono/Memory/SystemAlloc.h"

#if defined(_WIN32)
# include <windows.h>
# include <memoryapi.h>
namespace {
	uint8_t* Win32PageAlloc(size_t size, size_t alignment)
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

		return (uint8_t*) ptr;
	}
}

#elif defined(__linux__)
# include <sys/mman.h>
namespace {
	uint8_t* LinuxPageAlloc(size_t size, size_t alignment)
	{
		KG_ASSERT(Kargono::Utility::IsPowerOfTwo(alignment));
		KG_ASSERT(Kargono::Memory::MEM_ALIGN_4_KIB <= alignment);

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

		size_t aligned_size = Kargono::Utility::AlignForward(size, alignment);
		void* unaligned_ptr = mmap(nullptr, aligned_size, PROT_NONE, flags, -1, 0);
		if (unaligned_ptr == MAP_FAILED)
			return nullptr;

		uintptr_t aligned_base = Kargono::Utility::AlignForward((uintptr_t) unaligned_ptr, alignment);
		void* aligned_ptr = mmap((void*) aligned_base, size, prot, flags | MAP_FIXED, -1, 0);
		if (aligned_ptr == MAP_FAILED)
			return nullptr;

		if (Kargono::Utility::IsAlignedTo(aligned_base, Kargono::Memory::MEM_ALIGN_2_MIB))
			madvise(aligned_ptr, size, MADV_HUGEPAGE);

		munmap(unaligned_ptr, aligned_base - (uintptr_t) unaligned_ptr);

		return (uint8_t*) aligned_ptr;
	}
}

#else
# error "Unknown platform, must be one of: windows linux"
#endif

namespace Kargono::Memory
{
	uint8_t* System::GenAlloc(size_t dataSize, size_t alignment)
	{
		KG_ASSERT(Utility::IsPowerOfTwo(alignment));

		size_t upper_bound = dataSize + (alignment - 1);

		uintptr_t ptr = (uintptr_t) malloc(upper_bound);
		uintptr_t res = Utility::AlignForward(ptr, alignment);

		return (uint8_t*) res;
	}

	uint8_t* System::PageAlloc(size_t dataSize, size_t alignment)
	{
		if (alignment < MEM_ALIGN_4_KIB)
			alignment = MEM_ALIGN_4_KIB;

#if defined(_WIN32)
		return Win32PageAlloc(dataSize, alignment);
#elif defined(__linux__)
		return LinuxPageAlloc(dataSize, alignment);
#else
		return nullptr;
#endif
	}
}
