project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"
    linkoptions { "-IGNORE:4098", "-IGNORE:4006" }
    
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs 
    {
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
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