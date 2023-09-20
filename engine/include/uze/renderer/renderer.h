#pragma once

#include "uze/renderer/shader.h"
#include "uze/renderer/buffer.h"
#include "uze/renderer/vertex_array.h"
#include <string>

struct SDL_Window;

namespace uze
{

	struct RenderingCapabilities final
	{
		u32 gl_version_major;
		u32 gl_version_minor;
		std::string shading_language_version = "100 es";
		u32 num_texture_units;
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

		void bindShader(const Shader& shader);
		void draw(const VertexArray& vertex_array, u32 num_indices);

		std::shared_ptr<Shader> createShader(const ShaderSpecification& spec);
		std::shared_ptr<VertexBuffer> createVertexBuffer(const BufferSpecification& spec);
		std::shared_ptr<IndexBuffer> createIndexBuffer(const BufferSpecification& spec);
		std::unique_ptr<VertexArrayBuilder> createVertexArrayBuilder() const;

	private:

		SDL_Window* m_window{ nullptr };
		void* m_gl_context{ nullptr };
		bool m_valid{ false };
		RenderingCapabilities m_caps;

		std::unique_ptr<Shader> m_batch_shader{ nullptr };

	};

}