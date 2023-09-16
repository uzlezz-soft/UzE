#include "UzE/Engine.h"

#include <iomanip>
#include <iostream>
#include <SDL3/SDL.h>
#include "glad/gles3.h"
#include <SDL3_mixer/SDL_mixer.h>

namespace uze
{

	struct SDLContext
	{
		bool Success{ false };

		SDL_Window* Window{ nullptr };
		SDL_GLContext GLContext;

		SDLContext()
		{
			if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) return;

			int audioDevices = 10;
			auto devices = SDL_GetAudioOutputDevices(&audioDevices);

			for (int i = 0; i < audioDevices; ++i)
			{
				std::cout << i << ": " << devices[i] << "\n";
			}

			SDL_AudioSpec spec;
			spec.channels = 2;
			spec.format = MIX_DEFAULT_FORMAT;
			spec.freq = 44100;
			if (Mix_OpenAudio(devices[3], &spec) != 0)
			{
				SDL_free(devices);
				return;
			}
			SDL_free(devices);

			int rendererFlags = 0;
#if defined(_DEBUG) && _DEBUG
			rendererFlags |= SDL_GL_CONTEXT_DEBUG_FLAG;
#endif
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, rendererFlags);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

			auto windowFlags = SDL_WINDOW_OPENGL;
			Window = SDL_CreateWindow("Uzlezz Engine", 1600, 900, windowFlags);
			if (!Window) return;

			GLContext = SDL_GL_CreateContext(Window);
			if (!GLContext) return;
			SDL_GL_MakeCurrent(Window, GLContext);

			if (!gladLoadGLES2(SDL_GL_GetProcAddress))
				return;

			GLint major, minor;
			glGetIntegerv(GL_MAJOR_VERSION, &major);
			glGetIntegerv(GL_MINOR_VERSION, &minor);
			std::cout << std::setw(34) << std::left << "OpenGLES " << major << "." << MIX_FADING_OUT << "\n";
			std::cout << std::setw(34) << std::left << "OpenGL Shading Language Version: " << (char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
			std::cout << std::setw(34) << std::left << "OpenGL Vendor:" << (char*)glGetString(GL_VENDOR) << std::endl;
			std::cout << std::setw(34) << std::left << "OpenGL Renderer:" << (char*)glGetString(GL_RENDERER) << std::endl;

			Success = true;
		}

		~SDLContext()
		{
			if (GLContext)
				SDL_GL_DeleteContext(GLContext);

			if (Window)
				SDL_DestroyWindow(Window);

			SDL_Quit();
		}
	};

	struct Sound
	{
		Mix_Chunk* Chunk;

		Sound(std::string_view path)
		{
			Chunk = Mix_LoadWAV(path.data());
		}

		~Sound()
		{
			Mix_FreeChunk(Chunk);
		}

		void Play() const
		{
			Mix_PlayChannel(-1, Chunk, 0);
		}
	};

	void EntryPoint()
	{
		std::cout << "Hello World!\n";

		const SDLContext sdlContext;
		if (!sdlContext.Success)
		{
			std::cout << "Cannot initialize SDL. Exiting\n";
			return;
		}

		std::cout << SDL_GetPlatform() << "\n";

		Sound sound("C:/Users/User/Music/Soundpad/bigmak.wav");
		sound.Play();

		bool quit = false;
		while (!quit)
		{
			SDL_Event e;
			while (SDL_PollEvent(&e) != 0)
			{
				if (e.type == SDL_EVENT_QUIT)
					quit = true;
			}

			glClearColor(1.f, 1.f, 1.f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT);

			SDL_GL_SwapWindow(sdlContext.Window);
		}

	}

}