 project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"
    linkoptions { "-IGNORE:4098", "-IGNORE:4006","-IGNORE:4099", "-IGNORE:4996" }

    targetdir ("%{wks.location}/Binary/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/Intermediates/" .. outputdir .. "/%{prj.name}")

    pchheader "kgpch.h"
    pchsource "Source/kgpch.cpp"
    
    files 
    {
		"Source/Kargono.h",
		"Source/kgpch.h",
		"Source/kgpch.cpp",
		"Source/Kargono/**.h",
		"Source/Kargono/**.cpp",
        "%{wks.location}/Dependencies/stb_image/**.cpp",
        "%{wks.location}/Dependencies/stb_image/**.h",
        "%{wks.location}/Dependencies/ImGuizmo/ImGuizmo.h",
        "%{wks.location}/Dependencies/ImGuizmo/ImGuizmo.cpp",
        "%{wks.location}/Dependencies/hash_library/sha256.cpp",
        "%{wks.location}/Dependencies/hash_library/sha256.h",
        "%{wks.location}/Dependencies/hash_library/crc32.cpp",
        "%{wks.location}/Dependencies/hash_library/crc32.h",
        "%{wks.location}/Dependencies/optick/src/**.cpp",
        "%{wks.location}/Dependencies/optick/src/**.h",
        "%{wks.location}/Dependencies/ImGuiColorTextEdit/TextEditor.cpp",
        "%{wks.location}/Dependencies/ImGuiColorTextEdit/TextEditor.h",
        "Source/API/**.h",
        "Source/API/**.cpp"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
        "KG_RENDERER_OPENGL"
    }
    includedirs 
    {
        "Source",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.GLAD}",
        "%{IncludeDir.filewatch}",
        "%{IncludeDir.Box2D}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.imGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.ImGuiNotify}",
        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.OpenALSoft}",
        "%{IncludeDir.dr_wav}",
        "%{IncludeDir.hash_library}",
        "%{IncludeDir.msdf_atlas_gen}",
        "%{IncludeDir.msdfgen}",
        "%{IncludeDir.optick}",
        "%{IncludeDir.asio}",
        "%{IncludeDir.ImGuiColorTextEdit}"
    }

    libdirs
    {
    }

    links 
    { 
        "GLFW",
        "Box2D",
        "GLAD",
        "opengl32.lib",
        "imGui",
        "dwmapi.lib",
        "yaml-cpp",
        "msdf-atlas-gen"
    }

    filter "files:../Dependencies/ImGuizmo/**.cpp"
        flags{ "NoPCH" }

    filter "files:../Dependencies/hash_library/**.cpp"
        flags{ "NoPCH" }

    filter "files:../Dependencies/optick/src/**.cpp"
        flags{ "NoPCH" }
    filter "files:../Dependencies/ImGuiColorTextEdit/**.cpp"
        flags{ "NoPCH" }

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

        links 
        {
            "%{Library.WinSock}",
            "%{Library.WinMM}",
            "%{Library.WinVersion}",
            "%{Library.BCrypt}"
        }
    filter "system:linux"
       systemversion "latest"
       defines
       {
            "KG_PLATFORM_LINUX"
       }
    filter "configurations:Debug"
        defines "KG_DEBUG"
        runtime "Debug"
        symbols "on"
        links
        {
            "%{Library.ShaderC_Debug}",
            "%{Library.SPIRV_Cross_Debug}",
            "%{Library.SPIRV_Cross_GLSL_Debug}"
        }
        filter { "system:windows", "configurations:Debug" }
            postbuildcommands { "xcopy \"%{DynamicLibrary.OpenALSoft_Debug}\" \"%{cfg.buildtarget.directory}\" /y" }
            links
            {
                "%{Library.OpenALSoft_Debug}",
            }
        filter { "system:linux", "configurations:Debug" }
            linkoptions
            {
                "%{Library.OpenALSoft_Debug_Linux}"
            }
        
    filter "configurations:Release"
        defines "KG_RELEASE"
        runtime "Release"
        optimize "on"
        links
        {
            "%{Library.ShaderC_Release}",
            "%{Library.SPIRV_Cross_Release}",
            "%{Library.SPIRV_Cross_GLSL_Release}",
        }
        filter { "system:windows", "configurations:Release" }
            postbuildcommands { "xcopy \"%{DynamicLibrary.OpenALSoft_Release}\" \"%{cfg.buildtarget.directory}\" /y" }
            links 
            {
                "%{Library.OpenALSoft_Release}"
            }
        filter { "system:linux", "configurations:Release" }
            linkoptions
            {
                "%{Library.OpenALSoft_Release_Linux}"
            }
    filter "configurations:Dist"
        defines "KG_DIST"
        runtime "Release"
        optimize "on"
        symbols "off"
        links
        {
            "%{Library.ShaderC_Release}",
            "%{Library.SPIRV_Cross_Release}",
            "%{Library.SPIRV_Cross_GLSL_Release}"
        }
        filter { "system:windows", "configurations:Dist" }
            postbuildcommands { "xcopy \"%{DynamicLibrary.OpenALSoft_Dist}\" \"%{cfg.buildtarget.directory}\" /y" }
            links 
            {
                "%{Library.OpenALSoft_Dist}"
            }
        filter { "system:linux", "configurations:Release" }
            linkoptions
            {
                "%{Library.OpenALSoft_Dist_Linux}"
            }
        
