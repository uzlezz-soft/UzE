#include "uze/engine.h"
#include "uze/renderer/renderer.h"
#include "uze/renderer/shader.h"
#include "uze/platform.h"
#include "renderer/opengl.h"
#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include <iostream>

#include "glm/ext/matrix_transform.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace uze
{

	std::unique_ptr<Renderer> renderer;
	bool quit = false;

	static void gameLoop();

	static constexpr LogCategory log_engine { "Engine" };

	void EntryPoint()
	{
		renderer = std::make_unique<Renderer>();
		if (!renderer->isValid())
		{
			std::cerr << "Cannot init renderer\n";
			return;
		}

		uzLog(log_engine, Info, "Platform: {}", UZE_PLATFORM_STRING);

#if !defined(__EMSCRIPTEN__)
		std::cout << "Frame took: 0ms";
#endif

#if !defined(__EMSCRIPTEN__)
		while (!quit)
		{
			gameLoop();
		}
#else
		emscripten_set_main_loop(gameLoop, -1, 1);
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
		renderer->clear(0.5f, 1.f, 0.2f, 1.f);
		renderer->drawQuad(glm::mat4(1.0f), glm::vec4(0.0f, 0.5f, 0.3f, 1.0f));
		renderer->drawQuad(glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.2f, 0.0f)),
			glm::vec4(0.2f, 0.5f, 0.5f, 1.0f));
		renderer->endFrame();

#if !defined(__EMSCRIPTEN__)
		std::printf("\r                                                                                       \r");
#endif
		std::cout << "Frame took: " << renderer->getStatistics().frame_time_ms
			<< "ms; draw calls: " << renderer->getStatistics().num_draw_calls
			<< "; num quads: " << renderer->getStatistics().num_quads << "; PCIe: "
			<< renderer->getStatistics().data_transmitted << " bytes";
	}
	
}
