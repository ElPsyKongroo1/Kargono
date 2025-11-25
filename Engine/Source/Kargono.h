#pragma once

// This header file is meant to be included inside external applications
//		(Editor and Runtime) as an easy single include to provide all of
//		the core engine functionality.

//-----------------StateMachines-----------------//
#include "Modules/States/StatesContext.h"

//-----------------Assets-----------------//
#include "Modules/Assets/AssetsCommon.h"


//-----------------Audio-----------------//
#include "Modules/Audio/AudioContext.h"

//-----------------Core-----------------//
#include "Kargono/Core/Base.h"
#include "Modules/Core/Engine.h"
#include "Kargono/Core/Log.h"
#include "Kargono/Core/Assert.h"
#include "Kargono/Core/Timestep.h"
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/MouseCodes.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Core/Profiler.h"
#include "Kargono/Core/AppTick.h"

//-----------------ECS-----------------//
#include "Modules/ECS/Entity.h"
#include "Modules/ECSInternal/Assets/CustomComponent.h"

//-----------------Events-----------------//
#include "Modules/Events/KeyEvent.h"
#include "Modules/Events/MouseEvent.h"
#include "Modules/Events/NetworkingEvent.h"
#include "Modules/Events/SceneEvent.h"
#include "Modules/Events/PhysicsEvent.h"
#include "Modules/Events/EditorEvent.h"

//-----------------Input-----------------//
#include "Modules/Input/InputService.h"
#include "Modules/InputMap/Assets/InputMap.h"

//-----------------Math-----------------//
#include "Kargono/Math/Math.h"

//-----------------Network-----------------//
#include "Modules/Network/Client.h"
#include "Modules/Network/Server.h"

//-----------------Particles-----------------//
#include "Modules/Particles/ParticleContext.h"

//-----------------Project-----------------//
#include "Kargono/Projects/Project.h"

//-----------------Rendering-----------------//
#include "Modules/Rendering/RenderingService.h"
#include "Modules/Rendering/InputBuffer.h"
#include "Modules/Rendering/Assets/Shader.h"
#include "Modules/Rendering/Shape.h"
#include "Modules/Rendering/Framebuffer.h"
#include "Modules/Rendering/Assets/Texture2D.h"
#include "Modules/Rendering/VertexArray.h"
#include "Modules/Cameras/PerspectiveCamera.h"

//-----------------Scene-----------------//
#include "Modules/Scenes/Assets/Scene.h"
#include "Modules/GlobalState/Assets/GameState.h"

//-----------------Scripting-----------------//
#include "Modules/Scripting/ScriptModuleBinder.h"

//-----------------Runtime-UserInterface-----------------//
#include "Modules/RuntimeUI/RuntimeUIContext.h"
#include "Modules/RuntimeUI/Assets/Font.h"

//-----------------Editor-UserInterface-----------------//
#include "Modules/EditorUI/EditorUIInclude.h"

//-----------------Utilities-----------------//
#include "Kargono/Utility/Time.h"
#include "Kargono/Utility/OSCommands.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/Utility/Operations.h"
#include "Kargono/Utility/Conversions.h"
#include "Kargono/Utility/Random.h"
#include "Modules/FileSystem/FileSystem.h"
