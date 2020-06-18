#include <benchmark/benchmark.h>
#include "game_variables.h"
#include <lcf/data.h>

constexpr int max_vars = 1024; // Keep this a power of 2 so no expensive modulus instructions

static Game_Variables make(int size = max_vars) {
	lcf::Data::variables.resize(size);
	Game_Variables variables(Game_Variables::min_2k3, Game_Variables::max_2k3);
	variables.SetRange(1, size, 1);
	return variables;
}

template <typename F>
static void BM_VariableOp(benchmark::State& state, F&& op) {
	auto v = make();
	int i = 0;
	for (auto _: state) {
		op(v, i + 1, i + 1);
		i = (i + 1) % max_vars;
	}
}

static void BM_VariableGet(benchmark::State& state) {
	volatile int x = 0;
	BM_VariableOp(state, [&x](auto& v, auto id, auto) { x = v.Get(id); });
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
	BM_VariableOp(state, [](auto& v, auto, auto val) { v.SetRange(1, max_vars, val); });
}

BENCHMARK(BM_VariableSetRange);

static void BM_VariableAddRange(benchmark::State& state) {
	BM_VariableOp(state, [](auto& v, auto, auto val) { v.AddRange(1, max_vars, val); });
}

BENCHMARK(BM_VariableAddRange);

static void BM_VariableSubRange(benchmark::State& state) {
	BM_VariableOp(state, [](auto& v, auto, auto val) { v.SubRange(1, max_vars, val); });
}

BENCHMARK(BM_VariableSubRange);

static void BM_VariableMultRange(benchmark::State& state) {
	BM_VariableOp(state, [](auto& v, auto, auto val) { v.MultRange(1, max_vars, val); });
}

BENCHMARK(BM_VariableMultRange);

static void BM_VariableDivRange(benchmark::State& state) {
	BM_VariableOp(state, [](auto& v, auto, auto val) { v.DivRange(1, max_vars, val); });
}

BENCHMARK(BM_VariableDivRange);

static void BM_VariableModRange(benchmark::State& state) {
	BM_VariableOp(state, [](auto& v, auto, auto val) { v.ModRange(1, max_vars, val); });
}

BENCHMARK(BM_VariableModRange);

static void BM_VariableSetRangeVariable(benchmark::State& state) {
	BM_VariableOp(state, [](auto& v, auto, auto val) { v.SetRangeVariable(1, max_vars, val); });
}

BENCHMARK(BM_VariableSetRangeVariable);

static void BM_VariableSetRangeVariableIndirect(benchmark::State& state) {
	BM_VariableOp(state, [](auto& v, auto, auto val) { v.SetRangeVariableIndirect(1, max_vars, val); });
}

BENCHMARK(BM_VariableSetRangeVariableIndirect);

static void BM_VariableSetRangeRandom(benchmark::State& state) {
	BM_VariableOp(state, [](auto& v, auto, auto val) { v.SetRangeRandom(1, max_vars, -100, 100); });
}

BENCHMARK(BM_VariableSetRangeRandom);

BENCHMARK_MAIN();
