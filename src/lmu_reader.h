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

#ifndef _LMU_READER_H_
#define _LMU_READER_H_

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <iostream>
#include "main_data.h"

////////////////////////////////////////////////////////////
/// LMU Reader namespace
////////////////////////////////////////////////////////////
namespace LMU_Reader {
    RPG::Map LoadMap(const std::string &filename);
    RPG::Map ReadMapData(FILE* stream);
    RPG::Event ReadEvent(FILE* stream);
    RPG::EventPage ReadEventPage(FILE* stream);
    RPG::EventPageCondition ReadEventPageCondition(FILE* stream);
    RPG::MoveRoute ReadMoveRoute(FILE* stream);
    RPG::MoveCommand ReadMoveCommand(FILE* stream);
}

#endif
