#include "uze/engine.h"
#include "uze/renderer/renderer.h"
#include "uze/renderer/shader.h"
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
	std::shared_ptr<Shader> shader;
	std::shared_ptr<VertexBuffer> vertex_buffer;
	std::shared_ptr<IndexBuffer> index_buffer;
	std::shared_ptr<VertexArray> vertex_array;
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

		std::cout << "Platform: " << SDL_GetPlatform() << "\n";

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

		std::array<float, 8> vertices =
		{
			-0.5f, -0.5f,
			0.5f, -0.5f,
			0.5f, 0.5f,
			-0.5f, 0.5f
		};

		std::array<u32, 6> indices =
		{
			0, 1, 2,
			2, 3, 0
		};

		auto vertex_array_builder = renderer->createVertexArrayBuilder();
		{
			BufferSpecification vertex_buffer_spec;
			vertex_buffer_spec.data = vertices.data();
			vertex_buffer_spec.size = static_cast<u32>(vertices.size() * sizeof(float));
			vertex_buffer = renderer->createVertexBuffer(vertex_buffer_spec);

			VertexLayout vertex_buffer_layout;
			vertex_buffer_layout.push<float>(2);
			vertex_array_builder->addVertexBuffer(vertex_buffer, vertex_buffer_layout);
		}

		{
			BufferSpecification index_buffer_spec;
			index_buffer_spec.data = indices.data();
			index_buffer_spec.size = static_cast<u32>(indices.size() * sizeof(u32));
			index_buffer = renderer->createIndexBuffer(index_buffer_spec);
			vertex_array_builder->setIndexBuffer(index_buffer);
		}
		vertex_array = vertex_array_builder->build();

#if !defined(__EMSCRIPTEN__)
		std::cout << "Frame took: 0ms";
#endif
		//Sound sound("C:/Users/User/Music/Soundpad/bigmak.wav");
		//sound.Play();

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
		renderer->clear(1.f, 1.f, 1.f, 1.f);
		renderer->drawQuad(glm::mat4(1.0f), glm::vec4(0.0f, 0.5f, 0.3f, 1.0f));
		renderer->drawQuad(glm::translate(glm::mat4(1.0f), glm::vec3(0.3f, -0.2f, 0.0f)),
			glm::vec4(0.2f, 0.5f, 0.5f, 1.0f));
		renderer->endFrame();

#if !defined(__EMSCRIPTEN__)
		std::printf("\r                                                                                       \r");
#endif
		std::cout << "Frame took: " << renderer->getStatistics().frame_time_ms
			<< "ms; draw calls: " << renderer->getStatistics().num_draw_calls
			<< "; num quads: " << renderer->getStatistics().num_quads;
	}
	
}
