#include "uze/renderer/buffer.h"
#include "uze/renderer/renderer.h"
#include "opengl.h"

namespace uze
{

	static u32 s_currently_bound_vertex_buffer{ 0 };
	static u32 s_currently_bound_index_buffer{ 0 };

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

}
