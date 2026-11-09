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
 */

#ifndef EP_DYNRPG_PARTICLE_V2_H
#define EP_DYNRPG_PARTICLE_V2_H

#include <string_view>
#include <map>

#include "game_dynrpg.h"

class ParticleEffectV2;

namespace DynRpg {
	class ParticleV2 : public DynRpgPlugin {
	public:
		explicit ParticleV2(Game_DynRpg& instance);

		bool Invoke(std::string_view func, dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter) override;
		void Update() override;

	private:
		using EffectMap = std::map<std::string, std::unique_ptr<ParticleEffectV2>>;

		EffectMap pfx_list;
	};
}

#endif
