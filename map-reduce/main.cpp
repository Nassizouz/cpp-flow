#include <benchmark/benchmark.h>

#include "impl\flow.hpp"
#include "impl\classic.hpp"

#include "data\samples.hpp"

const auto test_flow = [](auto& bmark, const auto dataSample)
{
	bool success = true;
	std::string result;
	for (auto _: bmark)
	{
		bmark.PauseTiming();
		result = "";
		bmark.ResumeTiming();

		success = impl::flow{ .data= dataSample } >> cppflow::composition::flow +
		[&result](std::string successStr) 
		{ 
			result = std::move(successStr);
			return true;
		} +
		[](const impl::flow::error&) { return false; };
	}

	bmark.counters["success"] = success ? 1.0 : 0.0;
	if (success)
	{
		std::hash<std::string> hasher{};
		bmark.counters["result"] = double(hasher(result));
	}
};

const auto test_classic = [](auto& bmark, auto dataSample)
{ 
	impl::classic::status status = impl::classic::status::success;
	std::string result;
	for (auto _: bmark)
	{
		bmark.PauseTiming();
		result = "";
		bmark.ResumeTiming();

		result = impl::classic::map_reduce(status, dataSample);
	}

	bmark.counters["success"] = (status == impl::classic::status::success) ? 1.0 : 0.0;
	if (status == impl::classic::status::success)
	{
		std::hash<std::string> hasher{};
		bmark.counters["result"] = double(hasher(result));
	}
};

int main(int argc, char** argv)
{
	benchmark::RegisterBenchmark(R"({ "type": "flow", "valid_data": true, "count": 0 })", test_flow, data::samples::empty.data());
	benchmark::RegisterBenchmark(R"({ "type": "classic", "valid_data": true, "count": 0 })", test_classic, data::samples::empty.data());
	benchmark::RegisterBenchmark(R"({ "type": "flow", "valid_data": true, "count": 1 })", test_flow, data::samples::valid1.data());
	benchmark::RegisterBenchmark(R"({ "type": "classic", "valid_data": true, "count": 1 })", test_classic, data::samples::valid1.data());
	benchmark::RegisterBenchmark(R"({ "type": "flow", "valid_data": false, "count": 1 })", test_flow, data::samples::invalid1.data());
	benchmark::RegisterBenchmark(R"({ "type": "classic", "valid_data": false, "count": 1 })", test_classic, data::samples::invalid1.data());
	benchmark::RegisterBenchmark(R"({ "type": "flow", "valid_data": true, "count": 10 })", test_flow, data::samples::valid10.data());
	benchmark::RegisterBenchmark(R"({ "type": "classic", "valid_data": true, "count": 10 })", test_classic, data::samples::valid10.data());
	benchmark::RegisterBenchmark(R"({ "type": "flow", "valid_data": false, "count": 10 })", test_flow, data::samples::invalid10.data());
	benchmark::RegisterBenchmark(R"({ "type": "classic", "valid_data": false, "count": 10 })", test_classic, data::samples::invalid10.data());
	benchmark::RegisterBenchmark(R"({ "type": "flow", "valid_data": true, "count": 100 })", test_flow, data::samples::valid100.data());
	benchmark::RegisterBenchmark(R"({ "type": "classic", "valid_data": true, "count": 100 })", test_classic, data::samples::valid100.data());
	benchmark::RegisterBenchmark(R"({ "type": "flow", "valid_data": false, "count": 100 })", test_flow, data::samples::invalid100.data());
	benchmark::RegisterBenchmark(R"({ "type": "classic", "valid_data": false, "count": 100 })", test_classic, data::samples::invalid100.data());
	benchmark::RegisterBenchmark(R"({ "type": "flow", "valid_data": true, "count": 1000 })", test_flow, data::samples::valid1000.data());
	benchmark::RegisterBenchmark(R"({ "type": "classic", "valid_data": true, "count": 1000 })", test_classic, data::samples::valid1000.data());
	benchmark::RegisterBenchmark(R"({ "type": "flow", "valid_data": false, "count": 1000 })", test_flow, data::samples::invalid1000.data());
	benchmark::RegisterBenchmark(R"({ "type": "classic", "valid_data": false, "count": 1000 })", test_classic, data::samples::invalid1000.data());
	benchmark::RegisterBenchmark(R"({ "type": "flow", "valid_data": true, "count": 10000 })", test_flow, data::samples::valid10000.data());
	benchmark::RegisterBenchmark(R"({ "type": "classic", "valid_data": true, "count": 10000 })", test_classic, data::samples::valid10000.data());
	benchmark::RegisterBenchmark(R"({ "type": "flow", "valid_data": false, "count": 10000 })", test_flow, data::samples::invalid10000.data());
	benchmark::RegisterBenchmark(R"({ "type": "classic", "valid_data": false, "count": 10000 })", test_classic, data::samples::invalid10000.data());
	benchmark::RegisterBenchmark(R"({ "type": "flow", "valid_data": true, "count": 100000 })", test_flow, data::samples::valid100000.data());
	benchmark::RegisterBenchmark(R"({ "type": "classic", "valid_data": true, "count": 100000 })", test_classic, data::samples::valid100000.data());
	benchmark::RegisterBenchmark(R"({ "type": "flow", "valid_data": false, "count": 100000 })", test_flow, data::samples::invalid100000.data());
	benchmark::RegisterBenchmark(R"({ "type": "classic", "valid_data": false, "count": 100000 })", test_classic, data::samples::invalid100000.data());

	benchmark::Initialize(&argc, argv);
	benchmark::RunSpecifiedBenchmarks();
	benchmark::Shutdown();
}
