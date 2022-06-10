#include <benchmark/benchmark.h>
#include "filefinder_rtp.h"
#include "output.h"
#include "player.h"

static void BM_InitRtp2k(benchmark::State& state) {
	Output::SetLogLevel(LogLevel::Error);
	Player::engine = Player::EngineRpg2k;

	bool no_rtp_flag = false;
	bool no_rtp_warning_flag = false;
	for (auto _: state) {
		FileFinder_RTP(no_rtp_flag, no_rtp_warning_flag, "");
	}

	Player::engine = Player::EngineNone;
	Output::SetLogLevel(LogLevel::Debug);
}

BENCHMARK(BM_InitRtp2k);

static void BM_InitRtp2k3(benchmark::State& state) {
	Output::SetLogLevel(LogLevel::Error);
	Player::engine = Player::EngineRpg2k3;

	bool no_rtp_flag = false;
	bool no_rtp_warning_flag = false;
	for (auto _: state) {
		FileFinder_RTP(no_rtp_flag, no_rtp_warning_flag, "");
	}

	Player::engine = Player::EngineNone;
	Output::SetLogLevel(LogLevel::Debug);
}

BENCHMARK(BM_InitRtp2k3);

BENCHMARK_MAIN();
