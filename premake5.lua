workspace "Kargono"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Kargono/dependencies/GLFW/include"
IncludeDir["GLAD"] = "Kargono/dependencies/GLAD/include"
IncludeDir["spdlog"] = "Kargono/dependencies/spdlog"
IncludeDir["imGui"] = "Kargono/dependencies/imgui"


include "Kargono/dependencies/GLFW"
include "Kargono/dependencies/GLAD"
include "Kargono/dependencies/imGui"

project "Kargono"
    location "Kargono"
    kind "SharedLib"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "Kargono/kgpch.h"
    pchsource "Kargono/Kargono/kgpch.cpp"
    
    files 
    {
		"Kargono/Kargono.h",
		"Kargono/Kargono/**.h",
		"Kargono/Kargono/**.cpp",
		"Kargono/dependencies/implementation/**.h",
		"Kargono/dependencies/implementation/**.cpp",
        "Kargono/Platform/**.h",
        "Kargono/Platform/**.cpp"
    }

    includedirs 
    {
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.GLAD}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.imGui}",

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
        cppdialect "C++20"
        staticruntime "Default"
        systemversion "latest"

        defines 
        {
            "KG_PLATFORM_WINDOWS",
            "KG_BUILD_DLL",
            "GLFW_INCLUDE_NONE"
        }
        -- prebuildcommands 
        -- {
        --     "{COPYDIR} \"%{wks.location}Kargono/dependencies/dynamic_libraries\" \"%{cfg.buildtarget.directory}\""
        -- } 
        postbuildcommands 
        {
            "{COPYDIR} \"%{cfg.buildtarget.directory}\" \"%{cfg.buildtarget.directory}../Breakout\"",
            "{COPYDIR} \"%{cfg.buildtarget.directory}\" \"%{cfg.buildtarget.directory}../Sandbox3D\"",
            "{COPYDIR} \"%{cfg.buildtarget.directory}\" \"%{cfg.buildtarget.directory}../Sandbox2D\""
        }
    filter "configurations:Debug"
        defines "KG_DEBUG"
        buildoptions "/MDd"
        symbols "On"

    filter "configurations:Release"
        defines "KG_RELEASE"
        buildoptions "/MD"
        symbols "On"

    filter "configurations:Dist"
        defines "KG_DIST"
        buildoptions "/MD"
        symbols "On"
project "Sandbox2D"
    location "Sandbox2D"
    kind "ConsoleApp"
    language "C++"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "Sandbox2D/src/**.h",
        "Sandbox2D/src/**.cpp"
    }

    includedirs 
    {
        "%{IncludeDir.spdlog}",
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
        cppdialect "C++20"
        staticruntime "Default"
        systemversion "latest"

        defines 
        {
            "KG_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        defines "KG_DEBUG"
        buildoptions "/MDd"
        symbols "On"

    filter "configurations:Release"
        defines "KG_RELEASE"
        buildoptions "/MD"
        symbols "On"

    filter "configurations:Dist"
        defines "KG_DIST"
        buildoptions "/MD"
        symbols "On"
