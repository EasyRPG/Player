#include <benchmark/benchmark.h>
#include "game_variables.h"

constexpr int max_vars = 1024; // Keep this a power of 2 so no expensive modulus instructions

static Game_Variables make(int size = max_vars) {
	Data::variables.resize(size);
	Game_Variables variables(Game_Variables::min_2k3, Game_Variables::max_2k3);
	variables.Set(size, 0);
	return variables;
}

template <typename F>
static void BM_VariableOp(benchmark::State& state, F&& op) {
	auto v = make();
	int i = 0;
	for (auto _: state) {
		op(v, i + 1, i);
		i = (i + 1) % max_vars;
	}
}

static void BM_VariableGet(benchmark::State& state) {
	volatile int x = 0;
	BM_VariableOp(state, [&x](auto& v, auto id, auto val) { x = v.Get(id); });
}

BENCHMARK(BM_VariableGet);

static void BM_VariableSet(benchmark::State& state) {
	volatile int x = 0;
	BM_VariableOp(state, [&x](auto& v, auto id, auto val) { x = v.Set(id, val); });
}

BENCHMARK(BM_VariableSet);

static void BM_VariableAdd(benchmark::State& state) {
	volatile int x = 0;
	BM_VariableOp(state, [&x](auto& v, auto id, auto val) { x = v.Add(id, val); });
}

BENCHMARK(BM_VariableAdd);

static void BM_VariableSub(benchmark::State& state) {
	volatile int x = 0;
	BM_VariableOp(state, [&x](auto& v, auto id, auto val) { x = v.Sub(id, val); });
}

BENCHMARK(BM_VariableSub);

static void BM_VariableMult(benchmark::State& state) {
	volatile int x = 0;
	BM_VariableOp(state, [&x](auto& v, auto id, auto val) { x = v.Mult(id, val); });
}

BENCHMARK(BM_VariableMult);

static void BM_VariableDiv(benchmark::State& state) {
	volatile int x = 0;
	BM_VariableOp(state, [&x](auto& v, auto id, auto val) { x = v.Div(id, val); });
}

BENCHMARK(BM_VariableDiv);

static void BM_VariableMod(benchmark::State& state) {
	volatile int x = 0;
	BM_VariableOp(state, [&x](auto& v, auto id, auto val) { x = v.Mod(id, val); });
}

BENCHMARK(BM_VariableMod);

static void BM_VariableSetRange(benchmark::State& state) {
	BM_VariableOp(state, [](auto& v, auto id, auto val) { v.SetRange(1, max_vars, val); });
}

BENCHMARK(BM_VariableSetRange);

static void BM_VariableAddRange(benchmark::State& state) {
	BM_VariableOp(state, [](auto& v, auto id, auto val) { v.AddRange(1, max_vars, val); });
}

BENCHMARK(BM_VariableAddRange);

static void BM_VariableSubRange(benchmark::State& state) {
	BM_VariableOp(state, [](auto& v, auto id, auto val) { v.SubRange(1, max_vars, val); });
}

BENCHMARK(BM_VariableSubRange);

static void BM_VariableMultRange(benchmark::State& state) {
	BM_VariableOp(state, [](auto& v, auto id, auto val) { v.MultRange(1, max_vars, val); });
}

BENCHMARK(BM_VariableMultRange);

static void BM_VariableDivRange(benchmark::State& state) {
	BM_VariableOp(state, [](auto& v, auto id, auto val) { v.DivRange(1, max_vars, val); });
}

BENCHMARK(BM_VariableDivRange);

static void BM_VariableModRange(benchmark::State& state) {
	BM_VariableOp(state, [](auto& v, auto id, auto val) { v.ModRange(1, max_vars, val); });
}

BENCHMARK(BM_VariableModRange);

BENCHMARK_MAIN();
