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

#ifndef EP_DYNRPG_KEYBOARD_MOUSE_INPUT_H
#define EP_DYNRPG_KEYBOARD_MOUSE_INPUT_H

// Headers
#include <array>
#include <utility>
#include <vector>
#include "game_dynrpg.h"

namespace DynRpg {

class KeyboardMouseInput final : public DynRpgPlugin {
public:
	KeyboardMouseInput(Game_DynRpg& instance);
	~KeyboardMouseInput() override = default;

	bool Invoke(std::string_view func, dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter) override;
	void Update() override;

private:
	void LoadConfig();
	bool IsVirtualKeyPressed(int key_id) const;
	void RebuildUniqueSwitches();

	bool CheckKey(dyn_arg_list args);
	bool CheckAllKeys(dyn_arg_list args);
	bool SetKeySwitch(dyn_arg_list args);
	bool SetMouseVar(dyn_arg_list args);

	std::array<int, 256> key_switches = {};
	std::vector<std::pair<int, int>> active_bindings;
	std::vector<int> unique_switches;
	int mouse_x_var = 0;
	int mouse_y_var = 0;
};

} // namespace DynRpg

#endif
