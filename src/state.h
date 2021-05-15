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

#ifndef EP_STATE_H
#define EP_STATE_H
#include <vector>
#include <cstdint>
#include <lcf/rpg/state.h>

/** A vector of state conditions.
 * The index is the state_id - 1, and the value is the number of
 * turns a battler has had the state.
 */
using StateVec = std::vector<int16_t>;

class PermanentStates {
	public:
		void Add(int state_id);
		void Remove(int state_id);
		void Clear();
		bool Has(int state_id) const;
	private:
		std::vector<bool> states;
};

namespace State {


/**
 * @param state_id database state ID.
 * @param vector of states
 * @return whether states has the state.
 */
bool Has(int state_id, const StateVec& states);

/**
 * Adds a state to states if possible
 *
 * @param state_id database state ID.
 * @param vector of states
 * @param ps permanent states that can never be removed
 * @param allow_battle_states allow adding of battle only states
 * @return whether state was added.
 */
bool Add(int state_id, StateVec& states, const PermanentStates& ps, bool allow_battle_states);

/**
 * Removes a state from states if possible
 *
 * @param state_id database state ID.
 * @param vector of states
 * @param ps permanent states that can never be removed
 * @return whether state was removed.
 */
bool Remove(int state_id, StateVec& states, const PermanentStates& ps);

/**
 * Removes all states which end after battle.
 *
 * @param vector of states
 * @param ps permanent states that can never be removed
 * @return whether any state was removed.
 */
bool RemoveAllBattle(StateVec& states, const PermanentStates& ps);

/**
 * Removes all states.
 *
 * @param vector of states
 * @param ps permanent states that can never be removed
 * @return wheter any state was removed.
 */
bool RemoveAll(StateVec& states, const PermanentStates& ps);

/**
 * Checks all states and returns the highest priority restriction that different to
 * normal or normal if that is the only restriction.
 *
 * @return Highest priority non-normal restriction or normal if not restricted
 */
lcf::rpg::State::Restriction GetSignificantRestriction(const StateVec& states);

/**
 * Gets current battler state with highest priority.
 *
 * @return the highest priority state affecting the battler.
 *         Returns nullptr if no states.
 */
const lcf::rpg::State* GetSignificantState(const StateVec& states);

/**
 * Gets the state probability by rate (A-E).
 *
 * @param state_id State to test
 * @param rate State rate to get
 * @return state rate (probability)
 */
int GetStateRate(int state_id, int rate);

/**
 * Return vector with state objects.
 *
 * @param vector of states
 * @return vector which contains the given states as state objects
 */
const std::vector<lcf::rpg::State*> GetObjects(const StateVec& states);

/**
 * Return vector with state objects which are sorted by priority.
 *
 * @param vector of state objects
 * @return vector which contains the given state objects sorted by priority
 */
const std::vector<lcf::rpg::State*> SortedByPriority(const std::vector<lcf::rpg::State*>& states);

} //namespace State

inline bool State::Has(int state_id, const StateVec& states) {
	return static_cast<int>(states.size()) >= state_id && states[state_id - 1] > 0;
}


inline void PermanentStates::Add(int state_id) {
	if (static_cast<int>(states.size()) < state_id) {
		states.resize(state_id);
	}
	states[state_id - 1] = true;
}

inline void PermanentStates::Remove(int state_id) {
	if (static_cast<int>(states.size()) >= state_id) {
		states[state_id - 1] = 0;
	}
}

inline void PermanentStates::Clear() {
	states.clear();
}

inline bool PermanentStates::Has(int state_id) const {
	return static_cast<int>(states.size()) >= state_id && states[state_id - 1];
}

#endif
