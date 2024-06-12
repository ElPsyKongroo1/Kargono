# About Project

This project is a simple game editor for 2D projects (3D in the future). The project includes: a game editor (Editor), an exporting project (Runtime), a multiplayer server (Server), and a core engine library (Engine). The Editor, Server, and Runtime are all stand alone executables that depend on the Engine library. 

Feel free to try creating a game using the Editor or use an existing project (Currently only Pong is available and supported). Game Projects are stored in the Projects directory. New Projects need to be created by copy/pasting another project (I wil work on adding a new project button soon). 

The Editor only supports Windows. I do not have plans for supporting Linux or Mac for the Editor. The Runtime and Server executables are intended to be multi-platform, however, it only supports Windows currently. The latest version of this project exists in the Development-Branch. The Original-Kargono branch is an old version of the engine that probably will not build haha. I keep it there for reference.
# Quickstart

## Requirements
- Windows 64 bit is currently required for all executables.
- [Visual Studio](https://visualstudio.microsoft.com/downloads/) with C++ and C# support for projects. My build is vs2022. These configurations can be added on a fresh install of VS. Adding C#/C++ support to an existing installation requires opening the Visual Studio Installer Standalone app.
- C++ code standard of at least C++ 20. The build scripts deal with this, but I though I would mention it anyways.
- [Git](https://git-scm.com/downloads) for Windows.
- [Vulkan](https://vulkan.lunarg.com/) SDK with Debug Binaries. Version should be 3.0 or above.
## Instructions
- Clone this repository. No need to use recursive. Here is the command to run in a terminal/bash: git clone https://github.com/ElPsyKongroo1/Kargono.git
- Run BuildScripts/VS_GenProjects.bat to build Visual Studio Solution/Project files.
- Open Kargono.sln in central directory.
- Inside the Applications virtual directory, set either the Editor, Runtime, or Server as the default startup project.
- Build and Run!

# [Documentation](https://elpsykongroo1.github.io/Kargono/)
Documentation for getting started using the engine and internal descriptions of engine features. Documentation is not fully ready yet, but take a look if you want...
# Games
## Games in Progress
- Pong Clone
## Game Projects Ideas

- Breakout Clone
- Multiplayer Slap Game
- Dungeon Crawler

# Third-Party Dependencies

## Assimp ([Docs](https://assimp-docs.readthedocs.io/en/latest/))
Open Asset Import Library for loading and processing various 3D model file formats.
## Asio (Non-Boost Version) ([Docs](https://think-async.com/Asio/))
Backend that enables TCP/UDP network communication in the engine.
## Box2D ([Docs](https://box2d.org/documentation/))
Backend 2D Physics Library.
## Dr_wav ([Docs](https://github.com/mackron/dr_libs/tree/master))
Single-header library for decoding and encoding .wav audio files.
## entt ([Docs](https://github.com/skypjack/entt))
Entity Component System used for management of entities inside scenes.
## FileWatch ([Docs](https://github.com/ThomasMonkman/filewatch))
FileWatcher used to automatically update files in use in the editor.
## GLFW ([Docs](https://www.glfw.org/documentation.html))
Cross-Platform Library for creating and managing OS windows, OpenGL contexts, and handling input.

## GLEW ([Docs](https://glew.sourceforge.net/))
The OpenGL Extension Wrangler Library for handling OpenGL extensions and OpenGL function pointers.

## GLM ([Docs](https://github.com/g-truc/glm))
Mathematics library for graphics programming, providing vectors, matrices, and transformations.
## hash-library ([Docs](https://github.com/stbrumme/hash-library?tab=readme-ov-file))
Hash Library used for hashing assets for verification in the AssetManager and used in integrity checks in the Networking Code.
## ImGui ([Docs](https://github.com/ocornut/imgui))
Immediate Mode Graphical User Interface library for creating GUIs in C++. Used for EditorUI.
## ImGuizmo ([Docs](https://github.com/CedricGuillemet/ImGuizmo))
Tool produces a Guizmo that is usable in the Editor for modifying transforms and other applications.
## Mono ([Docs](https://www.mono-project.com/docs/about-mono/languages/cplusplus/))
Current system that allows engine to interact with C# Scripts.
## MSDF-Gen ([Docs](https://github.com/Chlumsky/msdfgen))
Generate msdf fonts for use in RuntimeUI
## MSDF Atlas Gen ([Docs](https://github.com/Chlumsky/msdf-atlas-gen))
Generate Glyph Atlas for MSDF Fonts. Built on top of MSDF-Gen.

## OpenGL ([Docs](https://www.khronos.org/opengl/))
Current Back-End Graphics API for rendering 2D and 3D graphics.
## OpenAL Soft ([Docs](https://github.com/kcat/openal-soft))
Cross-platform audio API for spatialized sound. Used as back-end for AudioEngine.
## Optick ([Docs](https://github.com/bombomby/optick))
Profiler used in Editor for Engine.

## stb_image ([Docs](https://github.com/nothings/stb/tree/master))
Single-header library for loading various image file formats.

## Vulkan ([Docs](https://vulkan.lunarg.com/))
Cross Compiler is used for shader generation.
