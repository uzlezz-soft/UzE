#pragma once


#include "uze/common.h"
#include <string_view>

namespace uze
{

	struct ShaderSpecification
	{
		virtual std::string_view getVertexSource() const = 0;
		virtual std::string_view getFragmentSource() const = 0;

		virtual ~ShaderSpecification() = default;
	};

	struct RawShaderSpecification final : public ShaderSpecification
	{
		std::string_view vertex_source;
		std::string_view fragment_source;

		RawShaderSpecification(std::string_view vertex_source_, std::string_view fragment_source_);

		std::string_view getVertexSource() const override { return vertex_source; }
		std::string_view getFragmentSource() const override { return fragment_source; }
	};

	class Renderer;

	class Shader final : public NonCopyable<Shader>
	{
	public:

		~Shader();

		bool isValid() const { return m_handle; }

	private:

		u32 m_handle{ 0 };

		Shader(const ShaderSpecification& spec, Renderer& renderer);

		friend class Renderer;
	};

}