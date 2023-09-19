#include "uze/renderer/renderer.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "glad/gles3.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

#include "SDL3_mixer/SDL_mixer.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace uze
{

	static bool createGLESContext(i32 major, i32 minor, SDL_Window*& window, SDL_GLContext& gl_context)
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);

		window = SDL_CreateWindow("Uzlezz Engine Window", 1600, 900, SDL_WINDOW_OPENGL);
		if (!window) return false;

		gl_context = SDL_GL_CreateContext(window);
		if (!gl_context)
		{
			SDL_DestroyWindow(window);
			window = nullptr;
			return false;
		}

		if (!gladLoadGLES2(SDL_GL_GetProcAddress))
		{
			SDL_GL_DeleteContext(gl_context);
			SDL_DestroyWindow(window);
			gl_context = nullptr;
			window = nullptr;
			return false;
		}

		return true;
	}

	Renderer::Renderer()
	{
		if (SDL_Init(SDL_INIT_VIDEO) != 0) return;

		int rendererFlags = 0;
#if defined(_DEBUG)
		rendererFlags |= SDL_GL_CONTEXT_DEBUG_FLAG;
#endif
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, rendererFlags);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		if (!createGLESContext(3, 0, m_window, m_gl_context))
			return;
		SDL_GL_MakeCurrent(m_window, m_gl_context);

		m_valid = true;

		i32 numTextureUnits = 0;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &numTextureUnits);

		GLint major, minor;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);

		m_caps.gl_version_major = major;
		m_caps.gl_version_minor = minor;
		{
			std::stringstream ss;
			ss << major << minor << "0 es";
			m_caps.shading_language_version = ss.str();
		}

		std::cout << std::setw(34) << std::left << "OpenGLES " << major << "." << minor << "\n";
		std::cout << std::setw(34) << std::left << "OpenGL Shading Language Version: " << (char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
		std::cout << std::setw(34) << std::left << "OpenGL Vendor:" << (char*)glGetString(GL_VENDOR) << std::endl;
		std::cout << std::setw(34) << std::left << "OpenGL Renderer:" << (char*)glGetString(GL_RENDERER) << std::endl;

		std::cout << std::setw(34) << std::left << "Num of available texture units: " << numTextureUnits << "\n";
	}

	Renderer::~Renderer()
	{
		if (!m_valid) return;

		SDL_GL_DeleteContext(m_gl_context);
		SDL_DestroyWindow(m_window);
	}

	void Renderer::clear(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Renderer::beginFrame()
	{
		
	}

	void Renderer::endFrame()
	{
		SDL_GL_SwapWindow(m_window);
	}

	std::shared_ptr<Shader> Renderer::createShader(const ShaderSpecification& spec)
	{
		return std::shared_ptr<Shader>(new Shader(spec, *this));
	}
}
