/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EP_INSTRUMENTATION_H
#define EP_INSTRUMENTATION_H

#if defined(PLAYER_INSTRUMENTATION) && PLAYER_INSTRUMENTATION == VTune
#define PLAYER_INSTRUMENTATION_VTUNE
#endif

#ifdef PLAYER_INSTRUMENTATION_VTUNE
#include <ittnotify.h>
#endif
#include <cassert>

class Instrumentation {
public:
	/**
	 * Must be called once on startup to initialize the instrumentation framework.
	 *
	 * @param name name of instrumentation domain
	 */
	static void Init(const char* name);

	/** Call at the beginning of a frame */
	static void FrameBegin();

	/** Call at the end of a frame */
	static void FrameEnd();

	/** RAII wrapper around FrameBegin() / FrameEnd() */
	class FrameScope {
	public:
		/**
		 * Create a FrameScope
		 *
		 * @param frame_begin if true, call Begin()
		 */
		FrameScope(bool frame_begin = true);

		FrameScope(const FrameScope&) = delete;
		FrameScope& operator=(const FrameScope&) = delete;

		/** Take ownership of Frame scope */
		FrameScope(FrameScope&&) noexcept;
		/** Take ownership of Frame scope */
		FrameScope& operator=(FrameScope&&) noexcept;

		/** Calls End() */
		~FrameScope();

		/** Calls FrameBegin() if not already begin */
		void Begin() noexcept;
		/** Calls FrameEnd() if Begin() was called */
		void End() noexcept;
		/** Disables the FrameScope */
		void Dismiss() noexcept;
	private:
		bool begun = false;
	};

private:
#ifdef PLAYER_INSTRUMENTATION_VTUNE
	static __itt_domain* domain;
#endif
};

inline void Instrumentation::FrameBegin() {
#ifdef PLAYER_INSTRUMENTATION_VTUNE
	assert(domain);
	__itt_frame_begin_v3(domain, nullptr);
#endif
}
inline void Instrumentation::FrameEnd() {
#ifdef PLAYER_INSTRUMENTATION_VTUNE
	assert(domain);
	__itt_frame_end_v3(domain, nullptr);
#endif
}

inline Instrumentation::FrameScope::FrameScope(bool frame_begin)
{
	if (frame_begin) {
		Begin();
	}
}

inline Instrumentation::FrameScope::FrameScope(FrameScope&& o) noexcept
	: begun(o.begun)
{
	o.begun = false;
}

inline Instrumentation::FrameScope& Instrumentation::FrameScope::operator=(FrameScope&& o) noexcept
{
	if (this != &o) {
		End();
		begun = o.begun;
		o.begun = false;
	}
	return *this;
}

inline Instrumentation::FrameScope::~FrameScope() {
	End();
}

inline void Instrumentation::FrameScope::Begin() noexcept {
	if (!begun) {
		Instrumentation::FrameBegin();
		begun = true;
	}
}

inline void Instrumentation::FrameScope::End() noexcept {
	if (begun) {
		Instrumentation::FrameEnd();
		begun = false;
	}
}

inline void Instrumentation::FrameScope::Dismiss() noexcept {
	begun = false;
}

#endif
