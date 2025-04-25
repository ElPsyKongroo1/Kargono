#pragma once

// This header file is meant to be included inside external applications
//		(Editor and Runtime) as an easy single include to provide all of
//		the core engine functionality.

//-----------------AI-----------------//
#include "AIModule/AIService.h"

//-----------------Assets-----------------//
#include "AssetModule/Asset.h"
#include "AssetModule/AssetService.h"

//-----------------Audio-----------------//
#include "AudioModule/Audio.h"

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
#include "ECSModule/Entity.h"
#include "ECSModule/EngineComponents.h"
#include "ECSModule/ProjectComponent.h"

//-----------------Events-----------------//
#include "EventModule/KeyEvent.h"
#include "EventModule/MouseEvent.h"
#include "EventModule/NetworkingEvent.h"
#include "EventModule/SceneEvent.h"
#include "EventModule/PhysicsEvent.h"
#include "EventModule/EditorEvent.h"

//-----------------Input-----------------//
#include "InputModule/InputService.h"
#include "InputMapModule/InputMap.h"

//-----------------Math-----------------//
#include "Kargono/Math/Math.h"

//-----------------Network-----------------//
#include "NetworkModule/Client.h"
#include "NetworkModule/Server.h"

//-----------------Particles-----------------//
#include "ParticleModule/ParticleService.h"

//-----------------Project-----------------//
#include "Kargono/Projects/Project.h"

//-----------------Rendering-----------------//
#include "RenderModule/RenderingService.h"
#include "RenderModule/InputBuffer.h"
#include "RenderModule/Shader.h"
#include "RenderModule/Shape.h"
#include "RenderModule/Framebuffer.h"
#include "RenderModule/Texture.h"
#include "RenderModule/VertexArray.h"
#include "RenderModule/EditorPerspectiveCamera.h"

//-----------------Scene-----------------//
#include "Kargono/Scenes/Scene.h"
#include "Kargono/Scenes/GameState.h"

//-----------------Scripting-----------------//
#include "ScriptModule/ScriptService.h"
#include "ScriptModule/ScriptModuleBuilder.h"

//-----------------Runtime-UserInterface-----------------//
#include "RuntimeUIModule/RuntimeUI.h"
#include "RuntimeUIModule/Font.h"

//-----------------Editor-UserInterface-----------------//
#include "EditorUIModule/EditorUI.h"

//-----------------Utilities-----------------//
#include "Kargono/Utility/Time.h"
#include "Kargono/Utility/OSCommands.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/Utility/Operations.h"
#include "Kargono/Utility/Conversions.h"
#include "Kargono/Utility/Random.h"
#include "FileSystemModule/FileSystem.h"
