#include "uze/common.h"
#include <cstdlib>

namespace uze
{

	struct Buffer
	{
		u8* data{ nullptr };
		u64 size{ 0 };

		Buffer() = default;

		Buffer(u64 size)
		{
			allocate(size);
		}

		Buffer(const Buffer&) = default;

		static Buffer copy(Buffer other)
		{
			Buffer result(other.size);
			std::memcpy(result.data, other.data, other.size);
			return result;
		}

		void allocate(u64 size_)
		{
			release();
			data = new u8[size_];
			size = size_;
		}

		void release()
		{
			delete[] data;
			data = nullptr;
			size = 0;
		}

		template <class T>
		T* as() const { return reinterpret_cast<T*>(data); }

		explicit operator bool() const
		{
			return data;
		}
	};

	struct ScopedBuffer
	{
		ScopedBuffer(Buffer buffer) : m_buffer(buffer) {}
		ScopedBuffer(u64 size) : m_buffer(size) {}
		~ScopedBuffer() { m_buffer.release(); }

		ScopedBuffer(const ScopedBuffer&) = delete;
		ScopedBuffer& operator=(const ScopedBuffer&) = delete;

		u8* data() const { return m_buffer.data; }
		u64 size() const { return m_buffer.size; }

		template <class T>
		T* as() { return m_buffer.as<T>(); }

		operator bool() const { return static_cast<bool>(m_buffer); }

	private:

		Buffer m_buffer;
	};

}