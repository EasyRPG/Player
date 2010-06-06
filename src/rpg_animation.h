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

#ifndef _RPG_ANIMATION_H_
#define _RPG_ANIMATION_H_

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include "rpg_animationtiming.h"
#include "rpg_animationframe.h"

////////////////////////////////////////////////////////////
/// RPG::Animation class
////////////////////////////////////////////////////////////
namespace RPG {
    class Animation {
    public:
        enum Scope {
            Scope_target = 0,
            Scope_screen = 1
        };
        enum Position {
            Position_up = 0,
            Position_middle = 1,
            Position_down = 2
        };

        Animation();
        
        int ID;
        std::string name;
        std::string animation_name;
        std::vector<RPG::AnimationTiming> timings;
        int scope;
        int position;
        std::vector<RPG::AnimationFrame> frames;
    };
}

#endif
