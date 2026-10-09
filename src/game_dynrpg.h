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

#ifndef EP_GAME_DYNRPG_H
#define EP_GAME_DYNRPG_H

#include <cstdint>
#include <locale>
#include <optional>
#include <vector>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include "output.h"
#include "utils.h"
#include <lcf/rpg/eventcommand.h>

// Headers
namespace lcf::rpg {
	class SaveEventExecFrame;
}

class DynRpgPlugin;
class Game_Interpreter;

using dyn_arg_list = const Span<std::string>;
using dynfunc = bool(*)(dyn_arg_list);

/** Contains helper functions for parsing */
namespace DynRpg {
	class EasyRpgPlugin;

	std::string ParseVarArg(std::string_view func_name, dyn_arg_list args, int index, bool& parse_okay);
	std::string ParseCommand(std::string command, std::vector<std::string>& params);

	namespace detail {
		template <typename T>
		inline bool parse_arg(std::string_view, dyn_arg_list, const int, T&, bool&) {
			static_assert(sizeof(T) == -1, "Only parsing int, float and std::string supported");
			return false;
		}

		// FIXME: Extracting floats that are followed by chars behaviour varies depending on the C++ library
		// see https://bugs.llvm.org/show_bug.cgi?id=17782
		template <>
		inline bool parse_arg(std::string_view func_name, dyn_arg_list args, const int i, float& value, bool& parse_okay) {
			if (!parse_okay) return false;
			value = 0.0;
			if (args[i].empty()) {
				parse_okay = true;
				return parse_okay;
			}
			std::istringstream iss(args[i]);
			iss.imbue(std::locale::classic());
			iss >> value;
			parse_okay = !iss.fail();
			if (!parse_okay) {
				Output::Warning("{}: Arg {} ({}) is not numeric", func_name, i, args[i]);
				parse_okay = false;
			}
			return parse_okay;
		}

		template <>
		inline bool parse_arg(std::string_view func_name, dyn_arg_list args, const int i, int& value, bool& parse_okay) {
			if (!parse_okay) return false;
			value = 0;
			if (args[i].empty()) {
				parse_okay = true;
				return parse_okay;
			}
			std::istringstream iss(args[i]);
			iss >> value;
			parse_okay = !iss.fail();
			if (!parse_okay) {
				Output::Warning("{}: Arg {} ({}) is not an integer", func_name, i, args[i]);
				parse_okay = false;
			}
			return parse_okay;
		}

		template <>
		inline bool parse_arg(std::string_view, dyn_arg_list args, const int i, std::string& value, bool& parse_okay) {
			if (!parse_okay) return false;
			value = args[i];
			parse_okay = true;
			return parse_okay;
		}

		template <typename Tuple, std::size_t... I>
		inline void parse_args(std::string_view func_name, dyn_arg_list in, Tuple& value, bool& parse_okay, std::index_sequence<I...>) {
			(void)std::initializer_list<bool>{parse_arg(func_name, in, I, std::get<I>(value), parse_okay)...};
		}
	}


	template <typename... Targs>
	std::tuple<Targs...> ParseArgs(std::string_view func_name, dyn_arg_list args, bool* parse_okay = nullptr) {
		std::tuple<Targs...> t;
		if (args.size() < sizeof...(Targs)) {
			if (parse_okay)
				*parse_okay = false;
			Output::Warning("{}: Got {} args (needs {} or more)", func_name, args.size(), sizeof...(Targs));
			return t;
		}
		bool okay = true;
		detail::parse_args(func_name, args, t, okay, std::make_index_sequence<sizeof...(Targs)>{});
		if (parse_okay)
			*parse_okay = okay;
		return t;
	}
}

/**
 * Implements DynRPG Patch (kinda, plugins cannot be executed directly and must be reimplemented)
 */
class Game_DynRpg {
public:
	bool Invoke(std::string_view command, Game_Interpreter* interpreter = nullptr);
	void Update();
	void Load(int slot);
	void Save(int slot);

	/**
	 * DynRPG onEventCommand: called before each event command is executed.
	 * Plugins (e.g. DynParams) may rewrite the command, like DynRPG plugins
	 * rewrite the script line in RPG_RT. The rewritten command replaces the
	 * current command of the frame.
	 *
	 * @param interpreter interpreter running the command
	 * @param frame its current frame, at the command about to be executed
	 * @return the command as it was before a plugin rewrote it, none when
	 *   unchanged. DynRPG restores it once the command was executed.
	 */
	std::optional<lcf::rpg::EventCommand> OnEventCommand(const Game_Interpreter& interpreter, lcf::rpg::SaveEventExecFrame& frame);

	/**
	 * The frames of an interpreter from first_frame on are replaced
	 * (a frame is pushed at first_frame, the stack is cleared or the
	 * interpreter destroyed): plugins drop the state of those scripts.
	 *
	 * @param interpreter the interpreter
	 * @param first_frame index of the first replaced frame
	 */
	void OnFramesReset(const Game_Interpreter* interpreter, int first_frame);

	/** @return raw text of the DynRPG comment being invoked (e.g. "@func 1, 2") */
	std::string_view GetCurrentComment() const { return current_comment; }

private:
	friend DynRpg::EasyRpgPlugin;

	std::string current_comment;

	bool Invoke(std::string_view func, dyn_arg_list args, Game_Interpreter* interpreter = nullptr);
	void InitPlugins();

	using dyn_rpg_func = std::unordered_map<std::string, dynfunc>;

	bool plugins_loaded = false;

	// Registered DynRpg Plugins
	std::vector<std::unique_ptr<DynRpgPlugin>> plugins;

	// DynRpg Function table
	dyn_rpg_func dyn_rpg_functions;
};

/** Base class for implementing a DynRpg Plugins */
class DynRpgPlugin {
public:
	explicit DynRpgPlugin(std::string identifier, Game_DynRpg& instance) : instance(instance), identifier(std::move(identifier)) {}
	DynRpgPlugin() = delete;
	virtual ~DynRpgPlugin() = default;

	std::string_view GetIdentifier() const { return identifier; }
	virtual bool Invoke(std::string_view func, dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter) = 0;
	virtual void Update() {}
	virtual void Load(const std::vector<uint8_t>&) {}
	virtual std::vector<uint8_t> Save() { return {}; }
	/**
	 * DynRPG onEventCommand, see Game_DynRpg::OnEventCommand
	 *
	 * @return the rewritten command, none to leave it unchanged
	 */
	virtual std::optional<lcf::rpg::EventCommand> OnEventCommand(const Game_Interpreter&, lcf::rpg::SaveEventExecFrame&) { return {}; }
	/** See Game_DynRpg::OnFramesReset */
	virtual void OnFramesReset(const Game_Interpreter*, int) {}

protected:
	Game_DynRpg& instance;

private:
	std::string identifier;
};

#endif
