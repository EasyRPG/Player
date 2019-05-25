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

#include "state.h"
#include "reader_util.h"
#include "data.h"
#include "output.h"
#include <cassert>

namespace State {

bool Add(int state_id, StateVec& states, const PermanentStates& ps, bool allow_battle_states) {
	const RPG::State* state = ReaderUtil::GetElement(Data::states, state_id);
	if (!state) {
		Output::Warning("State::Add: Can't add state with invalid ID %d", state_id);
		return false;
	}

	if (Has(RPG::State::kDeathID, states)) {
		return false;
	}

	if (!allow_battle_states && state->type == RPG::State::Persistence_ends) {
		return false;
	}

	if (state_id == RPG::State::kDeathID) {
		RemoveAll(states, ps);
	}

	if (state_id > static_cast<int>(states.size())) {
		states.resize(state_id);
	}

	states[state_id - 1] = 1;

	// Clear states that are more than 10 priority points below the
	// significant state
	const RPG::State* sig_state = GetSignificantState(states);

	for (int i = 0; i < (int)states.size(); ++i) {
		if (Data::states[i].priority <= sig_state->priority - 10 && !ps.Has(i + 1)) {
			states[i] = 0;
		}
	}

	return states[state_id - 1] != 0;
}

bool Remove(int state_id, StateVec& states, const PermanentStates& ps) {
	const RPG::State* state = ReaderUtil::GetElement(Data::states, state_id);
	if (!state) {
		Output::Warning("State::Remove: Can't remove state with invalid ID %d", state_id);
		return false;
	}

	if (state_id - 1 >= static_cast<int>(states.size())) {
		return false;
	}

	auto& st = states[state_id - 1];

	if (!st) {
		return false;
	}

	if (ps.Has(state_id)) {
		return false;
	}

	st = 0;
	return true;
}

void RemoveAllBattle(StateVec& states) {
	for (int i = 0; i < (int)states.size(); ++i) {
		auto state_id = i + 1;
		auto* state = ReaderUtil::GetElement(Data::states, state_id);
		if (state == nullptr) {
			Output::Warning("State::RemoveAllBattle: Can't remove state with invalid ID %d", state_id);
			continue;
		}
		if (state->type != RPG::State::Persistence_ends) {
			continue;
		}
		if (state->auto_release_prob > 0) {
			Remove(state_id, states, {});
		}
	}
}

void RemoveAll(StateVec& states, const PermanentStates& ps) {
	for (int i = 0; i < (int)states.size(); ++i) {
		auto state_id = i + 1;
		if (Has(state_id, states)) {
			Remove(state_id, states, ps);
		}
	}
}

RPG::State::Restriction GetSignificantRestriction(const StateVec& states) {
	// Priority is nomove > attack enemy > attack ally > normal

	RPG::State::Restriction sig_res = RPG::State::Restriction_normal;

	for (int i = 0; i < (int)states.size(); ++i) {
		auto state_id = i + 1;

		if (!Has(state_id, states)) {
			continue;
		}

		auto* state = ReaderUtil::GetElement(Data::states, state_id);
		if (state == nullptr) {
			Output::Warning("State::GetSignificantRestriction: Can't remove state with invalid ID %d", state_id);
			continue;
		}

		switch (state->restriction) {
			case RPG::State::Restriction_normal:
				break;
			case RPG::State::Restriction_do_nothing:
				return RPG::State::Restriction_do_nothing;
			case RPG::State::Restriction::Restriction_attack_enemy:
				if (sig_res == RPG::State::Restriction::Restriction_attack_ally
						|| sig_res == RPG::State::Restriction_normal) {
					sig_res = RPG::State::Restriction_attack_enemy;
				}
				break;
			case RPG::State::Restriction::Restriction_attack_ally:
				if (sig_res == RPG::State::Restriction_normal) {
					sig_res = RPG::State::Restriction_attack_ally;
				}
				break;
		}
	}

	return sig_res;

}
const RPG::State* GetSignificantState(const StateVec& states) {
	int priority = 0;
	const RPG::State* sig_state = NULL;

	for (int i = 0; i < (int)states.size(); ++i) {
		auto state_id = i + 1;

		if (!Has(state_id, states)) {
			continue;
		}

		auto* state = ReaderUtil::GetElement(Data::states, state_id);
		if (state == nullptr) {
			Output::Warning("State::GetSignificantState: Can't remove state with invalid ID %d", state_id);
			continue;
		}

		// Death has highest priority
		if (state->ID == RPG::State::kDeathID) {
			return state;
		}

		if (state->priority >= priority) {
			sig_state = state;
			priority = state->priority;
		}
	};

	return sig_state;

}

int GetStateRate(int state_id, int rate) {
	const RPG::State* state = ReaderUtil::GetElement(Data::states, state_id);

	if (!state) {
		Output::Warning("State::GetStateRate: Invalid state ID %d", state_id);
		return 0;
	}

	switch (rate) {
	case 0:
		return state->a_rate;
	case 1:
		return state->b_rate;
	case 2:
		return state->c_rate;
	case 3:
		return state->d_rate;
	case 4:
		return state->e_rate;
	default:;
	}

	assert(false && "bad rate");
	return 0;

}

} //namspace State
