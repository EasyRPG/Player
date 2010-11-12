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
#include "ldb_reader.h"
#include "ldb_chunks.h"
#include "reader.h"

////////////////////////////////////////////////////////////
/// Read System
////////////////////////////////////////////////////////////
RPG::System LDB_Reader::ReadSystem(Reader& stream) {
	RPG::System system;

	Reader::Chunk chunk_info;
	while (!stream.Eof()) {
		chunk_info.ID = stream.Read32(Reader::CompressedInteger);
		if (chunk_info.ID == ChunkData::END) {
			break;
		} else {
			chunk_info.length = stream.Read32(Reader::CompressedInteger);
			if (chunk_info.length == 0) continue;
		}
		switch (chunk_info.ID) {
		case ChunkSystem::ldb_id:
			system.ldb_id = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::boat_name:
			system.boat_name = stream.ReadString(chunk_info.length);
			break;
		case ChunkSystem::ship_name:
			system.ship_name = stream.ReadString(chunk_info.length);
			break;
		case ChunkSystem::airship_name:
			system.airship_name = stream.ReadString(chunk_info.length);
			break;
		case ChunkSystem::boat_index:
			system.boat_index = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::ship_index:
			system.ship_index = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::airship_index:
			system.airship_index = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::title_name:
			system.title_name = stream.ReadString(chunk_info.length);
			break;
		case ChunkSystem::gameover_name:
			system.gameover_name = stream.ReadString(chunk_info.length);
			break;
		case ChunkSystem::system_name:
			system.system_name = stream.ReadString(chunk_info.length);
			break;
		case ChunkSystem::system2_name:
			system.system2_name = stream.ReadString(chunk_info.length);
			break;
		case ChunkSystem::party_size:
			stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::party:
			stream.Read16(system.party, chunk_info.length);
			break;
		case ChunkSystem::menu_commands_size:
			stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::menu_commands:
			stream.Read16(system.menu_commands, chunk_info.length);
			break;
		case ChunkSystem::title_music:
			system.title_music = ReadMusic(stream);
			break;
		case ChunkSystem::battle_music:
			system.battle_music = ReadMusic(stream);
			break;
		case ChunkSystem::battle_end_music:
			system.battle_end_music = ReadMusic(stream);
			break;
		case ChunkSystem::inn_music:
			system.inn_music = ReadMusic(stream);
			break;
		case ChunkSystem::boat_music:
			system.boat_music = ReadMusic(stream);
			break;
		case ChunkSystem::ship_music:
			system.ship_music = ReadMusic(stream);
			break;
		case ChunkSystem::airship_music:
			system.airship_music = ReadMusic(stream);
			break;
		case ChunkSystem::gameover_music:
			system.gameover_music = ReadMusic(stream);
			break;
		case ChunkSystem::cursor_se:
			system.cursor_se = ReadSound(stream);
			break;
		case ChunkSystem::decision_se:
			system.decision_se = ReadSound(stream);
			break;
		case ChunkSystem::cancel_se:
			system.cancel_se = ReadSound(stream);
			break;
		case ChunkSystem::buzzer_se:
			system.buzzer_se = ReadSound(stream);
			break;
		case ChunkSystem::battle_se:
			system.battle_se = ReadSound(stream);
			break;
		case ChunkSystem::escape_se:
			system.escape_se = ReadSound(stream);
			break;
		case ChunkSystem::enemy_attack_se:
			system.enemy_attack_se = ReadSound(stream);
			break;
		case ChunkSystem::enemy_damaged_se:
			system.enemy_damaged_se = ReadSound(stream);
			break;
		case ChunkSystem::actor_damaged_se:
			system.actor_damaged_se = ReadSound(stream);
			break;
		case ChunkSystem::dodge_se:
			system.dodge_se = ReadSound(stream);
			break;
		case ChunkSystem::enemy_death_se:
			system.enemy_death_se = ReadSound(stream);
			break;
		case ChunkSystem::item_se:
			system.item_se = ReadSound(stream);
			break;
		case ChunkSystem::transition_out:
			system.transition_out = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::transition_in:
			system.transition_in = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::battle_start_fadeout:
			system.battle_start_fadeout = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::battle_start_fadein:
			system.battle_start_fadein = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::battle_end_fadeout:
			system.battle_end_fadeout = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::battle_end_fadein:
			system.battle_end_fadein = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::message_stretch:
			system.message_stretch = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::font_id:
			system.font_id = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::selected_condition:
			system.selected_condition = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::selected_hero:
			system.selected_hero = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::battletest_background:
			system.battletest_background = stream.ReadString(chunk_info.length);
			break;
		case ChunkSystem::battletest_data:
			for (int i = stream.Read32(Reader::CompressedInteger); i > 0; i--) {
				system.battletest_data.push_back(ReadTestBattler(stream));
			}
			break;
		case ChunkSystem::saved_times:
			system.saved_times = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkSystem::show_frame:
			system.show_frame = stream.ReadBool();
			break;
		case ChunkSystem::invert_animations:
			system.invert_animations = stream.ReadBool();
			break;
		default:
			stream.Seek(chunk_info.length, Reader::FromCurrent);
		}
	}
	return system;
}
