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

#ifndef EP_DYNRPG_PARAMS_H
#define EP_DYNRPG_PARAMS_H

#include <array>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include "game_dynrpg.h"
#include "filesystem_stream.h"

namespace DynRpg {
	/**
	 * DynParams plugin: rewrites the next event command (behaviour of DynParams.dll).
	 *
	 *   @dynparams_add_param I, V                    parameter I (1-based) := V
	 *   @dynparams_add_param V                       parameter <index> := V, then index + 1
	 *   @dynparams_set_index I                       set that index (default 5)
	 *   @dynparams_append_string text                append the raw text to the string parameter
	 *   @dynparams_append_number V                   append V to the string parameter
	 *   @dynparams_message_line_append_string L, "s" append to message line L (1-4)
	 *   @dynparams_message_line_append_number L, V   append V to message line L
	 *   @dynparams_choice_case_append_string C, "s"  append to choice C (1 and up)
	 *   @dynparams_choice_case_append_number C, V    append V to choice C
	 *   @dynparams_overwrite_next                    apply all of the above to the next command
	 *   @dynparams_clear_params                      drop everything
	 *   @dynparams_list_params                       log everything (debugging)
	 *   @dynparams_start_record                      log executed commands to DynPlugins/DynParamsRecord.txt
	 *   @dynparams_stop_record                       stop logging
	 *
	 * The next command skips END lines and block closers. Applying resets
	 * everything, including the index.
	 *
	 * Like DynParams.dll (which keys it by RPG::EventScriptData), the state
	 * belongs to the running script: each frame of each interpreter has its own.
	 * The rewritten command is restored once executed (DynRPG does that), but
	 * the message lines and choices that follow it stay rewritten.
	 */
	class ParamsPlugin : public DynRpgPlugin {
	public:
		ParamsPlugin(Game_DynRpg& instance) : DynRpgPlugin("DynParams", instance) {}

		bool Invoke(std::string_view func, dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter) override;
		std::optional<lcf::rpg::EventCommand> OnEventCommand(const Game_Interpreter& interpreter, lcf::rpg::SaveEventExecFrame& frame) override;
		void OnFramesReset(const Game_Interpreter* interpreter, int first_frame) override;

	private:
		static constexpr int default_index = 5;
		static constexpr int message_lines = 4;

		/** Overwrite orders built by the comment commands of one script */
		struct Script {
			std::vector<std::pair<int, int>> params;
			std::string text;
			bool text_set = false;
			std::array<std::string, message_lines> lines;
			bool lines_set = false;
			std::map<int, std::string> choices;
			int index = default_index;
			bool overwrite = false;
		};

		/** Script of an interpreter's frame (nullptr: comments invoked without an interpreter) */
		using ScriptKey = std::pair<const Game_Interpreter*, int>;

		static ScriptKey KeyOf(const Game_Interpreter* interpreter);
		void Apply(Script& script, lcf::rpg::EventCommand& com, lcf::rpg::SaveEventExecFrame& frame);
		void List(const Script& script) const;
		void Record(const lcf::rpg::EventCommand& com);

		std::map<ScriptKey, Script> scripts;
		bool record = false;
		Filesystem_Stream::OutputStream record_file;
	};
}

#endif
