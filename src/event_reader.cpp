//////////////////////////////////////////////////////////////////////////////////
/// This file is part of EasyRPG Player.
/// 
/// EasyRPG Player is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// 
/// EasyRPG Player is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with EasyRPG Player.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include "event_reader.h"
#include "reader.h"

////////////////////////////////////////////////////////////
/// Read EventCommand
////////////////////////////////////////////////////////////
RPG::EventCommand Event_Reader::ReadEventCommand(FILE* stream) {
    RPG::EventCommand event_command;
    event_command.code = Reader::CInteger(stream);
    if (event_command.code != 0) {
        event_command.indent = Reader::CInteger(stream);
        event_command.string = Reader::String(stream, Reader::CInteger(stream));
        for (int i = Reader::CInteger(stream); i > 0; i--) {
            event_command.parameters.push_back(Reader::CInteger(stream));
        }
    }
    return event_command;
}
