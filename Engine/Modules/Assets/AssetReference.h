#pragma once

#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/Module/AssetTag.h"

#include <functional>

namespace Kargono::Assets
{
	template<typename t_AssetType> requires AssetConcept<t_AssetType>
	class AssetManager;


	template<typename t_AssetType>
	struct AssetStorage
	{
		// Control block 
		size_t m_StrongRefCount{ 0 };
		size_t m_WeakRefCount{ 0 };
		// Data
		LoadState m_LoadState{ LoadState::Unloaded };
		t_AssetType* m_AssetPtr{ nullptr };
	};

	template<typename t_AssetType>
	struct WeakAssetReference
	{
	public:
		ExpectedRef GetAsset()
		{
			// Please use this function if you are unsure if the asset is loaded
			KG_ASSERT(IsValid());

			if (m_Asset && m_LoadState == LoadState::Loaded)
			{
				return *m_Asset;
			}
			return {};
		}
	private:
		AssetHandle m_AssetHandle{ k_EmptyHandle };
		AssetStorage* m_Storage{ nullptr };
	};

	template <typename t_AssetType>
	using WAssetRef = WeakAssetReference<t_AssetType>;

	template<typename t_AssetType>
	struct AssetReference
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		AssetReference() = default;
		AssetReference(AssetHandle handle, LoadState state, 
			t_AssetType* asset, AssetManager<t_AssetType>* context)
			: m_Handle(handle), m_LoadState(state), m_Asset(asset)
		{
			KG_ASSERT((state == LoadState::Loaded && asset) ||
				(state != LoadState::Loaded && !asset));
		}
		AssetReference(const AssetReference<t_AssetType>& other) = default;
		~AssetReference() = default;
	public:
		//==============================
		// Reset
		//==============================
		void Reset()
		{
			m_Handle = k_EmptyHandle;
			m_LoadState = LoadState::Unloaded;
			m_Asset = nullptr;
			i_Context = nullptr;
		}
	public:
		//==============================
		// Getters/Setters
		//==============================
		t_AssetType& GetAsset()
		{
			// Note that this function assumes you have verified the asset is loaded
			KG_ASSERT(IsUsable());
			return *m_Asset;
		}

		OptionalRef<t_AssetType> GetAssetOptional()
		{
			// Please use this function if you are unsure if the asset is loaded
			KG_ASSERT(IsValid());

			if (m_Asset && m_LoadState == LoadState::Loaded)
			{
				return *m_Asset;
			}
			return {};
		}

		AssetHandle GetHandle() const { return m_Handle;}
		LoadState GetLoadState() const { return m_LoadState;}
		AssetManager<t_AssetType>* GetContext() { i_Context; }

		bool IsValid() const
		{
			return m_Handle.IsValid() && static_cast<bool>(i_Context) && (
				(m_LoadState == LoadState::Loaded && m_Asset) ||
				(m_LoadState != LoadState::Loaded && !m_Asset));
		}

		bool IsEmpty() const
		{
			return !m_Handle.IsValid() && m_LoadState == LoadState::Unloaded && m_Asset == nullptr;
		}

		bool IsUsable() const
		{
			return IsValid() && !IsEmpty();
		}

		t_AssetType* GetAssetPtr()
		{
			// Note that this function assumes you have verified the asset is loaded
			KG_ASSERT(IsUsable());
			return m_Asset;
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
			i_Context = other.i_Context;

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

	private:
		//==============================
		// Injected Dependencies
		//==============================
		// Greater context that outlives reference
		AssetManager<t_AssetType>* i_Context{ nullptr };
	};

	template<typename t_AssetType>
	using AssetRef = AssetReference<t_AssetType>;
}