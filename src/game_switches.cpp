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

// Headers
#include "game_switches.h"
#include "output.h"
#include <lcf/reader_util.h>
#include <lcf/data.h>

constexpr int Game_Switches::kMaxWarnings;

void Game_Switches::WarnGet(int variable_id) const {
	Output::Debug("Invalid read sw[{}]!", variable_id);
	--_warnings;
}

bool Game_Switches::Set(int switch_id, bool value) {
	if (EP_UNLIKELY(ShouldWarn(switch_id, switch_id))) {
		Output::Debug("Invalid write sw[{}] = {}!", switch_id, value);
		--_warnings;
	}
	if (switch_id <= 0) {
		return false;
	}
	auto& ss = _switches;
	if (switch_id > static_cast<int>(ss.size())) {
		ss.resize(switch_id);
	}
	ss[switch_id - 1] = value;
	return value;
}

void Game_Switches::SetRange(int first_id, int last_id, bool value) {
	if (EP_UNLIKELY(ShouldWarn(first_id, last_id))) {
		Output::Debug("Invalid write sw[{},{}] = {}!", first_id, last_id, value);
		--_warnings;
	}
	auto& ss = _switches;
	if (last_id > static_cast<int>(ss.size())) {
		ss.resize(last_id, false);
	}
	for (int i = std::max(0, first_id - 1); i < last_id; ++i) {
		ss[i] = value;
	}
}

bool Game_Switches::Flip(int switch_id) {
	if (EP_UNLIKELY(ShouldWarn(switch_id, switch_id))) {
		Output::Debug("Invalid flip sw[{}]!", switch_id);
		--_warnings;
	}
	if (switch_id <= 0) {
		return false;
	}
	auto& ss = _switches;
	if (switch_id > static_cast<int>(ss.size())) {
		ss.resize(switch_id);
	}
	ss[switch_id - 1].flip();
	return ss[switch_id - 1];
}

void Game_Switches::FlipRange(int first_id, int last_id) {
	if (EP_UNLIKELY(ShouldWarn(first_id, last_id))) {
		Output::Debug("Invalid flip sw[{},{}]!", first_id, last_id);
		--_warnings;
	}
	auto& ss = _switches;
	if (last_id > static_cast<int>(ss.size())) {
		ss.resize(last_id);
	}
	for (int i = std::max(0, first_id - 1); i < last_id; ++i) {
		ss[i].flip();
	}
}

StringView Game_Switches::GetName(int _id) const {
	const auto* sw = lcf::ReaderUtil::GetElement(lcf::Data::switches, _id);

	if (!sw) {
		// No warning, is valid because the switch array resizes dynamic during runtime
		return {};
	} else {
		return sw->name;
	}
}

