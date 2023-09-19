#include "uze/renderer/shader.h"
#include "glad/gles3.h"

namespace uze
{

	RawShaderSpecification::RawShaderSpecification(std::string_view vertex_source_,
		std::string_view fragment_source_)
		: vertex_source(vertex_source_), fragment_source(fragment_source_) {}

	static u32 compileShader(GLenum type, std::string_view source)
	{
		const u32 id = glCreateShader(type);
		const char* src = source.data();
		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);

		i32 success = GL_FALSE;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if (success == GL_TRUE)
			return id;

		char info_log[1024];
		glGetShaderInfoLog(id, sizeof(info_log), nullptr, info_log);
		// TODO: Add logging
		glDeleteShader(id);
		return 0;
	}

	Shader::Shader(const ShaderSpecification& spec)
	{
		u32 vert = compileShader(GL_VERTEX_SHADER, spec.getVertexSource());
		u32 frag = compileShader(GL_FRAGMENT_SHADER, spec.getFragmentSource());

		auto at_exit = AtScopeExit([vert, frag]()
			{
				glDeleteShader(vert);
				glDeleteShader(frag);
			});

		if (!vert || !frag)
			return;

		u32 id = glCreateProgram();
		glAttachShader(id, vert);
		glAttachShader(id, frag);
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
		// TODO: Add logging
		glDeleteProgram(id);
	}

	Shader::~Shader()
	{
		if (isValid())
			glDeleteProgram(m_handle);
	}
}
