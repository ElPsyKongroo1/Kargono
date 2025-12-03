#pragma once

#include "Modules/Assets/AssetReference.h"
#include "Kargono/Core/Notifier.h"
#include "Modules/Assets/Metadata.h"

namespace Kargono::Assets
{
	// Callback types
	template<typename t_AssetType>
	using UpdateLoadStateCallback = std::function<void(LoadState, t_AssetType*)>;
	template<typename t_AssetType>
	using DeleteAssetCallback = std::function<void()>;
	template<typename t_AssetType>
	using UpdateMetadataCallback = std::function<void(Metadata<t_AssetType>& newMetadata)>;

	template<AssetConcept t_AssetType>
	struct NotificationHandlers
	{
		// Update load state
		ListenerIndex m_UpdateLoadStateIndex{ k_InvalidListenerIndex };
		UpdateLoadStateCallback<t_AssetType> m_PreUpdateLoadState{ nullptr };
		UpdateLoadStateCallback<t_AssetType> m_PostUpdateLoadState{ nullptr };
		// Delete asset
		ListenerIndex m_DeleteAssetIndex{ k_InvalidListenerIndex };
		DeleteAssetCallback<t_AssetType> m_PreDeleteAsset{ nullptr };
		DeleteAssetCallback<t_AssetType> m_PostDeleteAsset{ nullptr };
		// Update metadata
		ListenerIndex m_UpdateMetadataIndex{ k_InvalidListenerIndex };
		UpdateMetadataCallback<t_AssetType> m_UpdateMetadata{ nullptr };
	};

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
#ifdef KG_DEBUG
			// Catch inconsistent deregistrations
			KG_ASSERT(!IsNotifierRegistered());
#else
			// Attempt to handle gracefully if runtime is in use
			if (IsNotifierRegistered())
			{
				UnregisterNotifier();
			}
#endif
		}
	public:
		//==============================
		// Notification
		//==============================
		// Function called by notifier when asset is updated
		void NotifyUpdateLoadState(LoadState state, t_AssetType* asset)
		{
			KG_ASSERT(m_ListenerIndex != k_InvalidListenerIndex);
			KG_ASSERT(i_AssetReference);

			// Call pre update callback
			if (m_PreUpdateCallback)
			{
				m_PreUpdateCallback(state, asset);
			}

			// Update internal asset reference
			*i_AssetReference = AssetReference<t_AssetType>
				(
					GetAssetHandle(), 
					state, 
					asset, 
					i_AssetReference->GetContext()
				);

			// Call post update callback
			if (m_PostUpdateCallback)
			{
				m_PostUpdateCallback(state, asset);
			}
		}
		// Function called by notifier when asset is deleted
		void NotifyDeleteAsset()
		{
			KG_ASSERT(m_NotificationHandlers.m_DeleteAssetIndex != k_InvalidListenerIndex);
			KG_ASSERT(i_AssetReference);

			// Call pre update callback
			if (m_NotificationHandlers.m_PreDeleteAsset)
			{
				m_NotificationHandlers.m_PreDeleteAsset();
			}

			// Reset the reference
			i_AssetReference->Reset();

			// Call post update callback
			if (m_NotificationHandlers.m_PostDeleteAsset)
			{
				m_NotificationHandlers.m_PostDeleteAsset();
			}
		}

		void NotifyUpdateMetadata(Metadata<t_AssetType>& metadata)
		{
			KG_ASSERT(m_NotificationHandlers.m_UpdateMetadata != k_InvalidListenerIndex);
			KG_ASSERT(i_AssetReference);

			// Reset the reference
			i_AssetReference->Reset();

			// Call post update callback
			if (m_NotificationHandlers.m_UpdateMetadata)
			{
				m_NotificationHandlers.m_UpdateMetadata();
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

		ListenerIndex Reset()
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

		t_AssetType* GetAssetPtr()
		{
			// Ensure asset reference should be usable
			KG_ASSERT(IsAssetUsable());

			// Note that this function assumes you have verified the asset is loaded
			return &i_AssetReference.GetAsset();
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
		NotificationHandlers<t_AssetType> m_NotificationHandlers{};
	private:
		//==============================
		// Injected Depenencies
		//==============================
		AssetReference<t_AssetType> i_AssetReference{};
	};

	template<typename t_AssetType>
	using TAssetRef = TrackedAssetReference<t_AssetType>;
}