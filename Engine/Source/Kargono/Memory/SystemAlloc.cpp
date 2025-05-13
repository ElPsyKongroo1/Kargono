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

	Kargono::Memory::System::MirrorAllocResult Win32MirrorAlloc(size_t size, size_t mirrors)
	{
		KG_ASSERT(Kargono::Utility::IsAlignedTo(size, Kargono::Memory::MEM_ALIGN_4_KIB));

		/* TODO: double-check error handling w.r.t. UnmapViewOfFile(), CloseHandle(), and VirtualFree()
		 * ---
		 *  see: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createfilemappinga
		 *  see: https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc2
		 *  see: https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-mapviewoffile3
		 *  see: https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-unmapviewoffile
		 *  see: https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
		 */

		Kargono::Memory::System::MirrorAllocResult result{};

		result.len = size;
		result.cap = size * mirrors;

		result.memfd = CreateFileMapping(INVALID_HANDLE_VALUE, 0,
						 PAGE_READWRITE,
						 (DWORD) (result.len >> 32),
						 (DWORD) (result.len & 0xffffffff),
						 0);

		if (result.memfd == INVALID_HANDLE_VALUE)
			goto error;

		result.ptr = VirtualAlloc2(nullptr, nullptr, result.cap,
					   MEM_RESERVE | MEM_RESERVE_PLACEHOLDER,
					   PAGE_NOACCESS, nullptr, 0);

		if (result.ptr == nullptr) {
			CloseHandle(result.memfd);
			goto error;
		}

		for (size_t i = 0; i < mirrors; i++) {
			uint8_t *base = result.ptr + (result.len * i);

			VirtualFree(base, result.len, MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER);

			PVOID res = MapViewOfFile3(result.memfd, 0,
						   base, 0, result.len,
						   MEM_REPLACE_PLACEHOLDER,
						   PAGE_READWRITE,
						   nullptr, 0);
			if (res == nullptr) {
				VirtualFree(result.ptr, result.cap, MEM_RELEASE);
				CloseHandle(result.memfd);
				goto error;
			}
		}

		return result;

error:
		result.memfd = INVALID_HANDLE_VALUE;
		result.ptr = nullptr;
		result.len = result.cap = 0;

		return result;
	}
}

#elif defined(__linux__)
# define _GNU_SOURCE 1
# include <sys/mman.h>
# include <unistd.h>
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

	Kargono::Memory::System::MirrorAllocResult LinuxMirrorAlloc(size_t size, size_t mirrors)
	{
		KG_ASSERT(Kargono::Utility::IsAlignedTo(size, Kargono::Memory::MEM_ALIGN_4_KIB));

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

		Kargono::Memory::System::MirrorAllocResult result{};

		result.len = size;
		result.cap = size * mirrors;

		result.memfd = memfd_create("linux mirror alloc", MFD_CLOEXEC);
		if (result.memfd < 0)
			goto error;

		ftruncate(result.memfd, result.len);

		result.ptr = (uint8_t*) mmap(nullptr, result.cap, PROT_NONE, buffer_flags, -1, 0);
		if (result.ptr == MAP_FAILED) {
			close(result.memfd);
			goto error;
		}

		for (size_t i = 0; i < mirrors; i++) {
			uint8_t* base = result.ptr + (result.len * i);
			void* res = mmap(base, result.len, prot, mirror_flags, result.memfd, 0);
			if (res == MAP_FAILED) {
				unmap(result.ptr, result.cap);
				close(result.memfd);
				goto error;
			}
		}

		return result;

error:
		result.memfd = -1;
		result.ptr = nullptr;
		result.len = result.cap = 0;

		return result;
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

	System::MirrorAllocResult System::MirrorAlloc(size_t dataSize, size_t mirrors)
	{
#if defined(_WIN32)
		return Win32MirrorAlloc(dataSize, mirrors);
#elif defined(__linux__)
		return LinuxMirrorAlloc(dataSize, mirrors);
#else
		System::MirrorAllocResult result{};
		return result;
#endif
	}
}
