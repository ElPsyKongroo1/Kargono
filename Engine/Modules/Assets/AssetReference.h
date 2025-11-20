#pragma once

#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/Module/AssetTag.h"
#include "Kargono/Core/Notifier.h"

namespace Kargono::Assets
{
	template<AssetConcept t_AssetType>
	using UserCallback = std::function<void(LoadState, t_AssetType*)>;

	template<AssetConcept t_AssetType>
	using AssetUpdateNotifier = MultiNotifier<LoadState, t_AssetType*>;

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
		AssetReference(GenericAssetReference& data)
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

		bool IsUsable() const
		{
			return IsValid() && !IsEmpty();
		}

	public:
		//==============================
		// Operator Overloads
		//==============================
		AssetReference<t_AssetType>& operator=(const AssetReference<t_AssetType>& other)
		{
			m_Handle = other.m_Handle;
			m_LoadState = other.m_LoadState;
			m_Asset = other.m_Asset;

			return *this;
		}

		operator bool() const noexcept 
		{
			return IsUsable();
		}

		t_AssetType* operator->() 
		{
			return &GetAsset();
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

	template<typename t_AssetType>
	using AssetRef = AssetReference<t_AssetType>;

	template<AssetConcept t_AssetType>
	struct TrackedAssetReference
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		TrackedAssetReference() = default;
		TrackedAssetReference(AssetReference<t_AssetType> assetReference)
		{
			SetAssetReference(assetReference);
		}
		~TrackedAssetReference()
		{
			// Please unregister from notifier before destruction
			KG_ASSERT(!IsNotifierRegistered());
		}
	public:
		//==============================
		// Notification
		//==============================
		// Function called by notifier when asset is updated
		void OnNotify(LoadState state, t_AssetType* asset)
		{
			KG_ASSERT(m_ListenerIndex != k_InvalidListenerIndex);
			KG_ASSERT(i_AssetReference);

			// Call pre update callback
			if (m_PreUpdateCallback)
			{
				m_PreUpdateCallback(state, asset);
			}

			// Update internal asset reference
			i_AssetReference.OnUpdateAsset(state, asset);

			// Call post update callback
			if (m_PostUpdateCallback)
			{
				m_PostUpdateCallback(state, asset);
			}
		}

		void RegisterNotifier(AssetUpdateNotifier<t_AssetType>& notifier, UserCallback<t_AssetType> preCallback, UserCallback<t_AssetType> postCallback)
		{
			// Cannot register if already registered (leads to dangling pointers)
			KG_ASSERT(!IsNotifierRegistered());

			// Must have a valid asset reference to register with
			KG_ASSERT(i_AssetReference);
			KG_ASSERT(i_AssetReference.IsValid());

			// Register with notifier
			ListenerIndex listenerIndex = notifier.AddObserver(KG_BIND_CLASS_FN(OnNotify));
			KG_ASSERT(listenerIndex != k_InvalidListenerIndex);

			// Store internal data
			i_Notifier = &notifier;
			m_ListenerIndex = listenerIndex;
			m_PreUpdateCallback = preCallback;
			m_PostUpdateCallback = postCallback;
		}

		void UnregisterNotifier()
		{
			// Must be registered to unregister
			KG_ASSERT(IsNotifierRegistered());

			// Unregister from notifier
			i_Notifier->RemoveObserver(m_ListenerIndex);

			// Reset internal data
			i_Notifier = nullptr;
			m_ListenerIndex = k_InvalidListenerIndex;
			m_PreUpdateCallback = nullptr;
			m_PostUpdateCallback = nullptr;
		}

		[[nodiscard]] ListenerIndex Reset()
		{
			// Cache listener index
			ListenerIndex oldIndex{ m_ListenerIndex };

			// Unregister notifier if necessary
			if (IsNotifierRegistered())
			{
				UnregisterNotifier();
			}

			// Reset asset reference
			i_AssetReference.Reset();

			return oldIndex;
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
			return i_Notifier && m_ListenerIndex != k_InvalidListenerIndex;
		}

		void SetAssetReference(AssetReference<t_AssetType> assetReference)
		{
			KG_ASSERT(assetReference);
			// Cannot change asset reference if notifier already registered (leads to dangling pointers)
			// Please unregister first
			KG_ASSERT(!IsNotifierRegistered());

			i_AssetReference = assetReference;
		}

		bool IsAssetUsable() const
		{
			return i_AssetReference && i_AssetReference.IsUsable() && IsNotifierRegistered();
		}

		t_AssetType& GetAsset()
		{
			// Ensure asset reference should be usable
			KG_ASSERT(IsAssetUsable());

			// Note that this function assumes you have verified the asset is loaded
			return i_AssetReference.GetAsset();
		}

		AssetHandle GetAssetHandle()
		{
			return i_AssetReference.GetHandle();
		}

		AssetRef<t_AssetType> GetAssetRef() const
		{
			return i_AssetReference;
		}

	public:
		//==============================
		// Operator Overloads
		//==============================
		operator bool() const noexcept 
		{
			return IsAssetUsable();
		}

		t_AssetType* operator->() 
		{
			return &GetAsset();
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		// Notifier data
		ListenerIndex m_ListenerIndex{ k_InvalidListenerIndex };
		UserCallback<t_AssetType> m_PreUpdateCallback{ nullptr };
		UserCallback<t_AssetType> m_PostUpdateCallback{ nullptr };
	private:
		//==============================
		// Injected Depenencies
		//==============================
		AssetReference<t_AssetType> i_AssetReference{};
		AssetUpdateNotifier<t_AssetType>* i_Notifier{ nullptr };
	};

	template<typename t_AssetType>
	using TAssetRef = TrackedAssetReference<t_AssetType>;
}