#pragma once

#include <cstdint>
#include <memory>
#include <functional>
#include <chrono>
#include "uze/log.h"

namespace uze
{

	template <class T>
	struct NonCopyable
	{
		NonCopyable(const NonCopyable<T>&) = delete;
		NonCopyable& operator=(const NonCopyable<T>&) = delete;

		NonCopyable() = default;
	};

	struct UZE AtScopeExit final : public NonCopyable<AtScopeExit>
	{
		std::function<void()> m_func;

		AtScopeExit(std::function<void()>&& func)
			: m_func(func) {}
		~AtScopeExit() { m_func(); }
	};

	struct UZE Stopwatch final
	{
		Stopwatch() { reset(); }

		void reset() { m_start = std::chrono::high_resolution_clock::now(); }
		double getElapsedSeconds() const { return getElapsedMilliseconds() * 0.001; }
		double getElapsedMilliseconds() const
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(
				std::chrono::high_resolution_clock::now() - m_start).count() * 0.001 * 0.001;
		}

	private:

		std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
	};

}

#define UZE_EXPAND(x) x

#define UZE_CONCAT_IMPL(a, b) a##b
#define UZE_CONCAT(a, b) UZE_CONCAT_IMPL(a, b)