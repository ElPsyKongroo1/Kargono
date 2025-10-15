#pragma once

#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/Module/AssetTag.h"

namespace Kargono::Assets
{
	using UserCallback = std::function<void(LoadState, void*)>;

	struct AssetGenericData
	{
	public:
		AssetHandle m_Handle{ k_EmptyHandle };
		LoadState m_LoadState{ LoadState::Unloaded };
		void* m_DataPtr{ nullptr };
	};

	template<typename t_AssetType>
	struct AssetReference
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		AssetReference() = default;
		AssetReference(AssetHandle handle, LoadState state, t_AssetType* asset)
			: m_Handle(handle), m_LoadState(state), m_Asset(asset)
		{
			KG_ASSERT((state == LoadState::Loaded && asset) ||
				(state != LoadState::Loaded && !asset));
		}
		AssetReference(AssetGenericData& data)
			: m_Handle(data.m_Handle), m_LoadState(data.m_LoadState), m_Asset((t_AssetType*)data.m_DataPtr)
		{
			KG_ASSERT((data.m_LoadState == LoadState::Loaded && data.m_DataPtr) ||
				(data.m_LoadState != LoadState::Loaded && !data.m_DataPtr));
		}
		~AssetReference() {}
	public:
		//==============================
		// Notification
		//==============================
		void OnUpdateAsset(LoadState state, void* asset)
		{
			m_Asset = (t_AssetType*)asset;
			m_LoadState = state;

			KG_ASSERT(IsValid());
		}
	public:
		//==============================
		// Copy Constructor(s)
		//==============================
		AssetReference(const AssetReference<t_AssetType>& other)
			: m_Handle(other.m_Handle), m_LoadState(other.m_LoadState), m_Asset(other.m_Asset)
		{
			KG_ASSERT((m_LoadState == LoadState::Loaded && m_Asset) ||
				(m_LoadState != LoadState::Loaded && !m_Asset));
		}
	public:
		//==============================
		// Reset
		//==============================
		void Reset()
		{
			m_Handle = k_EmptyHandle;
			m_LoadState = LoadState::Unloaded;
			m_Asset = nullptr;
		}
	public:
		//==============================
		// Getters/Setters
		//==============================
		t_AssetType& GetAsset()
		{
			KG_ASSERT(m_Asset);
			KG_ASSERT(m_Handle != k_EmptyHandle);
			KG_ASSERT(m_LoadState == LoadState::Loaded);

			// Note that this function assumes you have verified the asset is loaded
			return *m_Asset;
		}

		OptionalRef<t_AssetType> GetAssetOptional()
		{
			if (m_Asset && m_LoadState == LoadState::Loaded)
			{
				return *m_Asset;
			}
			return {};
		}

		AssetHandle GetHandle() const
		{
			return m_Handle;
		}

		LoadState GetLoadState() const
		{
			return m_LoadState;
		}

		bool IsValid() const
		{
			return m_Handle != k_EmptyHandle && (
				(m_LoadState == LoadState::Loaded && m_Asset) ||
				(m_LoadState != LoadState::Loaded && !m_Asset));
		}

		bool IsEmpty() const
		{
			return m_Handle == k_EmptyHandle && m_LoadState == LoadState::Unloaded && m_Asset == nullptr;
		}

	public:
		//==============================
		// Operators
		//==============================
		AssetReference<t_AssetType>& operator=(const AssetReference<t_AssetType>& other)
		{
			m_Handle = other.m_Handle;
			m_LoadState = other.m_LoadState;
			m_Asset = other.m_Asset;

			return *this;
		}

	private:
		//==============================
		// Internal Fields
		//==============================
		// Asset data
		AssetHandle m_Handle{ Assets::k_EmptyHandle };
		LoadState m_LoadState{ LoadState::Unloaded };
		t_AssetType* m_Asset{ nullptr };
	};

	template<AssetConcept t_AssetType>
	struct AssetUpdateListener
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		AssetUpdateListener() = default;
		AssetUpdateListener(AssetReference<t_AssetType>* assetReference)
		{
			KG_ASSERT(assetReference);
			i_AssetReference = assetReference;
		}
		~AssetUpdateListener()
		{
			// Please unregister from notifier before destruction
			KG_ASSERT(!IsNotifierRegistered());
		}
	public:
		//==============================
		// Notification
		//==============================
		void OnNotify(LoadState state, void* asset)
		{
			KG_ASSERT(m_ListenerIndex != k_InvalidListenerIndex);
			KG_ASSERT(i_AssetReference);

			// Call pre update callback
			if (m_PreUpdateCallback)
			{
				m_PreUpdateCallback(state, asset);
			}

			// Update internal asset reference
			i_AssetReference->OnUpdateAsset(state, asset);

			// Call post update callback
			if (m_PostUpdateCallback)
			{
				m_PostUpdateCallback(state, asset);
			}
		}

		void RegisterNotifier(ListenerIndex listenerIndex, UserCallback preCallback, UserCallback postCallback)
		{
			// Must have a valid listener index
			KG_ASSERT(listenerIndex != k_InvalidListenerIndex);
			// Cannot register if already registered (leads to dangling pointers)
			KG_ASSERT(!IsNotifierRegistered());
			// Must have a asset reference to register with
			KG_ASSERT(i_AssetReference);
			KG_ASSERT(i_AssetReference->IsValid());

			m_ListenerIndex = listenerIndex;
			m_PreUpdateCallback = preCallback;
			m_PostUpdateCallback = postCallback;
		}

		void UnregisterNotifier()
		{
			KG_ASSERT(IsNotifierRegistered());
			m_ListenerIndex = k_InvalidListenerIndex;
			m_PreUpdateCallback = nullptr;
			m_PostUpdateCallback = nullptr;
		}
	public:
		//==============================
		// Getters/Setters
		//==============================
		ListenerIndex GetListenerIndex() const
		{
			return m_ListenerIndex;
		}

		bool IsNotifierRegistered() const
		{
			return m_ListenerIndex != k_InvalidListenerIndex;
		}

		void SetAssetReference(AssetReference<t_AssetType>* assetReference)
		{
			KG_ASSERT(assetReference);
			// Cannot change asset reference if notifier already registered (leads to dangling pointers)
			// Please unregister first
			KG_ASSERT(!IsNotifierRegistered());

			i_AssetReference = assetReference;
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		// Notifier data
		ListenerIndex m_ListenerIndex{ k_InvalidListenerIndex };
		UserCallback m_PreUpdateCallback{ nullptr };
		UserCallback m_PostUpdateCallback{ nullptr };
	private:
		//==============================
		// Injected Depenencies
		//==============================
		AssetReference<t_AssetType>* i_AssetReference{ nullptr };
	};
}