#pragma once

#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/Module/AssetTag.h"
#include "Modules/Memory/IAllocator.h"
#include "Modules/Memory/ReferenceCounting.h"

#include <functional>

namespace Kargono::Detail
{
	template <typename t_AssetType>
	struct AssetData
	{
		Assets::AssetHandle m_Handle{ Assets::k_EmptyHandle };
		Assets::LoadState m_LoadState{ Assets::LoadState::Unloaded };
		t_AssetType m_Asset;
	};
}

namespace Kargono::Assets
{
	template<typename t_AssetType>
	class StrongAssetReference
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
	public:
		t_AssetType& GetAsset()
		{
			KG_ASSERT(IsAssetAvailable());
			return m_StrongRef.Get().m_Asset;
		}
		LoadState GetLoadState() const
		{
			return m_StrongRef.Get().m_LoadState;
		}
		AssetHandle GetHandle() const
		{
			return m_StrongRef.Get().m_Handle;
		}
	public:
		//==============================
		// Operator Overloads
		//==============================
		Memory::StrongReference<Detail::AssetData<t_AssetType>>& operator Memory::StrongReference<Detail::AssetData<t_AssetType>>()
		{
			return m_StrongRef;
		}
	private:
		//==============================
		// Query State
		//==============================
		bool IsAssetAvailable() const
		{
			return GetLoadState() == LoadState::Loaded;
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		Memory::StrongReference<Detail::AssetData<t_AssetType>> m_StrongRef;
	};

	class WeakAssetReference
	{

	};

	/*
	template<typename t_AssetType>
	struct StrongAssetReference;

	template<typename t_AssetType>
	class AssetStorage
	{
	public:
		//==============================
		// Constructor(s)/Destructor(s)
		//==============================
		AssetStorage(Memory::IAllocator* parentAllocator)
			: i_ParentAlloc(parentAllocator)
		{
			KG_ASSERT(static_cast<bool>(parentAllocator));
		}
		~AssetStorage()
		{
			KG_ASSERT(AreAllReferencesClear());
		}

	public:
		template<typename... t_Args>
		Expected<StrongAssetReference<t_AssetType>> LoadAsset(Assets::AssetHandle handle, LoadState initialState, t_Args&&... args)
		{
			if (!IsStorageEmpty())
			{
				return {};
			}

			if (!handle.IsValid())
			{
				return {};
			}

			m_AssetPtr = i_ParentAlloc->Alloc<t_AssetType>(args);
			KG_ASSERT(static_cast<bool>(m_AssetPtr));
			m_LoadState = initialState;
			m_AssetHandle = handle;

			return StrongAssetReference<t_AssetType>(*this);
		}
	public:
		//==============================
		// Probe Internal State
		//==============================
		bool IsAssetAvailable() const
		{
			return m_StrongRefCount > 0 && IsAssetValid() && m_LoadState == LoadState::Loaded;
		}

		bool IsAssetValid() const
		{
			return static_cast<bool>(m_AssetPtr);
		}

		bool AreAllReferencesClear() const
		{
			return m_StrongRefCount == 0 && m_WeakRefCount == 0;
		}

		bool IsStorageEmpty() const
		{
			return AreAllReferencesClear() && !IsAssetValid() && m_LoadState == LoadState::Unloaded;
		}

	public:
		//==============================
		// Get Data
		//==============================
		t_AssetType& GetAsset()
		{
			KG_ASSERT(IsAssetAvailable());

			return *m_AssetPtr;
		}
		LoadState GetLoadState() const
		{
			return m_LoadState;
		}
		AssetHandle GetHandle() const
		{
			return m_AssetHandle;
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
					i_ParentAlloc->Dealloc<AssetStorage<t_AssetType>>(this);
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
				i_ParentAlloc->Dealloc<AssetStorage<t_AssetType>>(this);
			}
		}
	private:
		void ResetAsset()
		{
			KG_ASSERT(m_StrongRefCount == 0);
			KG_ASSERT(static_cast<bool>(i_ParentAlloc));
			KG_ASSERT(static_cast<bool>(m_AssetPtr));

			// Update load state
			m_LoadState = LoadState::Unloaded;

			// Deallocate the asset ptr
			bool success = i_ParentAlloc->Dealloc<t_AssetType>(m_AssetPtr);
			KG_ASSERT(success);
			m_AssetPtr = nullptr;
		}
	public:
		//==============================
		// Internal Fields
		//==============================
		// Control block 
		size_t m_StrongRefCount{ 0 };
		size_t m_WeakRefCount{ 0 };
		// Data
		LoadState m_LoadState{ LoadState::Unloaded };
		t_AssetType* m_AssetPtr{ nullptr };
		AssetHandle m_AssetHandle{ k_EmptyHandle };

		//==============================
		// Injected Dependencies
		//==============================
		Memory::IAllocator* i_ParentAlloc{ nullptr };
	};

	template<typename t_AssetType>
	struct StrongAssetReference
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		explicit StrongAssetReference(AssetStorage<t_AssetType>* storage)
			: i_Storage(storage)
		{
			KG_ASSERT(storage);
			i_Storage->AddStrongReference();
		}
		StrongAssetReference(const StrongAssetReference<t_AssetType>& otherRef)
			: i_Storage(otherRef.i_Storage)
		{
			KG_ASSERT(static_cast<bool>(otherRef.i_Storage));
			i_Storage->AddStrongReference();
		}

		explicit StrongAssetReference(StrongAssetReference<t_AssetType>&& otherRef)
		{
			// Nullify the other asset ref
			otherRef.i_Storage = nullptr;

			// No need to update strong reference count!
		}
		StrongAssetReference<t_AssetType>& operator=(const StrongAssetReference<t_AssetType>& otherRef)
		{
			if (this != &otherRef)
			{
				// Release current strong reference
				Release();

				// Add storage/reference from otherRef
				KG_ASSERT(otherRef.i_Storage);
				i_Storage = otherRef.i_Storage;
				i_Storage->AddStrongReference();
			}

			return *this;
		}
		StrongAssetReference<t_AssetType>& operator=(StrongAssetReference<t_AssetType>&& otherRef)
		{
			if (this != &otherRef)
			{
				// Release current strong reference
				Release();

				// Add storage/reference from otherRef
				KG_ASSERT(static_cast<bool>(otherRef.i_Storage));
				i_Storage = otherRef.i_Storage;

				// Do not change strong reference count!! Just nullify otherRef
				otherRef.i_Storage = nullptr;
			}

			return *this;
		}
		~StrongAssetReference()
		{
			Release();
		}
	public:
		//==============================
		// Get Data
		//==============================
		t_AssetType& GetAsset()
		{
			KG_ASSERT(i_Storage);
			return i_Storage->GetAsset();
		}
		LoadState GetLoadState() const
		{
			KG_ASSERT(i_Storage);
			return i_Storage->GetLoadState();
		}
		AssetHandle GetHandle() const
		{
			KG_ASSERT(i_Storage);
			return i_Storage->GetHandle();
		}

	public:
		//==============================
		// Operator Overloads
		//==============================
		t_AssetType* operator->()
		{
			return &i_Storage->GetAsset();
		}
	private:
		//==============================
		// Manage State
		//==============================
		void Release()
		{
			if (i_Storage)
			{
				i_Storage->RemoveStrongReference();
				i_Storage = nullptr;
			}
		}
	private:
		//==============================
		// Injected Dependency
		//==============================
		AssetStorage<t_AssetType>* i_Storage;
	};

	template<typename t_AssetType>
	struct WeakAssetReference
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		WeakAssetReference()
		{
			// Create empty reference
		}
		explicit WeakAssetReference(AssetStorage<t_AssetType>* storage)
			: i_Storage(storage)
		{
			KG_ASSERT(storage);
			i_Storage->AddWeakReference();
		}
		WeakAssetReference(const WeakAssetReference<t_AssetType>& otherRef)
		{
			if (otherRef.IsAssetValid())
			{
				// Move over storage
				i_Storage = otherRef.i_Storage;
				
				// Increase count
				i_Storage->AddWeakReference();
			}

			// Otherwise, just leave this instance null
		}
		explicit WeakAssetReference(WeakAssetReference<t_AssetType>&& otherRef)
		{
			if (otherRef.IsAssetValid())
			{
				// Move over storage
				i_Storage = otherRef.i_Storage;

				// Do not increase weak count! Taking ownership!!!
				otherRef.i_Storage = nullptr;
			}

			// Otherwise, just leave this instance null
		}

		WeakAssetReference<t_AssetType>& operator=(const WeakAssetReference<t_AssetType>& otherRef)
		{
			if (this != &otherRef)
			{
				// Reset this instance
				Release();

				if (otherRef.IsAssetValid())
				{
					// Move over storage
					i_Storage = otherRef.i_Storage;

					// Add to reference count
					i_Storage->AddWeakReference();
				}
			}

			return *this;
		}
		~WeakAssetReference()
		{
			Release();
		}
	public:
		//==============================
		// Get Strong
		//==============================
		Expected<StrongAssetReference<t_AssetType>> Lock()
		{
			if (IsAssetAvailable())
			{
				return StrongAssetReference<t_AssetType>(*i_Storage);
			}

			return {};
		}
	public:
		//==============================
		// Query State
		//==============================
		bool IsAssetAvailable() const
		{
			return static_cast<bool>(i_Storage) && i_Storage->IsAssetAvailable();
		}
		bool IsAssetValid() const
		{
			return static_cast<bool>(i_Storage) && i_Storage->IsAssetValid();
		}
	public:
		//==============================
		// Manage State
		//==============================
		void Release()
		{
			if (i_Storage)
			{
				i_Storage->RemoveWeakReference();
				i_Storage = nullptr;
			}
		}
	private:
		//==============================
		// Injected Dependency
		//==============================
		AssetStorage<t_AssetType>* i_Storage{ nullptr };
	};

	// Ease-of-use aliases
	template<typename t_AssetType>
	using SAssetRef = StrongAssetReference<t_AssetType>;

	template <typename t_AssetType>
	using WAssetRef = WeakAssetReference<t_AssetType>;
	*/
}