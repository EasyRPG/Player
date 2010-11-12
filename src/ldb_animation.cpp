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
/// Read Animation
////////////////////////////////////////////////////////////
RPG::Animation LDB_Reader::ReadAnimation(Reader& stream) {
	RPG::Animation animation;
	animation.ID = stream.Read32(Reader::CompressedInteger);

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
		case ChunkAnimation::name:
			animation.name = stream.ReadString(chunk_info.length);
			break;
		case ChunkAnimation::animation_name:
			animation.animation_name = stream.ReadString(chunk_info.length);
			break;
		case ChunkAnimation::timings:
			for (int i = stream.Read32(Reader::CompressedInteger); i > 0; i--) {
				animation.timings.push_back(ReadAnimationTiming(stream));
			}
			break;
		case ChunkAnimation::scope:
			animation.scope = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkAnimation::position:
			animation.position = stream.Read32(Reader::CompressedInteger);
			break;
		case ChunkAnimation::frames:
			for (int i = stream.Read32(Reader::CompressedInteger); i > 0; i--) {
				animation.frames.push_back(ReadAnimationFrame(stream));
			}
			break;
		default:
			stream.Seek(chunk_info.length, Reader::FromCurrent);
		}
	}
	return animation;
}
