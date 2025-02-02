project "EditorTesting"
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
        "%{wks.location}/Editor/Source/**.cpp",
        "%{wks.location}/Editor/Source/**.h",
    }

    includedirs 
    {
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.filewatch}",
        "%{IncludeDir.imGui}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.Box2D}",
        "%{IncludeDir.dr_wav}",
        "%{IncludeDir.OpenALSoft}",
        "%{wks.location}/Engine/Source",
        "%{wks.location}/Editor/Source",
        "%{IncludeDir.optick}",
        "%{IncludeDir.asio}",
        "%{IncludeDir.doctest}",
        "%{IncludeDir.ImGuiColorTextEdit}"
    }

    libdirs
    {

    }

    links 
    { 
        "Engine"  
    }

    defines 
    {
		"_CRT_SECURE_NO_WARNINGS",
        "KG_TESTING"
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

    filter "configurations:Debug"
        kind "ConsoleApp"
        links 
        {
            "%{Library.OpenALSoft_Debug}"
        }

        -- FIXME: No other platform support for moving .dll files.
        filter { "system:windows", "configurations:Debug" }
            postbuildcommands { "xcopy \"%{DynamicLibrary.OpenALSoft_Debug}\" \"%{cfg.buildtarget.directory}\" /y" }

        defines "KG_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        kind "ConsoleApp"
        links 
        {
            "%{Library.OpenALSoft_Release}"
        }

        -- FIXME: No other platform support for moving .dll files.
        filter { "system:windows", "configurations:Release" }
            postbuildcommands { "xcopy \"%{DynamicLibrary.OpenALSoft_Release}\" \"%{cfg.buildtarget.directory}\" /y" }
        defines "KG_RELEASE"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        kind "ConsoleApp"
        links 
        {
            "%{Library.OpenALSoft_Dist}"
        }
        -- FIXME: No other platform support for moving .dll files.
        filter { "system:windows", "configurations:Dist" }
            postbuildcommands { "xcopy \"%{DynamicLibrary.OpenALSoft_Dist}\" \"%{cfg.buildtarget.directory}\" /y" }
        defines "KG_DIST"
        runtime "Release"
        optimize "on"
        symbols "off"
