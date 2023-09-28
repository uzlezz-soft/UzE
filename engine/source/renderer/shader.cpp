#include "uze/renderer/shader.h"
#include "uze/renderer/renderer.h"
#include "opengl.h"
#include <sstream>

namespace uze
{

	static constexpr LogCategory log_shader { "Shader" };

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

	Shader::~Shader()
	{
		if (isValid())
			glDeleteProgram(m_handle);
	}
}
