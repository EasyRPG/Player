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
#include <vector>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include "output.h"
#include "utils.h"

// Headers
namespace lcf::rpg {
	class EventCommand;
}

class DynRpgPlugin;
class Game_Interpreter;

using dyn_arg_list = const Span<std::string>;
using dynfunc = bool(*)(dyn_arg_list);

/** Contains helper functions for parsing */
namespace DynRpg {
	class EasyRpgPlugin;

	std::string ParseVarArg(StringView func_name, dyn_arg_list args, int index, bool& parse_okay);
	std::string ParseCommand(std::string command, std::vector<std::string>& params);

	namespace detail {
		template <typename T>
		inline bool parse_arg(StringView, dyn_arg_list, const int, T&, bool&) {
			static_assert(sizeof(T) == -1, "Only parsing int, float and std::string supported");
			return false;
		}

		// FIXME: Extracting floats that are followed by chars behaviour varies depending on the C++ library
		// see https://bugs.llvm.org/show_bug.cgi?id=17782
		template <>
		inline bool parse_arg(StringView func_name, dyn_arg_list args, const int i, float& value, bool& parse_okay) {
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
		inline bool parse_arg(StringView func_name, dyn_arg_list args, const int i, int& value, bool& parse_okay) {
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
		inline bool parse_arg(StringView, dyn_arg_list args, const int i, std::string& value, bool& parse_okay) {
			if (!parse_okay) return false;
			value = args[i];
			parse_okay = true;
			return parse_okay;
		}

		template <typename Tuple, std::size_t... I>
		inline void parse_args(StringView func_name, dyn_arg_list in, Tuple& value, bool& parse_okay, std::index_sequence<I...>) {
			(void)std::initializer_list<bool>{parse_arg(func_name, in, I, std::get<I>(value), parse_okay)...};
		}
	}


	template <typename... Targs>
	std::tuple<Targs...> ParseArgs(StringView func_name, dyn_arg_list args, bool* parse_okay = nullptr) {
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
	bool Invoke(StringView command, Game_Interpreter* interpreter = nullptr);
	void Update();
	void Load(int slot);
	void Save(int slot);

private:
	friend DynRpg::EasyRpgPlugin;

	bool Invoke(StringView func, dyn_arg_list args, Game_Interpreter* interpreter = nullptr);
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

	StringView GetIdentifier() const { return identifier; }
	virtual bool Invoke(StringView func, dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter) = 0;
	virtual void Update() {}
	virtual void Load(const std::vector<uint8_t>&) {}
	virtual std::vector<uint8_t> Save() { return {}; }

protected:
	Game_DynRpg& instance;

private:
	std::string identifier;
};

#endif
