-- Kargono Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

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
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["mono"] = "%{wks.location}/Kargono/dependencies/mono/include"

LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["mono"] = "%{wks.location}/Kargono/dependencies/mono/lib/%{cfg.buildcfg}"

Library = {}
Library["mono"] = "%{LibraryDir.mono}/libmono-static-sgen.lib"

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