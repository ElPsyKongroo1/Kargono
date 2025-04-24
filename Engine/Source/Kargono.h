#pragma once

// This header file is meant to be included inside external applications
//		(Editor and Runtime) as an easy single include to provide all of
//		the core engine functionality.

//-----------------AI-----------------//
#include "AIPlugin/AIService.h"

//-----------------Assets-----------------//
#include "AssetsPlugin/Asset.h"
#include "AssetsPlugin/AssetService.h"

//-----------------Audio-----------------//
#include "AudioPlugin/Audio.h"

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

//-----------------ECS-----------------//
#include "ECSPlugin/Entity.h"
#include "ECSPlugin/EngineComponents.h"
#include "ECSPlugin/ProjectComponent.h"

//-----------------Events-----------------//
#include "EventsPlugin/KeyEvent.h"
#include "EventsPlugin/MouseEvent.h"
#include "EventsPlugin/NetworkingEvent.h"
#include "EventsPlugin/SceneEvent.h"
#include "EventsPlugin/PhysicsEvent.h"
#include "EventsPlugin/EditorEvent.h"

//-----------------Input-----------------//
#include "InputPlugin/InputService.h"
#include "InputMapPlugin/InputMap.h"

//-----------------Math-----------------//
#include "Kargono/Math/Math.h"

//-----------------Network-----------------//
#include "NetworkPlugin/Client.h"
#include "NetworkPlugin/Server.h"

//-----------------Particles-----------------//
#include "ParticlesPlugin/ParticleService.h"

//-----------------Project-----------------//
#include "Kargono/Projects/Project.h"

//-----------------Rendering-----------------//
#include "RenderingPlugin/RenderingService.h"
#include "RenderingPlugin/InputBuffer.h"
#include "RenderingPlugin/Shader.h"
#include "RenderingPlugin/Shape.h"
#include "RenderingPlugin/Framebuffer.h"
#include "RenderingPlugin/Texture.h"
#include "RenderingPlugin/VertexArray.h"
#include "RenderingPlugin/EditorPerspectiveCamera.h"

//-----------------Scene-----------------//
#include "Kargono/Scenes/Scene.h"
#include "Kargono/Scenes/GameState.h"

//-----------------Scripting-----------------//
#include "ScriptingPlugin/ScriptService.h"
#include "ScriptingPlugin/ScriptModuleBuilder.h"

//-----------------Runtime-UserInterface-----------------//
#include "RuntimeUIPlugin/RuntimeUI.h"
#include "RuntimeUIPlugin/Font.h"

//-----------------Editor-UserInterface-----------------//
#include "EditorUIPlugin/EditorUI.h"

//-----------------Utilities-----------------//
#include "Kargono/Utility/Time.h"
#include "Kargono/Utility/OSCommands.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/Utility/Operations.h"
#include "Kargono/Utility/Conversions.h"
#include "Kargono/Utility/Random.h"
#include "FileSystemPlugin/FileSystem.h"
