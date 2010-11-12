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
#include "rpg_terrain.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
RPG::Terrain::Terrain() {
	ID = 0;
	name = "";
	damage = 0;
	encounter_rate = 100;
	background_name = "";
	boat_pass = false;
	ship_pass = false;
	airship_pass = true;
	airship_land = true;
	bush_depth = 0;
	on_damage_se = false;
	background_type = 0;
	background_a_name = "";
	background_a_scrollh = false;
	background_a_scrollv = false;
	background_a_scrollh_speed = 0;
	background_a_scrollv_speed = 0;
	background_b = false;
	background_b_name = "";
	background_b_scrollh = false;
	background_b_scrollv = false;
	background_b_scrollh_speed = 0;
	background_b_scrollv_speed = 0;
	special_back_party_flag = false;
	special_back_enemies_flag = false;
	special_lateral_party_flag = false;
	special_lateral_enemies_flag = false;
	special_back_party = 15;
	special_back_enemies = 10;
	special_lateral_party = 10;
	special_lateral_enemies = 5;
	grid_location = 0;
	grid_a = 0;
	grid_b = 0;
	grid_c = 0;
}
