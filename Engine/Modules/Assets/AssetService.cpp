#include "kgpch.h"

#include "Modules/Assets/AssetService.h"

// TODO: NOTE THIS IS SUPER TEMPORARY UNTIL I GET THE MODULE SYSTEM WORKING AHHHHHH I HATE THIS SOOOO MUCH FAKK
namespace Kargono::Assets
{
	AssetManager<Rendering::Shader> s_ShaderManager;
	AssetManager<Rendering::Texture2D> s_Texture2DManager;
	AssetManager<Audio::AudioBuffer> s_AudioBufferManager;
	AssetManager<RuntimeUI::Font> s_FontManager;
	AssetManager<Scripting::Script> s_ScriptManager;
	AssetManager<ECSInternal::CustomComponent> s_CustomComponentManager;
	AssetManager<Scripting::CustomEnum> s_CustomEnumManager;
	AssetManager<EditorUI::ColorPalette> s_ColorPaletteManager;
	AssetManager<InputMap::InputMap> s_InputMapManager;
	AssetManager<Particles::EmitterConfig> s_EmitterConfigManager;
	AssetManager<GlobalState::GameState> s_GameStateManager;
	AssetManager<GlobalState::GlobalState> s_GlobalStateManager;
	AssetManager<RuntimeUI::UserInterface> s_UserInterfaceManager;
	AssetManager<States::State> s_StateManager;
	AssetManager<Scenes::Scene> s_SceneManager;
}