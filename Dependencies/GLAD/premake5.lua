project "GLAD"
	kind "StaticLib"
	language "C"
	staticruntime "on"
	linkoptions { "-IGNORE:4098", "-IGNORE:4006","-IGNORE:4099", "-IGNORE:4996" }
	targetdir ("%{wks.location}/Binary/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/Intermediates/" .. outputdir .. "/%{prj.name}")

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
	
	defines 
    {
		"_CRT_SECURE_NO_WARNINGS",
    }

    filter "system:windows"
    systemversion "latest"
    
