#include <benchmark/benchmark.h>
#include "filefinder.h"

static void BM_InitRtp(benchmark::State& state) {
	bool no_rtp_flag = false;
	bool no_rtp_warning_flag = false;
	for (auto _: state) {
		FileFinder::InitRtpPaths(no_rtp_flag, no_rtp_warning_flag);
	}
}

BENCHMARK(BM_InitRtp);

BENCHMARK_MAIN();
