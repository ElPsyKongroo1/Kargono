#pragma once

// This header file is meant to be included inside external applications
//		(Editor and Runtime) as an easy single include to provide all of
//		the core engine functionality.

//-----------------Assets-----------------//
#include "Kargono/Assets/Asset.h"
#include "Kargono/Assets/AssetManager.h"

//-----------------Audio-----------------//
#include "Kargono/Audio/AudioEngine.h"

//-----------------Core-----------------//
#include "Kargono/Core/Base.h"
#include "Kargono/Core/EngineCore.h"
#include "Kargono/Core/Log.h"
#include "Kargono/Core/Assert.h"
#include "Kargono/Core/Timestep.h"
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/MouseCodes.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Core/Profiler.h"
#include "Kargono/Core/AppTick.h"

//-----------------Events-----------------//
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Events/NetworkingEvent.h"

//-----------------Input-----------------//
#include "Kargono/Input/InputPolling.h"
#include "Kargono/Input/InputMode.h"

//-----------------Math-----------------//
#include "Kargono/Math/Math.h"

//-----------------Network-----------------//
#include "Kargono/Network/Network.h"

//-----------------Project-----------------//
#include "Kargono/Projects/Project.h"

//-----------------Renderer-----------------//
#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Renderer/RenderCommand.h"
#include "Kargono/Renderer/InputBuffer.h"
#include "Kargono/Renderer/Shader.h"
#include "Kargono/Renderer/Shape.h"
#include "Kargono/Renderer/Framebuffer.h"
#include "Kargono/Renderer/Texture.h"
#include "Kargono/Renderer/VertexArray.h"
#include "Kargono/Renderer/EditorCamera.h"

//-----------------Scene-----------------//
#include "Kargono/Scene/Scene.h"
#include "Kargono/Scene/Components.h"
#include "Kargono/Scene/Entity.h"
#include "Kargono/Scene/EntityClass.h"
#include "Kargono/Scene/GameState.h"

//-----------------Scripting-----------------//
#include "Kargono/Script/ScriptEngine.h"
#include "Kargono/Scripting/Scripting.h"
#include "Kargono/Scripting/ScriptModuleBuilder.h"

//-----------------Runtime-UserInterface-----------------//
#include "Kargono/RuntimeUI/Runtime.h"
#include "Kargono/RuntimeUI/Text.h"

//-----------------Editor-UserInterface-----------------//
#include "Kargono/EditorUI/Editor.h"

//-----------------Utilities-----------------//
#include "Kargono/Utility/Time.h"
#include "Kargono/Utility/OSCommands.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/Utility/Operations.h"
#include "Kargono/Utility/Conversions.h"