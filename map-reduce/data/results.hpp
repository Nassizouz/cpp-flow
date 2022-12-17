#pragma once

#include <cstring>

#include "types.hpp"

namespace data::results
{
	class base
	{
	public:
		virtual size_t str_size() const = 0;
		virtual char* /*next_buffer*/ to_string(char* buffer) const = 0;
		virtual ~base() = default;
	};

	class sum: public base
	{
	public:
		inline sum(const types::sum& item): 
		result{ item.items[0] + item.items[1] + item.items[2] }
		{}

		static constexpr char str_prefix[] = "result::sum::str:";
		inline size_t str_size() const final
		{
			return sizeof(str_prefix) + sizeof('\n'); // The null terminator is replaced with the result
		}

		inline char* /*next_buffer*/ to_string(char* buffer) const final
		{
			memcpy(buffer, str_prefix, sizeof(str_prefix) - sizeof('\0'));
			buffer += sizeof(str_prefix) - sizeof('\0');
			*buffer++ = char((result % 10) + '0');
			*buffer++ = '\n';
			
			return buffer;
		}

		uint32_t result;
	};

	class compare: public base
	{
	public:
		inline compare(const types::compare& item): 
		result{ item.a > item.b }
		{}

		static constexpr char str_prefix[] = "result::compare::str:";
		inline size_t str_size() const final
		{
			return sizeof(str_prefix) + sizeof('\n'); // The null terminator is replaced with the result
		}

		inline char* /*next_buffer*/ to_string(char* buffer) const final
		{
			memcpy(buffer, str_prefix, sizeof(str_prefix) - sizeof('\0'));
			buffer += sizeof(str_prefix) - sizeof('\0');
			*buffer++ = char(result ? 'G' : 'L');
			*buffer++ = '\n';
			
			return buffer; 
		}

		bool result;
	};
}
