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

#ifndef EP_DYNRPG_H
#define EP_DYNRPG_H

#include <cstdint>
#include <locale>
#include <vector>
#include <sstream>
#include <string>
#include <tuple>
#include "output.h"
#include "utils.h"

// Headers
namespace lcf {
namespace rpg {
	class EventCommand;
}
}

using dyn_arg_list = const Span<std::string>;
using dynfunc = bool(*)(dyn_arg_list);

/**
 * DynRPG namespace
 */
namespace DynRpg {
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

	void RegisterFunction(const std::string& name, dynfunc function);
	bool HasFunction(const std::string& name);
	std::string ParseVarArg(StringView func_name, dyn_arg_list args, int index, bool& parse_okay);
	std::string ParseCommand(const std::string& command, std::vector<std::string>& params);
	bool Invoke(const std::string& command);
	bool Invoke(const std::string& func, dyn_arg_list args);
	void Update();
	void Reset();
	void Load(int slot);
	void Save(int slot);

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

class DynRpgPlugin {
public:
	explicit DynRpgPlugin(std::string identifier) : identifier(std::move(identifier)) {}
	DynRpgPlugin() = delete;
	virtual ~DynRpgPlugin() {}

	const std::string& GetIdentifier() const { return identifier; }
	virtual void RegisterFunctions() {}
	virtual void Update() {}
	virtual void Load(const std::vector<uint8_t>&) {}
	virtual std::vector<uint8_t> Save() { return {}; }

private:
	std::string identifier;
};

#endif
