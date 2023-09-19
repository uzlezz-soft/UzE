#include "uze/engine.h"
#include "uze/renderer/renderer.h"
#include "uze/renderer/shader.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <entt/entt.hpp>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace uze
{

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

	std::unique_ptr<Renderer> renderer;
	std::shared_ptr<Shader> shader;
	bool quit = false;

	static void gameLoop();

	void EntryPoint()
	{
		renderer = std::make_unique<Renderer>();
		if (!renderer->isValid())
		{
			std::cerr << "Cannot init renderer\n";
			return;
		}

		std::cout << SDL_GetPlatform() << "\n";

		RawShaderSpecification spec(R"(
		precision mediump float;
		
		layout (location = 0) in vec2 a_Position;
		
		out vec2 Position;
		
		void main()
		{
			Position = a_Position;
			gl_Position = vec4(Position, 0.0, 1.0);
		}
)",

		R"(
		precision mediump float;
		
		out vec4 Color;

		in vec2 Position;

		void main()
		{
			Color = vec4(Position, 0.0, 1.0);
		}
)");
		shader = renderer->createShader(spec);

		//Sound sound("C:/Users/User/Music/Soundpad/bigmak.wav");
		//sound.Play();

#if !defined(__EMSCRIPTEN__)
		while (!quit)
		{
			gameLoop();
		}
#else
		emscripten_set_main_loop(gameLoop, 0, 1);
#endif
	}

	static void gameLoop()
	{
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_EVENT_QUIT)
			{
				quit = true;
#if defined(__EMSCRIPTEN__)
				emscripten_cancel_main_loop();
#endif
			}
		}

		renderer->beginFrame();
		renderer->clear(1.f, 1.f, 1.f, 1.f);
		renderer->endFrame();
	}
	
}
