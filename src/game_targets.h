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
#include <lcf/rpg/savetarget.h>

class Game_Targets {
	public:
		Game_Targets() = default;

		void SetSaveData(std::vector<lcf::rpg::SaveTarget> save);
		std::vector<lcf::rpg::SaveTarget> GetSaveData() const;

		void AddTeleportTarget(int map_id, int x, int y, bool switch_on, int switch_id);
		void RemoveTeleportTarget(int map_id);
		bool HasTeleportTargets() const;
		const lcf::rpg::SaveTarget* GetTeleportTarget(int map_id) const;
		const std::vector<lcf::rpg::SaveTarget>& GetTeleportTargets() const;

		void SetEscapeTarget(int map_id, int x, int y, bool switch_on, int switch_id);
		bool HasEscapeTarget() const;
		const lcf::rpg::SaveTarget& GetEscapeTarget() const;
	private:
		lcf::rpg::SaveTarget escape;
		std::vector<lcf::rpg::SaveTarget> teleports;
};


inline bool Game_Targets::HasTeleportTargets() const {
	return !teleports.empty();
}

inline const std::vector<lcf::rpg::SaveTarget>& Game_Targets::GetTeleportTargets() const {
	return teleports;
}

inline bool Game_Targets::HasEscapeTarget() const {
	return escape.map_id != 0;
}

inline const lcf::rpg::SaveTarget& Game_Targets::GetEscapeTarget() const {
	return escape;
}


#endif
