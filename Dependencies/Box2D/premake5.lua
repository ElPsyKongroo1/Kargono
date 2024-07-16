project "Box2D"
	kind "StaticLib"
	language "C++"
	cppdialect "C++11"
	staticruntime "on"
	linkoptions { "-IGNORE:4098", "-IGNORE:4006","-IGNORE:4099", "-IGNORE:4996" }
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",
		"include/**.h"
	}

	includedirs
	{
		"include",
		"src"
	}
	defines 
    {
		"_CRT_SECURE_NO_WARNINGS",
    }

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"
        symbols "off"
