#include "uze/engine.h"
#include "uze/renderer/renderer.h"
#include "uze/renderer/shader.h"
#include "uze/platform.h"
#include "uze/core/type_info.h"
#include "uze/core/job_system.h"
#include "uze/core/random.h"
#include "renderer/opengl.h"
#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include <iostream>
#include <fstream>

#include "glm/ext/matrix_transform.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <refl.hpp>
#include <thread>

namespace uze
{

	uzclass Entity : Object
	{
		UZE_OBJECT(Entity)

	public:

		serialize_field float health{ 0.0f };

	};

}

UZE_REFLECT(uze::Entity,
	type(uze::Entity, bases<uze::Object>),
	field(health)
)

namespace uze
{

	std::unique_ptr<::uze::Renderer> renderer;
	bool quit = false;

	static void gameLoop();

	static constexpr LogCategory log_engine { "Engine" };

	entt::registry registry;

	struct TransformComponent
	{
		glm::vec2 position;
		float rotation{ 0.0f };
	};

	struct SpriteRendererComponent
	{
		glm::vec4 color{ 1.0f };
		i8 z_layer{ 0 };
	};

	struct PlayerComponent
	{
		float speed{ 3.0f };
	};

	void EntryPoint()
	{
		renderer = std::make_unique<Renderer>();
		if (!renderer->isValid())
		{
			uzLog(log_engine, Error, "Cannot init renderer");
			return;
		}

		uzLog(log_engine, Info, "Platform: {}, Config: {}", UZE_PLATFORM_STRING, UZE_CONFIG_STRING);

		if (std::ifstream in("input.txt"); in.is_open())
		{
			uzLog(log_engine, Debug, "Opened `{}/input.txt!`", std::filesystem::current_path().generic_string());
			in.close();
		}
		else
		{
			uzLog(log_engine, Debug, "Cannot open file `{}/input.txt`", std::filesystem::current_path().generic_string());
		}

		Registry::init();
		//Registry::registerType<Object>();
		//Registry::registerType<Entity>();
		job_system::init();

		{
			EntityTest et;
			et.name = "Entity Test";
			et.health = 100;
			et.x = 2.0f;
			et.y = -1.5f;

			for (u64 i = 0; i < 10; ++i)
			{
				et.indices.push_back(i);
			}

			std::ofstream out("input.txt", std::ios::binary);

			BinarySerializer<decltype(et)>{}(out, et);
			uzLog(log_engine, Debug, "Serialized EntityTest to input.txt");
		}

		{
			EntityTest et2;
			std::ifstream in("input.txt", std::ios::binary);
			BinaryDeserializer<decltype(et2)>{}(in, et2);
		}

		Random random;
		float radius = 5.0f;
		for (u64 i = 0; i < 100; ++i)
		{
			const auto e = registry.create();
			registry.emplace<TransformComponent>(e).position = 
				glm::vec2((random.nextFloat() * 2.0f - 1) * radius,
					(random.nextFloat() * 2.0f - 1) * radius);
			registry.emplace<SpriteRendererComponent>(e,
				glm::vec4 { random.nextFloat(), random.nextFloat(),
					random.nextFloat(), random.nextFloat() },
				static_cast<i8>(random.next(-10, 12)));
		}

		{
			const auto e = registry.create();
			registry.emplace<TransformComponent>(e, glm::vec2{ 0.2f, -0.1f });
			registry.emplace<SpriteRendererComponent>(e).z_layer = 10;
			registry.emplace<PlayerComponent>(e);
		}

#if !defined(__EMSCRIPTEN__)
		std::cout << "Frame time: 0ms";
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

	static std::unordered_map<SDL_Keycode, bool> s_press_states;

	static Stopwatch sw;
	static float speed = 50.0f;

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
			else if (e.type == SDL_EVENT_KEY_DOWN)
			{
				s_press_states[e.key.keysym.sym] = true;
			}
			else if (e.type == SDL_EVENT_KEY_UP)
			{
				s_press_states[e.key.keysym.sym] = false;
			}
		}

		registry.view<TransformComponent, PlayerComponent>().each(
		[](auto& transform, auto& player)
			{
				glm::vec2 direction{ 0.0f, 0.0f };
				if (s_press_states[SDLK_w])
					direction.y += 1.0f;
				if (s_press_states[SDLK_s])
					direction.y -= 1.0f;
				if (s_press_states[SDLK_a])
					direction.x -= 1.0f;
				if (s_press_states[SDLK_d])
					direction.x += 1.0f;

				transform.position += direction * player.speed
					* static_cast<float>(sw.getElapsedSeconds());
			});

		sw.reset();

		renderer->beginFrame();
		renderer->clear(0.5f, 1.f, 0.2f, 1.f);

		static std::vector<entt::entity> entities_to_draw;
		entities_to_draw.clear();

		registry.view<TransformComponent, SpriteRendererComponent>().each(
			[](entt::entity e, auto& tc, auto& sprite)
			{
				for (u64 i = 0; i < entities_to_draw.size(); ++i)
				{
					if (registry.get<SpriteRendererComponent>(entities_to_draw[i]).z_layer
						< sprite.z_layer)
					{
						entities_to_draw.insert(entities_to_draw.begin() + i, e);
						return;
					}
				}

				entities_to_draw.push_back(e);
				
			});

		for (auto it = entities_to_draw.rbegin(); it != entities_to_draw.rend(); ++it)
		{
			auto e = *it;
			const auto& tc = registry.get<TransformComponent>(e);
			const auto& sprite = registry.get<SpriteRendererComponent>(e);

			renderer->drawQuad(tc.position, sprite.color);
		}

		renderer->endFrame();

#if UZE_PLATFORM != UZE_PLATFORM_WEB
		std::cout << "\r                                                                                       \r";
#endif
		std::cout << "Frame time: " << renderer->getStatistics().frame_time_ms
			<< "ms; draw calls: " << renderer->getStatistics().num_draw_calls
			<< "; num quads: " << renderer->getStatistics().num_quads << "; PCIe: "
			<< renderer->getStatistics().data_transmitted << " bytes";
#if UZE_PLATFORM == UZE_PLATFORM_WEB
		std::cout << "\n";
#endif
	}
	
}
