project "Runtime"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"
    linkoptions { "-IGNORE:4098", "-IGNORE:4006","-IGNORE:4099", "-IGNORE:4996" }
    targetdir ("%{wks.location}/Binary/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/Intermediates/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "Source/**.h",
        "Source/**.cpp",
		"%{wks.location}/Dependencies/hash_library/sha256.cpp",
        "%{wks.location}/Dependencies/hash_library/sha256.h",
        "%{wks.location}/Dependencies/hash_library/crc32.cpp",
        "%{wks.location}/Dependencies/hash_library/crc32.h"
    }

    includedirs 
    {
        "%{wks.location}/Runtime/Source",
        "%{wks.location}/Engine/Source",
        "%{wks.location}/Engine/Modules/AI/Source",
        "%{wks.location}/Engine/Modules/Assets/Source",
        "%{wks.location}/Engine/Modules/Audio/Source",
        "%{wks.location}/Engine/Modules/ECS/Source",
        "%{wks.location}/Engine/Modules/EditorUI/Source",
        "%{wks.location}/Engine/Modules/Events/Source",
        "%{wks.location}/Engine/Modules/FileSystem/Source",
        "%{wks.location}/Engine/Modules/Input/Source",
        "%{wks.location}/Engine/Modules/InputMap/Source",
        "%{wks.location}/Engine/Modules/Network/Source",
        "%{wks.location}/Engine/Modules/Particles/Source",
        "%{wks.location}/Engine/Modules/Physics2D/Source",
        "%{wks.location}/Engine/Modules/Rendering/Source",
        "%{wks.location}/Engine/Modules/RuntimeUI/Source",
        "%{wks.location}/Engine/Modules/Scripting/Source",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.filewatch}",
        "%{IncludeDir.imGui}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.Box2D}",
		"%{IncludeDir.hash_library}",
        "%{IncludeDir.dr_wav}",
        "%{IncludeDir.OpenALSoft}",
        "%{IncludeDir.optick}"

    }

    links 
    { 
        "Engine"
    }

    defines 
    {
        "KG_RUNTIME",
        "_CRT_SECURE_NO_WARNINGS"
    }

    filter "system:windows"
        
        systemversion "latest"
        buildoptions 
        {
            "/wd4996" 
        }
        defines 
        {
            "KG_PLATFORM_WINDOWS"
        }
    filter "system:linux"
        systemversion "latest"
	defines
	{
            "KG_PLATFORM_LINUX"
	}
    linkoptions { "`pkg-config --libs gtk4`" }
	links 
	{ 
            "GLFW",
            "Box2D",
            "GLAD",
            "imGui",
            "yaml-cpp",
            "msdf-atlas-gen",
            "msdfgen",
            "freetype",
            "%{Library.ShaderC_Linux}"
	}
    filter "configurations:Debug"
        kind "ConsoleApp"
        defines "KG_DEBUG"
        runtime "Debug"
        symbols "on"
        filter { "system:windows", "configurations:Debug" }
            postbuildcommands { "xcopy \"%{DynamicLibrary.OpenALSoft_Debug}\" \"%{cfg.buildtarget.directory}\" /y" }
            links
            {
                "%{Library.OpenALSoft_Debug}",
            }
        filter { "system:linux", "configurations:Debug" }
            links
            {
            	"%{DynamicLibrary.OpenALSoft_Debug_Linux}"
            }
    filter "configurations:Release"
        kind "ConsoleApp"
        defines "KG_RELEASE"
        runtime "Release"
        optimize "on"
        symbols "on"
        filter { "system:windows", "configurations:Release" }
            postbuildcommands { "xcopy \"%{DynamicLibrary.OpenALSoft_Release}\" \"%{cfg.buildtarget.directory}\" /y" }
            links 
            {
                "%{Library.OpenALSoft_Release}"
            }
        filter { "system:linux", "configurations:Release" }
            links
            {
            	"%{DynamicLibrary.OpenALSoft_Release_Linux}"
            }
    filter "configurations:Dist"
        kind "WindowedApp"
        defines "KG_DIST"
        runtime "Release"
        optimize "on"
        symbols "off"
        filter { "system:windows", "configurations:Dist" }
            postbuildcommands { "xcopy \"%{DynamicLibrary.OpenALSoft_Dist}\" \"%{cfg.buildtarget.directory}\" /y" }
            links 
            {
                "%{Library.OpenALSoft_Dist}"
            }
        filter { "system:linux", "configurations:Dist" }
            links
            {
            	"%{DynamicLibrary.OpenALSoft_Dist_Linux}"
            }
        
