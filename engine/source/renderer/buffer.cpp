#include "uze/renderer/buffer.h"
#include "glad/gles3.h"

namespace uze
{

	static u32 s_currently_bound_vertex_buffer{ 0 };

	VertexBuffer::VertexBuffer(const VertexBufferSpecification& spec)
	{
		glGenBuffers(1, &m_handle);
		bind();
		m_size = spec.size;
		m_usage = spec.dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
		glBufferData(GL_ARRAY_BUFFER, m_size, spec.data, m_usage);
	}

	VertexBuffer::~VertexBuffer()
	{
		glDeleteBuffers(1, &m_handle);
	}

	void VertexBuffer::updateData(const void* data, i64 size, i64 offset)
	{
		if (!isBound())
			bind();

		glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
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
}
