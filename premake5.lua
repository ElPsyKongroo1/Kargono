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

    defines
    {
        "$(ExternalCompilerOptions)"
    }

    flags
    {
        "MultiProcessorCompile"
    }

outputdir = "%{cfg.buildcfg}_%{cfg.system}"


group "Dependencies"
    include "Dependencies/GLFW"
    include "Dependencies/Box2D"
    include "Dependencies/GLAD"
    include "Dependencies/imgui"
    include "Dependencies/msdf_atlas_gen/msdfgen"
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
group "Testing"
    include "Testing/EditorTesting"
    include "Testing/RuntimeTesting"
    include "Testing/ServerTesting"
