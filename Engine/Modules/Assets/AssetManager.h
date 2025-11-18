#pragma once
#include "Kargono/Core/Base.h"
#include "Modules/Core/Engine.h"
#include "Kargono/Projects/Project.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Modules/Events/AssetEvent.h"
#include "Modules/Assets/Module/AssetTag.h"
#include "Modules/Assets/AssetReference.h"
#include "Modules/Memory/IAllocator.h"
#include "Modules/Assets/Concepts/ManageAssetConcepts.h"
#include "Modules/Assets/Concepts/RegistryConcept.h"

#include "API/Serialization/yamlcppAPI.h"

#include <bitset>
#include <tuple>
#include <unordered_map>

namespace Kargono::Assets
{
	using AssetRegistry = std::unordered_map<AssetHandle, Metadata>;
	using AssetCache = std::unordered_map<AssetHandle, GenericAssetReference>;

	class AssetManager
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		AssetManager() = default;
		AssetManager(Memory::IAllocator* backingAllocator)
		{
			KG_ASSERT(backingAllocator);
		}
		~AssetManager() = default;
	public:
		Optional<Metadata> GetMetadata(AssetHandle handle)
		{
			if (!m_AssetRegistry.contains(handle))
			{
				return {};
			}
			return m_AssetRegistry.at(handle);
		}

		template<AssetConcept t_AssetType> requires HasAssetCacheFlag<t_AssetType>
		AssetReference<t_AssetType> GetCachedReference(AssetHandle handle)
		{
			if (!m_AssetCache.contains(handle))
			{
				return {};
			}

			return { m_AssetCache.at(handle) };
		}

		template<AssetConcept t_AssetType>
		AssetReference<t_AssetType> GetAssetByHandle(AssetHandle handle)
		{
			// Check cache first
			if constexpr (HasAssetCacheFlag<t_AssetType>)
			{
				if (m_AssetCache.contains(handle))
				{
					return m_AssetCache[handle];
				}
			}

			// Check registry second
			if (m_AssetRegistry.contains(handle))
			{
				Metadata& metadata = m_AssetRegistry[handle];
				AssetReference<t_AssetType> newAssetRef{ DeserializeAssetImpl<t_AssetType>(metadata)};
				KG_ASSERT(newAssetRef.IsUsable());
				
				// Cache asset if applicable
				if constexpr (HasAssetCacheFlag<t_AssetType>)
				{
					GenericAssetReference cachedReference
					{
						newAssetRef.GetHandle(),
						newAssetRef.GetLoadState(),
						(void*)& newAssetRef.GetAsset()
					};
					m_AssetCache.insert({ handle, cachedReference });
				}
				return newAssetRef;
			}

			// Exit if asset is not found
			return {};
		}

		template<AssetConcept t_AssetType> requires HasFileLocation<t_AssetType>
		AssetReference<t_AssetType> GetAssetByFileLocation(const std::filesystem::path& queryLocation)
		{
			// Find asset handle
			AssetHandle handle{ GetAssetHandleByFileLocation<t_AssetType>(queryLocation) };

			// Check for invalid case
			if (handle == k_EmptyHandle)
			{
				return {};
			}

			// Get indicated asset
			return GetAssetByHandle<t_AssetType>(handle);
		}

		template<AssetConcept t_AssetType> requires HasGetAssetFromSpec<t_AssetType>
		AssetReference<t_AssetType> GetAssetBySpec(const typename t_AssetType::Spec& spec)
		{
			// Check each asset for matching spec
			for (const auto& [handle, metadata] : m_AssetRegistry)
			{
				if (GetAssetFromSpecImpl(metadata, spec))
				{
					return GetAssetByHandle<t_AssetType>(handle);
				}
			}

			// If no matching asset found, return empty reference
			return {};
		}
		

		bool HasAssetByHandle(AssetHandle handle)
		{
			return m_AssetRegistry.contains(handle);
		}

		template<AssetConcept t_AssetType>
		bool HasAssetByName(std::string_view assetName)
		{
			// Check for a matching name
			for (const auto& [handle, metadata] : m_AssetRegistry)
			{
				if (metadata.m_Name.StringView() == assetName)
				{
					return true;
				}
			}

			// Exit if no match is found
			return false;
		}

		template<AssetConcept t_AssetType> requires HasFileLocation<t_AssetType>
		bool HasAssetByFile(const std::filesystem::path& queryLocation)
		{
			// Find asset handle
			AssetHandle handle{ GetAssetHandleByFileLocation<t_AssetType>(queryLocation) };

			// Return boolean response
			if (handle == k_EmptyHandle)
			{
				return false;
			}
			return true;
		}

		template<AssetConcept t_AssetType> requires HasAssetCacheFlag<t_AssetType>
		bool HasCachedReferenceByHandle(AssetHandle handle)
		{
			return m_AssetCache.contains(handle);
		}

		template<AssetConcept t_AssetType> requires HasUpdateFromAsset<t_AssetType>
		bool UpdateAsset(AssetReference<t_AssetType> otherRef)
		{
			KG_ASSERT(otherRef.IsUsable());
			AssetHandle handle{ otherRef.GetHandle() };

			// Ensure asset exists and is cached
			if (!HasAssetByHandle(handle))
			{
				KG_WARN("Attempt to update asset when handle does not exist in registry");
				return false;
			}
			if (!HasCachedReferenceByHandle<t_AssetType>(handle))
			{
				KG_WARN("Attempt to update an asset that does not exist in the asset cache");
				return false;
			}

			// Get the current asset reference
			AssetReference<t_AssetType> currentRef{ m_AssetCache.at(handle) };
			KG_ASSERT(currentRef.IsUsable());

			// Get metadata
			Metadata& metadata{ GetRawMetadata(handle) };
			KG_ASSERT(metadata.IsValid());

			// Provide asset specific validation
			Ref<void> providedData{ nullptr };
			if constexpr (HasValidateUpdateFromAsset<t_AssetType>)
			{
				providedData = ValidateUpdateFromAssetImpl<t_AssetType>(metadata, otherRef);
			}

			// Update the asset using a user-defined update function
			if constexpr (HasCustomUpdateFromAsset<t_AssetType>)
			{
				CustomUpdateFromAssetImpl<t_AssetType>(metadata, currentRef, otherRef);
			}
			// Update the asset using a default override
			else if (HasDefaultUpdateFromAsset<t_AssetType>)
			{
				m_AssetCache.at(handle) =
				{
					handle,
					otherRef.GetLoadState(),
					(void*)&otherRef.GetAsset()
				};
			}

			// Update save files on disk if relevant
			if constexpr (HasAssetSaving<t_AssetType>)
			{
				SerializeAssetImpl<t_AssetType>(metadata, currentRef);
			}

			// Re-generate asset hash
			Utility::SHA256Hash resultHash{ GenerateAssetHash<t_AssetType>(metadata) };
			KG_ASSERT(!resultHash.IsEmpty());
			metadata.m_Hash = resultHash;

			// Send update event
			SendManageAssetEvent<t_AssetType>(Events::ManageAssetAction::UpdateAsset,
				metadata, providedData);
		}

		template<AssetConcept t_AssetType> requires HasSpecification<t_AssetType>
		void UpdateAsset(AssetHandle handle, const t_AssetType::Spec& spec)
		{
			// Ensure asset exists and is cached
			if (!HasAssetByHandle(handle))
			{
				KG_WARN("Attempt to update asset when handle does not exist in registry");
				return false;
			}
			if (!HasCachedReferenceByHandle<t_AssetType>(handle))
			{
				KG_WARN("Attempt to update an asset that does not exist in the asset cache");
				return false;
			}

			// Get associated metadata
			Metadata& metadata{ GetRawMetadata(handle) };

			// Provide asset specific validation
			Ref<void> providedData{ nullptr };
			if constexpr (HasValidateUpdateFromSpec<t_AssetType>)
			{
				providedData = ValidateUpdateFromSpecImpl<t_AssetType>(metadata, spec);
			}

			// Get the current asset reference
			AssetReference<t_AssetType> currentRef{ m_AssetCache.at(handle) };
			KG_ASSERT(currentRef.IsUsable());

			// Update current asset w/ spec
			UpdateFromSpecImpl<t_AssetType>(metadata, currentRef, spec);

			// Save asset files on-disk if relevant
			if constexpr (HasAssetSaving<t_AssetType>)
			{
				SerializeAssetImpl<t_AssetType>(metadata, assetReference);
			}

			// Re-generate asset hash
			Utility::SHA256Hash resultHash{ GenerateAssetHash<t_AssetType>(metadata) };
			KG_ASSERT(!resultHash.IsEmpty());
			metadata.m_Hash = resultHash;

			// Send update event
			SendManageAssetEvent<t_AssetType>(Events::ManageAssetAction::UpdateAsset,
				metadata, providedData);
		}

		template<AssetConcept t_AssetType>
		bool DeleteAsset(AssetHandle handle)
		{
			// Ensure asset exists
			if (!HasAssetByHandle(handle))
			{
				KG_WARN("Attempt to delete asset not found in registry.");
				return false;
			}

			// Get current metadata 
			Metadata& metadata{ GetRawMetadata(handle) };
			
			// Provide asset specific validation
			if constexpr (HasValidateDelete<t_AssetType>)
			{
				ValidateDeleteImpl<t_AssetType>(metadata);
			}

			// Send pre-delete event
			SendManageAssetEvent<t_AssetType>(Events::ManageAssetAction::PreDelete,
				metadata, nullptr);

			// Remove asset
			DeleteAssetFiles<t_AssetType>(metadata);
			RemoveAssetDataFromRegistry<t_AssetType>(handle);

			// Save the modified registry to disk
			SerializeAssetRegistry<t_AssetType>();

			// Send post-delete event
			SendManageAssetEvent<t_AssetType>(Events::ManageAssetAction::PostDelete,
				metadata, nullptr);

			return true;
		}

		template<AssetConcept t_AssetType>
		void ClearAssetRegistry()
		{
			// Create cache of all asset id's for stability
			std::vector<AssetHandle> m_AllAssetIDs{};
			m_AllAssetIDs.reserve(m_AssetRegistry.size());
			for (auto& [handle, metadata] : m_AssetRegistry)
			{
				m_AllAssetIDs.push_back(handle);
			}

			// Remove each asset
			for (AssetHandle handle : m_AllAssetIDs)
			{
				RemoveAssetDataFromRegistry<t_AssetType>(handle);
			}
			
			// Verify completion
			if constexpr (HasAssetCacheFlag<t_AssetType>)
			{
				KG_ASSERT(m_AssetCache.size() == 0);
			}
			KG_ASSERT(m_AssetRegistry.size() == 0);
		}

		bool GetIsAssetHidden(AssetHandle handle)
		{
			KG_ASSERT(m_AssetRegistry.contains(handle));

			Metadata& metadata{ GetRawMetadata(handle) };
			KG_ASSERT(metadata.IsValid());

			return metadata.m_IsHidden;
		}

		void SetIsAssetHidden(AssetHandle handle, bool isHidden)
		{
			KG_ASSERT(HasAssetByHandle(handle));

			Metadata& metadata{ GetRawMetadata(handle) };
			KG_ASSERT(metadata.IsValid());

			metadata.m_IsHidden = isHidden;
		}


		template<AssetConcept t_AssetType> requires HasCreateFromName<t_AssetType>
		AssetHandle CreateAssetFromName(const AssetCreationData& creationData)
		{
			// Validate asset name
			if (!ValidateAssetName<t_AssetType>(creationData.m_AssetName))
			{
				KG_WARN("Invalid asset name provided to create asset from name function: {}", 
					creationData.m_AssetName);
				return k_EmptyHandle;
			}
			
			// Handle file location information
			std::filesystem::path metadataFileDirectory = creationData.m_CreationDirectory;
			if constexpr (HasFileLocation<t_AssetType>)
			{
				// Validate creation directory
				bool validDirectoryPath{ ValidateCreationDirectory(metadataFileDirectory) };
				if (!validDirectoryPath)
				{
					KG_WARN("Creation directory validation failed for path: {}", 
						metadataFileDirectory.c_str());
					return k_EmptyHandle;
				}
				metadataFileDirectory = NormalizeAssetDirectory(metadataFileDirectory);
			}
			else
			{
				KG_ASSERT(creationData.m_CreationDirectory.empty(), 
					"Attempt to provide a creation directory for an asset type that does not support it");
			}

			// Create metadata
			Metadata newMetadata { CreateAssetMetadata(GetAssetIdentifier<t_AssetType>, creationData.m_AssetName
				metadataFileDirectory, creationData.m_IsHidden) };

			// Validate metadata
			if (!newMetadata.IsValid())
			{
				KG_WARN("Failed to create valid metadata for new {} asset with name {}", 
					t_AssetType::GetAssetName(), creationData.m_AssetName);
				return k_EmptyHandle;
			}

			// Create asset on disk
			CreateFromNameImpl<t_AssetType>(newMetadata);

			// Generate asset hash
			Utility::SHA256Hash resultHash{ GenerateAssetHash<t_AssetType>(newMetadata) };
			newMetadata.m_Hash = resultHash;

			// Register new asset
			m_AssetRegistry.insert({ newMetadata.m_Handle, newMetadata });
			SerializeAssetRegistry<t_AssetType>();

			// Fill in-memory cache
			if constexpr (HasAssetCacheFlag<t_AssetType>)
			{
				LoadAssetIntoCache<t_AssetType>(newMetadata);
			}

			// Validate asset hash
			if (!ValidateAssetHash<t_AssetType>(resultHash))
			{
				KG_WARN("Invalid asset hash generated from new asset");

				// Rollback asset creation
				bool deleteSuccess = DeleteAsset<t_AssetType>(newMetadata.m_Handle);
				KG_ASSERT(deleteSuccess);

				return k_EmptyHandle;
			}

			// Send creation event
			SendManageAssetEvent<t_AssetType>(Events::ManageAssetAction::Create, 
				newMetadata, nullptr);
			
			return newMetadata.m_Handle;
		}

		template<AssetConcept t_AssetType> requires HasCreateFromFile<t_AssetType>
		AssetHandle CreateAssetFromFile(const std::filesystem::path& sourcePath, const AssetCreationData& creationData)
		{
			// Ensure valid asset name is provided
			if (!ValidateAssetName(creationData.m_AssetName))
			{
				KG_WARN("Invalid asset name provided to create asset from file function: {}", 
					creationData.m_AssetName);
				return k_EmptyHandle;
			}

			// Handle file location information
			std::filesystem::path metadataFileDirectory = creationData.m_CreationDirectory;
			if constexpr (HasFileLocation<t_AssetType>)
			{
				// Validate creation directory
				bool validDirectoryPath{ ValidateCreationDirectory(metadataFileDirectory) };
				if (!validDirectoryPath)
				{
					KG_WARN("Creation directory validation failed for path: {}",
						metadataFileDirectory.c_str());
					return k_EmptyHandle;
				}
				metadataFileDirectory = NormalizeAssetDirectory(metadataFileDirectory);
			}
			else
			{
				KG_ASSERT(creationData.m_CreationDirectory.empty(),
					"Attempt to provide a creation directory for an asset type that does not support it");
			}

			// Check if source path is valid
			if (!ValidateSourcePath<t_AssetType>(sourcePath))
			{
				KG_WARN("Source path validation failed for provided path: {}", 
					sourcePath.string());
				return k_EmptyHandle;
			}

			// Create metadata
			Metadata newMetadata{ CreateAssetMetadata(GetAssetIdentifier<t_AssetType>, creationData.m_AssetName
				metadataFileDirectory, creationData.m_IsHidden) };

			// Validate metadata
			if (!newMetadata.IsValid())
			{
				KG_WARN("Failed to create valid metadata for new {} asset with name {}",
					t_AssetType::GetAssetName(), creationData.m_AssetName);
				return k_EmptyHandle;
			}

			// Create asset files on disk
			CreateFromFileImpl<t_AssetType>(newMetadata, sourcePath);

			// Generate asset hash
			Utility::SHA256Hash resultHash{ GenerateAssetHash<t_AssetType>(newMetadata) };
			newMetadata.m_Hash = resultHash;

			// Add new asset into asset registry
			m_AssetRegistry.insert({ newHandle, newMetadata });
			SerializeAssetRegistry<t_AssetType>();

			// Fill in-memory cache
			if constexpr (HasAssetCacheFlag<t_AssetType>)
			{
				LoadAssetIntoCache<t_AssetType>(newMetadata);
			}

			// Validate asset hash
			if (!ValidateAssetHash<t_AssetType>(resultHash))
			{
				KG_WARN("Invalid asset hash generated from new asset");

				// Rollback asset creation
				bool deleteSuccess = DeleteAsset<t_AssetType>(newMetadata.m_Handle);
				KG_ASSERT(deleteSuccess);

				// Return empty handle
				return k_EmptyHandle;
			}

			// Send creation event
			SendManageAssetEvent<t_AssetType>(Events::ManageAssetAction::Create,
				newMetadata, nullptr);

			return newMetadata.m_Handle;
		}

		template<AssetConcept t_AssetType> requires HasCreateFromSpec<t_AssetType>
		AssetHandle CreateAssetFromSpec(const AssetCreationData& creationData, 
			const typename t_AssetType::Spec& spec)
		{
			// Handle validation
			if constexpr (HasValidateCreateFromSpec<t_AssetType>)
			{
				bool validateSuccess = t_AssetType::ValidateCreateFromSpecImpl(creationData, spec);
				if (!validateSuccess)
				{
					KG_WARN("Validation of asset specification failed");
					return k_EmptyHandle;
				}
			}

			// Ensure valid asset name is provided
			if (!ValidateAssetName<t_AssetType>(creationData.m_AssetName))
			{
				KG_WARN("Invalid asset name provided to create asset from file function: {}",
					creationData.m_AssetName);
				return k_EmptyHandle;
			}

			// Handle file location information
			std::filesystem::path metadataFileDirectory = creationData.m_CreationDirectory;
			if constexpr (HasFileLocation<t_AssetType>)
			{
				// Validate creation directory
				bool validDirectoryPath{ ValidateCreationDirectory(metadataFileDirectory) };
				if (!validDirectoryPath)
				{
					KG_WARN("Creation directory validation failed for path: {}",
						metadataFileDirectory.c_str());
					return k_EmptyHandle;
				}
				metadataFileDirectory = NormalizeAssetDirectory(metadataFileDirectory);
			}
			else
			{
				KG_ASSERT(creationData.m_CreationDirectory.empty(),
					"Attempt to provide a creation directory for an asset type that does not support it");
			}

			// Create metadata
			Metadata newMetadata{ CreateAssetMetadata(GetAssetIdentifier<t_AssetType>, 
				creationData.m_AssetName, metadataFileDirectory, creationData.m_IsHidden) };

			// Validate metadata
			if (!newMetadata.IsValid())
			{
				KG_WARN("Failed to create valid metadata for new {} asset with name {}",
					t_AssetType::GetAssetName(), creationData.m_AssetName);
				return k_EmptyHandle;
			}

			// Create asset
			CreateFromSpecImpl<t_AssetType>(newMetadata, spec);

			// Generate asset hash
			Utility::SHA256Hash resultHash{ GenerateAssetHash<t_AssetType>(newMetadata) };

			// Incorporate spec hash if applicable
			if constexpr (HasHashFromSpec<t_AssetType>)
			{
				// Generate spec hash
				Utility::SHA256Hash specHash{ GetHashFromSpecImpl<t_AssetType>(spec) };
				resultHash = resultHash ^ specHash;
			}
			newMetadata.m_Hash = resultHash;

			// Register new asset
			m_AssetRegistry.insert({ newHandle, newMetadata });
			SerializeAssetRegistry<t_AssetType>(); // Update Registry File on Disk

			// Fill in-memory cache
			if constexpr (HasAssetCacheFlag<t_AssetType>)
			{
				LoadAssetIntoCache<t_AssetType>(newMetadata);
			}

			// Validate asset hash
			if (!ValidateAssetHash<t_AssetType>(resultHash))
			{
				KG_WARN("Invalid asset hash generated from new asset");

				// Rollback asset creation
				bool deleteSuccess = DeleteAsset<t_AssetType>(newMetadata.m_Handle);
				KG_ASSERT(deleteSuccess);

				// Return empty handle
				return k_EmptyHandle;
			}

			// Send creation event
			SendManageAssetEvent<t_AssetType>(Events::ManageAssetAction::Create,
				newMetadata, nullptr);

			return newMetadata.m_Handle;
		}

		template<AssetConcept t_AssetType>
		void SerializeAssetRegistry()
		{
			// Get registry path
			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			std::filesystem::path registryPath{ GetAssetRegistryPath<t_AssetType>() };
			KG_ASSERT(!registryPath.empty(),
				"Generated empty registry path for asset type" );
			
			// Set up serializer
			YAML::Emitter serializer;
			serializer << YAML::BeginMap;
			serializer << YAML::Key << "Registry" << YAML::Value << t_AssetType::GetAssetName().CString();

			// Serialize other registry specific data
			if constexpr (HasRegistryData<t_AssetType>)
			{
				SerializeRegistrySpecificData<t_AssetType>(serializer);
			}

			// Serialize all assets
			serializer << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;
			for (auto& [handle, metadata] : m_AssetRegistry)
			{
				serializer << YAML::BeginMap; // Asset Map
				serializer << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);

				serializer << YAML::Key << "MetaData" << YAML::Value;
				serializer << YAML::BeginMap; // MetaData Map
				serializer << YAML::Key << "Name" << YAML::Value << metadata.m_Name;
				serializer << YAML::Key << "Hash" << YAML::Value << metadata.m_Hash;
				serializer << YAML::Key << "IsHidden" << YAML::Value << metadata.m_IsHidden;
				if constexpr (HasFileLocation<t_AssetType>)
				{
					serializer << YAML::Key << "FileDirectory" << YAML::Value << metadata.GetAssetRelativeFilePath<t_AssetType>().string();
				}
				if constexpr (HasIntermediates<t_AssetType>)
				{
					serializer << YAML::Key << "IntermediateDirectory" << YAML::Value << metadata.GetAssetRelativeIntermediatePath<t_AssetType>({}).string();
				}
				serializer << YAML::Key << "AssetIdentifier" << YAML::Value << metadata.m_TypeIdentifier;

				if constexpr (HasMetadata<t_AssetType>)
				{
					SerializeAssetSpecificMetadata<t_AssetType>(serializer, metadata);
				}
				
				serializer << YAML::EndMap; // Close metadata map
				serializer << YAML::EndMap; // Close asset map
			}
			serializer << YAML::EndSeq; // Close asset sequence
			serializer << YAML::EndMap; // Close registry map

			Utility::FileSystem::CreateNewDirectory(registryPath.parent_path());

			std::ofstream fout(registryPath);
			fout << serializer.c_str();
		}

		template<AssetConcept t_AssetType>
		void DeserializeAssetRegistry()
		{
			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			// Clear current registry
			m_AssetRegistry.clear();

			// Get registry path
			std::filesystem::path registryPath{ GetAssetRegistryPath<t_AssetType>() };
			KG_ASSERT(!registryPath.empty(),
				"Generated empty registry path for asset type");

			// Handle missing registry file gracefully
			if (!Utility::FileSystem::PathExists(registryPath))
			{
				KG_WARN("No .kgreg file found at provided registry path {}. Creating a new one.", registryPath.string());
				SerializeAssetRegistry<t_AssetType>();
			}

			// Load registry file
			YAML::Node data;
			try
			{
				data = YAML::LoadFile(registryPath.string());
			}
			catch (YAML::ParserException e)
			{
				KG_WARN("Failed to load {} file {}\n  {}", k_Config.m_FileExtension.CString(), registryPath.string(), e.what());
				return;
			}

			// Opening registry node 
			if (!data["Registry"]) 
			{ 
				KG_WARN("Could not validate initial registry node for the file: {}", registryPath.string());
				return; 
			}

			// Validate registry name
			const std::string registryName = data["Registry"].as<std::string>();
			KG_INFO("Deserializing {} Registry", registryName);

			// Open registry specific data
			if constexpr (HasRegistryData<t_AssetType>)
			{
				DeserializeRegistrySpecificData<t_AssetType>(data);
			}

			// Opening all assets 
			YAML::Node assets = data["Assets"];
			if (assets)
			{
				for (const YAML::Node& asset : assets)
				{
					Metadata newMetadata{};
					newMetadata.m_Handle = asset["AssetHandle"].as<uint64_t>();

					// Get generic metadata
					YAML::Node metadataNode = asset["MetaData"];
					newMetadata.m_Name = metadataNode["Name"].as<std::string>();
					newMetadata.m_Hash = metadataNode["Hash"].as<std::string>();
					newMetadata.m_IsHidden = metadataNode["IsHidden"].as<bool>();
					newMetadata.m_TypeIdentifier = metadataNode["AssetIdentifier"].as<AssetIdentifier>();

					// Get file location
					if constexpr (HasFileLocation<t_AssetType>)
					{
						newMetadata.m_FileDirectory = metadataNode["FileDirectory"].as<std::string>();
					}

					// Open asset specific metadata
					if constexpr (HasMetadata<t_AssetType>)
					{
						DeserializeAssetSpecificMetadata(metadataNode, newMetadata);
					}

					// Add asset to in memory registry 
					m_AssetRegistry.insert({ newMetadata.m_Handle, newMetadata });
				}
			}
		}

		template<AssetConcept t_AssetType>
		void LoadAllAssetIntoCache() requires HasAssetCacheFlag<t_AssetType>
		{
			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			// Revalidate active registry
			DeserializeAssetRegistry<t_AssetType>();

			// Load every asset into memory
			for (auto& [handle, metadata] : m_AssetRegistry)
			{
				// TODO: Maybe skip already loaded assets. Maybe add an option for clearing the asset cache first?
				LoadAssetIntoCache<t_AssetType>(metadata);
			}
		}

		template<AssetConcept t_AssetType> requires HasFileLocation<t_AssetType>
		bool SetAssetFileDirectory(AssetHandle handle, const std::filesystem::path& newFileDirectory)
		{
			// Validate asset exists
			if (!HasAssetByHandle(handle))
			{
				KG_WARN("Could not locate provided asset when attempting to its update file location");
				return false;
			}

			// Validate new file directory
			std::filesystem::path cachedDirectory = newFileDirectory;
			if constexpr (HasFileLocation<t_AssetType>)
			{
				// Validate creation directory
				bool validDirectoryPath{ ValidateCreationDirectory(cachedDirectory) };
				if (!validDirectoryPath)
				{
					KG_WARN("Creation directory validation failed for path: {}",
						cachedDirectory.c_str());
					return false;
				}
				metadataFileDirectory = NormalizeAssetDirectory(cachedDirectory);
			}
			else
			{
				KG_ASSERT(newFileDirectory.empty(),
					"Attempt to provide a creation directory for an asset type that does not support it");
			}

			// Get metadata
			Metadata& metadata { GetRawMetadata(handle) };

			// Update directory inside metadata
			metadata.m_FileDirectory = cachedDirectory;

			// Save changes to disk
			SerializeAssetRegistry<t_AssetType>();

			// Send update event
			SendManageAssetEvent<t_AssetType>(Events::ManageAssetAction::UpdateAssetInfo,
				metadata, nullptr);
			return true;
		}

		AssetRegistry& GetAssetRegistry()
		{
			return m_AssetRegistry;
		}

		AssetCache& GetAssetCache()
		{
			return m_AssetCache;
		}

		size_t GetAssetRegistrySize()
		{
			return m_AssetRegistry.size();
		}

	private:
		template<AssetConcept t_AssetType> requires HasFileLocation<t_AssetType>
		AssetHandle GetAssetHandleFromFileLocation(const std::filesystem::path& queryFileLocation)
		{
			// Validate query file location
			if (!ValidateQueryFileLocation<t_AssetType>(queryFileLocation))
			{
				KG_WARN("File location validation failed for provided path: {}", 
					queryFileLocation.string());
				return k_EmptyHandle;
			}

			std::filesystem::path normalizedQueryPath{ NormalizeQueryFileLocation(queryFileLocation)};

			// Check each asset in the registry for a matching file location
			for (auto& [handle, metadata] : m_AssetRegistry)
			{
				if (metadata.GetAssetRelativeFilePath<t_AssetType>().string().c_str() == 
					normalizedQueryPath.string().c_str())
				{
					return handle;
				}
			}

			// If could not find asset, return empty handle
			return k_EmptyHandle;
		}

		template<AssetConcept t_AssetType>
		AssetReference<t_AssetType> DeserializeAssetImpl(Metadata& metadata)
		{
			// Check if asset already exists
			t_AssetType* newAsset{ nullptr };
			bool assetExists{ false };
			if constexpr (HasAssetCacheFlag<t_AssetType>)
			{
				if (m_AssetCache.contains(metadata.m_Handle))
				{
					AssetReference<t_AssetType> assetReference{m_AssetCache.at(metadata.m_Handle)};
					newAsset = &assetReference.GetAsset();
					assetExists = true;
				}
			}

			// Allocate new asset if it does not already exist
			if (!assetExists)
			{
				newAsset = i_BackingAllocator->Alloc<t_AssetType>();
			}

			// Deserialize asset
			DeserializeAssetContext deserializeContext
			{ 
				&metadata
			};
			newAsset->Deserialize((void*)(&deserializeContext));

			// Return asset reference
			return { metadata.m_Handle, LoadState::Loaded, newAsset };
		}

		template<AssetConcept t_AssetType>
		void SerializeAssetImpl(Metadata& metadata, AssetReference<t_AssetType> assetReference)
		{
			// Ensure asset type supports serialization
			KG_ASSERT(assetReference.IsUsable(), "Attempt to serialize an invalid asset reference");

			// Get the actual asset
			t_AssetType& asset{ assetReference.GetAsset() };

			// Serialize asset
			SerializeAssetContext serializeContext
			{
				&metadata
			};
			asset.Serialize((void*)(&serializeContext));
		}

		template<AssetConcept t_AssetType> requires HasRegistryData<t_AssetType>
		void DeserializeRegistrySpecificData(YAML::Node& registryNode)
		{
			// Check if the registry data already exists
			if (!m_RegistrySpecificData)
			{
				m_RegistrySpecificData = i_BackingAllocator->Alloc<typename t_AssetType::RegistryData>();
			}

			// Get registry specific data
			typename t_AssetType::RegistryData* registryData = (typename t_AssetType::RegistryData*)m_RegistrySpecificData;

			// Deserialize registry
			DeserializeRegistryContext context{ &registryNode };
			registryData->Deserialize((void*)&context);
		}

		template<AssetConcept t_AssetType> requires HasRegistryData<t_AssetType>
		void SerializeRegistrySpecificData(YAML::Emitter& serializer)
		{
			// Get registry specific data reference
			typename t_AssetType::RegistryData* registryData = (typename t_AssetType::RegistryData*)m_RegistrySpecificData;
			KG_ASSERT(registryData);

			// Serialize registry
			SerializeRegistryContext context{ &serializer };
			registryData->Serialize((void*)&context);
		}

		template<AssetConcept t_AssetType> requires HasMetadata<t_AssetType>
		void DeserializeAssetSpecificMetadata(YAML::Node& node, Metadata& metadata)
		{
			// Get specific metadata
			typename t_AssetType::Metadata* specificMetadata = metadata.GetSpecificMetaData<typename t_AssetType::Metadata>();
			if (!specificMetadata)
			{
				specificMetadata = i_BackingAllocator->Alloc<typename t_AssetType::Metadata>();
				metadata.SetSpecificMetaData(specificMetadata);
			}

			// Deserialize specific metadata
			DeserializeMetaDataContext context{ &node, &metadata };
			specificMetadata->Deserialize((void*)&context);
		}

		template<AssetConcept t_AssetType> requires HasMetadata<t_AssetType>
		void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Metadata& metadata)
		{
			// Get specific metadata
			typename t_AssetType::Metadata* specificMetadata = metadata.GetSpecificMetaData<typename t_AssetType::Metadata>();
			KG_ASSERT(specificMetadata);

			// Serialize specific metadata
			SerializeMetaDataContext context{ &serializer, &metadata };
			specificMetadata->Serialize((void*)&context);
		}

		std::filesystem::path NormalizeAssetDirectory(const std::filesystem::path& creationDirectory)
		{
			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			std::filesystem::path pathRelativeToAssetDir
			{ 
				Utility::FileSystem::GetRelativePath(paths.GetAssetDirectory(), creationDirectory) 
			};

			return Utility::FileSystem::ConvertToUnixStylePath(pathRelativeToAssetDir);
		}

		std::filesystem::path NormalizeQueryFileLocation(const std::filesystem::path& queryFileLocation)
		{
			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };
			std::filesystem::path pathRelativeToAssetDir
			{
				Utility::FileSystem::GetRelativePath(paths.GetAssetDirectory(), queryFileLocation)
			};
			return Utility::FileSystem::ConvertToUnixStylePath(pathRelativeToAssetDir);
		}

		template<AssetConcept t_AssetType>
		std::filesystem::path GetAssetRegistryPath()
		{
			if constexpr (!HasIntermediates<t_AssetType>)
			{
				return {};
			}

			Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			std::stringstream path;
			path << projectPaths.GetIntermediateDirectory().string() <<
				GetModuleName<t_AssetType>() << "/" << GetTypeName<t_AssetType>()
				<< "/" << GetTypeName<t_AssetType>() << "Registry" << ".kgreg";
			return path.str();
		}

		template <AssetConcept t_AssetType>
		Utility::SHA256Hash GenerateAssetHash(Metadata metadata)
		{
			Utility::SHA256Hash resultHash{};

			// Get checksum from name
			if (!metadata.m_Name.IsEmpty())
			{
				Utility::SHA256Hash nameChecksum{};
				nameChecksum = Utility::FileSystem::SHA256HashFromString(metadata.m_Name.CString());
				resultHash = resultHash ^ nameChecksum;
			}

			// Get checksum from file data
			if constexpr (HasFileLocation<t_AssetType>)
			{
				Utility::SHA256Hash fileChecksum{};
				fileChecksum = Utility::FileSystem::SHA256HashFromFile(metadata.GetAssetFullFilePath<t_AssetType>());
				resultHash = resultHash ^ fileChecksum;
			}

			// Get checksum from intermediate data
			if constexpr (HasIntermediates<t_AssetType>)
			{
				Utility::SHA256Hash intermediateChecksum{};
				std::filesystem::path intermediateFolder{ metadata.GetAssetFullIntermediatePath<t_AssetType>({})};
				for (const FixedStrBuf16& extension : t_AssetType::GetIntermediateExtensions())
				{
					intermediateFolder.replace_extension(extension.CString());
					Utility::SHA256Hash currentIntermediateChecksum{ Utility::FileSystem::SHA256HashFromFile(intermediateFolder) };
					intermediateChecksum = intermediateChecksum ^ currentIntermediateChecksum;
				}
				resultHash = resultHash ^ intermediateChecksum;
			}

			return resultHash;
		}

		template<AssetConcept t_AssetType>
		void DeleteAssetFiles(Metadata& metadata)
		{
			// Delete the asset's data on-disk
			if constexpr (HasFileLocation<t_AssetType>)
			{
				std::filesystem::path fileLocation { metadata.GetAssetFullFilePath<t_AssetType>()};
				bool deleteSuccess{ Utility::FileSystem::DeleteSelectedFile(fileLocation) };
				KG_ASSERT(deleteSuccess);
			}

			// Delete the asset's intermediate(s) on-disk
			if constexpr (HasIntermediates<t_AssetType>)
			{
				std::filesystem::path intermediateFolder{ metadata.GetAssetRelativeIntermediatePath<t_AssetType>({})};
				intermediateFolder.replace_filename(metadata.m_Name);

				std::span<FixedBufStr16> allIntermediateExtensions{ t_AssetType::GetIntermediateExtensions };
				for (const FixedBufStr16& extension : allIntermediateExtensions)
				{
					intermediateFolder.replace_extension(extension.CString());
					KG_ASSERT(Utility::FileSystem::PathExists(intermediateFolder));
					bool deleteSuccess { Utility::FileSystem::DeleteSelectedFile(intermediateLocation) };
					KG_ASSERT(deleteSuccess);
				}
			}
		}

		template<AssetConcept t_AssetType>
		void RemoveAssetDataFromRegistry(AssetHandle handle)
		{
			// Delete in-memory copy of this asset
			if constexpr (HasAssetCacheFlag<t_AssetType>)
			{
				if (m_AssetCache.contains(handle))
				{
					// Get the relevant asset data
					GenericAssetReference& assetData{ m_AssetCache.at(handle)};
					KG_ASSERT(assetData.m_DataPtr);
					KG_ASSERT(assetData.m_Handle == handle);
					KG_ASSERT(assetData.m_LoadState != LoadState::Unloaded);

					// Deallocate if necessary
					if (assetData.m_LoadState == LoadState::Loaded)
					{
						// Deallocate the data
						i_BackingAllocator->Dealloc<t_AssetType>((t_AssetType*)assetData.m_DataPtr); // GOTTA LOVE UB
					}

					// Remove asset listing from cache map
					m_AssetCache.erase(assetHandle);
				}
			}

			// Delete the asset inside the registry
			m_AssetRegistry.erase(assetHandle);
		}

		template<AssetConcept t_AssetType> requires HasAssetCacheFlag<t_AssetType>
		void LoadAssetIntoCache(Metadata& metadata)
		{
			// Fill in-memory cache if appropriate
			KG_ASSERT(!m_AssetCache.contains(metadata.m_Handle));

			AssetReference<t_AssetType> newAssetRef{ DeserializeAssetImpl<t_AssetType>(metadata) };
			KG_ASSERT(newAssetRef.IsUsable());
			m_AssetCache.insert
			({
				metadata.m_Handle,
				{
					newAssetRef.GetHandle(),
					newAssetRef.GetLoadState(),
					(void*)&newAssetRef.GetAsset()
				}
			});
		}

		template<AssetConcept t_AssetType> requires HasUpdateFromAsset<t_AssetType>
		void CustomUpdateFromAssetImpl(Metadata& metadata,
			AssetReference<t_AssetType> currentRef, AssetReference<t_AssetType> otherRef)
		{
			// Ensure asset type supports serialization
			KG_ASSERT(metadata.IsValid());
			KG_ASSERT(currentRef.IsUsable());
			KG_ASSERT(otherRef.IsUsable());

			// Get the actual asset
			t_AssetType& currentAsset{ currentRef.GetAsset() };

			// Update the asset
			currentAsset.UpdateFromAsset(metadata, otherRef);
		}

		template<AssetConcept t_AssetType> requires HasValidateUpdateFromAsset<t_AssetType>
		Ref<void> ValidateUpdateFromAssetImpl(Metadata& metadata, AssetReference<t_AssetType> newAsset)
		{
			// Ensure asset reference is valid
			KG_ASSERT(metadata.IsValid());
			KG_ASSERT(newAsset.IsUsable());

			return t_AssetType::ValidateUpdateFromAsset(metadata, newAsset);
		};

		template<AssetConcept t_AssetType> requires HasUpdateFromSpec<t_AssetType>
		void UpdateFromSpecImpl(Metadata& metadata,
			AssetReference<t_AssetType> currentRef, const typename t_AssetType::Spec& spec)
		{
			// Ensure asset type supports serialization
			KG_ASSERT(metadata.IsValid());
			KG_ASSERT(currentRef.IsUsable());

			// Get the actual asset
			t_AssetType& currentAsset{ currentRef.GetAsset() };

			// Update the asset
			currentAsset.UpdateFromSpec(metadata, spec);
		}

		template<AssetConcept t_AssetType> requires HasValidateUpdateFromSpec<t_AssetType>
		Ref<void> ValidateUpdateFromSpecImpl(Metadata& metadata, const typename t_AssetType::Spec& spec)
		{
			KG_ASSERT(metadata.IsValid());

			// Call user-defined validate function
			return t_AssetType::ValidateUpdateFromSpec(metadata, spec);
		};

		template<AssetConcept t_AssetType> requires HasValidateDelete<t_AssetType>
		void ValidateDeleteImpl(Metadata& metadata)
		{
			KG_ASSERT(metadata.IsValid());

			// Call user-defined validate function
			t_AssetType::ValidateDelete(metadata);
		};

		template<AssetConcept t_AssetType> requires HasCreateFromName<t_AssetType>
		void CreateFromNameImpl(Metadata& metadata)
		{
			KG_ASSERT(metadata.IsValid());

			// Call user-defined create-from-name function
			t_AssetType::CreateFromName(metadata);
		};

		template <AssetConcept t_AssetType> requires HasCreateFromFile<t_AssetType>
		void CreateFromFileImpl(Metadata& metadata, const std::filesystem::path& sourcePath)
		{
			KG_ASSERT(metadata.IsValid());
			KG_ASSERT(!sourcePath.empty());

			// Call user-defined create-from-file function
			t_AssetType::CreateFromFile(metadata, sourcePath);
		};

		template <AssetConcept t_AssetType> requires HasCreateFromSpec<t_AssetType>
		void CreateFromSpecImpl(Metadata& metadata, const typename t_AssetType::Spec& spec)
		{
			KG_ASSERT(metadata.IsValid());

			// Call user-defined create-from-file function
			t_AssetType::CreateFromSpec(metadata, spec);
		};

		template <AssetConcept t_AssetType> requires HasValidateCreateFromSpec<t_AssetType>
		void ValidateCreateFromSpecImpl(const AssetCreationData& creationData, 
			const typename t_AssetType::Spec& spec)
		{
			// Call user-defined create-from-file function
			t_AssetType::ValidateCreateFromSpec(creationData, spec);
		};

		template <AssetConcept t_AssetType> requires HasGetAssetFromSpec<t_AssetType>
		bool GetAssetFromSpecImpl(Metadata& metadata, const typename t_AssetType::Spec& spec)
		{
			// Call user-defined function to check if an asset meets the provided specification requirements
			return t_AssetType::GetAssetFromSpec(metadata, spec);
		};

		template <AssetConcept t_AssetType> requires HasHashFromSpec<t_AssetType>
		bool GetHashFromSpecImpl(const typename t_AssetType::Spec& spec)
		{
			// Call user-defined function to get hash from spec
			return t_AssetType::GetHashFromSpec(spec);
		};

		template<AssetConcept t_AssetType>
		bool ValidateAssetName(std::string_view name)
		{
			// Ensure name is not empty
			if (name.empty())
			{
				return false;
			}

			// Ensure name is unique if required
			if constexpr (HasRequireUniqueNameFlag<t_AssetType>)
			{
				for (const auto& [handle, metadata] : m_AssetRegistry)
				{
					if (metadata.m_Name.StringView() == name)
					{
						KG_WARN("Attempt to create asset with non-unique name: {}", name);
						return false;
					}
				}
			}

			// If all checks pass, return true
			return true;
		}

		template<AssetConcept t_AssetType>
		bool ValidateAssetHash(Utility::SHA256Hash hash)
		{
			// Ensure hash is not empty
			if (hash.IsEmpty())
			{
				return false;
			}

			// Ensure hash is unique if required
			if constexpr (HasRequireUniqueHashFlag<t_AssetType>)
			{
				for (const auto& [handle, metadata] : m_AssetRegistry)
				{
					if (metadata.m_Hash == hash)
					{
						KG_WARN("Attempt to create asset with non-unique hash: {}", hash.CString());
						return false;
					}
				}
			}

			// If all checks pass, return true
			return true;
		}

		template<AssetConcept t_AssetType> requires HasImportExtensions<t_AssetType>
		bool ValidateSourcePath(const std::filesystem::path& sourcePath)
		{
			// Source path needs to be a regular file yeah?
			if (!Utility::FileSystem::IsRegularFile(sourcePath))
			{
				KG_WARN("Provided source path for asset creation is not a regular file: {}",
					sourcePath.string());
				return false;
			}

			// Source path needs to have an extension yeah?
			if (!Utility::FileSystem::HasFileExtension(sourcePath))
			{
				KG_WARN("Provided source path for asset creation does not have an extension: {}",
					sourcePath.string());
				return false;
			}

			// Source path needs to have a valid extension yeah?
			bool foundValidExtension{ false };
			for (const FixedBufStr16& extension : t_AssetType::GetImportExtensions())
			{
				// Continue if empty extension is found
				if (extension.IsEmpty())
				{
					continue;
				}

				// Check if extension matches
				if (sourcePath.extension().string().c_str() == extension.CString())
				{
					foundValidExtension = true;
					break;
				}
			}
			if (!foundValidExtension)
			{
				KG_WARN("Provided source path for asset creation does not have a valid extension: {}",
					sourcePath.string());
				return false;
			}
			return true;
		}

		bool ValidateCreationDirectory(const std::filesystem::path& creationDirectory)
		{
			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			// Path must be absolute
			if (!creationDirectory.is_absolute())
			{
				KG_WARN("Provided path for new asset creation is not absolute");
				return false;
			}

			// Ensure provided path is within the assets directory
			if (!Utility::FileSystem::DoesPathContainSubPath(paths.GetAssetDirectory(), creationDirectory))
			{
				KG_WARN("Provided path for new asset creation is not within asset directory");
				return false;
			}

			// Ensure provided path is not indicating a file
			if (Utility::FileSystem::HasFileExtension(creationDirectory))
			{
				KG_WARN("File provided as path to asset. Creation paths should only indicate a directory");
				return false;
			}

			// Create directory if it does not already exist
			if (!Utility::FileSystem::CreateNewDirectory(creationDirectory))
			{
				KG_WARN("Failed to ensure directory for new asset exists/was-created: {}",
					creationDirectory);
				return false;
			}

			return true;
		}

		template<AssetConcept t_AssetType> requires HasFileLocation<t_AssetType>
		bool ValidateQueryFileLocation(const std::filesystem::path& fileLocation)
		{
			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			if (fileLocation.is_absolute())
			{
				// Ensure provided path is within the assets directory
				if (!Utility::FileSystem::DoesPathContainSubPath(paths.GetAssetDirectory(), fileLocation))
				{
					KG_WARN("Provided query file location is not within the asset directory");
					return false;
				}
			}

			// Ensure provided path is a file
			if (!Utility::FileSystem::IsRegularFile(fileLocation))
			{
				KG_WARN("Provided query file location does not indicate an actual file, {}",
					fileLocation.c_str());
				return false;
			}

			// Ensure provided path has correct extension
			if (fileLocation.extension().string().c_str() != t_AssetType::GetFileExtension().CString())
			{
				KG_WARN("Provided query file location does not have the correct file extension for asset type {}, {}",
					t_AssetType::GetAssetName().CString(), fileLocation.c_str());
				return false;
			}

			return true;
		}

		template<AssetConcept t_AssetType>
		Metadata CreateAssetMetadata(AssetIdentifier identifier, std::string_view assetName, 
			const std::filesystem::path& assetDirectory, bool isHidden)
		{
			// Generate new ID
			AssetHandle newHandle{ RandomUUIDService::GetRandomUUID() };

			// Ensure ID is unique
			// TODO: Note we may need to load in entire registry before this loop to ensure uniqueness
			constexpr size_t k_MaxAttempts{ 100 };
			size_t attemptCount{ 0 };
			while (m_AssetRegistry.contains(newHandle) && attemptCount < k_MaxAttempts)
			{
				newHandle = RandomUUIDService::GetRandomUUID();
				attemptCount++;
			}

			// Handle failure to generate unique ID
			if (attemptCount >= k_MaxAttempts)
			{
				KG_WARN("Failed to generate a unique asset handle after {} attempts", k_MaxAttempts);
				return {};
			}

			// Create basic metadata
			Metadata newMetadata{};
			newMetadata.m_Handle = newHandle;
			newMetadata.m_Name = assetName;
			newMetadata.m_TypeIdentifier = identifier;
			newMetadata.m_IsHidden = isHidden;

			// Set file directory
			if (isHidden)
			{
				KG_ASSERT(assetDirectory.empty());
				newMetadata.m_FileDirectory = newMetadata.GetAssetRelativeHiddenFolder<t_AssetType>();
			}
			else
			{
				newMetadata.m_FileDirectory = assetDirectory;
			}
			return newMetadata;
		}

		Metadata& GetRawMetadata(AssetHandle handle)
		{
			KG_ASSERT(m_AssetRegistry.contains(handle));

			Metadata& metadata{ m_AssetRegistry.at(handle)};
			KG_ASSERT(metadata.IsValid());

			return metadata;
		}

		template<AssetConcept t_AssetType> requires HasFileLocation<t_AssetType>
		AssetHandle GetAssetHandleByFileLocation(const std::filesystem::path& queryLocation)
		{
			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			std::filesystem::path assetPath = queryLocation;

			// Convert provided path to relative (registry assets are stored as relative)
			if (assetPath.is_absolute())
			{
				assetPath = Utility::FileSystem::GetRelativePath(paths.GetAssetDirectory(), assetPath);
			}

			for (auto& [assetHandle, metadata] : m_AssetRegistry)
			{
				std::filesystem::path registryAssetPath = metadata.GetAssetRelativeFilePath<t_AssetType>();
				if (registryAssetPath.compare(assetPath) == 0)
				{
					return assetHandle
				}
			}
			return k_EmptyHandle;
		}

		template<AssetConcept t_AssetType>
		void SendManageAssetEvent(Metadata& metadata, Events::ManageAssetAction action, 
			Ref<void> optionalData = nullptr)
		{
			Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>
			(
				metadata.m_Handle,
				GetAssetIdentifier<t_AssetType>(),
				action,
				optionalData
			);
			EngineService::GetActiveEngine().GetThread().SubmitEvent(event);
		}
		
	private:
		//==============================
		// Internal Fields
		//==============================
		AssetRegistry m_AssetRegistry{};
		AssetCache m_AssetCache{};
		void* m_RegistrySpecificData{ nullptr };
	private:
		//==============================
		// Injected Dependencies
		//==============================
		Memory::IAllocator* i_BackingAllocator{ nullptr };
	};
}