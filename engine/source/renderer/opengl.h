#pragma once

#include "uze/common.h"
#include "uze/platform.h"
#include <filesystem>
#include <iostream>

#define UZE_OPENGL33   0
#define UZE_OPENGLES30 1

#if UZE_PLATFORM == UZE_PLATFORM_WEB
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <emscripten.h>

#define UZE_GL UZE_OPENGLES30
#define UZE_GL_STRING "OpenGL ES"
#else
#include "glad/gl33.h"

#define UZE_GL UZE_OPENGL33
#define UZE_GL_STRING "OpenGL"
#endif

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

namespace uze
{

	const char* openGLErrorToString(u32 err) noexcept;
	void logOpenGLError(u32 err, const char* file, u64 line);

}

#define glCheck(x) \
	do { \
		while (glGetError() != 0); \
		x; \
		auto gl__error = glGetError(); \
		if (gl__error != 0) \
			::uze::logOpenGLError(gl__error, __FILE__, __LINE__); \
	} while (0)