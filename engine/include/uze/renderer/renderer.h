#pragma once

#include "uze/renderer/shader.h"
#include <string>

struct SDL_Window;

namespace uze
{

	struct RenderingCapabilities final
	{
		u32 gl_version_major;
		u32 gl_version_minor;
		std::string shading_language_version = "100 es";
	};

	class Renderer final : NonCopyable<Renderer>
	{
	public:

		Renderer();
		~Renderer();

		bool isValid() const { return m_valid; }

		void clear(float r, float g, float b, float a);

		void beginFrame();
		void endFrame();

		const RenderingCapabilities& getCapabilities() const { return m_caps; }

		std::shared_ptr<Shader> createShader(const ShaderSpecification& spec);

	private:

		SDL_Window* m_window{ nullptr };
		void* m_gl_context{ nullptr };
		bool m_valid{ false };
		RenderingCapabilities m_caps;

		std::unique_ptr<Shader> m_batch_shader{ nullptr };

	};

}