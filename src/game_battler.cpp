/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "game_battler.h"
#include <algorithm>
#include "game_actor.h"
#include "util_macro.h"
#include "main_data.h"

////////////////////////////////////////////////////////////
Game_Battler::Game_Battler() :
	hp(0),
	sp(0),
	maxhp_plus(0),
	maxsp_plus(0),
	atk_plus(0),
	def_plus(0),
	spi_plus(0),
	agi_plus(0),
	hidden(false),
	immortal(false),
	damage_pop(false),
	damage(false),
	critical(false),
	animation_id(0),
	animation_hit(false),
	white_flash(false),
	blink(false),
	actor(NULL) {
}

////////////////////////////////////////////////////////////
void Game_Battler::SetGameActor(Game_Actor* _actor) {
	actor = _actor;
}

////////////////////////////////////////////////////////////
bool Game_Battler::HasState(int state_id) const {
	return (std::find(states.begin(), states.end(), state_id) != states.end());
}

////////////////////////////////////////////////////////////
std::vector<int> Game_Battler::GetStates() const {
	return states;
}

////////////////////////////////////////////////////////////
int Game_Battler::GetHp() const {
	return hp;
}

////////////////////////////////////////////////////////////
int Game_Battler::GetSp() const {
	return sp;
}

////////////////////////////////////////////////////////////
int Game_Battler::GetMaxHp() {
	if (actor == NULL) {
		return 0;
	} else {
		return actor->GetMaxHp();
	}
}

////////////////////////////////////////////////////////////
int Game_Battler::GetMaxSp() {
	if (actor == NULL) {
		return 0;
	} else {
		return actor->GetMaxSp();
	}
}

////////////////////////////////////////////////////////////
void Game_Battler::SetMaxHp(int _maxhp) {
	maxhp_plus += _maxhp - GetMaxHp(); 
	maxhp_plus = min(max(maxhp_plus, -999), 999);
	hp = min(GetMaxHp(), hp);
}

////////////////////////////////////////////////////////////
void Game_Battler::SetMaxSp(int _maxsp) {
	maxsp_plus += _maxsp - GetMaxSp(); 
	maxsp_plus = min(max(maxsp_plus, -999), 999);
	sp = min(GetMaxSp(), sp);
}

////////////////////////////////////////////////////////////
int Game_Battler::GetAtk() {
	int base_atk = actor->GetBaseAtk();
	int n = min(max(base_atk + atk_plus, 1), 999);

	for (std::vector<int>::iterator i = states.begin();
		i != states.end();
		i++) {
			// TODO 
			//n *= Data::states[(*i)]. / 100;
	}

	n = min(max(n, 1), 999);

	return n;
}

////////////////////////////////////////////////////////////
int Game_Battler::GetDef() {
	int base_def = actor->GetBaseDef();
	int n = min(max(base_def + def_plus, 1), 999);

	for (std::vector<int>::iterator i = states.begin();
		i != states.end();
		i++) {
			// TODO 
			//n *= Data::states[(*i)]. / 100;
	}

	n = min(max(n, 1), 999);

	return n;
}

////////////////////////////////////////////////////////////
int Game_Battler::GetSpi() {
	int base_spi = actor->GetBaseSpi();
	int n = min(max(base_spi + spi_plus, 1), 999);

	for (std::vector<int>::iterator i = states.begin();
		i != states.end();
		i++) {
			// TODO 
			//n *= Data::states[(*i)]. / 100;
	}

	n = min(max(n, 1), 999);

	return n;
}

////////////////////////////////////////////////////////////
int Game_Battler::GetAgi() {
	int base_agi = actor->GetBaseAgi();
	int n = min(max(base_agi + agi_plus, 1), 999);

	for (std::vector<int>::iterator i = states.begin();
		i != states.end();
		i++) {
			// TODO 
			//n *= Data::states[(*i)]. / 100;
	}

	n = min(max(n, 1), 999);

	return n;
}

////////////////////////////////////////////////////////////
void Game_Battler::SetAtk(int _atk) {
	atk_plus += _atk - GetAtk();
	atk_plus = min(max(atk_plus, 1), 999);
}

////////////////////////////////////////////////////////////
void Game_Battler::SetDef(int _def) {
	def_plus += _def - GetDef();
	def_plus = min(max(def_plus, 1), 999);
}

////////////////////////////////////////////////////////////
void Game_Battler::SetSpi(int _spi) {
	spi_plus += _spi - GetSpi();
	spi_plus = min(max(spi_plus, 1), 999);
}

////////////////////////////////////////////////////////////
void Game_Battler::SetAgi(int _agi) {
	agi_plus += _agi - GetAgi();
	agi_plus = min(max(agi_plus, 1), 999);
}