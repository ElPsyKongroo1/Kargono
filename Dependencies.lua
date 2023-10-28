-- Kargono Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

-- Include Directories: Header files for external dependencies
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Kargono/dependencies/GLFW/include"
IncludeDir["GLAD"] = "%{wks.location}/Kargono/dependencies/GLAD/include"
IncludeDir["Box2D"] = "%{wks.location}/Kargono/dependencies/Box2D/include"
IncludeDir["spdlog"] = "%{wks.location}/Kargono/dependencies/spdlog"
IncludeDir["filewatch"] = "%{wks.location}/Kargono/dependencies/filewatch"
IncludeDir["imGui"] = "%{wks.location}/Kargono/dependencies/imgui"
IncludeDir["glm"] = "%{wks.location}/Kargono/dependencies/glm"
IncludeDir["stb_image"] = "%{wks.location}/Kargono/dependencies/stb_image"
IncludeDir["entt"] = "%{wks.location}/Kargono/dependencies/entt/include"
IncludeDir["yaml_cpp"] = "%{wks.location}/Kargono/dependencies/yaml-cpp/include"
IncludeDir["ImGuizmo"] = "%{wks.location}/Kargono/dependencies/ImGuizmo"
IncludeDir["free_type"] = "%{wks.location}/Kargono/dependencies/free_type/include"
IncludeDir["hash_library"] = "%{wks.location}/Kargono/dependencies/hash_library"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["mono"] = "%{wks.location}/Kargono/dependencies/mono/include"
IncludeDir["OpenALSoft"] = "%{wks.location}/Kargono/dependencies/OpenAL/include"
IncludeDir["dr_wav"] = "%{wks.location}/Kargono/dependencies/dr_wav/include"

-- Include Directories: Points to a directory with a variable number if library files.
-- Linker will check these paths when finding external libraries.
LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["mono"] = "%{wks.location}/Kargono/dependencies/mono/lib/%{cfg.buildcfg}"

-- Library Locations: Points to a specific library file for Linker to link to final .exe
Library = {}
Library["mono"] = "%{LibraryDir.mono}/libmono-static-sgen.lib"
Library["OpenALSoft_Debug"] = "%{wks.location}/Kargono/dependencies/OpenAL/lib/debug/OpenAL32.lib"
Library["OpenALSoft_Release"] = "%{wks.location}/Kargono/dependencies/OpenAL/lib/release/OpenAL32.lib"
Library["OpenALSoft_Dist"] = "%{wks.location}/Kargono/dependencies/OpenAL/lib/dist/OpenAL32.lib"

Library["free_type"] = "%{wks.location}/Kargono/dependencies/free_type/lib/Release/freetype.lib"

Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"

-- Dynamic Library Location: Points to a specific .dll file to link associate with .exe.
-- These files are usually copied to final working directory of .exe in a pre-build command.
DynamicLibrary = {}

DynamicLibrary["OpenALSoft_Debug"] = "%{wks.location}Kargono\\dependencies\\OpenAL\\lib\\debug\\OpenAL32.dll"
DynamicLibrary["OpenALSoft_Release"] = "%{wks.location}Kargono\\dependencies\\OpenAL\\lib\\release\\OpenAL32.dll"
DynamicLibrary["OpenALSoft_Dist"] = "%{wks.location}Kargono\\dependencies\\OpenAL\\lib\\dist\\OpenAL32.dll"