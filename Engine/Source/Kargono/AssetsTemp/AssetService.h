#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Assets/Asset.h"


namespace Kargono
{
	namespace Scenes { class Scene; }
	namespace Scenes { class EntityClass; }
	namespace Scenes { class GameState; }
	namespace Projects { class Project; }
	namespace Rendering { class Shader; }
	namespace Rendering { class Texture2D; }
	namespace Audio { class AudioBuffer; }
	namespace RuntimeUI { class Font; }
	namespace Input { class InputMode; }
	namespace Projects { class Project; }
	namespace Scripting { class Script; }
	namespace RuntimeUI { class UserInterface; }
}

namespace Kargono::Assets
{

	class AssetServiceTemp
	{
	public:
		//==============================
		// Retrieve Assets
		//==============================
		static Ref<Audio::AudioBuffer> GetAudio(const AssetHandle& handle);
		static Ref<Scenes::EntityClass> GetEntityClass(const AssetHandle& handle);
		static Ref<RuntimeUI::Font> GetFont(const AssetHandle& handle);
		static Ref<Scenes::GameState> GetGameState(const AssetHandle& handle);
		static Ref<Input::InputMode> GetInputMode(const AssetHandle& handle);
		static Ref<Scenes::Scene> GetScene(const AssetHandle& handle);
		static Ref<Scripting::Script> GetScript(const AssetHandle& handle);
		static Ref<Kargono::Rendering::Shader> GetShader(const AssetHandle& handle);
		static Ref<Rendering::Texture2D> GetTexture(const AssetHandle& handle);
		static Ref<RuntimeUI::UserInterface> GetUserInterface(const AssetHandle& handle);

		//==============================
		// Query Asset Metadata
		//==============================
		static std::filesystem::path GetAudioIntermediateLocation(const AssetHandle& handle);
		static std::filesystem::path GetEntityClassIntermediateLocation(const AssetHandle& handle);
		static std::filesystem::path GetFontIntermediateLocation(const AssetHandle& handle);
		static std::filesystem::path GetGameStateIntermediateLocation(const AssetHandle& handle);
		static std::filesystem::path GetInputModeIntermediateLocation(const AssetHandle& handle);
		static std::filesystem::path GetSceneIntermediateLocation(const AssetHandle& handle);
		static std::filesystem::path GetScriptIntermediateLocation(const AssetHandle& handle);
		static std::filesystem::path GetShaderIntermediateLocation(const AssetHandle& handle);
		static std::filesystem::path GetTextureIntermediateLocation(const AssetHandle& handle);
		static std::filesystem::path GetUserInterfaceIntermediateLocation(const AssetHandle& handle);
	};
}
