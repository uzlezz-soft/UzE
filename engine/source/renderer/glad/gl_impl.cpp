#include "uze/platform.h"

#if UZE_PLATFORM == UZE_PLATFORM_WEB
#define GLAD_GLES2_IMPLEMENTATION
#include "gles3.h"
#else
#define GLAD_GL_IMPLEMENTATION
#include "gl33.h"
#endif