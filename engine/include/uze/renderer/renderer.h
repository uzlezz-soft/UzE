#pragma once

#include "uze/renderer/shader.h"

struct SDL_Window;

namespace uze
{

	class Renderer final : NonCopyable<Renderer>
	{
	public:

		Renderer();
		~Renderer();

		bool isValid() const { return m_valid; }

		void clear(float r, float g, float b, float a);

		void beginFrame();
		void endFrame();

	private:

		SDL_Window* m_window{ nullptr };
		void* m_gl_context{ nullptr };
		bool m_valid{ false };

	};

}