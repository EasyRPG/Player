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
/// Read Class
////////////////////////////////////////////////////////////
RPG::Class LDB_Reader::ReadClass(Reader& stream) {
	RPG::Class _class;
	_class.ID = stream.Read32(Reader::CompressedInteger);

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
		case ChunkClass::name:
			_class.name = stream.ReadString(chunk_info.length);
			break;
		case ChunkClass::two_swords_style:
			_class.name = stream.ReadBool();
			break;
		case ChunkClass::fix_equipment:
			_class.name = stream.ReadBool();
			break;
		case ChunkClass::auto_battle:
			_class.name = stream.ReadBool();
			break;
		case ChunkClass::super_guard:
			_class.name = stream.ReadBool();
			break;
		case ChunkClass::parameters:
			stream.Read16(_class.parameter_maxhp, chunk_info.length / 6);
			stream.Read16(_class.parameter_maxsp, chunk_info.length / 6);
			stream.Read16(_class.parameter_attack, chunk_info.length / 6);
			stream.Read16(_class.parameter_defense, chunk_info.length / 6);
			stream.Read16(_class.parameter_spirit, chunk_info.length / 6);
			stream.Read16(_class.parameter_agility, chunk_info.length / 6);
			break;
		case ChunkClass::exp_base:
			_class.exp_base = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkClass::exp_inflation:
			_class.exp_inflation = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkClass::exp_correction:
			_class.exp_correction = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkClass::unarmed_animation:
			_class.unarmed_animation = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkClass::skills:
			for (int i = stream.Read32(Reader::CompressedInteger); i > 0; i--) {
				_class.skills.push_back(ReadLearning(stream));
			}
			break;
		case ChunkClass::state_ranks_size:
			stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkClass::state_ranks:
			stream.Read8(_class.state_ranks, chunk_info.length);
			break;
		case ChunkClass::attribute_ranks_size:
			stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkClass::attribute_ranks:
			stream.Read8(_class.attribute_ranks, chunk_info.length);
			break;
		case ChunkClass::battle_commands:
			stream.Read32(_class.battle_commands, chunk_info.length);
			break;
		default:
			stream.Seek(chunk_info.length, Reader::FromCurrent);
		}
	}
	return _class;
}
