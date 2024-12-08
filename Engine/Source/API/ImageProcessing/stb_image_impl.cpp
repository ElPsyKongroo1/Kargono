#include "kgpch.h"

//==============================
// Dependency Implementation
//==============================
#if defined (KG_PLATFORM_LINUX)
	#define STBI_NO_SIMD // Here is a link to the issue: https://stackoverflow.com/questions/71495886/error-when-compiling-source-file-which-includes-stb-image-h/71496373
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
