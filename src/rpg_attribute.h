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

#ifndef _RPG_ATTRIBUTE_H_
#define _RPG_ATTRIBUTE_H_

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include <string>

////////////////////////////////////////////////////////////
/// RPG::Attribute class
////////////////////////////////////////////////////////////
namespace RPG {
    class Attribute {
    public:
        enum Type {
            Type_physical = 0,
            Type_magical = 1
        };

        Attribute();
        
        int ID;
        std::string name;
        int type;
        int a_rate;
        int b_rate;
        int c_rate;
        int d_rate;
        int e_rate;
    };
}

#endif
