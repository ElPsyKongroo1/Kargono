workspace "Kargono"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


project "Kargono"
    location "Kargono"
    kind "SharedLib"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "Kargono/Kargono/**.h",
        "Kargono/Kargono/**.cpp",
		"Kargono/Kargono.h",
		"Kargono/src/**.h",
		"Kargono/src/**.cpp",
		"Kargono/dependencies/implementation/**.h",
		"Kargono/dependencies/implementation/**.cpp",
		"Kargono/dependencies/include/imgui/**.cpp",
		"Kargono/dependencies/include/imgui/**.h",
    }

    includedirs 
    {
        "Kargono/dependencies/include",
        "Kargono"
    }

    libdirs
    {
        "Kargono/dependencies/libraries/GLFW",
        "Kargono/dependencies/libraries/OpenAL",
        "Kargono/dependencies/libraries/Assimp",
    }

    links 
    { 
        "glfw3", "opengl32", "assimp-vc143-mtd", "OpenAL32" 
    }

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "Default"
        systemversion "latest"

        defines 
        {
            "KG_PLATFORM_WINDOWS",
            "KG_BUILD_DLL"
        }
        prebuildcommands 
        {
            "{COPYDIR} \"%{wks.location}Kargono/dependencies/dynamic_libraries\" \"%{cfg.buildtarget.directory}\""
        } 
        postbuildcommands 
        {
            "{COPYDIR} \"%{cfg.buildtarget.directory}\" \"%{cfg.buildtarget.directory}../Breakout\"",
            "{COPYDIR} \"%{cfg.buildtarget.directory}\" \"%{cfg.buildtarget.directory}../Sandbox3D\"",
            "{COPYDIR} \"%{cfg.buildtarget.directory}\" \"%{cfg.buildtarget.directory}../Sandbox2D\""
        }
    filter "configurations:Debug"
        defines "KG_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "KG_RELEASE"
        symbols "On"

    filter "configurations:Dist"
        defines "KG_DIST"
        symbols "On"

project "Breakout"
    location "Breakout"
    kind "ConsoleApp"
    language "C++"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "Breakout/src/**.h",
        "Breakout/src/**.cpp"
    }

    includedirs 
    {
        "%{wks.location}/Kargono/dependencies/include",
        "%{wks.location}/Kargono"
    }

    libdirs
    {
        "%{wks.location}/Kargono/dependencies/libraries/GLFW",
        "%{wks.location}/Kargono/dependencies/libraries/OpenAL",
        "%{wks.location}/Kargono/dependencies/libraries/Assimp",
    }

    links 
    { 
        "glfw3", "opengl32", "assimp-vc143-mtd", "OpenAL32", "Kargono" 
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
        symbols "On"

    filter "configurations:Release"
        defines "KG_RELEASE"
        symbols "On"

    filter "configurations:Dist"
        defines "KG_DIST"
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
        "%{wks.location}/Kargono/dependencies/include",
        "%{wks.location}/Kargono"
    }

    libdirs
    {
        "%{wks.location}/Kargono/dependencies/libraries/GLFW",
        "%{wks.location}/Kargono/dependencies/libraries/OpenAL",
        "%{wks.location}/Kargono/dependencies/libraries/Assimp",
    }

    links 
    { 
        "glfw3", "opengl32", "assimp-vc143-mtd", "OpenAL32", "Kargono"  
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
        symbols "On"

    filter "configurations:Release"
        defines "KG_RELEASE"
        symbols "On"

    filter "configurations:Dist"
        defines "KG_DIST"
        symbols "On"




project "Sandbox3D"
    location "Sandbox3D"
    kind "ConsoleApp"
    language "C++"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "Sandbox3D/src/**.h",
        "Sandbox3D/src/**.cpp"
    }

    includedirs 
    {
        "%{wks.location}/Kargono/dependencies/include",
        "%{wks.location}/Kargono"
    }

    libdirs
    {
        "%{wks.location}/Kargono/dependencies/libraries/GLFW",
        "%{wks.location}/Kargono/dependencies/libraries/OpenAL",
        "%{wks.location}/Kargono/dependencies/libraries/Assimp",
    }

    links 
    { 
        "glfw3", "opengl32", "assimp-vc143-mtd", "OpenAL32", "Kargono" 
    }

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

        defines 
        {
            "KG_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        defines "KG_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "KG_RELEASE"
        symbols "On"

    filter "configurations:Dist"
        defines "KG_DIST"
        symbols "On"