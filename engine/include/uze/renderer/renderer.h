#pragma once

#include "uze/renderer/shader.h"
#include "uze/renderer/buffer.h"
#include "uze/renderer/vertex_array.h"
#include <string>
#include "glm/glm.hpp"

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

	struct RendererStatistics final
	{
		double frame_time_ms{ 0.0 };
		u32 num_vertices{ 0 };
		u32 num_quads{ 0 };
		u32 num_draw_calls{ 0 };
		u64 data_transmitted { 0 };

		void reset()
		{
			std::memset(this, 0, sizeof(RendererStatistics));
			m_start.reset();
		}

	private:

		Stopwatch m_start;

		friend class Renderer;
	};

	struct BatchData;
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
		const RendererStatistics& getStatistics() const { return m_stats; }

		void bindShader(const Shader& shader);
		void draw(const VertexArray& vertex_array);
		void draw(const VertexArray& vertex_array, i32 num_indices);
		void drawQuad(const glm::mat4& transform, const glm::vec4& color);

		std::shared_ptr<Shader> createShader(const ShaderSpecification& spec);
		std::shared_ptr<VertexBuffer> createVertexBuffer(const BufferSpecification& spec);
		std::shared_ptr<IndexBuffer> createIndexBuffer(const BufferSpecification& spec);
		std::shared_ptr<VertexArray> createVertexArray() const;

		void onDataTransfer(u64 amount);

	private:

		SDL_Window* m_window{ nullptr };
		void* m_gl_context{ nullptr };
		bool m_valid{ false };
		RenderingCapabilities m_caps;
		RendererStatistics m_stats;

		std::unique_ptr<BatchData> m_batch_data{ nullptr };

		void startBatch();
		void endBatch();
		void nextBatch();

	};

}