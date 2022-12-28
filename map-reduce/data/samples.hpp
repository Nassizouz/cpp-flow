#pragma once

#include <map>
#include <vector>

#include "types.hpp"

namespace data
{
	struct test
	{
		size_t count;
		bool valid;
		std::strong_ordering operator<=> (const test&) const = default;
	};
	extern std::map<test, std::vector<uint8_t>> samples;
}
