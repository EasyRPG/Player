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

#ifndef _GAME_TARGETS_H_
#define _GAME_TARGETS_H_

namespace RPG {
	class SaveTarget;
}

namespace Game_Targets {
	void AddTeleportTarget(int map_id, int x, int y, int switch_id);
	void RemoveTeleportTarget(int map_id);
	bool HasTeleportTarget();
	RPG::SaveTarget* GetTeleportTarget(int map_id);
	std::vector<RPG::SaveTarget*> GetTeleportTargets();
	void SetEscapeTarget(int map_id, int x, int y, int switch_id);
	bool HasEscapeTarget();
	RPG::SaveTarget* GetEscapeTarget();
}

#endif
