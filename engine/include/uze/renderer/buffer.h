#pragma once

#include "uze/common.h"
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace uze
{

	struct VertexBufferElement final
	{
		u32 count;
		u32 type;
		bool normalized;
	};

	struct VertexLayout final
	{
		template <class T>
		VertexLayout& push(u32 count)
		{
			return *this;
		}

		template <>
		VertexLayout& push<float>(u32 count)
		{
			m_elements.push_back({ count, 0x1406, false });
			m_stride += sizeof(float) * count;
			return *this;
		}

		template <>
		VertexLayout& push<i32>(u32 count)
		{
			m_elements.push_back({ count, 0x1404, false });
			m_stride += sizeof(i32) * count;
			return *this;
		}

		template <>
		VertexLayout& push<u32>(u32 count)
		{
			m_elements.push_back({ count, 0x1405, false });
			m_stride += sizeof(u32) * count;
			return *this;
		}

		template <>
		VertexLayout& push<u8>(u32 count)
		{
			m_elements.push_back({ count, 0x1401, true });
			m_stride += sizeof(u8) * count;
			return *this;
		}

		template <>
		VertexLayout& push<glm::vec2>(u32 count)
		{
			for (u32 i = 0; i < count; ++i)
				push<float>(2);
			return *this;
		}

		template <>
		VertexLayout& push<glm::vec4>(u32 count)
		{
			for (u32 i = 0; i < count; ++i)
				push<float>(4);
			return *this;
		}

		u32 getStride() const { return m_stride; }
		const std::vector<VertexBufferElement>& getElements() const { return m_elements; }

	private:

		std::vector<VertexBufferElement> m_elements;
		u32 m_stride{ 0 };
	};

	struct BufferSpecification final
	{
		bool dynamic = false;
		u32 size{ 0 };
		void* data{ nullptr };
	};

	class VertexBuffer final : NonCopyable<VertexBuffer>
	{
	public:

		~VertexBuffer();

		void updateData(const void* data, i64 size, i64 offset = 0);

		bool isDynamic() const { return m_usage == 0x88E8; }

	private:

		u32 m_handle{ 0 };
		u32 m_usage{ 0 };
		u32 m_size{ 0 };

		VertexBuffer(const BufferSpecification& spec);

		void bind();
		bool isBound() const;

		friend class Renderer;
		friend class VertexArrayBuilder;
	};

	class IndexBuffer final : NonCopyable<IndexBuffer>
	{
	public:

		~IndexBuffer();

		void updateData(const u32* data, i64 size, i64 offset = 0);
		u32 getCount() const { return m_count; }

	private:

		u32 m_handle{ 0 };
		u32 m_usage{ 0 };
		u32 m_count{ 0 };

		IndexBuffer(const BufferSpecification& spec);

		void bind();
		bool isBound() const;

		friend class Renderer;
		friend class VertexArrayBuilder;

	};

}
