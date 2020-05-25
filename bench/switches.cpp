#include <benchmark/benchmark.h>
#include "game_switches.h"
#include <lcf/data.h>

constexpr int max_sws = 1024; // Keep this a power of 2 so no expensive modulus instructions

static Game_Switches make(int size = max_sws) {
	lcf::Data::switches.resize(size);
	Game_Switches switches;
	switches.Set(size, false);
	return switches;
}

template <typename F>
static void BM_SwitchOp(benchmark::State& state, F&& op) {
	auto s = make();
	int i = 0;
	for (auto _: state) {
		op(s, i + 1, i);
		i = (i + 1) % max_sws;
	}
}

static void BM_SwitchGet(benchmark::State& state) {
	volatile int x = 0;
	BM_SwitchOp(state, [&x](auto& s, auto id, bool) { s.Get(id); });
}

BENCHMARK(BM_SwitchGet);

static void BM_SwitchSet(benchmark::State& state) {
	volatile int x = 0;
	BM_SwitchOp(state, [&x](auto& s, auto id, bool val) { x = s.Set(id, val); });
}

BENCHMARK(BM_SwitchSet);

static void BM_SwitchFlip(benchmark::State& state) {
	volatile int x = 0;
	BM_SwitchOp(state, [&x](auto& s, auto id, bool) { x = s.Flip(id); });
}

BENCHMARK(BM_SwitchFlip);

template <typename F>
static void BM_SwitchRangeOp(benchmark::State& state, F&& op) {
	auto s = make();
	int i = 0;
	for (auto _: state) {
		op(s, i + 1, i);
		i = (i + 1) % max_sws;
	}
}

static void BM_SwitchSetRange(benchmark::State& state) {
	BM_SwitchOp(state, [](auto& s, auto, bool val) { s.SetRange(1, max_sws, val); });
}

BENCHMARK(BM_SwitchSetRange);

static void BM_SwitchFlipRange(benchmark::State& state) {
	BM_SwitchOp(state, [](auto& s, auto, bool) { s.FlipRange(1, max_sws); });
}

BENCHMARK(BM_SwitchFlipRange);


BENCHMARK_MAIN();
