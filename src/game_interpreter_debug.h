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


#ifndef EP_GAME_INTERPRETER_DEBUG
#define EP_GAME_INTERPRETER_DEBUG

#include "game_interpreter_shared.h"
#include "game_character.h"
#include <lcf/rpg/saveeventexecstate.h>
#include "player.h"

class Game_CommonEvent;

namespace Debug {
	class ParallelInterpreterStates {
	private:
		std::vector<int> ev_ids;
		std::vector<int> ce_ids;

		std::vector<lcf::rpg::SaveEventExecState> state_ev;
		std::vector<lcf::rpg::SaveEventExecState> state_ce;

		ParallelInterpreterStates(std::vector<int> ev_ids, std::vector<int> ce_ids,
			std::vector<lcf::rpg::SaveEventExecState> state_ev, std::vector<lcf::rpg::SaveEventExecState> state_ce)
		: ev_ids(ev_ids), ce_ids(ce_ids), state_ev(state_ev), state_ce(state_ce) { }
	public:
		ParallelInterpreterStates() = default;

		inline int CountEventInterpreters() const { return ev_ids.size(); }
		inline int CountCommonEventInterpreters() const { return ce_ids.size(); }

		inline int Count() const { return ev_ids.size() + ce_ids.size(); }

		inline std::tuple<const int&, const lcf::rpg::SaveEventExecState&> GetEventInterpreter(int i) const {
			return std::tie(ev_ids[i], state_ev[i]);
		}
		inline std::tuple<const int&, const lcf::rpg::SaveEventExecState&> GetCommonEventInterpreter(int i) const {
			return std::tie(ce_ids[i], state_ce[i]);
		}

		static ParallelInterpreterStates GetCachedStates();
	};

	struct CallStackItem {
		bool is_ce;
		int evt_id, page_id;
		std::string name;
		int stack_item_no, cmd_current, cmd_count;
	};

	std::vector<CallStackItem> CreateCallStack(const int owner_evt_id, const lcf::rpg::SaveEventExecState& state);

	std::string FormatEventName(Game_Character const& ev);

	std::string FormatEventName(Game_CommonEvent const& ce);
}

#endif
