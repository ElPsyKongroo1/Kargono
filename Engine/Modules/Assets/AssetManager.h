#pragma once
#include "Kargono/Core/Base.h"
#include "Modules/Core/Engine.h"
#include "Kargono/Projects/Project.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Modules/Events/AssetEvent.h"
#include "Modules/Assets/Module/AssetTag.h"
#include "Modules/Assets/AssetReference.h"
#include "Kargono/Memory/IAllocator.h"
#include "Modules/Assets/Concepts/OptionalAssetConcepts.h"
#include "Modules/Assets/Concepts/RegistryConcept.h"

#include "API/Serialization/yamlcppAPI.h"

#include <bitset>
#include <tuple>
#include <unordered_map>

namespace Kargono::Assets
{
	using AssetRegistry = std::unordered_map<AssetHandle, Metadata>;
	using AssetCache = std::unordered_map<AssetHandle, AssetGenericData>;

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

		template<AssetConcept t_AssetType>
		AssetReference<t_AssetType> GetCachedReference(AssetHandle handle)
		{
			static_assert(t_AssetType::GetAssetFlags().IsFlagSet(AssetFlag::HasAssetCache),
				"Attempted to retrieve cached asset for asset wo/ cache flag set");

			if (!m_AssetCache.contains(handle))
			{
				return {};
			}

			return { m_AssetCache.at(handle) };
		}

		template<AssetConcept t_AssetType>
		AssetReference<t_AssetType> GetAsset(AssetHandle handle)
		{
			constexpr bool k_HasAssetCache{ t_AssetType::GetAssetFlags().IsFlagSet(AssetFlag::HasAssetCache)};

			Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };
			
			// Check cache first
			if constexpr (k_HasAssetCache)
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
				AssetReference<t_AssetType> newAssetRef{ DeserializeAsset(metadata)};
				KG_ASSERT(newAssetRef.IsValid());
				
				// Cache asset if applicable
				if constexpr (k_HasAssetCache)
				{
					AssetGenericData cachedReference
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
		AssetReference<t_AssetType> GetAsset(const std::filesystem::path& fileLocation)
		{
			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			std::filesystem::path assetPath = fileLocation;

			if (fileLocation.is_absolute())
			{
				assetPath = Utility::FileSystem::GetRelativePath(paths.GetAssetDirectory(), fileLocation);
			}

			for (auto& [assetHandle, metadata] : m_AssetRegistry)
			{
				if (metadata.m_FileLocation.compare(assetPath) == 0)
				{
					return GetAsset<t_AssetType>(assetHandle);
				}
			}
			// Return empty asset if the asset is not found in the registry
			KG_WARN("Invalid filepath provided to GetAsset(filepath) {}. Returning empty {} asset.", fileLocation.string(), t_AssetType::GetAssetName());
			return {};
		}

		bool HasAsset(AssetHandle handle)
		{
			return m_AssetRegistry.contains(handle);
		}

		// TODO: Redo this API, I hate it
		template<AssetConcept t_AssetType> requires HasFileLocation<t_AssetType>
		bool HasAsset(std::string_view assetName)
		{
			// Check for a matching name
			for (const auto& [handle, metadata] : m_AssetRegistry)
			{
				if (metadata.m_FileLocation.stem() == assetName)
				{
					return true;
				}
			}

			// Exit if no match is found
			return false;
		}

		template<AssetConcept t_AssetType> 
			requires HasFileLocation<t_AssetType> || HasIntermediates<t_AssetType>
		void SaveAsset(AssetReference<t_AssetType> assetReference)
		{
			KG_ASSERT(assetReference.IsValid() && !assetReference.IsEmpty(), "Attempt to save an invalid asset reference");

			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			// Ensure handle exists inside registry
			if (!m_AssetRegistry.contains(assetReference.GetHandle()))
			{
				KG_WARN("Attempt to save asset of type {} that does not exist in the asset registry", 
					t_AssetType::GetAssetName());
				return;
			}

			// Provide asset specific validation
			Ref<void> providedData{ nullptr } 
			if constexpr (HasSaveValidation<t_AssetType>)
			{
				providedData = SaveAssetValidation(assetReference);
			}

			// Find location of asset's data
			Metadata& metadata = m_AssetRegistry[assetReference.GetHandle()];

			// Update in memory asset if applicable
			if constexpr (k_HasAssetCache)
			{
				m_AssetCache.at(assetReference.GetHandle()) = 
				{ 
					assetReference.GetHandle(), 
					assetReference.GetLoadState(), 
					(void*)&assetReference.GetAsset()
				};
			}

			// Save asset data on-disk
			SerializeAsset<t_AssetType>(metadata, assetReference);

			// TODO: CHECKSUM FIX NEEDED
			// Get and update checksum
			const std::string currentCheckSum = Utility::FileSystem::ChecksumFromFile(dataLocation);

			// Ensure checksum is valid
			if (currentCheckSum.empty())
			{
				KG_WARN("Generated empty checksum while saving an asset");
			}
			metadata.m_CheckSum = currentCheckSum;

			Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>
			(
				assetReference.GetHandle(),
				GetAssetIdentifier<t_AssetType>(),
				Events::ManageAssetAction::UpdateAsset,
				providedData
			);
			EngineService::GetActiveEngine().GetThread().SubmitEvent(event);
		}

		template<AssetConcept t_AssetType>
		bool DeleteAsset(AssetHandle assetHandle)
		{
			if (!m_AssetRegistry.contains(assetHandle))
			{
				KG_WARN("Failed to delete {} asset. Asset was not found in registry.");
				return false;
			}

			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			// Find location of asset's data
			Metadata& metadata = m_AssetRegistry[assetHandle];
			
			// Pre-delete event and validation
			Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>
			(
				assetHandle, 
				GetAssetIdentifier<t_AssetType>(),
				Events::ManageAssetAction::PreDelete
			);

			if constexpr (HasDeleteValidation<t_AssetType>)
			{
				DeleteAssetValidation<t_AssetType>(assetHandle);
			}

			EngineService::GetActiveEngine().GetThread().OnEvent(event.get());

			DeleteAssetFiles<t_AssetType>(metadata);
			
			RemoveAssetDataFromRegistry<t_AssetType>(assetHandle);

			// Save the modified registry to disk
			SerializeAssetRegistry<t_AssetType>();

			// Post-delete event and validation
			Ref<Events::ManageAsset> postEvent = CreateRef<Events::ManageAsset>
			(
				assetHandle,
				GetAssetIdentifier<t_AssetType>(),
				Events::ManageAssetAction::PostDelete
			);
			EngineService::GetActiveEngine().GetThread().OnEvent(postEvent.get());

			return true;
		}

		template<AssetConcept t_AssetType>
		void ClearAssetRegistry()
		{
			constexpr bool k_HasAssetCache{ t_AssetType::GetAssetFlags().IsFlagSet(AssetFlag::HasAssetCache) };

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
			if constexpr (k_HasAssetCache)
			{
				KG_ASSERT(m_AssetCache.size() == 0);
			}
			KG_ASSERT(m_AssetRegistry.size() == 0);
		}

		bool GetIsAssetHidden(AssetHandle handle)
		{
			KG_ASSERT(m_AssetRegistry.contains(handle));
			Metadata& metadata = m_AssetRegistry.at(handle);
			KG_ASSERT(metadata.IsValid());

			return metadata.m_IsHidden;
		}

		void SetIsAssetHidden(AssetHandle handle, bool isHidden)
		{
			KG_ASSERT(m_AssetRegistry.contains(handle));
			Metadata& metadata = m_AssetRegistry.at(handle);

			metadata.m_IsHidden = isHidden;
		}

		template<AssetConcept t_AssetType> requires HasCreationFromName<t_AssetType>
		AssetHandle CreateAsset(const char* assetName, const std::filesystem::path& creationDirectory)
		{
			constexpr bool k_HasAssetCache{ t_AssetType::GetAssetFlags().IsFlagSet(AssetFlag::HasAssetCache) };

			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };
			
			// Validate creation directory
			bool validDirectoryPath = ValidateCreationDirectory(creationDirectory);
			if (!validDirectoryPath)
			{
				KG_WARN("Creation directory validation failed for path: {}", creationDirectory.c_str());
				return k_EmptyHandle;
			}

			// Create directory if it does not already exist
			bool directoryExists = Utility::FileSystem::CreateNewDirectory(creationDirectory);
			if (!directoryExists)
			{
				KG_WARN("Failed to create directory for new asset: {}", creationDirectory);
				return k_EmptyHandle
			}

			// Create new asset path
			std::filesystem::path newAssetPath
			{ 
				CreateNewAssetPath
				(
					creationDirectory, 
					assetName, 
					t_AssetType::GetFileExtension().CString()
				) 
			};


			Metadata newMetadata { CreateAssetMetadata(identifier, newAssetPath) };

			// Create File
			CreateAssetFromName(newMetadata, assetName,  paths.GetAssetDirectory() / newMetadata.m_FileLocation);

			// TODO: DEAL WITH THIS CHECKSUM BULLSHEEEEEEE...
			// Create Checksum
			const std::string currentCheckSum = Utility::FileSystem::ChecksumFromFile(paths.GetAssetDirectory() / newMetadata.m_FileLocation);
			if (currentCheckSum.empty())
			{
				KG_WARN("Generated empty checksum from the string {}", assetName);
				return Assets::k_EmptyHandle;
			}
			newMetadata.m_CheckSum = currentCheckSum;

			// Register new asset and return handle.
			m_AssetRegistry.insert({ newMetadata.m_Handle, newMetadata });
			SerializeAssetRegistry<t_AssetType>();

			// Fill in-memory cache if appropriate
			if constexpr (k_HasAssetCache)
			{
				AssetReference<t_AssetType> newAssetRef{ DeserializeAsset<t_AssetType>(newMetadata) };
				KG_ASSERT(newAssetRef.IsValid() && !newAssetRef.IsEmpty());
				m_AssetCache.insert({ newMetadata.m_Handle, 
					{ 
						newAssetRef.GetHandle(), 
						newAssetRef.GetLoadState(), 
						(void*)&newAssetRef.GetAsset()
					});
			}

			Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>
			(
				newMetadata.m_Handle, 
				k_Config.m_Identifier, 
				Events::ManageAssetAction::Create
			);
			EngineService::GetActiveEngine().GetThread().SubmitEvent(event);
			return newMetadata.m_Handle;
		}

		AssetHandle CreateAsset(typename t_AssetType::Spec& spec)
		{
			static_assert(HasCreationFromSpec<t_AssetType>,
				"Attempt to save an asset who's type does not spec creation");

			// Handle validation
			if constexpr (HasSpecValidation<t_AssetType>)
			{
				bool validateSuccess = t_AssetType::CreateSpecValidation(spec);
				if (!validateSuccess)
				{
					KG_WARN("Validation of asset specification failed");
					return k_EmptyHandle;
				}
			}

			// Create New Asset/Handle
			AssetHandle newHandle{ RandomUUIDService::GetRandomUUID() };
			Metadata newMetadata{};
			newMetadata.m_Handle = newHandle;
			newMetadata.m_TypeIdentifier = GetAssetIdentifier<t_AssetType>();
			newMetadata.m_CheckSum = currentCheckSum;

			// TODO: Ensure this section works (might need some constexpr stuff here)
			newMetadata.m_FileLocation = "";
			newMetadata.m_IntermediateLocation = m_RegistryLocation.parent_path() / ((std::string)newAsset.m_Handle + m_FileExtension.CString());

			// Create Intermediate
			t_AssetType::CreateAssetFromSpec(newMetadata, spec)

			// Register New Asset and Create Texture
			m_AssetRegistry.insert({ newHandle, newAsset });
			SerializeAssetRegistry(); // Update Registry File on Disk

			Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			// Fill in-memory cache if appropriate
			if constexpr (k_HasAssetCache)
			{
				std::filesystem::path assetPath = projectPaths.GetIntermediateDirectory() / newAsset.Data.IntermediateLocation;
				m_AssetCache.insert({ newHandle, DeserializeAsset(newMetadata, assetPath) });
			}

			Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>
			(
				newHandle,
				Assets::GetAssetIdentifier<t_AssetType>(),
				Events::ManageAssetAction::Create
			);
			EngineService::GetActiveEngine().GetThread().SubmitEvent(event);
			return newHandle;
		}

		AssetHandle CreateAsset(const char* assetName)
		{
			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };
			return CreateAsset(assetName, paths.GetAssetDirectory());
		}

		AssetHandle ImportAssetFromFile(const std::filesystem::path& sourcePath)
		{
			if (!Utility::FileSystem::HasFileExtension(sourcePath))
			{
				KG_WARN("Cannot import provided file path. Path does not contain a file extension. (i.e. it is not a file)");
				return Assets::k_EmptyHandle;
			}

			return ImportAssetFromFile(sourcePath, sourcePath.stem().string().c_str(), sourcePath.parent_path());
		}

		AssetHandle ImportAssetFromFile(const std::filesystem::path& sourcePath, const char* newFileName, const std::filesystem::path& creationPath)
		{
			static_assert(HasCreationFromFile<t_AssetType>,
				"Attempt to import an asset for a file type that does not support importing");

			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			// Ensure provided name is valid
			if (!newFileName || newFileName[0] == '\0')
			{
				KG_WARN("Empty/invalid name provided to import asset from file function");
				return Assets::k_EmptyHandle;
			}

			// Ensure source path is valid
			if (!Utility::FileSystem::HasFileExtension(sourcePath))
			{
				KG_WARN("Cannot import provided file path. Path does not contain a file extension. (i.e. it is not a file)");
				return Assets::k_EmptyHandle;
			}

			// Check if source path file extension is appropriate for this file type
			bool foundValidExtension{ false };
			for (const FixedBufStr16& extension : t_AssetType::GetImportExtensions())
			{
				// Continue if empty extension is found
				if (extension.IsEmpty())
				{
					continue;
				}

				if (sourcePath.extension().string().c_str() == extension.CString())
				{
					foundValidExtension = true;
					break;
				}
			}

			// Exit if extension is invalid
			if (!foundValidExtension)
			{
				KG_WARN("Invalid file extension ({}) for {} asset type. Valid file extensions for {} include: ");
				for (FixedBufStr16& extension : k_Config.m_ImportExtensions)
				{
					// Continue if empty extension is found
					if (extension.IsEmpty())
					{
						continue;
					}

					KG_WARN("  {}", extension.CString());
				}
				return Assets::k_EmptyHandle;
			}

			// Validate provided paths
			bool exportingToBaseAssetDir{ true };
			if (creationPath == paths.GetAssetDirectory())
			{
				exportingToBaseAssetDir = true;
			}
			else
			{
				exportingToBaseAssetDir = false;
				// Ensure provided path is within the assets directory
				if (!Utility::FileSystem::DoesPathContainSubPath(paths.GetAssetDirectory(), creationPath))
				{
					KG_WARN("Provided path for new asset importation is not within asset directory");
					return Assets::k_EmptyHandle;
				}

				// Ensure provided path is not indicating a file
				if (Utility::FileSystem::HasFileExtension(creationPath))
				{
					KG_WARN("File provided as path to asset. Destination paths should only indicate a directory");
					return Assets::k_EmptyHandle;
				}

				// Create path if it does not already exist
				Utility::FileSystem::CreateNewDirectory(creationPath);
			}

			// Create Checksum
			const std::string currentCheckSum = Utility::FileSystem::ChecksumFromFile(sourcePath);

			// Ensure checksum is valid
			if (currentCheckSum.empty())
			{
				KG_WARN("Generated empty checksum from file at {}", sourcePath.string());
				return Assets::k_EmptyHandle;
			}

			// Ensure duplicate asset is not found in registry.
			for (const auto& [handle, metadata] : m_AssetRegistry)
			{
				// Ensure names do not match inside asset registry
				if (metadata.m_FileLocation.stem().string() == newFileName)
				{
					KG_WARN("Attempt to instantiate and {} asset whose name ({}) is already taken in the registry", k_Config.m_Name.CString(), newFileName);
					return Assets::k_EmptyHandle;
				}
			}

			// Create New Asset/Handle
			AssetHandle newHandle{};
			Metadata newMetadata{};
			newMetadata.m_Handle = newHandle;
			newMetadata.m_TypeIdentifier = k_Config.m_Identifier;

			// Create asset file inside asset manager
			if constexpr (k_HasFileLocation)
			{
				newMetadata.m_FileLocation = Utility::FileSystem::ConvertToUnixStylePath(Utility::FileSystem::GetRelativePath(paths.GetAssetDirectory(), destinationPath / (newFileName + k_Config.m_FileExtension)));
				CreateAssetFromName(newMetadata, newFileName, paths.GetAssetDirectory() / newMetadata.m_FileLocation);
			}

			// Check if intermediates are used. If so, generate the intermediate.
			if constexpr (k_HasIntermediateLoc)
			{
				std::filesystem::path registryPath{ k_Config.m_RegistryPath };
				newMetadata.m_IntermediateLocation = Utility::FileSystem::ConvertToUnixStylePath(registryPath.parent_path() / ((std::string)newMetadata.m_Handle + k_Config.m_IntermediateExtension.CString()));
				CreateAssetFromFile(newMetadata, sourcePath, paths.GetIntermediateDirectory() / newMetadata.m_IntermediateLocation);
				newMetadata.m_CheckSum = currentCheckSum;
			}
			else
			{
				KG_ERROR("Attempt to import a file that does not generate an intermediate. I have not decided what happens in this case.");
			}


			std::filesystem::path assetPath;
			if constexpr (k_HasIntermediateLoc)
			{
				assetPath = paths.GetIntermediateDirectory() / newMetadata.m_IntermediateLocation;
			}
			else if (k_HasFileLocation)
			{
				assetPath = paths.GetAssetDirectory() / newMetadata.m_FileLocation;
			}
			else
			{
				KG_ERROR("Attempt to import a file that does not have a specified file nor intermediate location");
			}


			// Add new asset into asset registry
			m_AssetRegistry.insert({ newHandle, newMetadata });
			SerializeAssetRegistry();

			// Fill in-memory cache if appropriate
			if (k_HasAssetCache)
			{
				m_AssetCache.insert({ newHandle, DeserializeAsset(newMetadata, assetPath) });
			}

			Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>
			(
				newHandle, 
				k_Config.m_Identifier, 
				Events::ManageAssetAction::Create
			);
			EngineService::GetActiveEngine().GetThread().SubmitEvent(event);
			return newHandle;
		}

		template<AssetConcept t_AssetType>
		void SerializeAssetRegistry()
		{
			// Get registry path
			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			std::filesystem::path registryPath{ GetAssetRegistryPath<t_AssetType>() };
			
			// Set up serializer
			YAML::Emitter serializer;
			serializer << YAML::BeginMap;
			serializer << YAML::Key << "Registry" << YAML::Value << t_AssetType::GetAssetName().CString();

			// Serialize other registry specific data
			if constexpr (HasRegistryData<t_AssetType>)
			{
				SerializeRegistrySpecificData<t_AssetType>(serializer);
			}

			// Asset
			serializer << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;
			for (auto& [handle, metadata] : m_AssetRegistry)
			{
				serializer << YAML::BeginMap; // Asset Map
				serializer << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);

				serializer << YAML::Key << "MetaData" << YAML::Value;
				serializer << YAML::BeginMap; // MetaData Map
				serializer << YAML::Key << "Name" << YAML::Value << metadata.m_Name;
				serializer << YAML::Key << "CheckSum" << YAML::Value << metadata.m_CheckSum;
				if constexpr (HasFileLocation<t_AssetType>)
				{
					serializer << YAML::Key << "FileLocation" << YAML::Value << metadata.m_FileLocation.string();
				}
				if constexpr (HasIntermediates<t_AssetType>)
				{
					serializer << YAML::Key << "IntermediateLocation" << YAML::Value << GetAssetIntermediatePath<t_AssetType>();
				}
				serializer << YAML::Key << "AssetType" << YAML::Value << metadata.m_TypeIdentifier;

				SerializeAssetSpecificMetadata<t_AssetType>(serializer, metadata);
				
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
			// Clear current registry
			m_AssetRegistry.clear();

			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			std::filesystem::path registryPath = paths.GetIntermediateDirectory() / k_Config.m_RegistryPath.CString();

			if (!Utility::FileSystem::PathExists(registryPath))
			{
				KG_WARN("No .kgreg file found at provided registry path {}. Creating a new one.", registryPath.string());
				SerializeAssetRegistry<t_AssetType>();
			}
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

			const std::string registryName = data["Registry"].as<std::string>();
			KG_INFO("Deserializing {} Registry", registryName);

			// Open registry specific data
			DeserializeRegistrySpecificData(data);

			// Opening all assets 
			YAML::Node assets = data["Assets"];
			if (assets)
			{
				for (const YAML::Node& asset : assets)
				{
					Metadata newMetadata{};
					newMetadata.m_Handle = asset["AssetHandle"].as<uint64_t>();

					// Retrieving metadata for asset 
					YAML::Node metadataNode = asset["MetaData"];
					newMetadata.m_Name = metadataNode["Name"].as<std::string>();
					newMetadata.m_CheckSum = metadataNode["CheckSum"].as<std::string>();
					newMetadata.m_TypeIdentifier = metadataNode["AssetType"].as<std::string>();
					if (k_HasFileLocation)
					{
						newMetadata.m_FileLocation = metadataNode["FileLocation"].as<std::string>();
					}
					if (k_HasIntermediateLoc)
					{
						newMetadata.m_IntermediateLocation = metadataNode["IntermediateLocation"].as<std::string>();
					}

					// Open registry specific metadata
					DeserializeAssetSpecificMetadata(metadataNode, newMetadata);

					// Add asset to in memory registry 
					m_AssetRegistry.insert({ newMetadata.m_Handle, newMetadata });

				}
			}
		}

		void LoadAllAssetIntoCache()
		{
			// Ensure the current asset type supports caching
			static_assert(k_HasAssetCache);

			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			// Revalidate active registry
			DeserializeAssetRegistry();

			// Load every asset into memory
			for (auto& [handle, metadata] : m_AssetRegistry)
			{
				// TODO: Skip already loaded assets. Maybe add an option for clearing the asset cache first

				// Get the path to the underlying file
				std::filesystem::path assetPath;
				if (m_AssetCache.contains(handle))
				{
					continue;
				}

				if constexpr (k_HasIntermediateLoc)
				{
					assetPath = paths.GetIntermediateDirectory() / metadata.m_IntermediateLocation;
				}
				else if (k_HasFileLocation)
				{
					assetPath = paths.GetAssetDirectory() / metadata.m_FileLocation;
				}
				else
				{
					KG_ERROR("Failed to clear");
				}

				AssetReference<t_AssetType> newAsset = DeserializeAsset(metadata, assetPath);

				// Insert the asset into the cache
				m_AssetCache.insert({ handle, newAsset });
				
			}
		}

		bool SetAssetFileLocation(AssetHandle handle, const std::filesystem::path& newFileLocation)
		{
			// Validate asset exists
			if (!m_AssetRegistry.contains(handle))
			{
				KG_WARN("Could not locate provided asset when attempting to its update file location");
				return false;
			}

			// Ensure file path is not absolute
			if (newFileLocation.is_absolute())
			{
				KG_WARN("Attempt to update an asset's file location with a path that is absolute. The filepath should be relative.");
				return false;
			}

			// Ensure path provided is a file
			if (!Utility::FileSystem::HasFileExtension(newFileLocation))
			{
				KG_WARN("Attempt to update an asset's file location with a path that does not seem to be a file");
				return false;
			}

			// Get asset
			Metadata& metadata = m_AssetRegistry.at(handle);

			std::filesystem::path existingAssetExtension = metadata.m_FileLocation.extension();
			std::filesystem::path existingAssetName = metadata.m_FileLocation.stem();
			std::filesystem::path newFileLocationExtension = newFileLocation.extension();
			std::filesystem::path newFileLocationName = newFileLocation.stem();

			// Ensure extension does not change
			if (newFileLocationExtension != existingAssetExtension)
			{
				KG_WARN("Attempt to update an asset's file location with a path whose extension does not match the asset type");
				return false;
			}

			// Ensure file name does not change
			if (newFileLocationName != existingAssetName)
			{
				KG_WARN("Attempt to update an asset's file location with a path whose name does not match the original");
				return false;
			}

			// Update file location
			metadata.m_FileLocation = Utility::FileSystem::ConvertToUnixStylePath(newFileLocation);

			// Save changes to disk
			SerializeAssetRegistry();

			// Throw update asset event
			Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>
			(
				handle, 
				metadata.m_TypeIdentifier, 
				Events::ManageAssetAction::UpdateAssetInfo
			);
			EngineService::GetActiveEngine().GetThread().SubmitEvent(event);
			return true;
		}

		AssetRegistry& GetAssetRegistry()
		{
			return m_AssetRegistry;
		}

		AssetCache<t_AssetType>& GetAssetCache()
		{
			return m_AssetCache;
		}

		size_t GetAssetRegistrySize()
		{
			return m_AssetRegistry.size();
		}

		AssetHandle GetAssetHandleFromFileLocation(const std::filesystem::path& queryFileLocation)
		{
			static_assert(k_HasFileLocation,
				"Attempt to retrieve an asset handle using a file location when this asset type does not support storing file locations.");

			// Search for game state inside registry
			for (auto& [handle, metadata] : m_AssetRegistry)
			{
				if (metadata.m_FileLocation == queryFileLocation)
				{
					return handle;
				}
			}

			// If could not find asset, return null
			return Assets::k_EmptyHandle;
		}

		const ImportExtensionList& GetAssetValidImportExtensions()
		{
			return k_Config.m_ImportExtensions;
		}

		template<AssetConcept t_AssetType>
		AssetReference<t_AssetType> DeserializeAsset(Metadata& metadata)
		{
			constexpr bool k_HasAssetCache{ t_AssetType::GetAssetFlags().IsFlagSet(AssetFlag::HasAssetCache) };

			// Check if asset already exists
			t_AssetType* newAsset{ nullptr };
			bool assetExists{ false };
			if constexpr (k_HasAssetCache)
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
				&metadata, 
				GetAssetFullFilePath<t_AssetType>(metadata),
				GetAssetIntermediateFolder<t_AssetType>(metadata)
			};
			newAsset->Deserialize((void*)(&deserializeContext));

			// Return asset reference
			return { metadata.m_Handle, LoadState::Loaded, newAsset };
		}

		template<AssetConcept t_AssetType>
		void SerializeAsset(Metadata& metadata, AssetReference<t_AssetType> assetReference)
		{
			// Ensure asset type supports serialization
			KG_ASSERT(assetReference.IsValid() && !assetReference.IsEmpty(), "Attempt to serialize an invalid asset reference");

			// Get the actual asset
			t_AssetType& asset{ assetReference.GetAsset() };

			// Serialize asset
			SerializeAssetContext serializeContext
			{
				GetAssetFullFilePath<t_AssetType>(metadata),
				GetAssetIntermediateFolder<t_AssetType>(metadata)
			};
			asset.Serialize((void*)(&serializeContext));
		}

		void DeserializeRegistrySpecificData(YAML::Node& registryNode)
		{
			// Ensure asset type supports registry specific data
			static_assert(HasRegistryData<t_AssetType>);

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

		void DeserializeAssetSpecificMetadata(YAML::Node& node, Metadata& metadata)
		{
			// Ensure asset type supports metadata deserialization
			static_assert(HasMetadata<t_AssetType>);

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

		template<AssetConcept t_AssetType>
		void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Metadata& metadata)
		{
			// Ensure asset type supports metadata serialization
			static_assert(HasMetadata<t_AssetType>);

			// Get specific metadata
			typename t_AssetType::Metadata* specificMetadata = metadata.GetSpecificMetaData<typename t_AssetType::Metadata>();
			KG_ASSERT(specificMetadata);

			// Serialize specific metadata
			SerializeMetaDataContext context{ &serializer, &metadata };
			specificMetadata->Serialize((void*)&context);
		}

		Ref<void> SaveAssetValidation(AssetReference<t_AssetType> newAsset) 
		{
			// Ensure asset type supports save validation
			static_assert(HasSaveValidation<t_AssetType>);

			// Ensure asset reference is valid
			KG_ASSERT(newAsset.IsValid() && !newAsset.IsEmpty(), "New asset reference is invalid when validating changes");

			// Get relevant data
			AssetReference<t_AssetType> oldAssetRef = GetAsset(newAsset.GetHandle());
			KG_ASSERT(newAsset.IsValid() && !newAsset.IsEmpty(), "Old asset reference is invalid when validating changes");
			t_AssetType* oldAsset = &oldAssetRef.GetAsset();
			Metadata& metadata{ GetMetadata(newAsset.GetHandle()); };

			// Validate asset
			return oldAsset->SaveValidation(newAsset, metadata);
		};

		template<AssetConcept t_AssetType> requires HasDeleteValidation<t_AssetType>
		void DeleteAssetValidation(AssetHandle assetHandle) 
		{
			// Get asset
			AssetReference<t_AssetType> assetReference{ GetAsset<t_AssetType>(assetHandle) };

			// Validate asset
			KG_ASSERT(assetReference.IsValid() && !assetReference.IsEmpty(), "Attempt to validate an invalid asset reference");

			t_AssetType& asset = assetReference.GetAsset();
			Metadata& metadata{ GetMetadata(assetReference.GetHandle()); };

			asset.DeleteValidation(metadata);
		};

		template<AssetConcept t_AssetType> requires HasCreationFromName<t_AssetType>
		void CreateAssetFromName(std::string_view name, Metadata metadata, std::filesystem::path& assetPath)
		{
			// Create asset file
			t_AssetType::CreateAssetFromName(metadata, name, assetPath);
		};

		void CreateAssetFromFile(Metadata& metadata, std::filesystem::path& sourcePath, std::filesystem::path& assetPath) 
		{
			// Ensure asset type supports creation from name
			static_assert(HasCreationFromFile<t_AssetType>);

			// Create asset file
			t_AssetType::CreateAssetFromFile(metadata, sourcePath, assetPath);
		};

		std::filesystem::path CreateNewAssetPath(const std::filesystem::path& creationDirectory,
			const char* assetName, const char* assetExtension)
		{
			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			std::filesystem::path pathRelativeToAssetDir
			{ 
				Utility::FileSystem::GetRelativePath(paths.GetAssetDirectory(), creationDirectory) 
			};

			return Utility::FileSystem::ConvertToUnixStylePath(
				pathRelativeToAssetDir / (assetName + std::string(assetExtension)));
		}

		template<AssetConcept t_AssetType>
		std::filesystem::path GetAssetFullFilePath(Metadata& metadata)
		{
			if constexpr (!HasFileLocation<t_AssetType>)
			{
				return {};
			}

			Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			KG_ASSERT(!metadata.m_FileLocation.empty());
			return projectPaths.GetAssetDirectory() / metadata.m_FileLocation;
		}

		template<AssetConcept t_AssetType> requires HasFileLocation<t_AssetType>
		std::filesystem::path GetAssetFilePath(AssetHandle handle)
		{
			if (!m_AssetRegistry.contains(handle))
			{
				KG_WARN("Could not locate {} asset when attempting to retrieve it's file location using {} handle", k_Config.m_Name, handle);
				return {};
			}
			return m_AssetRegistry[handle].m_FileLocation;
		}

		template<AssetConcept t_AssetType>
		std::filesystem::path GetAssetIntermediatePath(Metadata& metadata)
		{
			if constexpr (!HasIntermediates<t_AssetType>)
			{
				return {};
			}

			KG_ASSERT(!metadata.m_Name.IsEmpty());

			std::stringstream pathWithoutExtension;
			pathWithoutExtension << GetModuleName<t_AssetType>() << "/" << 
				GetTypeName<t_AssetType>() << "/" << metadata.m_Name;
			return { pathWithoutExtension.str() };
		}

		template<AssetConcept t_AssetType>
		std::filesystem::path GetAssetFullIntermediatePath(Metadata& metadata)
		{
			if constexpr (!HasIntermediates<t_AssetType>)
			{
				return {};
			}

			Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			KG_ASSERT(!metadata.m_Name.IsEmpty());

			std::stringstream pathWithoutExtension;
			pathWithoutExtension << projectPaths.GetIntermediateDirectory().string() << 
				GetModuleName<t_AssetType>() << "/" << GetTypeName<t_AssetType>()
				<< "/" << metadata.m_Name;
			return { pathWithoutExtension.str() };
		}

		template<AssetConcept t_AssetType>
		std::filesystem::path GetAssetRegistryPath()
		{
			if constexpr (!HasIntermediates<t_AssetType>)
			{
				return;
			}

			Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

			std::stringstream path;
			path << projectPaths.GetIntermediateDirectory().string() <<
				GetModuleName<t_AssetType>() << "/" << GetTypeName<t_AssetType>()
				<< "/" << GetTypeName<t_AssetType>() << "Registry" << ".kgreg";
			return path.str();
		}

		template<AssetConcept t_AssetType>
		void DeleteAssetFiles(Metadata& metadata)
		{
			// Delete the asset's data on-disk
			if constexpr (HasFileLocation<t_AssetType>)
			{
				std::filesystem::path fileLocation{ GetAssetFullFilePath<t_AssetType>(metadata) };
				Utility::FileSystem::DeleteSelectedFile(fileLocation);
			}

			if constexpr (HasIntermediates<t_AssetType>)
			{
				std::filesystem::path intermediateFolder{ GetAssetIntermediateFolder<t_AssetType>(metadata) };

				std::span<FixedBufStr16> allIntermediateExtensions{ t_AssetType::GetIntermediateExtensions };
				for (const FixedBufStr16& extension : allIntermediateExtensions)
				{
					intermediateFolder.replace_extension(extension.CString());
					Utility::FileSystem::DeleteSelectedFile(intermediateLocation);
				}
			}
		}

		template<AssetConcept t_AssetType>
		void RemoveAssetDataFromRegistry(AssetHandle handle)
		{
			constexpr bool k_HasAssetCache{ t_AssetType::GetAssetFlags().IsFlagSet(AssetFlag::HasAssetCache) };

			// Delete in-memory copy of this asset
			if constexpr (k_HasAssetCache)
			{
				if (m_AssetCache.contains(handle))
				{
					// Get the relevant asset data
					AssetGenericData& assetData{ m_AssetCache.at(handle)};
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

		bool ValidateCreationDirectory(const std::filesystem::path& creationDirectory)
		{
			Projects::ProjectPaths& paths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

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
		}

		Metadata CreateAssetMetadata(AssetIdentifier identifier, const std::filesystem::path& assetPath)
		{
			// Generate new ID
			AssetHandle newHandle{ RandomUUIDService::GetRandomUUID() };

			// Create basic metadata
			Metadata newMetadata{};
			newMetadata.m_Handle = newHandle;
			newMetadata.m_TypeIdentifier = identifier;
			newMetadata.m_FileLocation = assetPath;
			return newMetadata;
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
