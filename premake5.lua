workspace "Kargono"
    startproject "Sandbox"
    architecture "x86_64"
    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Kargono/dependencies/GLFW/include"
IncludeDir["GLAD"] = "Kargono/dependencies/GLAD/include"
IncludeDir["spdlog"] = "Kargono/dependencies/spdlog"
IncludeDir["imGui"] = "Kargono/dependencies/imgui"
IncludeDir["glm"] = "Kargono/dependencies/glm"
IncludeDir["stb_image"] = "Kargono/dependencies/stb_image"

include "Kargono/dependencies/GLFW"
include "Kargono/dependencies/GLAD"
include "Kargono/dependencies/imGui"

project "Kargono"
    location "Kargono"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"
    linkoptions { "-IGNORE:4098", "-IGNORE:4006" }

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "Kargono/kgpch.h"
    pchsource "Kargono/Kargono/kgpch.cpp"
    
    files 
    {
		"Kargono/Kargono.h",
		"Kargono/Kargono/**.h",
		"Kargono/Kargono/**.cpp",
        "Kargono/dependencies/stb_image/**.cpp",
        "Kargono/dependencies/stb_image/**.h",
        "Kargono/Platform/**.h",
        "Kargono/Platform/**.cpp"
    }

    includedirs 
    {
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.GLAD}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.imGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "Kargono"
    }

    libdirs
    {
    }

    links 
    { 
        "GLFW",
        "GLAD",
        "opengl32.lib",
        "imGui",
        "dwmapi.lib"
    }

    filter "system:windows"
        systemversion "latest"

        defines 
        {
            "KG_BUILD_DLL",
            "GLFW_INCLUDE_NONE"
        }
        -- prebuildcommands 
        -- {
        --     "{COPYDIR} \"%{wks.location}Kargono/dependencies/dynamic_libraries\" \"%{cfg.buildtarget.directory}\""
        -- } 
        -- postbuildcommands 
        -- {
        --     "{COPYDIR} \"%{cfg.buildtarget.directory}\" \"%{cfg.buildtarget.directory}../Breakout\"",
        --     "{COPYDIR} \"%{cfg.buildtarget.directory}\" \"%{cfg.buildtarget.directory}../Sandbox3D\"",
        --     "{COPYDIR} \"%{cfg.buildtarget.directory}\" \"%{cfg.buildtarget.directory}../Sandbox\""
        -- }
    filter "configurations:Debug"
        defines "KG_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "KG_RELEASE"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "KG_DIST"
        runtime "Release"
        optimize "on"
project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"
    linkoptions { "-IGNORE:4098", "-IGNORE:4006" }
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "Sandbox/src/**.h",
        "Sandbox/src/**.cpp"
    }

    includedirs 
    {
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.glm}",
        "%{wks.location}/Kargono"

    }

    libdirs
    {

    }

    links 
    { 
        "Kargono"  
    }

    filter "system:windows"
        
        systemversion "latest"

    filter "configurations:Debug"
        defines "KG_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "KG_RELEASE"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "KG_DIST"
        runtime "Release"
        optimize "on"
