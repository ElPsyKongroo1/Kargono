#pragma once

#include "Kargono/Assets/Asset.h"
#include "Kargono/Renderer/Texture.h"
#include "Kargono/Renderer/Shader.h"
#include "Kargono/Audio/AudioEngine.h"
#include "Kargono/UI/Text.h"
#include "Kargono/UI/Runtime.h"
#include "Kargono/Scene/GameState.h"
#include "Kargono/Input/InputMode.h"
#include "Kargono/Scripting/Scripting.h"
#include "Kargono/Scripting/ScriptModuleBuilder.h"

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
	//============================================================
	// Asset Manager Class
	//============================================================
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

		// This function registers a new texture with the asset system using the provided filepath.
		//		This function takes the following steps:
		//		1. Create a checksum from the raw file provided and determine if the file already
		//		exists in the registry.
		//		2. Create the intermediate file format and store this format on disk.
		//		3. Register the new file with the in-memory s_TextureRegistry and the on disk texture
		//		registry
		//		4. Instantiate the new texture into memory and return a handle to the new texture.
		static AssetHandle ImportNewTextureFromFile(const std::filesystem::path& filePath);

		// This function registers a new texture with the asset system using the provided data buffer.
		//		This function takes the following steps:
		//		1. Create a checksum from the raw file provided and determine if the file already
		//		exists in the registry.
		//		2. Create the intermediate file format and store this format on disk.
		//		3. Register the new file with the in-memory s_TextureRegistry and the on disk texture
		//		registry
		//		4. Instantiate the new texture into memory and return a handle to the new texture.
		static AssetHandle ImportNewTextureFromData(Buffer buffer, int32_t width, int32_t height, int32_t channels);

		//==============================
		// Load and Retrieve In-Memory Texture
		//==============================
		// Function to get a texture with a given name
		static Ref<Texture2D> GetTexture(const AssetHandle& handle);
		
	private:
		//==============================
		// Internal Functionality
		//==============================
		// This function loads a texture from an external format into an intermediate format and
		//		stores the relevant asset metadata.
		static void CreateTextureIntermediateFromFile(const std::filesystem::path& filePath, Assets::Asset& newAsset);
		// This function loads a texture from a binary buffer and stores this data into an intermediate
		//		format along with relevant asset metadata.
		static void CreateTextureIntermediateFromBuffer(Buffer buffer, int32_t width, int32_t height, int32_t channels, Assets::Asset& newAsset);
		// Create Final Texture and to s_Textures
		static Ref<Texture2D> InstantiateTextureIntoMemory(Assets::Asset& asset);
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
		// Function finds shader associated with handle
		static Ref<Kargono::Shader> GetShader(const AssetHandle& handle);
		// Function finds shader associated with Specification and returns asset handle along with shader
		static std::tuple<AssetHandle, Ref<Kargono::Shader>> GetShader(const ShaderSpecification& shaderSpec);
	private:
		//==============================
		// Internal Functionality
		//==============================

		// This function registers a new shader with the asset system using the provided shader specification.
		//		This function takes the following steps:
		//		1. Create a checksum from the shader created in shader builder and determine if the file already
		//		exists in the registry. If the checksum fails but a specification exists that is identical, replace it.
		//		2. Create the intermediate file format and store this format on disk.
		//		3. Register the new file with the in-memory s_ShaderRegistry and the on disk shader
		//		registry
		//		4. Instantiate the new shader into memory and return a handle to the new shader.
		static AssetHandle CreateNewShader(const ShaderSpecification& shaderSpec);
		// This function takes the shader source, splits the shader text into fragment/vertex shaders, compiles
		//		the shaders into binary .kgvert/.kgfrag source files, and stores those files along with
		//		a fill text version for debugging as .kgsource. Finally metadata is created for the shader.
		static void CreateShaderIntermediate(const ShaderSource& shaderSource, Assets::Asset& newAsset, const ShaderSpecification& shaderSpec,
			const InputBufferLayout& inputLayout, const UniformBufferList& uniformLayout);
		// Create Final Shader and to s_Shaders
		static Ref<Kargono::Shader> InstantiateShaderIntoMemory(Assets::Asset& asset);
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

		// This function registers a new audio with the asset system using the provided filepath.
		//		This function takes the following steps:
		//		1. Create a checksum from the raw file provided and determine if the file already
		//		exists in the registry.
		//		2. Create the intermediate file format and store this format on disk.
		//		3. Register the new file with the in-memory s_AudioRegistry and the on disk audio
		//		registry
		//		4. Instantiate the new audio into memory and return a handle to the new audio.
		static AssetHandle ImportNewAudioFromFile(const std::filesystem::path& filePath);

		//==============================
		// Load and Retrieve In-Memory Audio
		//==============================

		// Function to get a texture with a given name
		static Ref<Audio::AudioBuffer> GetAudio(const AssetHandle& handle);
		// Function to get an audio buffer from its initial file location
		static std::tuple<AssetHandle, Ref<Audio::AudioBuffer>> GetAudio(const std::filesystem::path& filepath);
	private:
		//==============================
		// Internal Functionality
		//==============================

		// This function takes the filepath to the original audio file and creates a PCM binary buffer, then
		//		stores this binary buffer as an intermediate, then finally creates the relevant metadata for
		//		loading the intermediate later.
		static void CreateAudioIntermediateFromFile(const std::filesystem::path& filePath, Assets::Asset& newAsset);
		// Function to Load a new Audio from a buffer
		static Ref<Audio::AudioBuffer> InstantiateAudioIntoMemory(Assets::Asset& asset);
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

		// This function registers a new font with the asset system using the provided filepath.
		//		This function takes the following steps:
		//		1. Create a checksum from the raw file provided and determine if the file already
		//		exists in the registry.
		//		2. Create the intermediate file format and store this format on disk.
		//		3. Register the new file with the in-memory s_FontRegistry and the on disk font
		//		registry
		//		4. Instantiate the new font into memory and return a handle to the new font.
		static AssetHandle ImportNewFontFromFile(const std::filesystem::path& filePath);

		//==============================
		// Load and Retrieve In-Memory Font
		//==============================
		// Function to Load a new Font from a buffer
		static Ref<UI::Font> InstantiateFontIntoMemory(Assets::Asset& asset);
		// Function to get a texture with a given name
		static Ref<UI::Font> GetFont(const AssetHandle& handle);
	private:
		//==============================
		// Internal Functionality
		//==============================
		// This function uses the filepath to load the font file from disk into a Character Map which
		//		is used to create the .kgfont intermediate (series of Glyph images). The metadata is
		//		created for the font to load the intermediates correctly.
		static void CreateFontIntermediateFromFile(const std::filesystem::path& filePath, Assets::Asset& newAsset);
	private:
		// This registry holds a reference to all of the available fonts in the current project.
		//		Since the registry only holds references, it does not instantiate any of the objects
		//		itself. It holds an AssetHandle to identify an asset and an Assets::Asset which holds
		//		metadata that is necessary to load the intermediate file correctly.
		static std::unordered_map<AssetHandle, Assets::Asset> s_FontRegistry;
		// This map holds all of the audio that have been fully loaded into memory and are ready
		//		to use.
		static std::unordered_map<AssetHandle, Ref<UI::Font>> s_Fonts;

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

		// This function registers a new scene with the asset system using the provided filepath.
		//		This function takes the following steps:
		//		1. Create a checksum from the raw file provided and determine if the file already
		//		exists in the registry.
		//		2. Create the scene file on disk.
		//		3. Register the new file with the in-memory s_SceneRegistry and the on disk scene
		//		registry
		//		4. Return the Scene handle
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
		// This function creates a .kgscene file with the specified name and fills the provided asset file
		//		with relevant metadata.
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

		// This function registers a new uiobject with the asset system using the provided filepath.
		//		This function takes the following steps:
		//		1. Create a checksum from the raw file provided and determine if the file already
		//		exists in the registry.
		//		2. Create the uiobject file on disk.
		//		3. Register the new file with the in-memory s_UIObjectRegistry and the on disk uiobject
		//		registry
		//		4. Return the uiobject handle
		static AssetHandle CreateNewUIObject(const std::string& uiObjectName);

		//==============================
		// Save a UIObject
		//==============================
		// Save Current UIObject
		static void SaveUIObject(AssetHandle uiObjectHandle, Ref<UI::UIObject> uiObject);

		//==============================
		// Load and Retrieve In-Memory UIObject
		//==============================
		// Function to get a texture with a given name
		static Ref<UI::UIObject> GetUIObject(const AssetHandle& handle);
		static std::tuple<AssetHandle, Ref<UI::UIObject>> GetUIObject(const std::filesystem::path& filepath);

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
		// This function creates a .uiobject file with the specified name and fills the provided asset file
		//		with relevant metadata.
		static void CreateUIObjectFile(const std::string& uiObjectName, Assets::Asset& newAsset);
		// Save a single uiObject
		static void SerializeUIObject(Ref<UI::UIObject> uiObject, const std::filesystem::path& filepath);
		// Load a single uiObject
		static bool DeserializeUIObject(Ref<UI::UIObject> uiObject, const std::filesystem::path& filepath);
		// Instantiate a new uiObject
		static Ref<UI::UIObject> InstantiateUIObject(const Assets::Asset& uiObjectAsset);
	private:
		// This registry holds a reference to all of the available UIObject in the current project.
		//		Since the registry only holds references, it does not instantiate any of the objects
		//		itself. It holds an AssetHandle to identify an asset and an Assets::Asset which holds
		//		metadata that is necessary to load the intermediate file correctly.
		static std::unordered_map<AssetHandle, Assets::Asset> s_UIObjectRegistry;

	//============================================================
	// InputMode
	//============================================================
	public:
		//==============================
		// Manage InputMode Registry
		//==============================
		// Retrieve the current project's inputMode registry from disk and add to in-memory Registry
		//		(s_InputModeRegistry). This involves:
		//		1. Retrieving the registry file from the current project asset directory.
		//		2. Verifying the file is a registry file and opening the root node
		//		3. Read each asset, retrieve its metadata, retrieve its inputMode specific metadata,
		//		and instatiate each asset into the s_InputModeRegistry.
		static void DeserializeInputModeRegistry();
		// Save Current in-memory registry (s_InputModeRegistry) to disk location specified in
		//		current project. This involves:
		//		1. Open a new registry file that is located in the current project asset directory.
		//		2. Write each asset with its metadata and InputMode specific metadata into the new yaml file.
		//		3. Write the file to the output stream.
		static void SerializeInputModeRegistry();
		// This function clears both the s_InputModeRegistry and s_InputModes which should also call
		//		all destructors for in-memory InputModes.
		static void ClearInputModeRegistry();

		//==============================
		// Create New InputMode
		//==============================

		// This function registers a new inputMode with the asset system using the provided filepath.
		//		This function takes the following steps:
		//		1. Create a checksum from the raw file provided and determine if the file already
		//		exists in the registry.
		//		2. Create the inputMode file on disk.
		//		3. Register the new file with the in-memory s_InputModeRegistry and the on disk inputMode
		//		registry
		//		4. Return the inputMode handle
		static AssetHandle CreateNewInputMode(const std::string& inputModeName);

		//==============================
		// Save a InputMode
		//==============================
		// Save Current InputMode
		static void SaveInputMode(AssetHandle inputModeHandle, Ref<Kargono::InputMode> inputMode);

		//==============================
		// Load and Retrieve In-Memory InputMode
		//==============================
		// Function to get a texture with a given name
		static Ref<Kargono::InputMode> GetInputMode(const AssetHandle& handle);
		static std::tuple<AssetHandle, Ref<Kargono::InputMode>> GetInputMode(const std::filesystem::path& filepath);

		//==============================
		// Getters/Setters
		//==============================
		// Check if name already exists in registry
		static bool CheckInputModeExists(const std::string& inputModeName);

		// Returns the relative path from project directory of intermediate file
		static std::filesystem::path GetInputModeLocation(const AssetHandle& handle);

		static std::unordered_map<AssetHandle, Assets::Asset>& GetInputModeRegistry() { return s_InputModeRegistry; }
	private:
		//==============================
		// Internal Functionality
		//==============================
		// This function creates a .inputMode file with the specified name and fills the provided asset file
		//		with relevant metadata.
		static void CreateInputModeFile(const std::string& inputModeName, Assets::Asset& newAsset);
		// Save a single inputMode
		static void SerializeInputMode(Ref<Kargono::InputMode> inputMode, const std::filesystem::path& filepath);
		// Load a single inputMode
		static bool DeserializeInputMode(Ref<Kargono::InputMode> inputMode, const std::filesystem::path& filepath);
		// Instantiate a new inputMode
		static Ref<Kargono::InputMode> InstantiateInputMode(const Assets::Asset& inputModeAsset);
	private:
		// This registry holds a reference to all of the available InputMode in the current project.
		//		Since the registry only holds references, it does not instantiate any of the objects
		//		itself. It holds an AssetHandle to identify an asset and an Assets::Asset which holds
		//		metadata that is necessary to load the intermediate file correctly.
		static std::unordered_map<AssetHandle, Assets::Asset> s_InputModeRegistry;

	//============================================================
	// GameState
	//============================================================
	public:
		//==============================
		// Manage GameState Registry
		//==============================
		// Retrieve the current project's GameState registry from disk and add to in-memory Registry
		//		(s_GameStateRegistry). This involves:
		//		1. Retrieving the registry file from the current project asset directory.
		//		2. Verifying the file is a registry file and opening the root node
		//		3. Read each asset, retrieve its metadata, retrieve its GameState specific metadata,
		//		and instatiate each asset into the s_GameStateRegistry.
		static void DeserializeGameStateRegistry();
		// Save Current in-memory registry (s_GameStateRegistry) to disk location specified in
		//		current project. This involves:
		//		1. Open a new registry file that is located in the current project asset directory.
		//		2. Write each asset with its metadata and GameState specific metadata into the new yaml file.
		//		3. Write the file to the output stream.
		static void SerializeGameStateRegistry();
		// This function clears both the s_GameStateRegistry and s_GameStates which should also call
		//		all destructors for in-memory GameStates.
		static void ClearGameStateRegistry();

		//==============================
		// Create New GameState
		//==============================

		// This function registers a new GameState with the asset system using the provided filepath.
		//		This function takes the following steps:
		//		1. Create a checksum from the raw file provided and determine if the file already
		//		exists in the registry.
		//		2. Create the GameState file on disk.
		//		3. Register the new file with the in-memory s_GameStateRegistry and the on disk GameState
		//		registry
		//		4. Return the GameState handle
		static AssetHandle CreateNewGameState(const std::string& GameStateName);

		//==============================
		// Save a GameState
		//==============================
		// Save Current GameState
		static void SaveGameState(AssetHandle GameStateHandle, Ref<Kargono::GameState> GameState);

		//==============================
		// Load and Retrieve In-Memory GameState
		//==============================
		// Function to get a texture with a given name
		static Ref<Kargono::GameState> GetGameState(const AssetHandle& handle);
		static std::tuple<AssetHandle, Ref<Kargono::GameState>> GetGameState(const std::filesystem::path& filepath);

		//==============================
		// Getters/Setters
		//==============================
		// Check if name already exists in registry
		static bool CheckGameStateExists(const std::string& GameStateName);

		// Returns the relative path from project directory of intermediate file
		static std::filesystem::path GetGameStateLocation(const AssetHandle& handle);

		static std::unordered_map<AssetHandle, Assets::Asset>& GetGameStateRegistry() { return s_GameStateRegistry; }
	private:
		//==============================
		// Internal Functionality
		//==============================
		// This function creates a .GameState file with the specified name and fills the provided asset file
		//		with relevant metadata.
		static void CreateGameStateFile(const std::string& GameStateName, Assets::Asset& newAsset);
		// Save a single GameState
		static void SerializeGameState(Ref<Kargono::GameState> GameState, const std::filesystem::path& filepath);
		// Load a single GameState
		static bool DeserializeGameState(Ref<Kargono::GameState> GameState, const std::filesystem::path& filepath);
		// Instantiate a new GameState
		static Ref<Kargono::GameState> InstantiateGameState(const Assets::Asset& GameStateAsset);
	private:
		// This registry holds a reference to all of the available GameState in the current project.
		//		Since the registry only holds references, it does not instantiate any of the objects
		//		itself. It holds an AssetHandle to identify an asset and an Assets::Asset which holds
		//		metadata that is necessary to load the intermediate file correctly.
		static std::unordered_map<AssetHandle, Assets::Asset> s_GameStateRegistry;


	//============================================================
	// Script
	//============================================================
	public:
		//==============================
		// Manage Script Registry
		//==============================
		// Retrieve the current project's Script registry from disk and add to in-memory Registry
		//		(s_ScriptRegistry). This involves:
		//		1. Retrieving the registry file from the current project asset directory.
		//		2. Verifying the file is a registry file and opening the root node
		//		3. Read each asset, retrieve its metadata, retrieve its Script specific metadata,
		//		and instatiate each asset into the s_ScriptRegistry.
		static void DeserializeScriptRegistry();
		// Save Current in-memory registry (s_ScriptRegistry) to disk location specified in
		//		current project. This involves:
		//		1. Open a new registry file that is located in the current project asset directory.
		//		2. Write each asset with its metadata and Script specific metadata into the new yaml file.
		//		3. Write the file to the output stream.
		static void SerializeScriptRegistry();
		// This function clears both the s_ScriptRegistry and s_Script which should also call
		//		all destructors for in-memory Script.
		static void ClearScriptRegistry();

		//==============================
		// Import New Script
		//==============================

		// This function registers a new Script with the asset system using the provided filepath.
		//		This function takes the following steps:
		//		1. Create a checksum from the raw file provided and determine if the file already
		//		exists in the registry.
		//		2. Create the intermediate file format and store this format on disk.
		//		3. Register the new file with the in-memory s_ScriptRegistry and the on disk Script
		//		registry
		//		4. Instantiate the new Script into memory and return a handle to the new Script.
		static std::tuple<AssetHandle, bool> CreateNewScript(const std::string& name, const std::vector<WrappedVarType>& parameters,
			WrappedVarType returnValue, WrappedFuncType functionType);

		static void DeleteScript(AssetHandle handle);

		//==============================
		// Load and Retrieve In-Memory Script
		//==============================

		// Function to get a texture with a given name
		static Ref<Scripting::Script> GetScript(const AssetHandle& handle);
		// Function to get an Script buffer from its initial file location
		static std::tuple<AssetHandle, Ref<Scripting::Script>> GetScript(const std::filesystem::path& filepath);

		static std::unordered_map<AssetHandle, Ref<Scripting::Script>>& GetScriptMap()
		{
			return s_Scripts;
		}
	private:
		//==============================
		// Internal Functionality
		//==============================

		static void FillScriptMetadata(const std::string& name, const std::vector<WrappedVarType>& parameters,
			WrappedVarType returnValue, WrappedFuncType functionType, Assets::Asset& newAsset);
		// Function to Load a new Script from a buffer
		static Ref<Scripting::Script> InstantiateScriptIntoMemory(Assets::Asset& asset);
	private:
		// This registry holds a reference to all of the available Script in the current project.
		//		Since the registry only holds references, it does not instantiate any of the objects
		//		itself. It holds an AssetHandle to identify an asset and an Assets::Asset which holds
		//		metadata that is necessary to load the intermediate file correctly.
		static std::unordered_map<AssetHandle, Assets::Asset> s_ScriptRegistry;

		static std::unordered_map<AssetHandle, Ref<Scripting::Script>> s_Scripts;

		friend class Scripting::ScriptCore;
		friend class Scripting::ScriptModuleBuilder;

	//============================================================
	// Project
	//============================================================
	public:

	//==============================
	// Create New Project
	//==============================

	// This function simply instantiates a new project in the s_ActiveProject reference
	//		since only one project can exist at a time and there is no registry
	//		for projects.
	static Ref<Projects::Project> NewProject();


	//==============================
	// Save and Load a Project
	//==============================

	static Ref<Projects::Project> OpenProject(const std::filesystem::path& path);
	static bool SaveActiveProject(const std::filesystem::path& path);
	// Save a single project
	static bool SerializeProject(Ref<Projects::Project> project, const std::filesystem::path& filepath);
	// Load a single project
	static bool DeserializeProject(Ref<Projects::Project> project, const std::filesystem::path& filepath);

	private:
	//==============================
	// Internal Functionality
	//==============================

	static bool DeserializeServerVariables(Ref<Projects::Project> project, const std::filesystem::path& filepath);

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
			DeserializeInputModeRegistry();
			DeserializeScriptRegistry();
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
			SerializeInputModeRegistry();
			SerializeScriptRegistry();
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
			ClearInputModeRegistry();
			ClearScriptRegistry();
		}
	};
	

}
