#pragma once

#include <string>
#include <cinttypes>
#include <type_traits>

#include "macros.hpp"
#include "composition.hpp"

#include "data\types.hpp"
#include "data\results.hpp"

namespace impl
{
	namespace _flow
	{
		#pragma region utilities

		struct data_visitor
		{
			const data::types::base& item;

			struct error {};
			struct invalid_item: error {};

			FLOW_ACTION
			{
				switch (item.id)
				{
				case data::types::ids::end: FLOW_CONTINUE();
				case data::types::ids::sum: FLOW_CONTINUE(reinterpret_cast<const data::types::sum&>(item));
				case data::types::ids::compare: FLOW_CONTINUE(reinterpret_cast<const data::types::compare&>(item));
				case data::types::ids::nada: FLOW_CONTINUE(reinterpret_cast<const data::types::nada&>(item));
				default: FLOW_CONTINUE(invalid_item{});
				}
			}
		};

		struct data_walker
		{
			uint8_t const * cursor;

			struct error {};
			struct invalid_item: error {};

			FLOW_ACTION
			{
				return data_visitor{ .item= *reinterpret_cast<data::types::base const *>(cursor) } >> cppflow::composition::flow +
				[this, &FLOW_OP]{ FLOW_CONTINUE(*this); } +
				[this, &FLOW_OP](data_visitor::error /*e*/) { FLOW_CONTINUE(*this, invalid_item{}); } +
				[this, &FLOW_OP](auto& item) 
				{ 
					cursor += sizeof(item);
					FLOW_CONTINUE(*this, item);
				};
			}
		};

		#pragma endregion

		#pragma region reduce list

		class reduce_link
		{
		public:
			inline reduce_link(reduce_link const * &chain):
			previous{ chain }
			{
				chain = this;
			}

			inline void reduce(char* buffer) const { apply(buffer); }

		protected:
			void next(char* buffer) const { previous->apply(buffer); }

		private:
			virtual void apply(char* buffer) const = 0;
			reduce_link const * previous;
		};

		class reduce_end final: public reduce_link
		{
		public:
			using reduce_link::reduce_link;
		
		private:
			virtual void apply(char* /*buffer*/) const final {}
		};

		template <typename T>
		concept Result = std::is_base_of_v<data::results::base, T>;

		template <Result ResultT>
		class reduce_result final: public reduce_link, ResultT
		{
		public:
			template <typename... ResultArgTs>
			reduce_result(reduce_link const * &chain, ResultArgTs&&... resultArgs):
			reduce_link(chain),
			ResultT(std::forward<ResultArgTs>(resultArgs)...)
			{}

			using ResultT::str_size;

		private:
			virtual void apply(char* buffer) const final
			{
				buffer -= ResultT::str_size();
				ResultT::to_string(buffer);
				next(buffer);
			}
		};

		#pragma endregion
	}

	struct flow
	{
		uint8_t const * data;

		struct error {};
		struct invalid_data: error {};

		FLOW_ACTION
		{
			using result_t = decltype(FLOW_OP({ std::declval<std::string>() }));

			_flow::reduce_link const * chain;
			_flow::reduce_end ender{chain};

			size_t strSize = 0;
			return _flow::data_walker{ .cursor= data } >> cppflow::composition::reflow +
			[&chain, &strSize](auto&& self, _flow::data_walker& walker, const data::types::sum& item) -> result_t // sum_id handler
			{
				const _flow::reduce_result<data::results::sum> applier{chain, item };
				strSize += applier.str_size();

				return walker >> self;
			} +
			[&chain, &strSize](auto&& self, _flow::data_walker& walker, const data::types::compare& item) -> result_t // compare_id handler
			{
				const _flow::reduce_result<data::results::compare> applier{chain, item };
				strSize += applier.str_size();

				return walker >> self;
			} +
			[](auto&& self, _flow::data_walker& walker, const data::types::nada& /*item*/) -> result_t // nada_id handler
			{
				return walker >> self;
			} +
			[this, &FLOW_OP, &strSize, &chain](auto&& /*self*/, _flow::data_walker& /*walker*/) -> result_t //input buffer end
			{
				std::string retval;
				retval.resize_and_overwrite(strSize, [strSize, chain](char* buff, size_t)
				{
					chain->reduce(buff + strSize);
					return strSize;
				});
				FLOW_CONTINUE(retval);
			} +
			[&FLOW_OP](auto&& /*self*/, _flow::data_walker& /*walker*/, _flow::data_walker::error /*e*/) -> result_t // error handler
			{ FLOW_CONTINUE(invalid_data{}); };
		}
	};
}
