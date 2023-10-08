#include "uze/common.h"
#include <atomic>
#include <optional>

namespace uze
{

	template <class T>
	class ConcurrentQueue
	{
	public:

		void push(const T& data);

		std::optional<T> pop();

	private:

		struct Node
		{
			T data;
			std::atomic<Node*> next;
			Node(T data_) : data(data_) {}
		};

		std::atomic<Node*> m_head;
		std::atomic<Node*> m_tail;
	};

	template <class T>
	inline void ConcurrentQueue<T>::push(const T& data)
	{
		const std::atomic<Node*> node = new Node(data);
		Node* old_tail = m_tail.load();
		while (!old_tail->next.compare_exchange_weak(nullptr, node))
			old_tail = m_tail.load();

		m_tail.compare_exchange_weak(old_tail, node);
	}

	template <class T>
	inline std::optional<T> ConcurrentQueue<T>::pop()
	{
		Node* old_head = m_head.load();
		while (old_head && !m_head.compare_exchange_weak(old_head, old_head->next))
			old_head = m_head.load();

		return old_head ? old_head->data : T{};
	}

}