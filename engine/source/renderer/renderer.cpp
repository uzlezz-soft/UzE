#include "uze/renderer/renderer.h"
#include "opengl.h"

#include <array>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "uze/renderer/vertex_array.h"

#include <glm/glm.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#if defined(_WIN32)
extern "C" __declspec(dllexport) uze::u32 NvOptimusEnablement = 0x00000001;
extern "C" __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif

namespace uze
{

	static constexpr LogCategory log_renderer { "Renderer" };

	const char* openGLErrorToString(u32 err) noexcept
	{
		switch (err)
		{
		case GL_NO_ERROR: return "GL_NO_ERROR";

		case GL_INVALID_ENUM: return "GL_INVALID_ENUM";

		case GL_INVALID_VALUE: return "GL_INVALID_VALUE";

		case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";

		case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";

		case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
		}

		return "NO_ERROR";
	}

	namespace
	{
		bool createGLContext(i32 major, i32 minor, SDL_Window*& window, SDL_GLContext& gl_context)
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);

#if UZE_GL == UZE_OPENGL33
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif

			window = SDL_CreateWindow("Uzlezz Engine Window", 1600, 900, SDL_WINDOW_OPENGL);
			if (!window) return false;

			gl_context = SDL_GL_CreateContext(window);
			if (!gl_context)
			{
				SDL_DestroyWindow(window);
				window = nullptr;
				return false;
			}

#if UZE_PLATFORM != UZE_PLATFORM_WEB
#if UZE_GL == UZE_OPENGLES30
			auto result__ = gladLoadGLES(SDL_GL_GetProcAddress);
#else
			auto result_ = gladLoadGL(SDL_GL_GetProcAddress);
#endif
			if (!result_)
			{
				SDL_GL_DeleteContext(gl_context);
				SDL_DestroyWindow(window);
				gl_context = nullptr;
				window = nullptr;
				return false;
			}
#endif

			return true;
		}
	}

	constexpr u64 quad_vertex_count = 4;
	constexpr u64 quad_index_count = 6;
	constexpr u64 max_quads_in_one_batch = 20000;
	constexpr u64 max_quad_vertices = max_quads_in_one_batch * quad_vertex_count;
	constexpr u64 max_quad_indices = max_quads_in_one_batch * quad_index_count;

	struct QuadVertex
	{
		glm::vec4 position_tex_index_tiling;
		glm::vec4 color;
	};

	struct BatchData
	{
		std::shared_ptr<VertexArray> quad_vertex_array;
		std::shared_ptr<VertexBuffer> quad_vertex_buffer;
		std::shared_ptr<IndexBuffer> quad_index_buffer;
		std::shared_ptr<Shader> quad_shader;

		std::unique_ptr<QuadVertex[]> quad_vertices_base{ nullptr };
		QuadVertex* quad_vertices_ptr{ nullptr };

		u32 quad_index_count{ 0 };
		std::array<glm::vec2, quad_vertex_count> quad_vertex_positions;

		glm::mat4 projection{ 1.0f };
		glm::mat4 view{ 1.0f };
		glm::mat4 view_projection{ 1.0f };
	};

	Renderer::Renderer()
	{
		if (SDL_Init(SDL_INIT_VIDEO) != 0) return;

		int rendererFlags = 0;
#if defined(_DEBUG)
		rendererFlags |= SDL_GL_CONTEXT_DEBUG_FLAG;
#endif
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, rendererFlags);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#if UZE_GL == UZE_OPENGLES30
		if (!createGLContext(3, 0, m_window, m_gl_context))
			return;
#else
		if (!createGLContext(3, 3, m_window, m_gl_context))
			return;
#endif
		SDL_GL_MakeCurrent(m_window, m_gl_context);


		// Try to enable adaptive V-Sync
		if (SDL_GL_SetSwapInterval(-1) != 0)
			// If there's no support for adaptive V-Sync,
			// Enable standard V-Sync
			SDL_GL_SetSwapInterval(1);

		m_valid = true;

		i32 num_texture_units = 0;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &num_texture_units);

		GLint major, minor;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);

		m_caps.gl_version_major = major;
		m_caps.gl_version_minor = minor;
		{
			std::stringstream ss;
			ss << major << minor << "0";
#if UZE_GL == UZE_OPENGLES30
			ss << " es";
#endif
			m_caps.shading_language_version = ss.str();
		}
		m_caps.num_texture_units = num_texture_units;

		m_batch_data = std::make_unique<BatchData>();

		uzLog(log_renderer, Info, "########################### RENDERER INFO ###########################");
		uzLog(log_renderer, Info, "OpenGL ES: {}.{}", major, minor);
		uzLog(log_renderer, Info, "Shading Language: {}", (char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
		uzLog(log_renderer, Info, "Vendor: {}", (char*)glGetString(GL_VENDOR));
		uzLog(log_renderer, Info, "Renderer: {}", (char*)glGetString(GL_RENDERER));
		uzLog(log_renderer, Info, "Available texture units: {}", num_texture_units);
		uzLog(log_renderer, Info, "########################### RENDERER INFO ###########################");

		const auto default_vert_source = R"(
precision mediump float;

layout (location = 0) in vec4 a_position_tex_index_tiling;
layout (location = 1) in vec4 a_color;

layout (std140) uniform CameraData
{
	mat4 view_projection;
};

out vec2 position;
out vec4 color;
out vec2 tex_coord;
out float tex_index;
out float tiling;

const vec2 quad_tex_coords[4] = vec2[] (vec2(0, 1), vec2(0, 0), vec2(1, 0), vec2(1, 1));

void main()
{
	int quad_vertex_index = gl_VertexID % 4;
	tex_coord = quad_tex_coords[quad_vertex_index];
	position = a_position_tex_index_tiling.xy;

	gl_Position = /*view_projection **/ vec4(position, 0.0, 1.0);
	color = a_color;
	tex_index = a_position_tex_index_tiling.z;
	tiling = a_position_tex_index_tiling.w;
}
		)";

		std::stringstream frag_ss;
		frag_ss << R"(
precision highp float;

out vec4 out_color;

in vec2 position;
in vec4 color;
in vec2 tex_coord;
in float tex_index;
in float tiling;

uniform sampler2D u_textures[)";
		frag_ss << m_caps.num_texture_units;
		frag_ss << R"(];
void main()
{
	vec4 color_ = color;
	switch (int(tex_index))
	{
)";
		for (u32 i = 0; i < m_caps.num_texture_units; ++i)
		{
			frag_ss << "		case " << i << ": color_ *= texture(u_textures[" << i << "], tex_coord * tiling); break;\n";
		}
		frag_ss << "	}\n	out_color = color;\n}";

		auto default_frag_src = frag_ss.str();
		RawShaderSpecification spec(default_vert_source, default_frag_src);
		m_batch_data->quad_shader = std::unique_ptr<Shader>(new Shader(spec, *this));



		m_batch_data->quad_vertex_array = createVertexArray();

		BufferSpecification vertex_buffer_spec;
		vertex_buffer_spec.dynamic = true;
		vertex_buffer_spec.size = max_quad_vertices * sizeof(QuadVertex);
		m_batch_data->quad_vertex_buffer = createVertexBuffer(vertex_buffer_spec);

		m_batch_data->quad_vertices_base = std::make_unique<QuadVertex[]>(max_quad_vertices);
		u32* quad_indices = new u32[max_quad_indices];
		u32 offset = 0;
		for (u32 i = 0; i < max_quad_indices; i += quad_index_count)
		{
			quad_indices[i + 0] = offset + 0;
			quad_indices[i + 1] = offset + 1;
			quad_indices[i + 2] = offset + 2;

			quad_indices[i + 3] = offset + 0;
			quad_indices[i + 4] = offset + 2;
			quad_indices[i + 5] = offset + 3;

			offset += quad_vertex_count;
		}

		BufferSpecification index_buffer_spec;
		index_buffer_spec.size = sizeof(*quad_indices) * max_quad_indices;
		index_buffer_spec.data = quad_indices;
		m_batch_data->quad_index_buffer = createIndexBuffer(index_buffer_spec);
		glFlush();
		delete[] quad_indices;

		VertexLayout layout;
		layout.push<glm::vec4>(1).push<glm::vec4>(1);
		if (layout.getStride() != sizeof(QuadVertex))
			uzLog(log_renderer, Warn, "layout.stride != sizeof(QuadVertex)");

		m_batch_data->quad_vertex_array->addVertexBuffer(m_batch_data->quad_vertex_buffer, layout);
		m_batch_data->quad_vertex_array->setIndexBuffer(m_batch_data->quad_index_buffer);

		m_batch_data->quad_vertex_positions[0] = glm::vec2(-0.5f, 0.5f);
		m_batch_data->quad_vertex_positions[1] = glm::vec2(-0.5f, -0.5f);
		m_batch_data->quad_vertex_positions[2] = glm::vec2(0.5f, -0.5f);
		m_batch_data->quad_vertex_positions[3] = glm::vec2(0.5f, 0.5f);
	}

	Renderer::~Renderer()
	{
		if (!m_valid) return;

		SDL_GL_DeleteContext(m_gl_context);
		SDL_DestroyWindow(m_window);
	}

	void Renderer::clear(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Renderer::beginFrame()
	{
		m_stats.reset();
		startBatch();
	}

	void Renderer::endFrame()
	{
		endBatch();
		SDL_GL_SwapWindow(m_window);
		m_stats.frame_time_ms = m_stats.m_start.getElapsedMilliseconds();
	}

	void Renderer::bindShader(const Shader& shader)
	{
		glUseProgram(shader.m_handle);
	}

	void Renderer::draw(const VertexArray& vertex_array)
	{
		glBindVertexArray(vertex_array.m_handle);
		glDrawElements(GL_TRIANGLES, static_cast<GLint>(vertex_array.getIndexBuffer()->getCount()), GL_UNSIGNED_INT, nullptr);
		m_stats.num_draw_calls++;
	}

	void Renderer::draw(const VertexArray& vertex_array, i32 num_indices)
	{
		glBindVertexArray(vertex_array.m_handle);
		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, nullptr);
		m_stats.num_draw_calls++;
	}

	void Renderer::drawQuad(const glm::mat4& transform, const glm::vec4& color)
	{
		if (m_batch_data->quad_index_count + quad_index_count >= max_quad_indices)
			nextBatch();

		for (u64 i = 0; i < quad_vertex_count; ++i)
		{
			const auto position = transform * glm::vec4(m_batch_data->quad_vertex_positions[i], 0.0f, 1.0f);

			m_batch_data->quad_vertices_ptr->position_tex_index_tiling =
				glm::vec4(position.x, position.y, 0.0f, 1.0f);
			m_batch_data->quad_vertices_ptr->color = color;
			m_batch_data->quad_vertices_ptr++;
		}

		m_batch_data->quad_index_count += quad_index_count;
		m_stats.num_quads++;
	}

	std::shared_ptr<Shader> Renderer::createShader(const ShaderSpecification& spec)
	{
		return std::shared_ptr<Shader>(new Shader(spec, *this));
	}

	std::shared_ptr<VertexBuffer> Renderer::createVertexBuffer(const BufferSpecification& spec)
	{
		return std::shared_ptr<VertexBuffer>(new VertexBuffer(spec, *this));
	}

	std::shared_ptr<IndexBuffer> Renderer::createIndexBuffer(const BufferSpecification& spec)
	{
		return std::shared_ptr<IndexBuffer>(new IndexBuffer(spec, *this));
	}

	std::shared_ptr<VertexArray> Renderer::createVertexArray() const
	{
		return std::shared_ptr<VertexArray>(new VertexArray());
	}

	void Renderer::onDataTransfer(u64 amount)
	{
		m_stats.data_transmitted += amount;
	}

	void Renderer::startBatch()
	{
		m_batch_data->quad_index_count = 0;
		m_batch_data->quad_vertices_ptr = m_batch_data->quad_vertices_base.get();
	}

	void Renderer::endBatch()
	{
		if (!m_batch_data->quad_index_count) return;

		u32 num_vertices = static_cast<u32>(static_cast<double>(m_batch_data->quad_index_count) / 1.5);
		u32 data_size = num_vertices * sizeof(QuadVertex);
		/*u32 data_size = static_cast<u32>(reinterpret_cast<u8*>(m_batch_data->quad_vertices_ptr)
			- reinterpret_cast<u8*>(m_batch_data->quad_vertices_base.get()));*/

		m_batch_data->quad_vertex_buffer->updateData(m_batch_data->quad_vertices_base.get(), data_size, 0);

		bindShader(*m_batch_data->quad_shader);
		draw(*m_batch_data->quad_vertex_array, static_cast<i32>(m_batch_data->quad_index_count));
		m_stats.num_vertices += num_vertices;
	}

	void Renderer::nextBatch()
	{
		endBatch();
		startBatch();
	}
}
