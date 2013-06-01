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

#include "game_battleaction.h"
#include "game_battler.h"
//#include "game_battlecommand.h"
#include "game_temp.h"
#include "main_data.h"
#include "game_message.h"
#include "game_actor.h"

Game_BattleAction::AttackSingle::AttackSingle(Game_Battler* source, Game_Battler* target) {
	this->source = source;
	this->target = target;
	this->animation = NULL;
}

bool Game_BattleAction::AttackSingle::Execute() {

	return true;
}
