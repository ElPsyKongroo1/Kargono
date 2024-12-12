-- Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

-- Include Directories: Header files for external Dependencies
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Dependencies/GLFW/include"
IncludeDir["GLAD"] = "%{wks.location}/Dependencies/GLAD/include"
IncludeDir["Box2D"] = "%{wks.location}/Dependencies/Box2D/include"
IncludeDir["spdlog"] = "%{wks.location}/Dependencies/spdlog"
IncludeDir["filewatch"] = "%{wks.location}/Dependencies/filewatch"
IncludeDir["imGui"] = "%{wks.location}/Dependencies/imgui"
IncludeDir["glm"] = "%{wks.location}/Dependencies/glm"
IncludeDir["stb_image"] = "%{wks.location}/Dependencies/stb_image"
IncludeDir["entt"] = "%{wks.location}/Dependencies/entt/include"
IncludeDir["yaml_cpp"] = "%{wks.location}/Dependencies/yaml-cpp/include"
IncludeDir["ImGuizmo"] = "%{wks.location}/Dependencies/ImGuizmo"
IncludeDir["ImGuiNotify"] = "%{wks.location}/Dependencies/ImGuiNotify"
--IncludeDir["free_type"] = "%{wks.location}/Dependencies/free_type/include"
IncludeDir["hash_library"] = "%{wks.location}/Dependencies/hash_library"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["OpenALSoft"] = "%{wks.location}/Dependencies/OpenAL/include"
IncludeDir["msdfgen"] = "%{wks.location}/Dependencies/msdf_atlas_gen/msdfgen"
IncludeDir["msdf_atlas_gen"] = "%{wks.location}/Dependencies/msdf_atlas_gen"
IncludeDir["dr_wav"] = "%{wks.location}/Dependencies/dr_wav/include"
IncludeDir["optick"] = "%{wks.location}/Dependencies/optick/include"
IncludeDir["asio"] = "%{wks.location}/Dependencies/asio/include"
IncludeDir["doctest"] = "%{wks.location}/Dependencies/doctest"
IncludeDir["ImGuiColorTextEdit"] = "%{wks.location}/Dependencies/ImGuiColorTextEdit"

-- Include Directories: Points to a directory with a variable number if library files.
-- Linker will check these paths when finding external libraries.
LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Linux"] = "/usr/lib/x86_64-linux-gnu"

-- Library Locations: Points to a specific library file for Linker to link to final .exe
Library = {}
Library["OpenALSoft_Debug"] = "%{wks.location}/Dependencies/OpenAL/lib/debug/OpenAL32.lib"
Library["OpenALSoft_Release"] = "%{wks.location}/Dependencies/OpenAL/lib/release/OpenAL32.lib"
Library["OpenALSoft_Dist"] = "%{wks.location}/Dependencies/OpenAL/lib/dist/OpenAL32.lib"
Library["OpenALSoft_Debug_Linux"] = "%{wks.location}/Dependencies/OpenAL/lib/debug/:libopenal.a"
Library["OpenALSoft_Release_Linux"] = "%{wks.location}/Dependencies/OpenAL/lib/release/:libopenal.a"
Library["OpenALSoft_Dist_Linux"] = "%{wks.location}/Dependencies/OpenAL/lib/dist/:libopenal.a"

-- Library["msdf_gen_Debug"] = "%{wks.location}/Dependencies/msdf_gen/lib/debug/msdfgen-core.lib"
-- Library["msdf_gen_Release"] = "%{wks.location}/Dependencies/msdf_gen/lib/release/msdfgen-core.lib"
-- Library["msdf_gen_Dist"] = "%{wks.location}/Dependencies/msdf_gen/lib/dist/msdfgen-core.lib"

-- Library["msdf_gen_ext_Debug"] = "%{wks.location}/Dependencies/msdf_gen/lib/debug/msdfgen-ext.lib"
-- Library["msdf_gen_ext_Release"] = "%{wks.location}/Dependencies/msdf_gen/lib/release/msdfgen-ext.lib"
-- Library["msdf_gen_ext_Dist"] = "%{wks.location}/Dependencies/msdf_gen/lib/dist/msdfgen-ext.lib"

-- Library["msdf_atlas_gen_Debug"] = "%{wks.location}/Dependencies/msdf_atlas_gen/lib/debug/msdf-atlas-gen.lib"
-- Library["msdf_atlas_gen_Release"] = "%{wks.location}/Dependencies/msdf_atlas_gen/lib/release/msdf-atlas-gen.lib"
-- Library["msdf_atlas_gen_Dist"] = "%{wks.location}/Dependencies/msdf_atlas_gen/lib/dist/msdf-atlas-gen.lib"

--Library["free_type"] = "%{wks.location}/Dependencies/free_type/lib/Release/freetype.lib"

Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

Library["ShaderC_Linux"] = "%{LibraryDir.VulkanSDK_Linux}/:libshaderc_combined.a"
-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"

-- Dynamic Library Location: Points to a specific .dll file to link associate with .exe.
-- These files are usually copied to final working directory of .exe in a pre-build command.
DynamicLibrary = {}

DynamicLibrary["OpenALSoft_Debug"] = "%{wks.location}Dependencies\\OpenAL\\lib\\debug\\OpenAL32.dll"
DynamicLibrary["OpenALSoft_Release"] = "%{wks.location}Dependencies\\OpenAL\\lib\\release\\OpenAL32.dll"
DynamicLibrary["OpenALSoft_Dist"] = "%{wks.location}Dependencies\\OpenAL\\lib\\dist\\OpenAL32.dll"
DynamicLibrary["OpenALSoft_Debug_Linux"] = "%{wks.location}Dependencies/OpenAL/lib/debug/libopenal.so"
DynamicLibrary["OpenALSoft_Release_Linux"] = "%{wks.location}Dependencies/OpenAL/lib/release/libopenal.so"
DynamicLibrary["OpenALSoft_Dist_Linux"] = "%{wks.location}Dependencies/OpenAL/lib/dist/libopenal.so"
