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

#include <algorithm>
#include <list>
#include "game_party_base.h"
#include "utils.h"

Game_Party_Base::~Game_Party_Base() {
}

void Game_Party_Base::GetBattlers(std::vector<Game_Battler*>& out) {
	int count = GetBattlerCount();
	for (int i = 0; i < count; ++i) {
		Game_Battler* battler = &(*this)[i];
		out.push_back(battler);
	}
}

void Game_Party_Base::GetActiveBattlers(std::vector<Game_Battler*>& out) {
	int count = GetBattlerCount();
	for (int i = 0; i < count; ++i) {
		Game_Battler* battler = &(*this)[i];
		if (battler->Exists()) {
			out.push_back(battler);
		}
	}
}

void Game_Party_Base::GetDeadBattlers(std::vector<Game_Battler*>& out) {
	int count = GetBattlerCount();
	for (int i = 0; i < count; ++i) {
		Game_Battler* battler = &(*this)[i];
		if (battler->IsDead()) {
			out.push_back(battler);
		}
	}
}

Game_Battler* Game_Party_Base::GetNextActiveBattler(Game_Battler* battler) {
	std::vector<Game_Battler*> battlers;
	GetBattlers(battlers);

	std::vector<Game_Battler*>::const_iterator it;
	it = std::find(battlers.begin(), battlers.end(), battler);

	if (it == battlers.end()) {
		return NULL;
	}

	for (++it; it != battlers.end(); ++it) {
		Game_Battler* b = *it;
		if (b->Exists()) {
			return b;
		}
	}

	// None found after battler, try from the beginning now
	for (it = battlers.begin(); *it != battler; ++it) {
		Game_Battler* b = *it;
		if (b->Exists()) {
			return b;
		}
	}

	return NULL;
}

Game_Battler* Game_Party_Base::GetRandomActiveBattler() {
	std::vector<Game_Battler*> battlers;
	GetActiveBattlers(battlers);
	if (battlers.empty()) {
		return NULL;
	}
	return battlers[Utils::GetRandomNumber(0, battlers.size() - 1)];
}

Game_Battler* Game_Party_Base::GetRandomDeadBattler() {
	std::vector<Game_Battler*> battlers;
	GetDeadBattlers(battlers);
	if (battlers.empty()) {
		return NULL;
	}

	return battlers[Utils::GetRandomNumber(0, battlers.size() - 1)];
}

bool Game_Party_Base::IsAnyActive() {
	return GetRandomActiveBattler() != NULL;
}

int Game_Party_Base::GetAverageAgility() {
	std::vector<Game_Battler*> battlers;
	int agi = 0;

	GetBattlers(battlers);

	std::vector<Game_Battler*>::const_iterator it;

	for (it = battlers.begin(); it != battlers.end(); ++it) {
		agi += (*it)->GetAgi();
	}

	return agi /= battlers.size();
}

bool Game_Party_Base::IsAnyControllable() {
	std::vector<Game_Battler*> battlers;
	GetBattlers(battlers);

	std::vector<Game_Battler*>::const_iterator it;

	for (it = battlers.begin(); it != battlers.end(); ++it) {
		if ((*it)->GetSignificantRestriction() == RPG::State::Restriction_normal) {
			return true;
		}
	}

	return false;
}

void Game_Party_Base::ResetBattle() {
	std::vector<Game_Battler*> battlers;
	GetBattlers(battlers);

	std::vector<Game_Battler*>::const_iterator it;

	for (it = battlers.begin(); it != battlers.end(); ++it) {
		(*it)->ResetBattle();
	}
}
