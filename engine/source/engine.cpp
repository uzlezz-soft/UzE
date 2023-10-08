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

#include "glm/ext/matrix_transform.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <refl.hpp>
#include <thread>


struct Serializable : refl::attr::usage::field {};

template <class T>
void serialize(std::ostream& os, T&& value)
{
	constexpr auto type = refl::reflect<T>();
	os << type.name << ":\n";
	for_each(refl::reflect(value).members, [&](auto member)
	{
		if constexpr (is_readable(member) && refl::descriptor::has_attribute<Serializable>(member))
		{
			os << get_display_name(member) << "=";
			os << member(value) << "\n";
		}
	});
}

namespace uze
{
	struct Vec2
	{
		float x, y;
	};
}

REFL_AUTO(
	type(uze::Vec2),
	field(x, Serializable{}),
	field(y, Serializable{})
)

namespace uze
{

	class Entity : Object
	{
		UZE_OBJECT(Entity)

	public:

		float health{ 0.0f };

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
			std::cerr << "Cannot init renderer\n";
			return;
		}

		uzLog(log_engine, Info, "Platform: {}", UZE_PLATFORM_STRING);

		Registry::init();
		//Registry::registerType<Object>();
		//Registry::registerType<Entity>();
		job_system::init();

		Vec2 v2{ -1.5f, 2.0f };
		serialize(getOutputStream(), v2);

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

	glm::vec3 quad2_position{-0.5f, -0.2f, 0.0f};
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

			const auto transform = glm::translate(glm::mat4(1.0f), glm::vec3(tc.position, 0.0f));
			renderer->drawQuad(transform, sprite.color);
		}

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
