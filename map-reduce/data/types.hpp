#pragma once

#include <cinttypes>

namespace data::types
{
	enum class ids
	{
		end = 0,
		sum = 1,
		compare = 2,
		nada = 3,
		invalid = 4
	};

#pragma pack(push, 1)
	struct base
	{
		ids id;
	};

	struct sum: base
	{
		inline sum(): base{ ids::sum } {}
		inline sum(uint32_t first, uint32_t second, uint32_t third): 
		base{ ids::sum },
		items{first, second, third}
		{}

		uint32_t items[3] = {0};
	};

	struct compare: base
	{
		inline compare(): base{ ids::compare } {}
		inline compare(uint32_t first, uint32_t second):
		base{ ids::compare },
		a{ first },
		b{ second }
		{}

		uint32_t a = 0;
		uint32_t b = 0;
	};

	struct nada: base
	{
		inline nada():
		base{ ids::nada }
		{}
	};

	struct end: base
	{
		inline end():
		base{ ids::end }
		{}
	};
#pragma pack(pop)
}
