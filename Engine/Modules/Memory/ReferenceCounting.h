#pragma once

#include "Modules/Memory/IAllocator.h"
#include "Kargono/Core/Base.h"

#include <functional>

namespace Kargono::Memory
{
	template<typename t_Type>
	struct StrongReference;

	template<typename t_Type>
	class ControlBlock
	{
	public:
		//==============================
		// Constructor(s)/Destructor(s)
		//==============================
		ControlBlock(Memory::IAllocator* parentAllocator)
			: i_ParentAlloc(parentAllocator)
		{
			KG_ASSERT(static_cast<bool>(parentAllocator));
		}
		~ControlBlock()
		{
			KG_ASSERT(AreAllReferencesClear());
		}

	public:
		template<typename... t_Args>
		Expected<StrongReference<t_Type>> StoreReference(t_Args&&... args)
		{
			if (!IsStorageEmpty())
			{
				return {};
			}

			m_DataPtr = i_ParentAlloc->Alloc<t_Type>(args);
			KG_ASSERT(static_cast<bool>(m_DataPtr));

			return StrongReference<t_Type>(*this);
		}
	public:
		//==============================
		// Probe Internal State
		//==============================
		bool IsRefAvailable() const
		{
			return m_StrongRefCount > 0 && static_cast<bool>(m_DataPtr);
		}

		bool AreAllReferencesClear() const
		{
			return m_StrongRefCount == 0 && m_WeakRefCount == 0;
		}

		bool IsStorageEmpty() const
		{
			return AreAllReferencesClear() && !IsRefAvailable();
		}

	public:
		//==============================
		// Get Data
		//==============================
		t_Type& Get()
		{
			KG_ASSERT(IsRefAvailable());

			return *m_DataPtr;
		}

		const t_Type& Get() const
		{
			retur
		}
	public:
		//==============================
		// Manage Reference Counts
		//==============================
		void AddStrongReference()
		{
			KG_ASSERT(IsAssetValid());

			m_StrongRefCount++;
		}
		void RemoveStrongReference()
		{
			KG_ASSERT(m_StrongRefCount > 0);
			m_StrongRefCount--;

			if (m_StrongRefCount == 0)
			{
				ResetAsset();

				if (m_WeakRefCount == 0)
				{
					i_ParentAlloc->Dealloc<ControlBlock<t_Type>>(this);
				}
			}
		}
		void AddWeakReference()
		{
			m_WeakRefCount++;
		}
		void RemoveWeakReference()
		{
			KG_ASSERT(m_WeakRefCount > 0)
				m_WeakRefCount--;

			if (AreAllReferencesClear())
			{
				i_ParentAlloc->Dealloc<ControlBlock<t_Type>>(this);
			}
		}
	private:
		void ResetAsset()
		{
			KG_ASSERT(m_StrongRefCount == 0);
			KG_ASSERT(static_cast<bool>(i_ParentAlloc));
			KG_ASSERT(static_cast<bool>(m_DataPtr));

			// Deallocate the asset ptr
			bool success = i_ParentAlloc->Dealloc<t_Type>(m_DataPtr);
			KG_ASSERT(success);
			m_DataPtr = nullptr;
		}
	public:
		//==============================
		// Internal Fields
		//==============================
		// Control block 
		size_t m_StrongRefCount{ 0 };
		size_t m_WeakRefCount{ 0 };
		// Data
		t_Type* m_DataPtr{ nullptr };
		//==============================
		// Injected Dependencies
		//==============================
		Memory::IAllocator* i_ParentAlloc{ nullptr };
	};

	template<typename t_Type>
	struct StrongReference
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		explicit StrongReference(ControlBlock<t_Type>* controlBlock)
			: i_ControlBlock(controlBlock)
		{
			KG_ASSERT(static_cast<bool>(controlBlock));
			i_ControlBlock->AddStrongReference();
		}
		StrongReference(const StrongReference<t_Type>& otherRef)
			: i_ControlBlock(otherRef.i_ControlBlock)
		{
			KG_ASSERT(static_cast<bool>(otherRef.i_ControlBlock));
			i_ControlBlock->AddStrongReference();
		}

		explicit StrongReference(StrongReference<t_Type>&& otherRef)
		{
			// Nullify the other asset ref
			otherRef.i_ControlBlock = nullptr;

			// No need to update strong reference count!
		}
		StrongReference<t_Type>& operator=(const StrongReference<t_Type>& otherRef)
		{
			if (this != &otherRef)
			{
				// Release current strong reference
				Release();

				// Add controlBlock/reference from otherRef
				KG_ASSERT(static_cast<bool>(otherRef.i_ControlBlock));
				i_ControlBlock = otherRef.i_ControlBlock;
				i_ControlBlock->AddStrongReference();
			}

			return *this;
		}
		StrongReference<t_Type>& operator=(StrongReference<t_Type>&& otherRef)
		{
			if (this != &otherRef)
			{
				// Release current strong reference
				Release();

				// Add controlBlock/reference from otherRef
				KG_ASSERT(static_cast<bool>(otherRef.i_ControlBlock));
				i_ControlBlock = otherRef.i_ControlBlock;

				// Do not change strong reference count!! Just nullify otherRef
				otherRef.i_ControlBlock = nullptr;
			}

			return *this;
		}
		~StrongReference()
		{
			Release();
		}
	public:
		//==============================
		// Get Data
		//==============================
		t_Type& Get()
		{
			KG_ASSERT(static_cast<bool>(i_ControlBlock));
			return i_ControlBlock->Get();
		}
		const t_Type& Get() const
		{
			KG_ASSERT(static_cast<bool>(i_ControlBlock));
			return i_ControlBlock->Get();
		}
	public:
		//==============================
		// Operator Overloads
		//==============================
		t_Type* operator->()
		{
			return &Get();
		}
	private:
		//==============================
		// Manage State
		//==============================
		void Release()
		{
			if (i_ControlBlock)
			{
				i_ControlBlock->RemoveStrongReference();
				i_ControlBlock = nullptr;
			}
		}
	private:
		//==============================
		// Injected Dependency
		//==============================
		ControlBlock<t_Type>* i_ControlBlock;
	};

	template<typename t_Type>
	struct WeakReference
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		WeakReference()
		{
			// Create empty reference
		}
		explicit WeakReference(ControlBlock<t_Type>* controlBlock)
			: i_ControlBlock(controlBlock)
		{
			KG_ASSERT(static_cast<bool>(controlBlock));
			i_ControlBlock->AddWeakReference();
		}
		WeakReference(const WeakReference<t_Type>& otherRef)
		{
			if (otherRef.IsRefAvailable())
			{
				// Move over controlBlock
				i_ControlBlock = otherRef.i_ControlBlock;

				// Increase count
				i_ControlBlock->AddWeakReference();
			}

			// Otherwise, just leave this instance null
		}
		explicit WeakReference(WeakReference<t_Type>&& otherRef)
		{
			if (otherRef.IsRefAvailable())
			{
				// Move over controlBlock
				i_ControlBlock = otherRef.i_ControlBlock;

				// Do not increase weak count! Taking ownership!!!
				otherRef.i_ControlBlock = nullptr;
			}

			// Otherwise, just leave this instance null
		}

		WeakReference<t_Type>& operator=(const WeakReference<t_Type>& otherRef)
		{
			if (this != &otherRef)
			{
				// Reset this instance
				Release();

				if (otherRef.IsRefAvailable())
				{
					// Move over controlBlock
					i_ControlBlock = otherRef.i_ControlBlock;

					// Add to reference count
					i_ControlBlock->AddWeakReference();
				}
			}

			return *this;
		}
		~WeakReference()
		{
			Release();
		}
	public:
		//==============================
		// Get Strong
		//==============================
		Expected<StrongReference<t_Type>> Lock()
		{
			if (IsRefAvailable())
			{
				return StrongReference<t_Type>(*i_ControlBlock);
			}

			return {};
		}
	public:
		//==============================
		// Query State
		//==============================
		bool IsRefAvailable() const
		{
			return static_cast<bool>(i_ControlBlock) && i_ControlBlock->IsRefAvailable();
		}
	public:
		//==============================
		// Manage State
		//==============================
		void Release()
		{
			if (i_ControlBlock)
			{
				i_ControlBlock->RemoveWeakReference();
				i_ControlBlock = nullptr;
			}
		}
	private:
		//==============================
		// Injected Dependency
		//==============================
		ControlBlock<t_Type>* i_ControlBlock{ nullptr };
	};

	// Ease-of-use aliases
	template<typename t_Type>
	using SRef = StrongReference<t_Type>;

	template <typename t_Type>
	using WRef = WeakReference<t_Type>;
}