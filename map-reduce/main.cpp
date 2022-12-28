#include <format>

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

void register_benchmarks(const auto& test, const auto& sample)
{
	static std::vector<std::string> namesGlobalStorage{};
	const auto& flowName = namesGlobalStorage.emplace_back(std::vformat(R"({{ "type": {}, "valid_data": {}, "count": {} }})", std::make_format_args("\"flow\"", test.valid, test.count)));
	benchmark::RegisterBenchmark(flowName.c_str(), test_flow, sample.data());
	const auto& classicName = namesGlobalStorage.emplace_back(std::vformat(R"({{ "type": {}, "valid_data": {}, "count": {} }})", std::make_format_args("\"classic\"", test.valid, test.count)));
	benchmark::RegisterBenchmark(classicName.c_str(), test_classic, sample.data());
}

int main(int argc, char** argv)
{
	for (const auto& [test, sample]: data::samples) register_benchmarks(test, sample);

	benchmark::Initialize(&argc, argv);
	benchmark::RunSpecifiedBenchmarks();
	benchmark::Shutdown();
}
