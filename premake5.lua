-- include "./dependencies/premake/premake_customization/solution_items.lua"
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

    -- solution_items
	-- {
	-- 	".editorconfig"
	-- }

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
    include "Engine/dependencies/msdf_atlas_gen"
    include "Engine/dependencies/yaml-cpp"
group ""

group "Engine"
    include "ScriptEngine" 
    include "Engine"
group ""

group "Applications"
    include "Editor"
    include "Runtime"
    include "Server"
group ""
