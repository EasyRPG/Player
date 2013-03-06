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

#include "game_unit.h"
#include "game_battler.h"

tBattlerArray Game_Unit::GetMembers() {
	return tBattlerArray();
}

void Game_Unit::GetExistingMembers(tBattlerArray& ret_val) {
	tBattlerArray members(GetMembers());
	tBattlerArray ex_members;
	tBattlerArray::iterator it;

	Game_Battler* battler;
	for (it = members.begin(); it != members.end(); ++it) {
		battler = *it;
		if ( battler->Exists() ) {
			ex_members.push_back(battler);
		}
	}

	ret_val.swap(ex_members);
}

void Game_Unit::GetDeadMembers(tBattlerArray& ret_val) {
	tBattlerArray members(GetMembers());
	tBattlerArray ex_members;
	tBattlerArray::iterator it;

	Game_Battler* battler;
	for (it = members.begin(); it != members.end(); ++it) {
		battler = *it;
		if ( battler->IsDead() ) {
			ex_members.push_back(battler);
		}
	}

	ret_val.swap(ex_members);
}
