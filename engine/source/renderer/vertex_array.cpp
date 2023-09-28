#include "uze/renderer/vertex_array.h"
#include "opengl.h"

namespace uze
{

	VertexArray::VertexArray()
	{
		glGenVertexArrays(1, &m_handle);
	}

	VertexArray::~VertexArray()
	{
		if (m_handle)
			glDeleteVertexArrays(1, &m_handle);
	}

	void VertexArray::addVertexBuffer(const std::shared_ptr<VertexBuffer>& buffer, const VertexLayout& layout)
	{
		glBindVertexArray(m_handle);
		buffer->bind();

		const auto& elements = layout.getElements();
		for (const auto& element : elements)
		{
			glEnableVertexAttribArray(m_attribute_index);
			glVertexAttribPointer(m_attribute_index++, element.count, element.type,
				element.normalized, layout.getStride(), reinterpret_cast<const void*>(element.offset));

			m_vertex_buffers.push_back(buffer);
		}
	}

	void VertexArray::setIndexBuffer(const std::shared_ptr<IndexBuffer>& buffer)
	{
		glBindVertexArray(m_handle);
		buffer->bind();

		m_index_buffer = buffer;
	}

}
