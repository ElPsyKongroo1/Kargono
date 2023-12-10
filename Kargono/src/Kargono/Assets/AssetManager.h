#pragma once

#include "Kargono/Assets/Asset.h"
#include "Kargono/Renderer/Texture.h"
#include "Kargono/Renderer/Shader.h"
#include "Kargono/Audio/AudioEngine.h"
#include "Kargono/Text/TextEngine.h"
#include "Kargono/UI/RuntimeUI.h"

#include <filesystem>
#include <tuple>


namespace Kargono
{
	class Scene;
	class Project;


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
	// Audio
	//============================================================
	public:
		// Retrieve the current project's Audio Registry from disk and Add to in-memory register
		static void DeserializeAudioRegistry();
		// Save Current in-memory registry to disk
		static void SerializeAudioRegistry();
		// Function to Load a new Audio from a file
		static AssetHandle ImportNewAudioFromFile(const std::filesystem::path& filePath);
		// Function to Load a new Audio from a buffer
		static Ref<AudioBuffer> InstantiateAudioIntoMemory(Asset& asset);
		// Function to get a texture with a given name
		static Ref<AudioBuffer> GetAudio(const AssetHandle& handle);
		// Function to get an audio buffer from its initial file location
		static std::tuple<AssetHandle, Ref<AudioBuffer>> GetAudio(const std::filesystem::path& filepath);
		// Clear Audio s_AudioRegistry and s_Audio
		static void ClearAudioRegistry();
	private:
		// Imports Audio Data into intermediate format from a file!
		static void CreateAudioIntermediateFromFile(const std::filesystem::path& filePath, Asset& newAsset);
	private:
		// Registry (Location of all items, even if they are not loaded into memory yet)
		static std::unordered_map<AssetHandle, Asset> s_AudioRegistry;
		// Maps to assets already loaded into memory
		static std::unordered_map<AssetHandle, Ref<AudioBuffer>> s_Audio;


	//============================================================
	// Font
	//============================================================
	public:
		// Retrieve the current project's Font Registry from disk and Add to in-memory register
		static void DeserializeFontRegistry();
		// Save Current in-memory registry to disk
		static void SerializeFontRegistry();
		// Function to Load a new Font from a file
		static AssetHandle ImportNewFontFromFile(const std::filesystem::path& filePath);
		// Function to Load a new Font from a buffer
		static Ref<Font> InstantiateFontIntoMemory(Asset& asset);
		// Function to get a texture with a given name
		static Ref<Font> GetFont(const AssetHandle& handle);
		// Clear Font s_FontRegistry and s_Font
		static void ClearFontRegistry();
	private:
		// Imports Font Data into intermediate format from a file!
		static void CreateFontIntermediateFromFile(const std::filesystem::path& filePath, Asset& newAsset);
	private:
		// Registry (Location of all items, even if they are not loaded into memory yet)
		static std::unordered_map<AssetHandle, Asset> s_FontRegistry;
		// Maps to assets already loaded into memory
		static std::unordered_map<AssetHandle, Ref<Font>> s_Fonts;

	//============================================================
	// Scene
	//============================================================
	public:
		// Retrieve the current project's Scene Registry from disk and Add to in-memory register
		static void DeserializeSceneRegistry();
		// Save Current in-memory registry to disk
		static void SerializeSceneRegistry();
		// Save a single scene
		static void SerializeScene(Ref<Scene> scene,const std::filesystem::path& filepath);
		// Load a single scene
		static bool DeserializeScene(Ref<Scene> scene, const std::filesystem::path& filepath);
		// Check if name already exists in registry
		static bool CheckSceneExists(const std::string& sceneName);
		// Function to Load a new Scene from a file
		static AssetHandle CreateNewScene(const std::string& sceneName);
		// Save Current Scene
		static void SaveScene(AssetHandle sceneHandle , Ref<Scene> scene);
		// Function to get a texture with a given name
		static Ref<Scene> GetScene(const AssetHandle& handle);
		static std::tuple<AssetHandle, Ref<Scene>> GetScene(const std::filesystem::path& filepath);
		// Instantiate a new scene
		static Ref<Scene> InstantiateScene(const Asset& sceneAsset);
		// Clear Scene s_SceneRegistry and s_Scene
		static void ClearSceneRegistry();
		static std::unordered_map<AssetHandle, Asset>& GetSceneRegistry() { return s_SceneRegistry; }
	private:
		// Imports Scene Data into intermediate format from a file!
		static void CreateSceneFile(const std::string& sceneName, Asset& newAsset);
	private:
		// Registry (Location of all items, even if they are not loaded into memory yet)
		static std::unordered_map<AssetHandle, Asset> s_SceneRegistry;

	//============================================================
	// UIObject
	//============================================================
	public:
		// Retrieve the current project's UIObject Registry from disk and Add to in-memory register
		static void DeserializeUIObjectRegistry();
		// Save Current in-memory registry to disk
		static void SerializeUIObjectRegistry();
		// Save a single uiObject
		static void SerializeUIObject(Ref<UIEngine::UIObject> uiObject, const std::filesystem::path& filepath);
		// Load a single uiObject
		static bool DeserializeUIObject(Ref<UIEngine::UIObject> uiObject, const std::filesystem::path& filepath);
		// Check if name already exists in registry
		static bool CheckUIObjectExists(const std::string& uiObjectName);
		// Function to Load a new UIObject from a file
		static AssetHandle CreateNewUIObject(const std::string& uiObjectName);
		// Save Current UIObject
		static void SaveUIObject(AssetHandle uiObjectHandle, Ref<UIEngine::UIObject> uiObject);
		// Returns the relative path from project directory of intermediate file
		static std::filesystem::path GetUIObjectLocation(const AssetHandle& handle);
		// Function to get a texture with a given name
		static Ref<UIEngine::UIObject> GetUIObject(const AssetHandle& handle);
		static std::tuple<AssetHandle, Ref<UIEngine::UIObject>> GetUIObject(const std::filesystem::path& filepath);
		// Instantiate a new uiObject
		static Ref<UIEngine::UIObject> InstantiateUIObject(const Asset& uiObjectAsset);
		// Clear UIObject s_UIObjectRegistry and s_UIObject
		static void ClearUIObjectRegistry();
		static std::unordered_map<AssetHandle, Asset>& GetUIObjectRegistry() { return s_UIObjectRegistry; }
	private:
		// Imports UIObject Data into intermediate format from a file!
		static void CreateUIObjectFile(const std::string& uiObjectName, Asset& newAsset);
	private:
		// Registry (Location of all items, even if they are not loaded into memory yet)
		static std::unordered_map<AssetHandle, Asset> s_UIObjectRegistry;


	//============================================================
	// Project
	//============================================================
	public:
	// Save a single project
	static bool SerializeProject(Ref<Project> project, const std::filesystem::path& filepath);
	// Load a single project
	static bool DeserializeProject(Ref<Project> project, const std::filesystem::path& filepath);

	//============================================================
	// General API
	//============================================================
	public:

		// Deserializes all registries into memory
		static void DeserializeAll()
		{
			DeserializeShaderRegistry();
			DeserializeTextureRegistry();
			DeserializeAudioRegistry();
			DeserializeFontRegistry();
			DeserializeSceneRegistry();
			DeserializeUIObjectRegistry();
		}

		// Serializes all registries into disk storage
		static void SerializeAll()
		{
			SerializeShaderRegistry();
			SerializeTextureRegistry();
			SerializeAudioRegistry();
			SerializeFontRegistry();
			SerializeSceneRegistry();
			SerializeUIObjectRegistry();
		}

		// Clears all Registries and In-Memory Assets
		static void ClearAll()
		{
			ClearTextureRegistry();
			ClearShaderRegistry();
			ClearAudioRegistry();
			ClearFontRegistry();
			ClearSceneRegistry();
			ClearUIObjectRegistry();
		}
	};
	

}
