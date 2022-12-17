#include "classic.hpp"

#include <vector>
#include <memory>

#include "data\results.hpp"

std::string impl::classic::map_reduce(status& status, uint8_t const * data)
{
	std::vector<std::unique_ptr<data::results::base>> mapResult;
	size_t size = 0;
	for (auto cursor = reinterpret_cast<data::types::base const*>(data); cursor->id != data::types::ids::end;)
	{
		switch (cursor->id)
		{
		case (data::types::ids::sum):
		{
			const auto item = reinterpret_cast<data::types::sum const*>(cursor);
			auto result = std::make_unique<data::results::sum>(*item);
			size += result->str_size();
			mapResult.emplace_back(std::move(result));

			cursor = reinterpret_cast<data::types::base const*>(item + 1);
			break;
		}
		case (data::types::ids::compare):
		{
			const auto item = reinterpret_cast<data::types::compare const*>(cursor);
			auto result = std::make_unique<data::results::compare>(*item);
			size += result->str_size();
			mapResult.emplace_back(std::move(result));

			cursor = reinterpret_cast<data::types::base const*>(item + 1);
			break;
		}
		case (data::types::ids::nada):
		{
			const auto item = reinterpret_cast<data::types::nada const*>(cursor);
			cursor = reinterpret_cast<data::types::base const*>(item + 1);
			break;
		}
		default: 
		{
			status = status::invalid_data;
			return {};
		}
		}
	}

	std::string retval;
	retval.resize_and_overwrite(size, [&mapResult, size](char* buff, size_t)
	{
		for (const auto& item: mapResult) buff = item->to_string(buff);
		return size;
	});

	status = status::success;
	return retval;
}