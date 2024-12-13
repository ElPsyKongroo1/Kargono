#pragma once
#if defined(KG_PLATFORM_WINDOWS)
    #include <GLFW/glfw3.h>
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
#endif
#if defined(KG_PLATFORM_LINUX)
    #include <GLFW/glfw3.h>
    #define GLFW_EXPOSE_NATIVE_WAYLAND
    #include <GLFW/glfw3native.h>
#endif
