#include "kgpch.h"

#include "Modules/Assets/AssetService.h"
#include "Modules/Assets/ShaderManager.h"
#include "Modules/Rendering/Shader.h"
#include "Modules/Rendering/ShaderBuilder.h"

#include "API/Serialization/yamlcppAPI.h"



namespace Kargono::Assets
{
	AssetHandle ShaderManager::CreateNewShader(const Rendering::ShaderSpecification& shaderSpec)
	{
		// Create Checksum
		auto [shaderSource, bufferLayout, uniformList] = Rendering::ShaderBuilder::BuildShader(shaderSpec);
		std::string currentCheckSum = Utility::FileSystem::SHA256HashFromString(shaderSource.c_str());

		// Ensure checksum is valid
		if (currentCheckSum.empty())
		{
			KG_WARN("Generated empty checksum from the string {}", shaderSource);
			return Assets::k_EmptyHandle;
		}

		// Ensure duplicate asset is not found in registry.
		for (const auto& [handle, asset] : m_AssetRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_WARN("Attempt to instantiate duplicate {} asset. Returning existing asset.", m_AssetName);
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{ RandomUUIDService::GetRandomUUID() };
		Assets::AssetInfo newAsset{};
		newAsset.m_Handle = newHandle;
		newAsset.Data.Type = m_AssetType;
		newAsset.Data.CheckSum = currentCheckSum;
		newAsset.Data.IntermediateLocation = m_RegistryLocation.parent_path() / (std::string)newHandle;

		// Create intermediate and save intermediate to disk
		CreateShaderIntermediate(shaderSource, newAsset, shaderSpec, bufferLayout, uniformList);

		// Register New Asset and Create Shader
		m_AssetRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeAssetRegistry(); // Update Registry File on Disk

		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths()};
		Ref<Kargono::Rendering::Shader> newShader = DeserializeAsset(newAsset, projectPaths.GetIntermediateDirectory() / newAsset.Data.IntermediateLocation);
		m_AssetCache.insert({ newHandle, newShader });

		Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>
		(
			newHandle, 
			AssetType::Shader, 
			Events::ManageAssetAction::Create
		);
		
		EngineService::GetActiveEngine().GetThread().SubmitEvent(event);

		return newHandle;
	}

	std::tuple<AssetHandle, Ref<Kargono::Rendering::Shader>> ShaderManager::GetShader(const Rendering::ShaderSpecification& shaderSpec)
	{
		for (const auto& [assetHandle, shaderRef] : m_AssetCache)
		{
			if (shaderRef->GetSpecification() == shaderSpec)
			{
				return std::make_tuple(assetHandle, shaderRef);
			}
		}

		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

		for (auto& [assetHandle, asset] : m_AssetRegistry)
		{
			Assets::ShaderMetaData metadata = *static_cast<Assets::ShaderMetaData*>(asset.Data.SpecificFileData.get());
			if (metadata.ShaderSpec == shaderSpec)
			{
				Ref<Kargono::Rendering::Shader> newShader = DeserializeAsset(asset, 
					projectPaths.GetIntermediateDirectory() / asset.Data.IntermediateLocation);
				m_AssetCache.insert({ asset.m_Handle, newShader });
				return std::make_tuple(assetHandle, newShader);
			}
		}

		// Create New Shader if none exists already
		AssetHandle newShaderHandle = CreateNewShader(shaderSpec);
		return std::make_tuple(newShaderHandle, GetAsset(newShaderHandle));

	}

	void ShaderManager::CreateShaderIntermediate(const Rendering::ShaderSource& shaderSource, Assets::AssetInfo& newAsset, const Rendering::ShaderSpecification& shaderSpec,
		const Rendering::InputBufferLayout& inputLayout, const Rendering::UniformBufferList& uniformLayout)
	{
#if defined(KG_EXPORT_SERVER) || defined(KG_EXPORT_RUNTIME)
		KG_ERROR("Attempt to create/compile new shader during runtime!");
#endif
		// Create Shader Binary
		auto shaderSources = Utility::PreProcess(shaderSource);

		std::unordered_map<GLenum, std::vector<uint32_t>> openGLSPIRV;
#if !defined(KG_EXPORT_SERVER) && !defined(KG_EXPORT_RUNTIME)
		Utility::CompileBinaries(newAsset.m_Handle, shaderSources, openGLSPIRV);
#endif
		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

		// Save binary intermediates for all shader stages!
		for (const auto& [stage, source] : openGLSPIRV)
		{
			std::string intermediatePathWithExtension = newAsset.Data.IntermediateLocation.string() + Utility::ShaderBinaryFileExtension(stage);
			std::filesystem::path intermediateFullPath = projectPaths.GetIntermediateDirectory() / intermediatePathWithExtension;

			Utility::FileSystem::CreateNewDirectory(intermediateFullPath.parent_path());
			std::ofstream out(intermediateFullPath, std::ios::out | std::ios::binary);
			if (out.is_open())
			{
				// TODO: Add Checking to ensure output was successful!
				auto& data = source;
				out.write((char*)data.data(), data.size() * sizeof(uint32_t));
				out.flush();
				out.close();
			}
		}

		// File Location
		std::string debugString = shaderSource;
		std::filesystem::path shaderTextFile = projectPaths.GetIntermediateDirectory() / (newAsset.Data.IntermediateLocation.string() + ".source");
		Utility::FileSystem::WriteFileString(shaderTextFile, debugString);

		// Load In-Memory Metadata Object
		newAsset.Data.FileLocation = shaderTextFile;
		Ref<Assets::ShaderMetaData> metadata = CreateRef<Assets::ShaderMetaData>();
		metadata->m_ShaderSpec = shaderSpec;
		metadata->m_InputLayout = inputLayout;
		metadata->m_UniformList = uniformLayout;
		newAsset.Data.SpecificFileData = metadata;
	}

	
}
