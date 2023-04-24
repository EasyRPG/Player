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

#ifndef EP_GAME_SWITCHES_H
#define EP_GAME_SWITCHES_H

// Headers
#include <vector>
#include <string>
#include <lcf/data.h>
#include "compiler.h"
#include "string_view.h"

/**
 * Game_Switches class
 */
class Game_Switches {
public:
	using Switches_t = std::vector<bool>;
	static constexpr int kMaxWarnings = 10;

	Game_Switches() = default;

	void SetData(Switches_t s);
	const Switches_t& GetData() const;

	void SetLowerLimit(size_t limit);

	bool Get(int switch_id) const;
	int GetInt(int switch_id) const;

	bool Set(int switch_id, bool value);
	void SetRange(int first_id, int last_id, bool value);

	bool Flip(int switch_id);
	void FlipRange(int first_id, int last_id);

	StringView GetName(int switch_id) const;

	bool IsValid(int switch_id) const;

	int GetSize() const;
	int GetSizeWithLimit() const;

	void SetWarning(int w);

private:
	bool ShouldWarn(int first_id, int last_id) const;
	void WarnGet(int variable_id) const;

	Switches_t _switches;
	size_t lower_limit = 0;
	mutable int _warnings = kMaxWarnings;
};


inline void Game_Switches::SetData(Switches_t s) {
	_switches = std::move(s);
}

inline const Game_Switches::Switches_t& Game_Switches::GetData() const {
	return _switches;
}

inline void Game_Switches::SetLowerLimit(size_t limit) {
	lower_limit = limit;
}

inline int Game_Switches::GetSize() const {
	return static_cast<int>(_switches.size());
}

inline int Game_Switches::GetSizeWithLimit() const {
	return std::max<int>(lower_limit, _switches.size());
}

inline bool Game_Switches::IsValid(int variable_id) const {
	return variable_id > 0 && variable_id <= GetSizeWithLimit();
}

inline bool Game_Switches::ShouldWarn(int first_id, int last_id) const {
	return (first_id <= 0 || last_id > GetSizeWithLimit()) && (_warnings > 0);
}

inline bool Game_Switches::Get(int switch_id) const {
	if (EP_UNLIKELY(ShouldWarn(switch_id, switch_id))) {
		WarnGet(switch_id);
	}
	if (switch_id <= 0 || switch_id > static_cast<int>(_switches.size())) {
		return false;
	}
	return _switches[switch_id - 1];
}

inline int Game_Switches::GetInt(int switch_id) const {
	return Get(switch_id) ? 1 : 0;
}

inline void Game_Switches::SetWarning(int w) {
	_warnings = w;
}

#endif
