#pragma once


#include "uze/common.h"
#include <string_view>

namespace uze
{

	struct UZE ShaderSpecification
	{
		virtual std::string_view getVertexSource() const = 0;
		virtual std::string_view getFragmentSource() const = 0;

		virtual ~ShaderSpecification() = default;
	};

	struct UZE RawShaderSpecification final : ShaderSpecification
	{
		std::string_view vertex_source;
		std::string_view fragment_source;

		RawShaderSpecification(std::string_view vertex_source_, std::string_view fragment_source_);

		std::string_view getVertexSource() const override { return vertex_source; }
		std::string_view getFragmentSource() const override { return fragment_source; }
	};

	class Renderer;
	class UZE ShaderPreprocessor
	{
	public:

		virtual ~ShaderPreprocessor() = default;

		virtual void preprocess(const Renderer& renderer, std::string_view shader, std::string& vertex, std::string& fragment) = 0;
		std::string_view getTypeName() const { return m_type_name; }

	protected:

		ShaderPreprocessor(std::string_view type_name)
			: m_type_name(type_name) {}

	private:

		std::string m_type_name;
	};

	class UZE DefaultShaderPreprocessor : public ShaderPreprocessor
	{
	public:

		DefaultShaderPreprocessor() : ShaderPreprocessor("default") {}

		void preprocess(const Renderer& renderer, std::string_view shader, std::string& vertex, std::string& fragment) override;

	};

	class UZE Shader final : NonCopyable<Shader>
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