project "GLAD"
	kind "StaticLib"
	language "C"
	staticruntime "off"
	linkoptions { "-IGNORE:4098", "-IGNORE:4006","-IGNORE:4099", "-IGNORE:4996" }
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/glad/glad.h",
        "include/KHR/khrplatform.h",
        "src/glad.c"
	}

    includedirs
    {

        "include"
    }

    filter "system:windows"
    systemversion "latest"
    
