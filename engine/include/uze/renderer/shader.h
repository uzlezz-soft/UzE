#pragma once


#include "uze/common.h"
#include <string_view>

namespace uze
{

	struct ShaderSpecification
	{
		virtual std::string_view getVertexSource() const = 0;
		virtual std::string_view getFragmentSource() const = 0;
	};

	struct RawShaderSpecification final : public ShaderSpecification
	{
		std::string_view vertex_source;
		std::string_view fragment_source;

		RawShaderSpecification(std::string_view vertex_source_, std::string_view fragment_source_);

		std::string_view getVertexSource() const override { return vertex_source; }
		std::string_view getFragmentSource() const override { return fragment_source; }
	};

	class Shader final : public NonCopyable<Shader>
	{
	public:

		Shader(const ShaderSpecification& spec);
		~Shader();

		bool isValid() const { return m_handle; }

	private:

		u32 m_handle{ 0 };
	};

}