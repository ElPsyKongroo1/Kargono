#pragma once

// This header file is meant to be included inside external applications
//		(Editor and Runtime) as an easy single include to provide all of
//		the core engine functionality.

//-----------------AI-----------------//
#include "Kargono/AI/AIService.h"

//-----------------Assets-----------------//
#include "Kargono/Assets/Asset.h"
#include "Kargono/Assets/AssetService.h"

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

//-----------------ECS-----------------//
#include "Kargono/ECS/Entity.h"
#include "Kargono/ECS/EngineComponents.h"
#include "Kargono/ECS/ProjectComponent.h"

//-----------------Events-----------------//
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Events/MouseEvent.h"
#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Events/SceneEvent.h"
#include "Kargono/Events/PhysicsEvent.h"
#include "Kargono/Events/EditorEvent.h"

//-----------------Input-----------------//
#include "Kargono/Input/InputService.h"
#include "Kargono/Input/InputMap.h"

//-----------------Math-----------------//
#include "Kargono/Math/Math.h"

//-----------------Network-----------------//
#include "Kargono/Network/Client.h"
#include "Kargono/Network/Server.h"

//-----------------Particles-----------------//
#include "Kargono/Particles/ParticleService.h"

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
#include "Kargono/Rendering/EditorPerspectiveCamera.h"

//-----------------Scene-----------------//
#include "Kargono/Scenes/Scene.h"
#include "Kargono/Scenes/GameState.h"

//-----------------Scripting-----------------//
#include "Kargono/Scripting/ScriptService.h"
#include "Kargono/Scripting/ScriptModuleBuilder.h"

//-----------------Runtime-UserInterface-----------------//
#include "Kargono/RuntimeUI/RuntimeUI.h"
#include "Kargono/RuntimeUI/Font.h"

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
