#include "uze/renderer/buffer.h"
#include "uze/renderer/renderer.h"
#include "opengl.h"

namespace uze
{

	static u32 s_currently_bound_vertex_buffer{ 0 };
	static u32 s_currently_bound_index_buffer{ 0 };
	static u32 s_currently_bound_uniform_buffer{ 0 };

	VertexBuffer::VertexBuffer(const BufferSpecification& spec, Renderer& renderer)
		: m_renderer(renderer)
	{
		glGenBuffers(1, &m_handle);
		bind();
		m_size = spec.size;
		m_usage = spec.dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
		glCheck(glBufferData(GL_ARRAY_BUFFER, m_size, spec.data, m_usage));
		if (spec.data)
			renderer.onDataTransfer(m_size);
	}

	VertexBuffer::~VertexBuffer()
	{
		glDeleteBuffers(1, &m_handle);
	}

	void VertexBuffer::updateData(const void* data, i64 size, i64 offset)
	{
		if (!isBound())
			bind();

		glCheck(glBufferSubData(GL_ARRAY_BUFFER, offset, size, data));
		m_renderer.onDataTransfer(size);
	}

	void VertexBuffer::bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_handle);
		s_currently_bound_vertex_buffer = m_handle;
	}

	bool VertexBuffer::isBound() const
	{
		return s_currently_bound_vertex_buffer == m_handle;
	}

	IndexBuffer::IndexBuffer(const BufferSpecification& spec, Renderer& renderer)
		: m_renderer(renderer)
	{
		glGenBuffers(1, &m_handle);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_handle);
		m_usage = spec.dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
		glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, spec.size, spec.data, m_usage));
		m_count = spec.size / 4;
		if (spec.data)
			renderer.onDataTransfer(spec.size);
	}

	IndexBuffer::~IndexBuffer()
	{
		glDeleteBuffers(1, &m_handle);
	}

	void IndexBuffer::updateData(const u32* data, i64 size, i64 offset)
	{
		if (!isBound())
			bind();

		glCheck(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data));
		m_renderer.onDataTransfer(size);
	}

	void IndexBuffer::bind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_handle);
		s_currently_bound_index_buffer = m_handle;
	}

	bool IndexBuffer::isBound() const
	{
		return s_currently_bound_index_buffer == m_handle;
	}

	UniformBuffer::UniformBuffer(const UniformBufferSpecification& spec, Renderer& renderer)
		: m_renderer(renderer), m_name(spec.name), m_binding(spec.binding)
	{
		glGenBuffers(1, &m_handle);
		bind();
		m_size = spec.size;
		m_usage = spec.dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
		glCheck(glBufferData(GL_UNIFORM_BUFFER, m_size, spec.data, m_usage));

		if (spec.data)
			renderer.onDataTransfer(m_size);

		glBindBufferBase(GL_UNIFORM_BUFFER, m_binding, m_handle);
		renderer.registerUniformBuffer(*this);
	}

	void UniformBuffer::bind()
	{
		glBindBuffer(GL_UNIFORM_BUFFER, m_handle);
		s_currently_bound_uniform_buffer = m_handle;
	}

	bool UniformBuffer::isBound() const
	{
		return s_currently_bound_uniform_buffer == m_handle;
	}

	void UniformBuffer::updateData(const void* data, i64 size, i64 offset)
	{
		if (!isBound())
			bind();

		glCheck(glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data));
		m_renderer.onDataTransfer(size);
	}

	UniformBuffer::~UniformBuffer()
	{
		m_renderer.unregisterUniformBuffer(*this);
		glDeleteBuffers(1, &m_handle);
	}
}
