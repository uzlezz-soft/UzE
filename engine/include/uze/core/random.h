#pragma once

#include "uze/common.h"
#include <random>

namespace uze
{

	class UZE Random
	{
	public:

		Random();
		Random(u32 seed);

		i32 next();
		i32 next(i32 max_value);
		i32 next(i32 min_value, i32 max_value);

		float nextFloat();
		float nextFloat(float min_value, float max_value);

	private:

		std::mt19937 m_random_number_generator;

	};

}