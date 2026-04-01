/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 * Based on DynRPG Particle Effects V1 by Kazesui. (MIT license)
 */

#ifndef EP_DYNRPG_PARTICLE_V1_H_
#define EP_DYNRPG_PARTICLE_V1_H_

#include "game_dynrpg.h"

namespace DynRpg {
    class ParticleV1 : public DynRpgPlugin {
    public:
        ParticleV1(Game_DynRpg& instance);
        ~ParticleV1() override;

        bool Invoke(std::string_view func, dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter) override;
        void Update() override;
        void OnMapChange();
    };
}

#endif
