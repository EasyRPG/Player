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
#include "event_reader.h"
#include "reader.h"

////////////////////////////////////////////////////////////
/// Read EventCommand
////////////////////////////////////////////////////////////
RPG::EventCommand Event_Reader::ReadEventCommand(Reader& stream) {
	RPG::EventCommand event_command;
	event_command.code = stream.Read32(Reader::CompressedInteger);
	if (event_command.code != 0) {
		event_command.indent = stream.Read32(Reader::CompressedInteger);
		event_command.string = stream.ReadString(stream.Read32(Reader::CompressedInteger));
		for (int i = stream.Read32(Reader::CompressedInteger); i > 0; i--) {
			event_command.parameters.push_back(stream.Read32(Reader::CompressedInteger));
		}
	}
	return event_command;
}
