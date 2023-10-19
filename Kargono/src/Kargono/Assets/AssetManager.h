#pragma once

#include "Kargono/Assets/Asset.h"
#include "Kargono/Renderer/Texture.h"
#include "Kargono/Renderer/Shader.h"

#include <filesystem>
#include <tuple>


namespace Kargono {

	// Main API for getting resources at runtime!
	class AssetManager
	{
	//============================================================
	// Textures
	//============================================================
	public:
		// Retrieve the current project's Texture Registry from disk and Add to in-memory register
		static void DeserializeTextureRegistry();
		// Save Current in-memory registry to disk
		static void SerializeTextureRegistry();
		// Function to Load a new Texture from a file
		static AssetHandle ImportNewTextureFromFile(const std::filesystem::path& filePath);
		// Function to Load a new Texture from a buffer
		static AssetHandle ImportNewTextureFromData(Buffer buffer, int32_t width, int32_t height, int32_t channels);
		// Create Final Texture and to s_Textures
		static Ref<Texture2D> InstantiateTextureIntoMemory(Asset& asset);
		// Function to get a texture with a given name
		static Ref<Texture2D> GetTexture(const AssetHandle& handle);
		// Clear Texture s_TextureRegistry and s_Textures
		static void ClearTextureRegistry();
	private:
		// Imports Texture Data into intermediate format from a file!
		static void CreateTextureIntermediateFromFile(const std::filesystem::path& filePath, Asset& newAsset);
		// Imports Texture Data into intermediate format from a buffer!
		static void CreateTextureIntermediateFromBuffer(Buffer buffer, int32_t width, int32_t height, int32_t channels, Asset& newAsset);
	private:
		// Registry (Location of all items, even if they are not loaded into memory yet)
		static std::unordered_map<AssetHandle, Asset> s_TextureRegistry;
		// Maps to assets already loaded into memory
		static std::unordered_map<AssetHandle, Ref<Texture2D>> s_Textures;

	//============================================================
	// Shaders
	//============================================================
	public:
		// Retrieve the current project's Shader Registry from disk and Adds it to the in-memory registry
		static void DeserializeShaderRegistry();
		// Save Current in-memory registry to disk
		static void SerializeShaderRegistry();
		// Create Final Shader and to s_Shaders
		static Ref<Shader> InstantiateShaderIntoMemory(Asset& asset);
		// Function finds shader associated with handle
		static Ref<Shader> GetShader(const AssetHandle& handle);
		// Function finds shader associated with Specification and returns asset handle along with shader
		static std::tuple<AssetHandle, Ref<Shader>> GetShader(const Shader::ShaderSpecification& shaderSpec);
		// Clear Shader s_ShaderRegistry and s_Shaders
		static void ClearShaderRegistry();
	private:
		// Function to Load a new Shader
		static AssetHandle CreateNewShader(const Shader::ShaderSpecification& shaderSpec);
		// Imports Shader Data into intermediate format!
		static void CreateShaderIntermediate(const Shader::ShaderSource& shaderSource, Asset& newAsset, const Shader::ShaderSpecification& shaderSpec,
			const InputBufferLayout& inputLayout, const UniformBufferList& uniformLayout);
	private:
		// Registry (Location of all items, even if they are not loaded into memory yet)
		static std::unordered_map<AssetHandle, Asset> s_ShaderRegistry;
		// Maps to assets already loaded into memory
		static std::unordered_map<AssetHandle, Ref<Shader>> s_Shaders;


	//============================================================
	// General API
	//============================================================
	public:

		// Deserializes all registries into memory
		static void DeserializeAll()
		{
			DeserializeShaderRegistry();
			DeserializeTextureRegistry();
		}

		// Serializes all registries into disk storage
		static void SerializeAll()
		{
			SerializeShaderRegistry();
			SerializeTextureRegistry();
		}

		// Clears all Registries and In-Memory Assets
		static void ClearAll()
		{
			ClearTextureRegistry();
			ClearShaderRegistry();
		}
	};
	

}
