project "Testing"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"
    linkoptions { "-IGNORE:4098", "-IGNORE:4006","-IGNORE:4099" }
    targetdir ("%{wks.location}/Binary/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/Intermediates/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "Source/**.h",
        "Source/**.cpp"
    }

    includedirs 
    {
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.filewatch}",
        "%{IncludeDir.imGui}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.Box2D}",
        "%{IncludeDir.dr_wav}",
        "%{IncludeDir.OpenALSoft}",
        "%{wks.location}/Engine/Source",
        "%{wks.location}/Runtime/Source",
        "%{IncludeDir.optick}",
        "%{IncludeDir.asio}",
        "%{IncludeDir.doctest}"
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
        "KG_EXPORT=0"
    }

    filter "system:windows"
        
        systemversion "latest"

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