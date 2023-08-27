include "./dependencies/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"
workspace "Kargono"
    startproject "Kargono-Editor"
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
    include "dependencies/premake"
    include "Kargono/dependencies/GLFW"
    include "Kargono/dependencies/Box2D"
    include "Kargono/dependencies/GLAD"
    include "Kargono/dependencies/imGui"
    include "Kargono/dependencies/yaml-cpp"
group ""

include "Kargono"
include "Sandbox"
include "Kargono-Editor"