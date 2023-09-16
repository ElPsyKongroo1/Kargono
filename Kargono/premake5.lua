project "Kargono"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"
    linkoptions { "-IGNORE:4098", "-IGNORE:4006","-IGNORE:4099" }

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "kgpch.h"
    pchsource "src/Kargono/kgpch.cpp"
    
    files 
    {
		"src/Kargono.h",
		"src/Kargono/**.h",
		"src/Kargono/**.cpp",
        "dependencies/stb_image/**.cpp",
        "dependencies/stb_image/**.h",
        "dependencies/ImGuizmo/ImGuizmo.h",
        "dependencies/ImGuizmo/ImGuizmo.cpp",
        "src/API/**.h",
        "src/API/**.cpp"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
        
    }
    includedirs 
    {
        "src",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.GLAD}",
        "%{IncludeDir.filewatch}",
        "%{IncludeDir.Box2D}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.imGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.mono}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.VulkanSDK}"
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
        "%{Library.mono}",
        "Kargono-ScriptCore"
    }

    filter "files:dependencies/ImGuizmo/**.cpp"
        flags{ "NoPCH" }

    filter "system:windows"
        systemversion "latest"

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


    filter "configurations:Release"
        defines "KG_RELEASE"
        runtime "Release"
        optimize "on"

        links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

    filter "configurations:Dist"
        defines "KG_DIST"
        runtime "Release"
        optimize "on"

        links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}