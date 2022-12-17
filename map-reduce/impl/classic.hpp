#pragma once

#include <string>

namespace impl::classic
{
	enum class status
	{
		success,
		invalid_data
	};

	std::string map_reduce(status& status, uint8_t const * data);
}
