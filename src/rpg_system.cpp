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
#include "rpg_system.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
RPG::System::System() {
	ldb_id = 0;
	boat_name = "";
	ship_name = "";
	airship_name = "";
	boat_index = 0;
	ship_index = 0;
	airship_index = 0;
	title_name = "";
	gameover_name = "";
	system_name = "";
	system2_name = "";
	transition_out = 0;
	transition_in = 0;
	battle_start_fadeout = 0;
	battle_start_fadein = 0;
	battle_end_fadeout = 0;
	battle_end_fadein = 0;
	message_stretch = 0;
	font_id = 0;
	selected_condition = 0;
	selected_hero = 0;
	battletest_background = "";
	saved_times = 0;
	show_frame = false;
	invert_animations = false;
}
