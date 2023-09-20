#include "uze/renderer/renderer.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "glad/gles3.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

#include "uze/renderer/vertex_array.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace uze
{

	static bool createGLESContext(i32 major, i32 minor, SDL_Window*& window, SDL_GLContext& gl_context)
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);

		window = SDL_CreateWindow("Uzlezz Engine Window", 1600, 900, SDL_WINDOW_OPENGL);
		if (!window) return false;

		gl_context = SDL_GL_CreateContext(window);
		if (!gl_context)
		{
			SDL_DestroyWindow(window);
			window = nullptr;
			return false;
		}

		if (!gladLoadGLES2(SDL_GL_GetProcAddress))
		{
			SDL_GL_DeleteContext(gl_context);
			SDL_DestroyWindow(window);
			gl_context = nullptr;
			window = nullptr;
			return false;
		}

		return true;
	}

	Renderer::Renderer()
	{
		if (SDL_Init(SDL_INIT_VIDEO) != 0) return;

		int rendererFlags = 0;
#if defined(_DEBUG)
		rendererFlags |= SDL_GL_CONTEXT_DEBUG_FLAG;
#endif
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, rendererFlags);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		if (!createGLESContext(3, 0, m_window, m_gl_context))
			return;
		SDL_GL_MakeCurrent(m_window, m_gl_context);

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
			ss << major << minor << "0 es";
			m_caps.shading_language_version = ss.str();
		}
		m_caps.num_texture_units = num_texture_units;

		std::cout << std::setw(34) << std::left << "OpenGLES " << major << "." << minor << "\n";
		std::cout << std::setw(34) << std::left << "OpenGL Shading Language Version: " << (char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
		std::cout << std::setw(34) << std::left << "OpenGL Vendor:" << (char*)glGetString(GL_VENDOR) << std::endl;
		std::cout << std::setw(34) << std::left << "OpenGL Renderer:" << (char*)glGetString(GL_RENDERER) << std::endl;

		std::cout << std::setw(34) << std::left << "Num of available texture units: " << num_texture_units << "\n";

		const auto default_vert_source = R"(
precision mediump float;

layout (location = 0) in vec4 a_position_tex_index_tiling;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_tex_coord;

layout (std140) uniform CameraData
{
	mat4 view_projection;
};

out vec4 color;
out vec2 tex_coord;
out float tex_index;
out float tiling;

void main()
{
	gl_Position = view_projection * vec4(a_position_tex_index_tiling.xy, 0.0, 1.0);
	color = a_color;
	tex_coord = a_tex_coord;
	tex_index = a_position_tex_index_tiling.z;
	tiling = a_position_tex_index_tiling.w;
}
		)";

		std::stringstream frag_ss;
		frag_ss << R"(
precision highp float;

out vec4 out_color;

in vec4 color;
in vec2 tex_coord;
in float tex_index;
in float tiling;

uniform sampler2D u_textures[)";
		frag_ss << m_caps.num_texture_units;
		frag_ss << R"(];
void main()
{
	vec4 color = color;
	switch (int(tex_index))
	{
)";
		for (u32 i = 0; i < m_caps.num_texture_units; ++i)
		{
			frag_ss << "		case " << i << ": color *= texture(u_textures[" << i << "], tex_coord * tiling); break;\n";
		}
		frag_ss << "	}\n	out_color = color;\n}";

		auto default_frag_src = frag_ss.str();
		RawShaderSpecification spec(default_vert_source, default_frag_src);
		m_batch_shader = std::unique_ptr<Shader>(new Shader(spec, *this));
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
		bindShader(*m_batch_shader);
	}

	void Renderer::endFrame()
	{
		SDL_GL_SwapWindow(m_window);
	}

	void Renderer::bindShader(const Shader& shader)
	{
		glUseProgram(shader.m_handle);
	}

	void Renderer::draw(const VertexArray& vertex_array, u32 num_indices)
	{
		glBindVertexArray(vertex_array.m_handle);
		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, nullptr);
	}

	std::shared_ptr<Shader> Renderer::createShader(const ShaderSpecification& spec)
	{
		return std::shared_ptr<Shader>(new Shader(spec, *this));
	}

	std::shared_ptr<VertexBuffer> Renderer::createVertexBuffer(const BufferSpecification& spec)
	{
		return std::shared_ptr<VertexBuffer>(new VertexBuffer(spec));
	}

	std::shared_ptr<IndexBuffer> Renderer::createIndexBuffer(const BufferSpecification& spec)
	{
		return std::shared_ptr<IndexBuffer>(new IndexBuffer(spec));
	}

	std::unique_ptr<VertexArrayBuilder> Renderer::createVertexArrayBuilder() const
	{
		return std::unique_ptr<VertexArrayBuilder>(new VertexArrayBuilder());
	}
}
