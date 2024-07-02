#pragma once

// This header file is meant to be included inside external applications
//		(Editor and Runtime) as an easy single include to provide all of
//		the core engine functionality.

//-----------------Assets-----------------//
#include "Kargono/Assets/Asset.h"
#include "Kargono/Assets/AssetManager.h"

//-----------------Audio-----------------//
#include "Kargono/Audio/Audio.h"

//-----------------Core-----------------//
#include "Kargono/Core/Base.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Core/Log.h"
#include "Kargono/Core/Assert.h"
#include "Kargono/Core/Timestep.h"
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/MouseCodes.h"
#include "Kargono/Core/Application.h"
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

//-----------------Rendering-----------------//
#include "Kargono/Rendering/RenderingService.h"
#include "Kargono/Rendering/InputBuffer.h"
#include "Kargono/Rendering/Shader.h"
#include "Kargono/Rendering/Shape.h"
#include "Kargono/Rendering/Framebuffer.h"
#include "Kargono/Rendering/Texture.h"
#include "Kargono/Rendering/VertexArray.h"
#include "Kargono/Rendering/EditorCamera.h"

//-----------------Scene-----------------//
#include "Kargono/Scenes/Scene.h"
#include "Kargono/Scenes/Components.h"
#include "Kargono/Scenes/Entity.h"
#include "Kargono/Scenes/EntityClass.h"
#include "Kargono/Scenes/GameState.h"

//-----------------Scripting-----------------//
#include "Kargono/Scripting/Scripting.h"
#include "Kargono/Scripting/ScriptModuleBuilder.h"

//-----------------Runtime-UserInterface-----------------//
#include "Kargono/RuntimeUI/RuntimeUI.h"
#include "Kargono/RuntimeUI/Text.h"

//-----------------Editor-UserInterface-----------------//
#include "Kargono/EditorUI/EditorUI.h"

//-----------------Utilities-----------------//
#include "Kargono/Utility/Time.h"
#include "Kargono/Utility/OSCommands.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/Utility/Operations.h"
#include "Kargono/Utility/Conversions.h"
#include "Kargono/Utility/Random.h"
#include "Kargono/Utility/FileSystem.h"
