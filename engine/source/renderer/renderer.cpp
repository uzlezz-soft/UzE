#include "uze/renderer/renderer.h"
#include "opengl.h"

#include <array>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "uze/renderer/vertex_array.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>

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

	void logOpenGLError(u32 err, const char* file, u64 line)
	{
		uzLog(log_renderer, Error, "`{}`, line {}", std::filesystem::path(file).filename().generic_string(), line);
		uzLog(log_renderer, Error, "OpenGL Error: {}", openGLErrorToString(err));
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

	struct SceneData
	{
		glm::mat4 view_projection;
	};

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

		uzLog(log_renderer, Info, "########################### RENDERER INFO ###########################");
		uzLog(log_renderer, Info, "{}: {}.{}", UZE_GL_STRING, major, minor);
		uzLog(log_renderer, Info, "Shading Language: {}", (char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
		uzLog(log_renderer, Info, "Vendor: {}", (char*)glGetString(GL_VENDOR));
		uzLog(log_renderer, Info, "Renderer: {}", (char*)glGetString(GL_RENDERER));
		uzLog(log_renderer, Info, "Available texture units: {}", num_texture_units);
		uzLog(log_renderer, Info, "########################### RENDERER INFO ###########################");

		m_scene_data = std::make_unique<SceneData>();
		m_batch_data = std::make_unique<BatchData>();

		UniformBufferSpecification scene_buffer_spec;
		scene_buffer_spec.binding = 0;
		scene_buffer_spec.name = "Scene";
		scene_buffer_spec.dynamic = true;
		scene_buffer_spec.size = sizeof(SceneData);
		m_scene_buffer = createUniformBuffer(scene_buffer_spec);

		registerShaderPreprocessor<DefaultShaderPreprocessor>();



		constexpr std::string_view quad_shader = R"(
#shader_type default

vec4 fragment(Input input)
{
	return input.color;
}
)";
		m_batch_data->quad_shader = createShader(quad_shader);



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

		const float aspect = 1.6f / 0.9f;
		m_scene_data->view_projection = glm::ortho(-3.0f * aspect, 3.0f * aspect, -3.0f, 3.0f);
		m_scene_buffer->updateData(m_scene_data.get(), sizeof(SceneData));

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
		auto shader = std::shared_ptr<Shader>(new Shader(spec, *this));
		registerUniformBuffersForShader(*shader);
		return shader;
	}

	std::shared_ptr<Shader> Renderer::createShader(std::string_view source)
	{
		constexpr std::string_view shader_type_directive = "#shader_type";

		auto pos = source.find(shader_type_directive);
		if (pos == std::string_view::npos)
		{
			uzLog(log_renderer, Error, "Cannot create shader from source: expected `#shader_type` directive");
			return nullptr;
		}
		pos += shader_type_directive.size();

		if (source[pos] != ' ' && source[pos] != '\t')
		{
			uzLog(log_renderer, Error, "Error near `#shader_type` directive");
			return nullptr;
		}
		++pos;

		while (source[pos] == ' ' || source[pos] == '\t')
			++pos;

		std::stringstream shader_type_ss;
		while (source[pos] != '\n' && source[pos] != ' ' && source[pos] != '\t')
			shader_type_ss << source[pos++];

		const auto it = m_shader_preprocessors.find(shader_type_ss.str());
		if (it == m_shader_preprocessors.end())
		{
			uzLog(log_renderer, Error, "Cannot find preprocessor for shader of type `{}`", shader_type_ss.str());
			return nullptr;
		}

		std::string vertex;
		std::string fragment;
		it->second->preprocess(*this, std::string_view(source).substr(pos), vertex, fragment);

		const RawShaderSpecification spec(vertex, fragment);
		return createShader(spec);
	}

	std::shared_ptr<VertexBuffer> Renderer::createVertexBuffer(const BufferSpecification& spec)
	{
		return std::shared_ptr<VertexBuffer>(new VertexBuffer(spec, *this));
	}

	std::shared_ptr<IndexBuffer> Renderer::createIndexBuffer(const BufferSpecification& spec)
	{
		return std::shared_ptr<IndexBuffer>(new IndexBuffer(spec, *this));
	}

	std::shared_ptr<UniformBuffer> Renderer::createUniformBuffer(const UniformBufferSpecification& spec)
	{
		return std::shared_ptr<UniformBuffer>(new UniformBuffer(spec, *this));
	}

	std::shared_ptr<VertexArray> Renderer::createVertexArray() const
	{
		return std::shared_ptr<VertexArray>(new VertexArray());
	}

	void Renderer::onDataTransfer(u64 amount)
	{
		m_stats.data_transmitted += amount;
	}

	void Renderer::registerUniformBuffer(UniformBuffer& buffer)
	{
		m_uniform_buffers.push_back(&buffer);
	}

	void Renderer::unregisterUniformBuffer(const UniformBuffer& buffer)
	{
		for (auto it = m_uniform_buffers.begin(); it != m_uniform_buffers.end(); ++it)
		{
			if (*it == &buffer)
			{
				m_uniform_buffers.erase(it);
				break;
			}
		}
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

	void Renderer::registerShaderPreprocessorImpl(std::unique_ptr<ShaderPreprocessor> pp)
	{
		auto type_name = pp->getTypeName();

		if (m_shader_preprocessors.find(type_name.data()) != m_shader_preprocessors.end())
		{
			uzLog(log_renderer, Warn, "Shader preprocessor for `{}` shaders already registered");
			return;
		}

		m_shader_preprocessors[type_name.data()] = std::move(pp);
		uzLog(log_renderer, Info, "Registered shader preprocessor for `{}` shaders", type_name);
	}

	void Renderer::registerUniformBuffersForShader(const Shader& shader)
	{
		for (auto& ub : m_uniform_buffers)
		{
			if (const u32 index = glGetUniformBlockIndex(shader.m_handle, ub->m_name.data()))
				glUniformBlockBinding(shader.m_handle, index, ub->m_binding);
		}
	}
}
