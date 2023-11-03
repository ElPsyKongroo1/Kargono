project "Kargono-ScriptCore"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.7.2"
    
    targetdir ("../Kargono-Editor/Resources/Scripts")
    objdir ("../Kargono-Editor/Resources/Scripts/Intermediates")
    
    filter { "system:windows" }
        postbuildcommands { "{COPYDIR} \"%{wks.location}Kargono-Editor/Resources/Scripts\" \"%{wks.location}Runtime/Resources/Scripts\"" }

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