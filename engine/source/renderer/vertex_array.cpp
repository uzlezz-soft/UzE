#include "uze/renderer/vertex_array.h"
#include "glad/gles3.h"

namespace uze
{

	namespace
	{
		constexpr u32 getSizeOfType(u32 type)
		{
			switch (type)
			{
				case GL_INT: return sizeof(GLint);
				case GL_FLOAT: return sizeof(GLfloat);
				case GL_UNSIGNED_INT: return sizeof(GLuint);
				case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
			}

			return 0;
		}
	}

	VertexArrayBuilder::VertexArrayBuilder()
	{
		reset();
	}


	VertexArray::~VertexArray()
	{
		if (m_handle)
			glDeleteVertexArrays(1, &m_handle);
	}

	VertexArray::VertexArray(u32 handle, const std::vector<std::shared_ptr<VertexBuffer>>& vertex_buffers,
		const std::shared_ptr<IndexBuffer>& index_buffer)
			: m_handle(handle), m_vertex_buffers(vertex_buffers), m_index_buffer(index_buffer)
	{
	}

	VertexArrayBuilder& VertexArrayBuilder::addVertexBuffer(const std::shared_ptr<VertexBuffer>& buffer, const VertexLayout& layout)
	{
		glBindVertexArray(m_handle);
		buffer->bind();

		const auto& elements = layout.getElements();
		std::uintptr_t offset = 0;
		for (const auto& element : elements)
		{
			glEnableVertexAttribArray(m_attribute_index);
			glVertexAttribPointer(m_attribute_index, element.count, element.type,
				element.normalized, layout.getStride(), reinterpret_cast<const void*>(offset));

			offset += element.count * getSizeOfType(element.type);
			++m_attribute_index;
		}

		m_vertex_buffers.push_back(buffer);
		return *this;
	}

	VertexArrayBuilder& VertexArrayBuilder::setIndexBuffer(const std::shared_ptr<IndexBuffer>& buffer)
	{
		glBindVertexArray(m_handle);
		buffer->bind();
		m_index_buffer = buffer;
		return *this;
	}

	std::shared_ptr<VertexArray> VertexArrayBuilder::build()
	{
		auto vertex_array = std::shared_ptr<VertexArray>(new VertexArray(m_handle, m_vertex_buffers, m_index_buffer));
		m_handle = 0;
		reset();
		return vertex_array;
	}

	void VertexArrayBuilder::reset()
	{
		if (m_handle)
			glDeleteVertexArrays(1, &m_handle);

		glGenBuffers(1, &m_handle);
		m_attribute_index = 0;
		m_vertex_buffers.clear();
		m_index_buffer.reset();
	}
}
