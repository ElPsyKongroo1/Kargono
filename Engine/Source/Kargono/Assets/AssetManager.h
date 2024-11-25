#pragma once
#include "Kargono/Core/Base.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Events/AssetEvent.h"

#include "API/Serialization/yamlcppAPI.h"

#include <bitset>
#include <tuple>

// Functions to call

// Virtual functions to inject functionality


//==============================
// Manage Registry
//==============================

// DeserializeRegistry
// SerializeRegistry
// Clear Registry
// GetAssetRegistry()

//==============================
// Import Asset
//==============================

// ImportNewAssetFromFile
// ImportNewAssetFromData()

//==============================
// Read Asset from Registry
//==============================

// GetAsset(handle)

enum AssetManagerOptions : uint8_t
{
	None = 0, // Default value
	HasAssetCache = 1, // Store cache of the filetype in runtime memory for easy reuse
	HasIntermediateLocation = 2, // Specify that this asset manager generates an intermediate file to be stored in the project's Intermediates directory
	HasFileLocation = 3, // Specify that this asset manager stores a file somewhere in the project's Assets directory
	HasFileImporting = 4, // Specify that this asset manager is capable of importing the asset into the system from an external file
	HasAssetSaving = 5, // Specify that this asset manager is capable of saving to the underlying file data for it's type of asset
	HasAssetCreationFromName = 6 // Specify that this asset manager is capable of creating the underlying file data for it's type of asset
};

namespace Kargono::Assets
{
	using AssetRegistry = std::unordered_map<AssetHandle, Assets::AssetInfo>;

	template <typename AssetValue>
	class AssetManager
	{
	public:

		AssetInfo GetAssetInfo(AssetHandle handle)
		{
			if (!m_AssetRegistry.contains(handle))
			{
				return {};
			}

			return m_AssetRegistry.at(handle);
		}

		Ref<AssetValue> GetAsset(AssetHandle handle)
		{
			KG_ASSERT(Projects::ProjectService::GetActive(), "There is no active project when retrieving asset!");

			if (m_Flags.test(AssetManagerOptions::HasAssetCache))
			{
				if (m_AssetCache.contains(handle))
				{
					return m_AssetCache[handle];
				}
			}

			if (m_AssetRegistry.contains(handle))
			{
				auto asset = m_AssetRegistry[handle];
				std::filesystem::path assetPath = 
					(m_Flags.test(AssetManagerOptions::HasIntermediateLocation) ? 
						Projects::ProjectService::GetActiveIntermediateDirectory() / asset.Data.IntermediateLocation : 
						Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.FileLocation);
				Ref<AssetValue> newAsset = DeserializeAsset(asset, assetPath);
				if (m_Flags.test(AssetManagerOptions::HasAssetCache))
				{
					m_AssetCache.insert({ asset.Handle, newAsset });
				}
				return newAsset;
			}

			//TODO: Maybe put back
			//KG_WARN("No {} asset is associated with {} handle. Returning an empty asset reference", m_AssetName, handle);
			return nullptr;
		}

		std::tuple<AssetHandle, Ref<AssetValue>> GetAsset(const std::filesystem::path& fileLocation)
		{
			KG_ASSERT(Projects::ProjectService::GetActive(), "Attempt to use Project Field without active project!");
			KG_ASSERT(m_Flags.test(AssetManagerOptions::HasFileLocation), 
				"Attempt to retrieve a asset using a file location when this asset type does not support storing file locations");

			std::filesystem::path assetPath = fileLocation;

			if (fileLocation.is_absolute())
			{
				assetPath = Utility::FileSystem::GetRelativePath(Projects::ProjectService::GetActiveAssetDirectory(), fileLocation);
			}

			for (auto& [assetHandle, asset] : m_AssetRegistry)
			{
				if (asset.Data.FileLocation.compare(assetPath) == 0)
				{
					return { assetHandle, GetAsset(assetHandle) };
				}
			}
			// Return empty asset if the asset is not found in the registry
			KG_WARN("Invalid filepath provided to GetAsset(filepath) {}. Returning empty {} asset.", fileLocation.string(), m_AssetName);
			return {Assets::EmptyHandle, nullptr};
		}
		std::filesystem::path GetAssetFileLocation(AssetHandle handle)
		{
			KG_ASSERT(m_Flags.test(AssetManagerOptions::HasFileLocation),
				"Attempt to retrieve an asset's file location when none is support by the defined asset type.");
			if (!m_AssetRegistry.contains(handle))
			{
				KG_WARN("Could not locate {} asset when attempting to retrieve it's file location using {} handle", m_AssetName, handle);
				return std::filesystem::path();
			}
			return m_AssetRegistry[handle].Data.FileLocation;
		}

		std::filesystem::path GetAssetIntermediateLocation(AssetHandle handle)
		{
			KG_ASSERT(m_Flags.test(AssetManagerOptions::HasIntermediateLocation), 
				"Attempt to retrieve asset intermediate when none is generated for defined type");
			if (!m_AssetRegistry.contains(handle))
			{
				KG_WARN("Could not locate {} asset when attempting to retrieve it's intermediate location with {} handle", m_AssetName, handle);
				return std::filesystem::path();
			}
			return m_AssetRegistry[handle].Data.IntermediateLocation;
		}


		bool HasAsset(AssetHandle handle)
		{
			return m_AssetRegistry.contains(handle);
		}

		// TODO: Redo this API, I hate it
		bool HasAsset(const std::string& assetName)
		{
			KG_ASSERT(m_Flags.test(AssetManagerOptions::HasFileLocation), 
				"Attempt to query the state of an asset using an asset name when asset type does not support using a file location.");

			// Check for a matching name
			for (const auto& [handle, asset] : m_AssetRegistry)
			{
				if (asset.Data.FileLocation.stem() == assetName)
				{
					return true;
				}
			}

			// Exit if no match is found
			return false;
		}

		void SaveAsset(AssetHandle assetHandle, Ref<AssetValue> assetReference)
		{
			KG_ASSERT(m_Flags.test(AssetManagerOptions::HasAssetSaving), "Attempt to save an asset who's type does not support data modification");

			// Ensure handle exists inside registry
			if (!m_AssetRegistry.contains(assetHandle))
			{
				KG_WARN("Attempt to save asset of type {} that does not exist in the asset registry", m_AssetName);
				return;
			}

			// Provide asset specific validation
			Ref<void> providedData = SaveAssetValidation(assetReference , assetHandle);

			// Find location of asset's data
			Assets::AssetInfo& asset = m_AssetRegistry[assetHandle];
			std::filesystem::path dataLocation {};
			if (m_Flags.test(AssetManagerOptions::HasIntermediateLocation))
			{
				dataLocation = Projects::ProjectService::GetActiveIntermediateDirectory() / asset.Data.IntermediateLocation;
			}
			else if (m_Flags.test(AssetManagerOptions::HasFileLocation))
			{
				dataLocation = Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.FileLocation;
			}
			else
			{
				KG_ASSERT(false, "Attempt to save an asset that does not have a specified file nor intermediate location");
			}

			// Update in memory asset if applicable
			if (m_Flags.test(AssetManagerOptions::HasAssetCache))
			{
				m_AssetCache.at(assetHandle) = assetReference;
			}

			// Save asset data on-disk
			SerializeAsset(assetReference, dataLocation);

			// Get and update checksum
			const std::string currentCheckSum = Utility::FileSystem::ChecksumFromFile(dataLocation);

			// Ensure checksum is valid
			if (currentCheckSum.empty())
			{
				KG_WARN("Generated empty checksum while saving an asset");
			}
			asset.Data.CheckSum = currentCheckSum;

			Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>(assetHandle, asset.Data.Type, Events::ManageAssetAction::UpdateAsset, providedData);
			EngineService::SubmitToEventQueue(event);
		}

		bool DeleteAsset(AssetHandle assetHandle)
		{
			if (!m_AssetRegistry.contains(assetHandle))
			{
				KG_WARN("Failed to delete {} asset. Asset was not found in registry.");
				return false;
			}

			// Find location of asset's data
			Assets::AssetInfo& asset = m_AssetRegistry[assetHandle];
			
			// Process delete event and validation
			Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>(assetHandle, asset.Data.Type, Events::ManageAssetAction::Delete);
			DeleteAssetValidation(assetHandle);
			EngineService::OnEvent(event.get());

			// Delete the asset's data on-disk
			if (m_Flags.test(AssetManagerOptions::HasIntermediateLocation))
			{
				std::filesystem::path intermediateLocation = Projects::ProjectService::GetActiveIntermediateDirectory() / asset.Data.IntermediateLocation;
				Utility::FileSystem::DeleteSelectedFile(intermediateLocation);
			}
			if (m_Flags.test(AssetManagerOptions::HasFileLocation))
			{
				std::filesystem::path fileLocation = Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.FileLocation;
				Utility::FileSystem::DeleteSelectedFile(fileLocation);
			}
			
			// Delete the asset inside the registry
			m_AssetRegistry.erase(assetHandle);

			// Delete in-memory copy of this asset
			if (m_Flags.test(AssetManagerOptions::HasAssetCache))
			{
				m_AssetCache.erase(assetHandle);
			}

			// Save the modified registry to disk
			SerializeAssetRegistry();

			return true;
		}

		void ClearAssetRegistry()
		{
			if (m_Flags.test(AssetManagerOptions::HasAssetCache))
			{
				m_AssetCache.clear();
			}
			m_AssetRegistry.clear();
		}

		AssetHandle CreateAsset(const char* assetName, const std::filesystem::path& creationPath)
		{
			KG_ASSERT(m_Flags.test(AssetManagerOptions::HasAssetCreationFromName), "Attempt to save an asset who's type does not support data creation");
			
			bool usingBaseAssetDir{true};
			if (creationPath == Projects::ProjectService::GetActiveAssetDirectory())
			{
				usingBaseAssetDir = true;
			}
			else
			{
				usingBaseAssetDir = false;
				// Ensure provided path is within the assets directory
				if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveAssetDirectory(), creationPath))
				{
					KG_WARN("Provided path for new asset creation is not within asset directory");
					return Assets::EmptyHandle;
				}

				// Ensure provided path is not indicating a file
				if (Utility::FileSystem::HasFileExtension(creationPath))
				{
					KG_WARN("File provided as path to asset. Creation paths should only indicate a directory");
					return Assets::EmptyHandle;
				}

				// Create path if it does not already exist
				Utility::FileSystem::CreateNewDirectory(creationPath);
			}

			// Create New Asset/Handle
			AssetHandle newHandle{};
			Assets::AssetInfo newAsset{};
			newAsset.Handle = newHandle;
			newAsset.Data.Type = m_AssetType;
			if (usingBaseAssetDir)
			{
				newAsset.Data.FileLocation = Utility::FileSystem::ConvertToUnixStylePath(assetName + m_FileExtension);
			}
			else
			{
				newAsset.Data.FileLocation = Utility::FileSystem::ConvertToUnixStylePath(Utility::FileSystem::GetRelativePath(Projects::ProjectService::GetActiveAssetDirectory(), creationPath) / (assetName + m_FileExtension));
			}

			// TODO: Fixme, this is temporary since a code path that uses an intermediate location is not yet necessary
			KG_ASSERT(!m_Flags.test(AssetManagerOptions::HasIntermediateLocation), "Code path for intermediates is not yet supported");

			// Create File
			CreateAssetFileFromName(assetName, newAsset, Projects::ProjectService::GetActiveAssetDirectory() / newAsset.Data.FileLocation);

			// Create Checksum
			const std::string currentCheckSum = Utility::FileSystem::ChecksumFromFile(Projects::ProjectService::GetActiveAssetDirectory() / newAsset.Data.FileLocation);

			// Ensure checksum is valid
			if (currentCheckSum.empty())
			{
				KG_WARN("Generated empty checksum from the string {}", assetName);
				return Assets::EmptyHandle;
			}
			newAsset.Data.CheckSum = currentCheckSum;

			// Register New Asset and return handle.
			m_AssetRegistry.insert({ newHandle, newAsset });
			SerializeAssetRegistry();

			// Fill in-memory cache if appropriate
			if (m_Flags.test(AssetManagerOptions::HasAssetCache))
			{
				m_AssetCache.insert({ newHandle, DeserializeAsset(newAsset, Projects::ProjectService::GetActiveAssetDirectory() / newAsset.Data.FileLocation) });
			}

			Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>(newHandle, newAsset.Data.Type, Events::ManageAssetAction::Create);
			EngineService::SubmitToEventQueue(event);
			return newHandle;
		}

		AssetHandle CreateAsset(const char* assetName)
		{
			return CreateAsset(assetName, Projects::ProjectService::GetActiveAssetDirectory());
		}

		AssetHandle ImportAssetFromFile(const std::filesystem::path& sourcePath)
		{
			if (!Utility::FileSystem::HasFileExtension(sourcePath))
			{
				KG_WARN("Cannot import provided file path. Path does not contain a file extension. (i.e. it is not a file)");
				return Assets::EmptyHandle;
			}

			return ImportAssetFromFile(sourcePath, sourcePath.stem().string().c_str(), sourcePath.parent_path());
		}

		AssetHandle ImportAssetFromFile(const std::filesystem::path& sourcePath, const char* newFileName, const std::filesystem::path& destinationPath)
		{
			KG_ASSERT(m_Flags.test(AssetManagerOptions::HasFileImporting), "Attempt to import an asset for a file type that does not support importing");

			// Ensure provided name is valid
			if (!newFileName || newFileName[0] == '\0')
			{
				KG_WARN("Empty/invalid name provided to import asset from file function");
				return Assets::EmptyHandle;
			}

			// Ensure source path is valid
			if (!Utility::FileSystem::HasFileExtension(sourcePath))
			{
				KG_WARN("Cannot import provided file path. Path does not contain a file extension. (i.e. it is not a file)");
				return Assets::EmptyHandle;
			}

			// Check if source path file extension is appropriate for this file type
			bool foundValidExtension{ false };
			for (auto& extension : m_ValidImportFileExtensions)
			{
				if (sourcePath.extension().string() == extension)
				{
					foundValidExtension = true;
					break;
				}
			}

			// Exit if extension is invalid
			if (!foundValidExtension)
			{
				KG_WARN("Invalid file extension ({}) for {} asset type. Valid file extensions for {} include: ");
				for (auto& extension : m_ValidImportFileExtensions)
				{
					KG_WARN("  {}", extension);
				}
				return Assets::EmptyHandle;
			}

			// Validate provided paths
			bool exportingToBaseAssetDir{ true };
			if (destinationPath == Projects::ProjectService::GetActiveAssetDirectory())
			{
				exportingToBaseAssetDir = true;
			}
			else
			{
				exportingToBaseAssetDir = false;
				// Ensure provided path is within the assets directory
				if (!Utility::FileSystem::DoesPathContainSubPath(Projects::ProjectService::GetActiveAssetDirectory(), destinationPath))
				{
					KG_WARN("Provided path for new asset importation is not within asset directory");
					return Assets::EmptyHandle;
				}

				// Ensure provided path is not indicating a file
				if (Utility::FileSystem::HasFileExtension(destinationPath))
				{
					KG_WARN("File provided as path to asset. Destination paths should only indicate a directory");
					return Assets::EmptyHandle;
				}

				// Create path if it does not already exist
				Utility::FileSystem::CreateNewDirectory(destinationPath);
			}


			// Create Checksum
			const std::string currentCheckSum = Utility::FileSystem::ChecksumFromFile(sourcePath);

			// Ensure checksum is valid
			if (currentCheckSum.empty())
			{
				KG_WARN("Generated empty checksum from file at {}", sourcePath.string());
				return Assets::EmptyHandle;
			}

			// Ensure duplicate asset is not found in registry.
			for (const auto& [handle, asset] : m_AssetRegistry)
			{
				// Ensure names do not match inside asset registry
				if (asset.Data.FileLocation.stem().string() == newFileName)
				{
					KG_WARN("Attempt to instantiate and {} asset whose name ({}) is already taken in the registry", m_AssetName, newFileName);
					return Assets::EmptyHandle;
				}
			}

			// Create New Asset/Handle
			AssetHandle newHandle{};
			Assets::AssetInfo newAsset{};
			newAsset.Handle = newHandle;
			newAsset.Data.Type = m_AssetType;

			// Create asset file inside asset manager
			if (m_Flags.test(AssetManagerOptions::HasFileLocation))
			{
				newAsset.Data.FileLocation = Utility::FileSystem::ConvertToUnixStylePath(Utility::FileSystem::GetRelativePath(Projects::ProjectService::GetActiveAssetDirectory(), destinationPath / (newFileName + m_FileExtension)));
				CreateAssetFileFromName(newFileName, newAsset, Projects::ProjectService::GetActiveAssetDirectory() / newAsset.Data.FileLocation);
			}

			// Check if intermediates are used. If so, generate the intermediate.
			if (m_Flags.test(AssetManagerOptions::HasIntermediateLocation))
			{
				newAsset.Data.IntermediateLocation = Utility::FileSystem::ConvertToUnixStylePath(m_RegistryLocation.parent_path() / ((std::string)newAsset.Handle + m_IntermediateExtension.CString()));
				CreateAssetIntermediateFromFile(newAsset, sourcePath, Projects::ProjectService::GetActiveIntermediateDirectory() / newAsset.Data.IntermediateLocation);
				newAsset.Data.CheckSum = currentCheckSum;
			}
			else
			{
				KG_ERROR("Attempt to import a file that does not generate an intermediate. I have not decided what happens in this case.");
			}

			// TODO: Make sure to modify the code below if fixing the asset above
			std::filesystem::path assetPath =
				(m_Flags.test(AssetManagerOptions::HasIntermediateLocation) ?
					Projects::ProjectService::GetActiveIntermediateDirectory() / newAsset.Data.IntermediateLocation :
					Projects::ProjectService::GetActiveAssetDirectory() / newAsset.Data.FileLocation);

			// Add new asset into asset registry
			m_AssetRegistry.insert({ newHandle, newAsset });
			SerializeAssetRegistry();

			// Fill in-memory cache if appropriate
			if (m_Flags.test(AssetManagerOptions::HasAssetCache))
			{
				m_AssetCache.insert({ newHandle, DeserializeAsset(newAsset, assetPath) });
			}

			Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>(newHandle, newAsset.Data.Type, Events::ManageAssetAction::Create);
			EngineService::SubmitToEventQueue(event);
			return newHandle;
		}
		void SerializeAssetRegistry()
		{
			// Get registry path
			KG_ASSERT(Projects::ProjectService::GetActive(), "There is no currently loaded project when attempting to serialize an asset");
			const std::filesystem::path registryPath = Projects::ProjectService::GetActiveIntermediateDirectory() / m_RegistryLocation;
			
			// Set up serializer
			YAML::Emitter serializer;
			serializer << YAML::BeginMap;
			serializer << YAML::Key << "Registry" << YAML::Value << m_AssetName;

			// Serialize other registry specific data
			SerializeRegistrySpecificData(serializer);

			// Asset
			serializer << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;
			for (auto& [handle, asset] : m_AssetRegistry)
			{
				serializer << YAML::BeginMap; // Asset Map
				serializer << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);

				serializer << YAML::Key << "MetaData" << YAML::Value;
				serializer << YAML::BeginMap; // MetaData Map
				serializer << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
				if (m_Flags.test(AssetManagerOptions::HasFileLocation))
				{
					serializer << YAML::Key << "FileLocation" << YAML::Value << asset.Data.FileLocation.string();
				}
				if (m_Flags.test(AssetManagerOptions::HasIntermediateLocation))
				{
					serializer << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
				}
				serializer << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);

				SerializeAssetSpecificMetadata(serializer, asset);
				
				serializer << YAML::EndMap; // Close metadata map
				serializer << YAML::EndMap; // Close asset map
			}
			serializer << YAML::EndSeq; // Close asset sequence
			serializer << YAML::EndMap; // Close registry map

			Utility::FileSystem::CreateNewDirectory(registryPath.parent_path());

			std::ofstream fout(registryPath);
			fout << serializer.c_str();
		}

		void DeserializeAssetRegistry()
		{
			// Clear current registry
			m_AssetRegistry.clear();

			// Get on-disk registry path
			KG_ASSERT(Projects::ProjectService::GetActive(), "There is no currently loaded project when attempting to serialize an asset");
			const std::filesystem::path registryPath = Projects::ProjectService::GetActiveIntermediateDirectory() / m_RegistryLocation;

			if (!Utility::FileSystem::PathExists(registryPath))
			{
				KG_WARN("No .kgreg file found at provided registry path {}", registryPath.string());
				return;
			}
			YAML::Node data;
			try
			{
				data = YAML::LoadFile(registryPath.string());
			}
			catch (YAML::ParserException e)
			{
				KG_WARN("Failed to load {} file {}\n  {}", m_FileExtension , registryPath.string(), e.what());
				return;
			}
			// Opening registry node 
			if (!data["Registry"]) 
			{ 
				KG_WARN("Could not validate initial registry node for the file: {}", registryPath.string());
				return; 
			}

			std::string registryName = data["Registry"].as<std::string>();
			KG_INFO("Deserializing {} Registry", registryName);

			// Open registry specific data
			DeserializeRegistrySpecificData(data);

			// Opening all assets 
			auto assets = data["Assets"];
			if (assets)
			{
				for (auto asset : assets)
				{
					Assets::AssetInfo newAsset{};
					newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

					// Retrieving metadata for asset 
					auto metadata = asset["MetaData"];
					newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
					newAsset.Data.Type = Utility::StringToAssetType(metadata["AssetType"].as<std::string>());
					if (m_Flags.test(AssetManagerOptions::HasFileLocation))
					{
						newAsset.Data.FileLocation = metadata["FileLocation"].as<std::string>();
					}
					if (m_Flags.test(AssetManagerOptions::HasIntermediateLocation))
					{
						newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
					}

					// Open registry specific metadata
					DeserializeAssetSpecificMetadata(metadata, newAsset);

					// Add asset to in memory registry 
					m_AssetRegistry.insert({ newAsset.Handle, newAsset });

				}
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
			Assets::AssetInfo& currentAsset = m_AssetRegistry.at(handle);

			std::filesystem::path existingAssetExtension = currentAsset.Data.FileLocation.extension();
			std::filesystem::path existingAssetName = currentAsset.Data.FileLocation.stem();
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
			currentAsset.Data.FileLocation = Utility::FileSystem::ConvertToUnixStylePath(newFileLocation);

			// Save changes to disk
			SerializeAssetRegistry();

			// Throw update asset event
			Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>(handle, currentAsset.Data.Type, Events::ManageAssetAction::UpdateAssetInfo);
			EngineService::SubmitToEventQueue(event);
			return true;
		}

		std::unordered_map<AssetHandle, AssetInfo>& GetAssetRegistry()
		{
			return m_AssetRegistry;
		}

		std::unordered_map<AssetHandle, Ref<AssetValue>>& GetAssetCache()
		{
			return m_AssetCache;
		}

		AssetHandle GetAssetHandleFromFileLocation(const std::filesystem::path& queryFileLocation)
		{
			// Search for game state inside registry
			for (auto& [handle, asset] : m_AssetRegistry)
			{
				if (asset.Data.FileLocation == queryFileLocation)
				{
					return handle;
				}
			}

			// If could not find asset, return null
			return Assets::EmptyHandle;
		}

		

		const std::vector<std::string>& GetAssetValidImportExtensions()
		{
			return m_ValidImportFileExtensions;
		}

		virtual void SerializeAsset(Ref<AssetValue> assetReference, const std::filesystem::path& assetPath) 
		{ 
			KG_ERROR("Attempt to serialize an asset that does not override the base class's implentation of SerializeAsset()");
		};

		virtual void CreateAssetFileFromName(const std::string& name, AssetInfo& asset, const std::filesystem::path& assetPath)
		{
			KG_ERROR("Attempt to create an asset from a name that does not override the base class's implmentation of CreateAssetFileFromName()");
		}

		virtual Ref<void> SaveAssetValidation(Ref<AssetValue> newAsset, AssetHandle assetHandle) { return nullptr; };
		virtual void DeleteAssetValidation(AssetHandle assetHandle) {};
		virtual Ref<AssetValue> DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath) = 0;
		virtual void SerializeRegistrySpecificData(YAML::Emitter& serializer) {};
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset) {};
		virtual void DeserializeRegistrySpecificData(YAML::Node& registryNode) {};
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset) {};
		virtual void CreateAssetIntermediateFromFile(AssetInfo& newAsset, const std::filesystem::path& fullFileLocation, const std::filesystem::path& fullIntermediateLocation) {};
		
	protected:
		std::string m_AssetName{ "Uninitialized Asset Name" };
		FixedString16 m_FileExtension { ".kgfile" };
		FixedString16 m_IntermediateExtension{ ".kgbinary" };
		AssetType m_AssetType{ AssetType::None };
		std::filesystem::path m_RegistryLocation{""};
		std::vector<std::string> m_ValidImportFileExtensions{};
		std::unordered_map<AssetHandle, Assets::AssetInfo> m_AssetRegistry{};
		std::unordered_map<AssetHandle, Ref<AssetValue>> m_AssetCache{};
		std::bitset<8> m_Flags {0b00000000};
	};
}
