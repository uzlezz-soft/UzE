#pragma once

#include "uze/renderer/buffer.h"

namespace uze
{

	class VertexArray final : NonCopyable<VertexArray>
	{
	public:

		~VertexArray();

		const std::vector<std::shared_ptr<VertexBuffer>>& getVertexBuffers() const { return m_vertex_buffers; }
		const std::shared_ptr<IndexBuffer>& getIndexBuffer() const { return m_index_buffer; }

		void addVertexBuffer(const std::shared_ptr<VertexBuffer>& buffer, const VertexLayout& layout);
		void setIndexBuffer(const std::shared_ptr<IndexBuffer>& buffer);

	private:

		u32 m_handle{ 0 };
		u32 m_attribute_index{ 0 };
		std::vector<std::shared_ptr<VertexBuffer>> m_vertex_buffers{ nullptr };
		std::shared_ptr<IndexBuffer> m_index_buffer{ nullptr };

		VertexArray();

		friend class Renderer;
	};

}