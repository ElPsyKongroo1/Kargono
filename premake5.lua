include "./dependencies/premake/premake_customization/solution_items.lua"

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

IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Kargono/dependencies/GLFW/include"
IncludeDir["GLAD"] = "%{wks.location}/Kargono/dependencies/GLAD/include"
IncludeDir["spdlog"] = "%{wks.location}/Kargono/dependencies/spdlog"
IncludeDir["imGui"] = "%{wks.location}/Kargono/dependencies/imgui"
IncludeDir["glm"] = "%{wks.location}/Kargono/dependencies/glm"
IncludeDir["stb_image"] = "%{wks.location}/Kargono/dependencies/stb_image"
IncludeDir["entt"] = "%{wks.location}/Kargono/dependencies/entt/include"

group "Dependencies"
    include "dependencies/premake"
    include "Kargono/dependencies/GLFW"
    include "Kargono/dependencies/GLAD"
    include "Kargono/dependencies/imGui"
group ""

include "Kargono"
include "Sandbox"
include "Kargono-Editor"