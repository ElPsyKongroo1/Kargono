project "ScriptEngine"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.7.2"
    
    targetdir ("../Editor/Resources/Scripts")
    objdir ("../Editor/Resources/Scripts/Intermediates")
    
    filter { "system:windows" }
        postbuildcommands { "{COPYDIR} \"%{wks.location}Editor/Resources/Scripts\" \"%{wks.location}Runtime/Resources/Scripts\"" }

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