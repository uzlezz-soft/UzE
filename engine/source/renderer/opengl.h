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
#else
#include "glad/gl33.h"

#define UZE_GL UZE_OPENGL33
#endif

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

namespace uze
{

	const char* openGLErrorToString(u32 err) noexcept;

}

#define glCheck(x) \
	do { \
		while (glGetError() != 0); \
		x; \
		auto gl__error = glGetError(); \
		if (gl__error != 0) \
			std::cout << "OpenGLDebug: File " << std::filesystem::path(__FILE__).filename() \
			<< ", line " << __LINE__ << ":  " << ::uze::openGLErrorToString(gl__error) << "\n"; \
	} while (0)