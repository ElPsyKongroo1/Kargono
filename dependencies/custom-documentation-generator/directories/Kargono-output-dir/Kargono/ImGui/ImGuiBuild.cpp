/// @brief Include the Kargono precompiled header file
#include "Kargono/kgpch.h"

/// @brief Disable warning 4996
#pragma warning(disable : 4996) // KG_WARNING_DISABLE 
/// @brief Define _CRT_SECURE_NO_WARNINGS to suppress warnings related to using unsafe functions
#define _CRT_SECURE_NO_WARNINGS
/// @brief Define IMGUI_IMPL_OPENGL_LOADER_GLAD for ImGui OpenGL support
#define IMGUI_IMPL_OPENGL_LOADER_GLAD

/// @brief Include the ImGui OpenGL backend implementation file
#include <backends/imgui_impl_opengl3.cpp>
/// @brief Include the ImGui GLFW backend implementation file
#include <backends/imgui_impl_glfw.cpp>

