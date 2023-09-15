KargonoRootDir = "../../../.."

workspace "Facerun"
architecture "x86_64"
startproject "Facerun"
    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Facerun"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.7.2"
    
    targetdir ("Binaries")
    objdir ("Intermediates")
    
    files 
    {
		"Source/**.cs",
		"Properties/**.cs"
    }

    links
    {
        "Kargono-ScriptCore"
    }

    filter "configurations:Debug"
        optimize "Off"
        symbols "Default"

    filter "configurations:Release"
        optimize "On"
        symbols "Default"

    filter "configurations:Dist"
        optimize "Full"
        symbols "Off"

group "Kargono"
    include (KargonoRootDir .. "/Kargono-ScriptCore")
group ""