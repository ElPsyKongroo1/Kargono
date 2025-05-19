project "Editor"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"
    linkoptions { "-IGNORE:4098", "-IGNORE:4006","-IGNORE:4099", "-IGNORE:4996" }
    targetdir ("%{wks.location}/Binary/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/Intermediates/" .. outputdir .. "/%{prj.name}")

    files 
    {
	    "%{wks.location}/Dependencies/hash_library/sha256.cpp",
        "%{wks.location}/Dependencies/hash_library/sha256.h",
        "%{wks.location}/Dependencies/hash_library/crc32.cpp",
        "%{wks.location}/Dependencies/hash_library/crc32.h",
		"%{wks.location}/Dependencies/implot/**.h",
        "%{wks.location}/Dependencies/implot/**.cpp",
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
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.hash_library}",
        "%{IncludeDir.dr_wav}",
        "%{IncludeDir.OpenALSoft}",
        "%{wks.location}/Engine/Source",
        "%{wks.location}/Editor/Source",
        "%{wks.location}/Engine",
        "%{IncludeDir.optick}",
		"%{IncludeDir.implot}",
        "%{IncludeDir.ImGuiColorTextEdit}",
    }
    
    links 
    { 
        "Engine"
    }

    defines 
    {
        "KG_EDITOR",
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
            "%{Library.ShaderC_Linux}",
            "gtk-4",
            --"gdk-4",
            "gio-2.0",
            "pangoft2-1.0",
            "gdk_pixbuf-2.0",
            "pangocairo-1.0",
            "cairo",
            "pango-1.0",
            "freetype",
            "fontconfig",
            "gobject-2.0",
            "gmodule-2.0",
            "gthread-2.0",
            "rt",
            "glib-2.0"
        }
        buildoptions {"`pkg-config --cflags gtk4`"}
        linkoptions { "-pthread" }

    filter "configurations:Debug"
        kind "ConsoleApp"
        defines "KG_DEBUG"
        runtime "Debug"
        symbols "on"
        filter { "system:windows", "configurations:Debug" }
            postbuildcommands { "xcopy \"%{DynamicLibrary.OpenALSoft_Debug}\" \"%{cfg.buildtarget.directory}\" /y" }
            links
            {
                "%{Library.OpenALSoft_Debug}"
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
        
