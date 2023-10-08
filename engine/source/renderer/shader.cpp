#include "uze/renderer/shader.h"
#include "uze/renderer/renderer.h"
#include "opengl.h"
#include <sstream>

namespace uze
{

	static constexpr LogCategory log_shader { "Shader" };

	static constexpr char* batch_vertex_shader = R"(
precision mediump float;

layout (location = 0) in vec4 a_position_tex_index_tiling;
layout (location = 1) in vec4 a_color;

layout (std140) uniform Scene
{
	mat4 view_projection;
};

out vec2 position_;
out vec4 color_;
out vec2 tex_coord_;
out float tex_index_;
out float tiling_;

const vec2 quad_tex_coords[4] = vec2[] (vec2(0, 1), vec2(0, 0), vec2(1, 0), vec2(1, 1));

void main()
{
	int quad_vertex_index = gl_VertexID % 4;
	tex_coord_ = quad_tex_coords[quad_vertex_index];
	position_ = a_position_tex_index_tiling.xy;

	gl_Position = view_projection * vec4(position_, 0.0, 1.0);
	color_ = a_color;
	tex_index_ = a_position_tex_index_tiling.z;
	tiling_ = a_position_tex_index_tiling.w;
}
)";

	RawShaderSpecification::RawShaderSpecification(std::string_view vertex_source_,
		std::string_view fragment_source_)
		: vertex_source(vertex_source_), fragment_source(fragment_source_) {}

	static u32 compileShader(GLenum type, std::string_view source, const Renderer& renderer)
	{
		const u32 id = glCreateShader(type);
		std::stringstream ss;
		ss << "#version " << renderer.getCapabilities().shading_language_version << "\n#line 1\n" << source;
		auto src = ss.str();
		const char* c_src = src.data();
		glCheck(glShaderSource(id, 1, &c_src, nullptr));
		glCheck(glCompileShader(id));

		i32 success = GL_FALSE;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if (success == GL_TRUE)
			return id;

		char info_log[1024];
		glGetShaderInfoLog(id, sizeof(info_log), nullptr, info_log);
		uzLog(log_shader, Error, "Cannot compile shader: {}", info_log);
		glDeleteShader(id);
		return 0;
	}

	Shader::Shader(const ShaderSpecification& spec, Renderer& renderer)
	{
		u32 vert = compileShader(GL_VERTEX_SHADER, spec.getVertexSource(), renderer);
		u32 frag = compileShader(GL_FRAGMENT_SHADER, spec.getFragmentSource(), renderer);

		auto at_exit = AtScopeExit([vert, frag]()
			{
				glDeleteShader(vert);
				glDeleteShader(frag);
			});

		if (!vert || !frag)
			return;

		u32 id = glCreateProgram();
		glCheck(glAttachShader(id, vert));
		glCheck(glAttachShader(id, frag));
		glLinkProgram(id);

		i32 success = GL_FALSE;
		glGetProgramiv(id, GL_LINK_STATUS, &success);
		if (success == GL_TRUE)
		{
			m_handle = id;
			return;
		}

		char info_log[1024];
		glGetShaderInfoLog(id, sizeof(info_log), nullptr, info_log);
		uzLog(log_shader, Error, "Cannot link shader: {}", info_log);
		glDeleteProgram(id);
	}

	void DefaultShaderPreprocessor::preprocess(const Renderer& renderer, std::string_view shader, std::string& vertex, std::string& fragment)
	{
		static std::string fragment_source;
		if (fragment_source.empty())
		{
			std::stringstream ss;
			ss << R"(
precision highp float;
out vec4 color;

in vec2 position_;
in vec4 color_;
in vec2 tex_coord_;
in float tex_index_;
in float tiling_;

uniform sampler2D u_textures[)";
			ss << renderer.getCapabilities().num_texture_units << R"(];

struct Input
{
	vec2 position;
	vec4 color;
	vec2 tex_coord;
	int tex_index;
	float tiling;
};

vec4 fragment(Input);

void main()
{
	Input fsi;
	fsi.position = position_;
	fsi.color = color_;
	fsi.tex_coord = tex_coord_;
	fsi.tex_index = int(tex_index_);
	fsi.tiling = tiling_;
	
	color = fragment(fsi);
}

vec4 sampleTexture(int tex_index, vec2 tex_coord)
{
	switch (tex_index)
	{

)";

			for (u64 i = 0; i < renderer.getCapabilities().num_texture_units; ++i)
			{
				ss << "\t\tcase " << i << ": return texture(u_textures[" << i << "], tex_coord);\n";
			}

			ss << R"(	}
	return vec4(0.0);
}
)";
			fragment_source = ss.str();
		}


		vertex = batch_vertex_shader;
		std::stringstream ss;
		ss << fragment_source << shader;
		fragment = ss.str();
	}

	Shader::~Shader()
	{
		if (isValid())
			glDeleteProgram(m_handle);
	}
}
