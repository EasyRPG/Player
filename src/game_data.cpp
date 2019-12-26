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

#include "game_data.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_party.h"
#include "player.h"

Game_Data::Data Game_Data::data = {};

void Game_Data::Reset() {
	data = {};
}

void Game_Data::SetupNewGame() {
	Reset();

	data.game_switches = std::make_unique<Game_Switches>();

	auto min_var = Player::IsRPG2k3() ? Game_Variables::min_2k3 : Game_Variables::min_2k;
	auto max_var = Player::IsRPG2k3() ? Game_Variables::max_2k3 : Game_Variables::max_2k;
	data.game_variables = std::make_unique<Game_Variables>(min_var, max_var);

	data.game_party = std::make_unique<Game_Party>();
}

void Game_Data::SetupLoadGame(RPG::Save save) {
	SetupNewGame();

	data.game_switches->SetData(std::move(save.system.switches));
	data.game_variables->SetData(std::move(save.system.variables));
	data.game_party->SetupFromSave(std::move(save.inventory));
}

void Game_Data::WriteSaveGame(RPG::Save& save) {
	save.inventory = GetParty().GetSaveData();
	save.system.switches = GetSwitches().GetData();
	save.system.variables = GetVariables().GetData();
}
