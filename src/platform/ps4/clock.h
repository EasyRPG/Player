#ifndef EP_PLATFORM_PS4_CLOCK_H
#define EP_PLATFORM_PS4_CLOCK_H

#include <orbis/libkernel.h>
#include <cstdint>
#include <chrono>

struct Ps4Clock {
	using rep = int64_t;
	using period = std::micro;
	using duration = std::chrono::duration<rep,period>;
	using time_point = std::chrono::time_point<Ps4Clock,duration>;

	static constexpr bool is_steady = true;

	static time_point now();

	template <typename R, typename P>
	static void SleepFor(std::chrono::duration<R,P> dt);

	static constexpr const char* Name();
};

inline Ps4Clock::time_point Ps4Clock::now() {
	auto ticks = sceKernelGetProcessTime();
	return time_point(duration(ticks));
}

template <typename R, typename P>
inline void Ps4Clock::SleepFor(std::chrono::duration<R,P> dt) {
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(dt);
	sceKernelUsleep(us.count());
}

constexpr const char* Ps4Clock::Name() {
	return "Ps4Clock";
}

#endif
