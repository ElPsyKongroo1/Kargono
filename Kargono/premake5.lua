project "Kargono"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"
    linkoptions { "-IGNORE:4098", "-IGNORE:4006" }

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "Kargono/kgpch.h"
    pchsource "Kargono/kgpch.cpp"
    
    files 
    {
		"Kargono.h",
		"Kargono/**.h",
		"Kargono/**.cpp",
        "dependencies/stb_image/**.cpp",
        "dependencies/stb_image/**.h",
        "Platform/**.h",
        "Platform/**.cpp"
    }

    defines 
    {
        "_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
    }
    includedirs 
    {
        "",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.GLAD}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.imGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.yaml_cpp}"
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
        "dwmapi.lib",
        "yaml-cpp"
    }

    filter "system:windows"
        systemversion "latest"

        defines 
        {

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