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

#ifndef EP_GAME_TARGETS_H
#define EP_GAME_TARGETS_H

#include <vector>
#include "rpg_savetarget.h"

class Game_Targets {
	public:
		Game_Targets() = default;

		void SetSaveData(std::vector<RPG::SaveTarget> save);
		const std::vector<RPG::SaveTarget>& GetSaveData();

		void AddTeleportTarget(int map_id, int x, int y, bool switch_on, int switch_id);
		void RemoveTeleportTarget(int map_id);
		bool HasTeleportTarget() const;
		RPG::SaveTarget* GetTeleportTarget(int map_id);
		std::vector<RPG::SaveTarget*> GetTeleportTargets();
		void SetEscapeTarget(int map_id, int x, int y, bool switch_on, int switch_id);
		bool HasEscapeTarget() const;
		RPG::SaveTarget* GetEscapeTarget();
	private:
		std::vector<RPG::SaveTarget>::iterator FindTarget(int id, bool create);

		std::vector<RPG::SaveTarget> data;
};

inline void Game_Targets::SetSaveData(std::vector<RPG::SaveTarget> save) {
	data = std::move(save);
}

inline const std::vector<RPG::SaveTarget>& Game_Targets::GetSaveData() {
	return data;
}


#endif
