-- include "./Dependencies/premake/premake_customization/solution_items.lua"
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
    include "Dependencies/GLFW"
    include "Dependencies/Box2D"
    include "Dependencies/GLAD"
    include "Dependencies/imGui"
    include "Dependencies/msdf_atlas_gen"
    include "Dependencies/yaml-cpp"
group ""

group "Engine"
    include "Engine"
group ""

group "Applications"
    include "Editor"
    include "Runtime"
    include "Server"
group ""
group "Tools"
    include "Testing"
