include "./dependencies/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"
workspace "Kargono"
    startproject "Editor"
    architecture "x86_64"
    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    solution_items
	{
		".editorconfig"
	}

    flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


group "Dependencies"
    include "Engine/dependencies/GLFW"
    include "Engine/dependencies/Box2D"
    include "Engine/dependencies/GLAD"
    include "Engine/dependencies/imGui"
    include "Engine/dependencies/yaml-cpp"
group ""

group "Core"
    include "ScriptEngine" 
    include "Engine"
group ""

group "Tools"
    include "Editor"
    include "Runtime"
group ""
