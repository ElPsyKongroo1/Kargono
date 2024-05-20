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
	namespace Projects { class Project; }
	
}

//============================================================
// Assets Namespace
//============================================================
// This namespace holds all of the structs that the AssetManager deals with
//		to manage Assets inside of the engine. Assets includes audio, textures,
//		shaders, fonts, scenes, etc... These assets hold an AssetHandle to
//		uniquely identify the asset and a metadata object that holds extra details
//		about the asset. These details include the file location, a checksum,
//		and Asset specific details such as its type and further asset specific metadata. // TODO: THIS NEEDS REWORK!
namespace Kargono::Assets
{
	
	// Main API for getting resources at runtime!
	class AssetManager
	{
	//============================================================
	// Textures
	//============================================================
	public:
		//==============================
		// Manage Texture Registry
		//==============================
		// Retrieve the current project's texture registry from disk and add to in-memory Registry
		//		(s_TextureRegistry). This involves:
		//		1. Retrieving the registry file from the current project asset directory.
		//		2. Verifying the file is a registry file and opening the root node
		//		3. Read each asset, retrieve its metadata, retrieve its texture specific metadata,
		//		and instatiate each asset into the s_TextureRegistry.
		static void DeserializeTextureRegistry();
		// Save Current in-memory registry (s_TextureRegistry) to disk location specified in
		//		current project. This involves:
		//		1. Open a new registry file that is located in the current project asset directory.
		//		2. Write each asset with its metadata and texture specific metadata into the new yaml file.
		//		3. Write the file to the output stream.
		static void SerializeTextureRegistry();
		// This function clears both the s_TextureRegistry and s_Textures which should also call
		//		all destructors for in-memory Textures.
		static void ClearTextureRegistry();

		//==============================
		// Import New Texture
		//==============================
		// Function to Load a new Texture from a file
		static AssetHandle ImportNewTextureFromFile(const std::filesystem::path& filePath);
		// Function to Load a new Texture from a buffer
		static AssetHandle ImportNewTextureFromData(Buffer buffer, int32_t width, int32_t height, int32_t channels);

		//==============================
		// Load and Retrieve In-Memory Texture
		//==============================
		// Create Final Texture and to s_Textures
		static Ref<Texture2D> InstantiateTextureIntoMemory(Assets::Asset& asset);
		// Function to get a texture with a given name
		static Ref<Texture2D> GetTexture(const AssetHandle& handle);
		
	private:
		//==============================
		// Internal Functionality
		//==============================
		// Imports Texture Data into intermediate format from a file!
		static void CreateTextureIntermediateFromFile(const std::filesystem::path& filePath, Assets::Asset& newAsset);
		// Imports Texture Data into intermediate format from a buffer!
		static void CreateTextureIntermediateFromBuffer(Buffer buffer, int32_t width, int32_t height, int32_t channels, Assets::Asset& newAsset);
	private:
		// This registry holds a reference to all of the available textures in the current project.
		//		Since the registry only holds references, it does not instantiate any of the objects
		//		itself. It holds an AssetHandle to identify an asset and an Assets::Asset which holds
		//		metadata that is necessary to load the intermediate file correctly.
		static std::unordered_map<AssetHandle, Assets::Asset> s_TextureRegistry;
		// This map holds all of the textures that have been fully loaded into memory and are ready
		//		to use.
		static std::unordered_map<AssetHandle, Ref<Texture2D>> s_Textures;

	//============================================================
	// Shaders
	//============================================================
	public:
		//==============================
		// Manage Shader Registry
		//==============================
		// Retrieve the current project's shader registry from disk and add to in-memory Registry
		//		(s_ShaderRegistry). This involves:
		//		1. Retrieving the registry file from the current project asset directory.
		//		2. Verifying the file is a registry file and opening the root node
		//		3. Read each asset, retrieve its metadata, retrieve its shader specific metadata,
		//		and instatiate each asset into the s_ShaderRegistry.
		static void DeserializeShaderRegistry();
		// Save Current in-memory registry (s_ShaderRegistry) to disk location specified in
		//		current project. This involves:
		//		1. Open a new registry file that is located in the current project asset directory.
		//		2. Write each asset with its metadata and shader specific metadata into the new yaml file.
		//		3. Write the file to the output stream.
		static void SerializeShaderRegistry();
		// This function clears both the s_ShaderRegistry and s_Shaders which should also call
		//		all destructors for in-memory Shaders.
		static void ClearShaderRegistry();

		//==============================
		// Load and Retrieve In-Memory Shader
		//==============================
		// Create Final Shader and to s_Shaders
		static Ref<Kargono::Shader> InstantiateShaderIntoMemory(Assets::Asset& asset);
		// Function finds shader associated with handle
		static Ref<Kargono::Shader> GetShader(const AssetHandle& handle);
		// Function finds shader associated with Specification and returns asset handle along with shader
		static std::tuple<AssetHandle, Ref<Kargono::Shader>> GetShader(const Shader::ShaderSpecification& shaderSpec);
	private:
		//==============================
		// Internal Functionality
		//==============================
		// Function to Load a new Shader
		static AssetHandle CreateNewShader(const Shader::ShaderSpecification& shaderSpec);
		// Imports Shader Data into intermediate format!
		static void CreateShaderIntermediate(const Shader::ShaderSource& shaderSource, Assets::Asset& newAsset, const Shader::ShaderSpecification& shaderSpec,
			const InputBufferLayout& inputLayout, const UniformBufferList& uniformLayout);
	private:
		// This registry holds a reference to all of the available shaders in the current project.
		//		Since the registry only holds references, it does not instantiate any of the objects
		//		itself. It holds an AssetHandle to identify an asset and an Assets::Asset which holds
		//		metadata that is necessary to load the intermediate file correctly.
		static std::unordered_map<AssetHandle, Assets::Asset> s_ShaderRegistry;
		// This map holds all of the shaders that have been fully loaded into memory and are ready
		//		to use.
		static std::unordered_map<AssetHandle, Ref<Kargono::Shader>> s_Shaders;


	//============================================================
	// Audio
	//============================================================
	public:
		//==============================
		// Manage Audio Registry
		//==============================
		// Retrieve the current project's audio registry from disk and add to in-memory Registry
		//		(s_AudioRegistry). This involves:
		//		1. Retrieving the registry file from the current project asset directory.
		//		2. Verifying the file is a registry file and opening the root node
		//		3. Read each asset, retrieve its metadata, retrieve its audio specific metadata,
		//		and instatiate each asset into the s_AudioRegistry.
		static void DeserializeAudioRegistry();
		// Save Current in-memory registry (s_AudioRegistry) to disk location specified in
		//		current project. This involves:
		//		1. Open a new registry file that is located in the current project asset directory.
		//		2. Write each asset with its metadata and audio specific metadata into the new yaml file.
		//		3. Write the file to the output stream.
		static void SerializeAudioRegistry();
		// This function clears both the s_AudioRegistry and s_Audio which should also call
		//		all destructors for in-memory Audio.
		static void ClearAudioRegistry();

		//==============================
		// Import New Audio
		//==============================
		// Function to Load a new Audio from a file
		static AssetHandle ImportNewAudioFromFile(const std::filesystem::path& filePath);

		//==============================
		// Load and Retrieve In-Memory Audio
		//==============================
		// Function to Load a new Audio from a buffer
		static Ref<Audio::AudioBuffer> InstantiateAudioIntoMemory(Assets::Asset& asset);
		// Function to get a texture with a given name
		static Ref<Audio::AudioBuffer> GetAudio(const AssetHandle& handle);
		// Function to get an audio buffer from its initial file location
		static std::tuple<AssetHandle, Ref<Audio::AudioBuffer>> GetAudio(const std::filesystem::path& filepath);
	private:
		//==============================
		// Internal Functionality
		//==============================
		// Imports Audio Data into intermediate format from a file!
		static void CreateAudioIntermediateFromFile(const std::filesystem::path& filePath, Assets::Asset& newAsset);
	private:
		// This registry holds a reference to all of the available audio in the current project.
		//		Since the registry only holds references, it does not instantiate any of the objects
		//		itself. It holds an AssetHandle to identify an asset and an Assets::Asset which holds
		//		metadata that is necessary to load the intermediate file correctly.
		static std::unordered_map<AssetHandle, Assets::Asset> s_AudioRegistry;
		// This map holds all of the audio that have been fully loaded into memory and are ready
		//		to use.
		static std::unordered_map<AssetHandle, Ref<Audio::AudioBuffer>> s_Audio;


	//============================================================
	// Font
	//============================================================
	public:
		//==============================
		// Manage Font Registry
		//==============================
		// Retrieve the current project's font registry from disk and add to in-memory Registry
		//		(s_FontRegistry). This involves:
		//		1. Retrieving the registry file from the current project asset directory.
		//		2. Verifying the file is a registry file and opening the root node
		//		3. Read each asset, retrieve its metadata, retrieve its font specific metadata,
		//		and instatiate each asset into the s_FontRegistry.
		static void DeserializeFontRegistry();
		// Save Current in-memory registry (s_FontRegistry) to disk location specified in
		//		current project. This involves:
		//		1. Open a new registry file that is located in the current project asset directory.
		//		2. Write each asset with its metadata and font specific metadata into the new yaml file.
		//		3. Write the file to the output stream.
		static void SerializeFontRegistry();
		// This function clears both the s_FontRegistry and s_Fonts which should also call
		//		all destructors for in-memory Fonts.
		static void ClearFontRegistry();

		//==============================
		// Import New Font
		//==============================
		// Function to Load a new Font from a file
		static AssetHandle ImportNewFontFromFile(const std::filesystem::path& filePath);

		//==============================
		// Load and Retrieve In-Memory Font
		//==============================
		// Function to Load a new Font from a buffer
		static Ref<Kargono::Font> InstantiateFontIntoMemory(Assets::Asset& asset);
		// Function to get a texture with a given name
		static Ref<Kargono::Font> GetFont(const AssetHandle& handle);
	private:
		//==============================
		// Internal Functionality
		//==============================
		// Imports Font Data into intermediate format from a file!
		static void CreateFontIntermediateFromFile(const std::filesystem::path& filePath, Assets::Asset& newAsset);
	private:
		// This registry holds a reference to all of the available fonts in the current project.
		//		Since the registry only holds references, it does not instantiate any of the objects
		//		itself. It holds an AssetHandle to identify an asset and an Assets::Asset which holds
		//		metadata that is necessary to load the intermediate file correctly.
		static std::unordered_map<AssetHandle, Assets::Asset> s_FontRegistry;
		// This map holds all of the audio that have been fully loaded into memory and are ready
		//		to use.
		static std::unordered_map<AssetHandle, Ref<Kargono::Font>> s_Fonts;

	//============================================================
	// Scene
	//============================================================
	public:
		//==============================
		// Manage Scene Registry
		//==============================
		// Retrieve the current project's scene registry from disk and add to in-memory Registry
		//		(s_SceneRegistry). This involves:
		//		1. Retrieving the registry file from the current project asset directory.
		//		2. Verifying the file is a registry file and opening the root node
		//		3. Read each asset, retrieve its metadata, retrieve its scene specific metadata,
		//		and instatiate each asset into the s_SceneRegistry.
		static void DeserializeSceneRegistry();
		// Save Current in-memory registry (s_SceneRegistry) to disk location specified in
		//		current project. This involves:
		//		1. Open a new registry file that is located in the current project asset directory.
		//		2. Write each asset with its metadata and scene specific metadata into the new yaml file.
		//		3. Write the file to the output stream.
		static void SerializeSceneRegistry();
		// This function clears both the s_SceneRegistry and s_Scenes which should also call
		//		all destructors for in-memory Scenes.
		static void ClearSceneRegistry();

		//==============================
		// Create New Scene
		//==============================
		// Function to Load a new Scene from a file
		static AssetHandle CreateNewScene(const std::string& sceneName);

		//==============================
		// Save a Scene
		//==============================
		// Save/Update Current Scene
		static void SaveScene(AssetHandle sceneHandle, Ref<Kargono::Scene> scene);
		// Save a single scene
		static void SerializeScene(Ref<Kargono::Scene> scene, const std::filesystem::path& filepath);
		// Load a single scene
		static bool DeserializeScene(Ref<Kargono::Scene> scene, const std::filesystem::path& filepath);

		//==============================
		// Load and Retrieve In-Memory Scene
		//==============================
		static Ref<Kargono::Scene> GetScene(const AssetHandle& handle);
		static std::tuple<AssetHandle, Ref<Kargono::Scene>> GetScene(const std::filesystem::path& filepath);
		// Instantiate a new scene
		static Ref<Kargono::Scene> InstantiateScene(const Assets::Asset& sceneAsset);


		//==============================
		// Getters/Setters
		//==============================
		// Check if name already exists in registry
		static bool CheckSceneExists(const std::string& sceneName);
		// TODO: Commentplzzzz
		static std::unordered_map<AssetHandle, Assets::Asset>& GetSceneRegistry() { return s_SceneRegistry; }

	private:
		//==============================
		// Internal Functionality
		//==============================
		// Imports Scene Data into intermediate format from a file!
		static void CreateSceneFile(const std::string& sceneName, Assets::Asset& newAsset);
	private:
		// This registry holds a reference to all of the available scenes in the current project.
		//		Since the registry only holds references, it does not instantiate any of the objects
		//		itself. It holds an AssetHandle to identify an asset and an Assets::Asset which holds
		//		metadata that is necessary to load the intermediate file correctly.
		static std::unordered_map<AssetHandle, Assets::Asset> s_SceneRegistry;

	//============================================================
	// UIObject
	//============================================================
	public:
		//==============================
		// Manage UIObject Registry
		//==============================
		// Retrieve the current project's uiObject registry from disk and add to in-memory Registry
		//		(s_UIObjectRegistry). This involves:
		//		1. Retrieving the registry file from the current project asset directory.
		//		2. Verifying the file is a registry file and opening the root node
		//		3. Read each asset, retrieve its metadata, retrieve its uiObject specific metadata,
		//		and instatiate each asset into the s_UIObjectRegistry.
		static void DeserializeUIObjectRegistry();
		// Save Current in-memory registry (s_UIObjectRegistry) to disk location specified in
		//		current project. This involves:
		//		1. Open a new registry file that is located in the current project asset directory.
		//		2. Write each asset with its metadata and UIObject specific metadata into the new yaml file.
		//		3. Write the file to the output stream.
		static void SerializeUIObjectRegistry();
		// This function clears both the s_UIObjectRegistry and s_UIObjects which should also call
		//		all destructors for in-memory UIObjects.
		static void ClearUIObjectRegistry();

		//==============================
		// Create New UIObject
		//==============================
		// Function to Load a new UIObject from a file
		static AssetHandle CreateNewUIObject(const std::string& uiObjectName);

		//==============================
		// Save a UIObject
		//==============================
		// Save Current UIObject
		static void SaveUIObject(AssetHandle uiObjectHandle, Ref<UIEngine::UIObject> uiObject);

		//==============================
		// Load and Retrieve In-Memory UIObject
		//==============================
		// Function to get a texture with a given name
		static Ref<UIEngine::UIObject> GetUIObject(const AssetHandle& handle);
		static std::tuple<AssetHandle, Ref<UIEngine::UIObject>> GetUIObject(const std::filesystem::path& filepath);
		// Instantiate a new uiObject
		static Ref<UIEngine::UIObject> InstantiateUIObject(const Assets::Asset& uiObjectAsset);

		//==============================
		// Getters/Setters
		//==============================
		// Check if name already exists in registry
		static bool CheckUIObjectExists(const std::string& uiObjectName);

		// Returns the relative path from project directory of intermediate file
		static std::filesystem::path GetUIObjectLocation(const AssetHandle& handle);

		static std::unordered_map<AssetHandle, Assets::Asset>& GetUIObjectRegistry() { return s_UIObjectRegistry; }
	private:
		//==============================
		// Internal Functionality
		//==============================
		// Imports UIObject Data into intermediate format from a file!
		static void CreateUIObjectFile(const std::string& uiObjectName, Assets::Asset& newAsset);
		// Save a single uiObject
		static void SerializeUIObject(Ref<UIEngine::UIObject> uiObject, const std::filesystem::path& filepath);
		// Load a single uiObject
		static bool DeserializeUIObject(Ref<UIEngine::UIObject> uiObject, const std::filesystem::path& filepath);
	private:
		// This registry holds a reference to all of the available UIObject in the current project.
		//		Since the registry only holds references, it does not instantiate any of the objects
		//		itself. It holds an AssetHandle to identify an asset and an Assets::Asset which holds
		//		metadata that is necessary to load the intermediate file correctly.
		static std::unordered_map<AssetHandle, Assets::Asset> s_UIObjectRegistry;


	//============================================================
	// Project
	//============================================================
	public:
	//==============================
	// Save and Load a Project
	//==============================

	static Ref<Projects::Project> NewProject();
	static Ref<Projects::Project> OpenProject(const std::filesystem::path& path);
	static bool SaveActiveProject(const std::filesystem::path& path);
	// Save a single project
	static bool SerializeProject(Ref<Projects::Project> project, const std::filesystem::path& filepath);
	// Load a single project
	static bool DeserializeProject(Ref<Projects::Project> project, const std::filesystem::path& filepath);

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
