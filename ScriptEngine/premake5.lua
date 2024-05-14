project "ScriptEngine"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.7.2"
    
    targetdir ("../Editor/resources/Scripts")
    objdir ("../Editor/resources/Scripts/Intermediates")
    
    filter { "system:windows" }
        postbuildcommands { "{COPYDIR} \"%{wks.location}Editor/resources/Scripts\" \"%{wks.location}Runtime/resources/Scripts\"" }

    files 
    {
		"Source/**.cs",
		"Properties/**.cs"
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