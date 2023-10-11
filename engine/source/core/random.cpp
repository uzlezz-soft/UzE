#include "uze/core/random.h"

namespace uze
{

	Random::Random() : Random(std::random_device()())
	{
	}

	Random::Random(u32 seed) : m_random_number_generator(seed)
	{
	}

	i32 Random::next()
	{
		return next(0, std::numeric_limits<i32>::max());
	}

	i32 Random::next(i32 maxValue)
	{
		return next(0, maxValue);
	}

	i32 Random::next(i32 min_value, i32 max_value)
	{
		std::uniform_int_distribution distribution(min_value, max_value);
		return distribution(m_random_number_generator);
	}

	float Random::nextFloat()
	{
		return nextFloat(0.0f, 1.0f);
	}

	float Random::nextFloat(float min_value, float max_value)
	{
		std::uniform_real_distribution<> distribution(min_value, max_value);
		return distribution(m_random_number_generator);
	}

}