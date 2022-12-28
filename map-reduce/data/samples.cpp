#include "samples.hpp"

#include <random>

#include "types.hpp"

namespace
{
	std::vector<uint8_t> generate_valid(size_t count);
	std::vector<uint8_t> generate_invalid(size_t count);

	uint32_t rand_number()
	{
		static std::mt19937 dev{0x12345678};
		static std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX);

		return dis(dev);
	}

	std::vector<uint8_t> generate_valid(size_t count)
	{
		std::vector<uint8_t> retVal;
		for (size_t i = 0; i < count; ++i)
		{
			if (i % 3 == 0)
			{
				const data::types::sum item{rand_number(), rand_number(), rand_number()};
				retVal.insert(retVal.end(), reinterpret_cast<uint8_t const *>(&item), reinterpret_cast<uint8_t const *>(&item) + sizeof(item));
			}
			else if (i % 3 == 1)
			{
				const data::types::compare item{rand_number(), rand_number()};
				retVal.insert(retVal.end(), reinterpret_cast<uint8_t const *>(&item), reinterpret_cast<uint8_t const *>(&item) + sizeof(item));
			}
			else if (i % 3 == 2)
			{
				const data::types::nada item{};
				retVal.insert(retVal.end(), reinterpret_cast<uint8_t const *>(&item), reinterpret_cast<uint8_t const *>(&item) + sizeof(item));
			}
		}

		const data::types::end endItem{};
		retVal.insert(retVal.end(), reinterpret_cast<uint8_t const *>(&endItem), reinterpret_cast<uint8_t const *>(&endItem) + sizeof(endItem));
		return retVal;
	}

	std::vector<uint8_t> generate_invalid(size_t count)
	{
		auto data = generate_valid(count);
		data.back() = static_cast<uint8_t>(data::types::ids::invalid);
		return data;
	}

}

std::map<data::test, std::vector<uint8_t>> data::samples = 
{
	{ data::test{ 0, true }, generate_valid(0) },
	{ data::test{ 1, true }, generate_valid(1) },
	{ data::test{ 1, false }, generate_invalid(1) },
	{ data::test{ 10, true }, generate_valid(10) },
	{ data::test{ 10, false }, generate_invalid(10) },
	{ data::test{ 100, true }, generate_valid(100) },
	{ data::test{ 100, false }, generate_invalid(100) },
	{ data::test{ 1000, true }, generate_valid(1000) },
	{ data::test{ 1000, false }, generate_invalid(1000) },
	{ data::test{ 10000, true }, generate_valid(10000) },
	{ data::test{ 10000, false }, generate_invalid(10000) },
	{ data::test{ 100000, true }, generate_valid(100000) },
	{ data::test{ 100000, false }, generate_invalid(100000) },
	{ data::test{ 1000000, true }, generate_valid(1000000) },
	{ data::test{ 1000000, false }, generate_invalid(1000000) }
};
