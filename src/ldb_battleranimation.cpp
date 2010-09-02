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
/// Read BattlerAnimation
////////////////////////////////////////////////////////////
RPG::BattlerAnimation LDB_Reader::ReadBattlerAnimation(Reader& stream) {
	RPG::BattlerAnimation battler_animation;
	battler_animation.ID = stream.Read32(Reader::CompressedInteger);

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
		case ChunkBattlerAnimation::name:
			battler_animation.name = stream.ReadString(chunk_info.length);
			break;
		case ChunkBattlerAnimation::speed:
			battler_animation.speed = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkBattlerAnimation::base_data:
			for (int i = stream.Read32(Reader::CompressedInteger); i > 0; i--) {
				battler_animation.base_data.push_back(ReadBattlerAnimationExtension(stream));
			}
			break;
		case ChunkBattlerAnimation::weapon_data:
			for (int i = stream.Read32(Reader::CompressedInteger); i > 0; i--) {
				battler_animation.weapon_data.push_back(ReadBattlerAnimationExtension(stream));
			}
			break;
		default:
			stream.Seek(chunk_info.length, Reader::FromCurrent);
		}
	}
	return battler_animation;
}
