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

#include "registry.h"
#include "output.h"

#include <stdint.h>
#include <cassert>
#include <cctype>
#include <ciso646>
#include <cstdlib>

#include <algorithm>
#include <fstream>
#include <vector>

#include <boost/variant.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <boost/optional.hpp>
#include <boost/regex/pending/unicode_iterator.hpp>
#include <boost/format.hpp>

#include <boost/spirit/include/qi_action.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_operator.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_symbols.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/spirit/include/support_ascii.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

namespace {
std::string get_wine_prefix() {
	return
			getenv("WINEPREFIX")? getenv("WINEPREFIX"):
			getenv("HOME")? std::string(getenv("HOME")).append("/.wine"):
			"";
}

using boost::container::vector;
using boost::format;

typedef vector<uint8_t> binary_type;
typedef boost::variant<std::string, binary_type, uint32_t> section_value;
typedef boost::container::flat_map<std::string, section_value> section;
typedef boost::container::flat_map<std::string, section> section_list;

namespace spirit = boost::spirit;
namespace qi = spirit::qi;
namespace phoenix = boost::phoenix;


struct parse_registry {
	std::string line;
	unsigned line_number;
	section_list result;
	std::string::const_iterator i, end;

	void error(format const& fmt) {
		Output::Warning(
			"Wine registry error: %s\nline %d: \"%s\"",
			fmt.str().c_str(), line_number, line.c_str());
	}

	std::istream& getline(std::istream& ifs) {
		line.clear();
		std::string tmp;
		do {
			if(not line.empty() and *line.rbegin() == '\\') { line.erase(line.rbegin().base()); }
			std::getline(ifs, tmp);
			++line_number;
			line += tmp;
		} while(ifs and not tmp.empty() and *tmp.rbegin() == '\\');
		if(not line.empty()) { assert(*line.rbegin() != '\\'); }
		i = line.begin(), end = line.end();
		return ifs;
	}

	template<char term> std::string parse_str() {
		using qi::phrase_parse;
		using qi::char_;
		using qi::symbols;
		using qi::uint_parser;

		symbols<char, char> escape;
		escape.add
				("\\a", '\a')("\\b", '\b')("\\e", '\e')("\\f", '\f')
				("\\n", '\n')("\\r", '\r')("\\t", '\t')("\\v", '\v')
				("\\\\", '\\')("\\\"", '"');

		std::string ret;
		if(not phrase_parse(
			   i, end, *(escape | ~char_(term)) >> term, ~char_, ret)) {

			error(format("string parse error"));
			return std::string();
		}

		typedef boost::u8_to_u32_iterator<std::string::const_iterator> u8_to_u32;
		typedef boost::u32_to_u16_iterator<vector<uint32_t>::const_iterator> u32_to_u16;
		typedef boost::u16_to_u32_iterator<vector<uint16_t>::const_iterator> u16_to_u32;
		typedef boost::u32_to_u8_iterator<vector<uint32_t>::const_iterator> u32_to_u8;

		// utf-8 -> utf-16
		vector<uint32_t> utf32(
			u8_to_u32(ret.begin(), ret.begin(), ret.end()),
			u8_to_u32(ret.end(), ret.begin(), ret.end()));
		vector<uint16_t> utf16(u32_to_u16(utf32.begin()), u32_to_u16(utf32.end()));

		vector<uint16_t> escaped;
		escaped.reserve(utf16.size());
		vector<uint16_t>::const_iterator
				escaping = utf16.begin(), escaping_end = utf16.end();
		uint_parser<uint16_t, 8, 1, 3> octal;
		uint_parser<uint16_t, 16, 1, 4> hex;

		if(not phrase_parse(
			   escaping, escaping_end,
			   *(('\\' >> (('x' >> hex) | octal)) | char_),
			   ~char_, escaped) or escaping != utf16.end()) {
			Output::Debug("unicode escaping error");
		}

		// utf-16 -> utf-8
		utf32.assign(u16_to_u32(escaped.begin(), escaped.begin(), escaped.end()),
					 u16_to_u32(escaped.end(), escaped.begin(), escaped.end()));
		ret.assign(u32_to_u8(utf32.begin()), u32_to_u8(utf32.end()));

		return ret;
	}

	section_value parse_value() {
		using qi::phrase_parse;
		using qi::_1;
		using qi::uint_parser;
		using qi::char_;
		using qi::uint_;
		using spirit::ascii::space;
		using phoenix::push_back;

		enum { STRING, BINARY, INTEGER };
		struct prefix {
			std::string pre;
			int type;
		} const prefixes[] = {
			{ "\"", STRING },
			{ "str:\"", STRING },
			{ "str(2):\"", STRING },
			{ "str(7):\"", STRING },
			{ "hex", BINARY },
			{ "dword:", INTEGER },
			{ "", -1 },
		};

		for(prefix const* pre = prefixes; not pre->pre.empty(); ++pre) {
			if(size_t(i - end) < pre->pre.size()) { continue; }

			if(std::string(i, i + pre->pre.size()) != pre->pre) { continue; }

			i += pre->pre.size();
			switch(pre->type) {
				case STRING:
					return (*i == '\'')? std::string() : parse_str<'\"'>();
				case BINARY: {
					binary_type ret;
					uint_parser<uint8_t, 16, 2, 2> octed;
					if(not phrase_parse(i, end, -("(" >> uint_ >> ")") >> ":" >>
										octed[push_back(phoenix::ref(ret), _1)] % ',' >>
										-char_(')'), space)) {
						error(format("cannot parse %s") % pre->pre);
					}
					return ret;
				}
				case INTEGER: {
					uint32_t ret;
					uint_parser<uint32_t, 16, 8, 8> dword;
					if(not phrase_parse(i, end, dword, space, ret)) {
						error(format("cannot parse %s") % pre->pre);
					}
					return ret;
				}
				default: assert(false);
			}
		}

		return std::string();
	}

	void skip_space() {
		while(i < end and std::isspace(*i)) { ++i; }
	}

	parse_registry(std::string const& name) {
		std::ifstream ifs(name.c_str(), std::ios_base::binary | std::ios_base::in);
		if(not ifs) {
			error(format("file open error: \"%s\"") % name);
			return;
		}

		line_number = 1;

		getline(ifs);
		if(line != "WINE REGISTRY Version 2") {
			error(format("file signature error"));
			return;
		}

		section current_section;
		std::string current_section_name;

		while(getline(ifs)) {
			skip_space();
			if(i >= line.end()) { continue; } // empty line

			switch(*i) {
				case '[':
					if(not current_section_name.empty()) {
						result[current_section_name] = current_section;
						current_section.clear();
					}
					++i; // skip '['
					current_section_name = parse_str<']'>();
					break;
				case '@': break; // skip
				case '\"': {
					if(current_section_name.empty()) {
						error(format("value without key"));
						return;
					}
					++i; // skip '\"'
					std::string const val_name = parse_str<'\"'>();
					skip_space();
					if(i >= line.end() or *i != '=') {
						error(format("unexpected char or end of line"));
						return;
					}
					assert(*i == '=');
					++i; // skip '='
					skip_space();
					current_section[val_name] = parse_value();
				} break;
				case '#': break; // skip
				case ';': break; // comment line
				default:
					error(format("invalid line"));
					return;
			}
		}
		result[current_section_name] = current_section;

		return;
	}
};

section_list const& get_section(HKEY key) {
	static section_list const empty_sec;
	static section_list local_machine, current_user;

	switch(key) {
		case HKEY_LOCAL_MACHINE:
			if(local_machine.empty()) {
				local_machine = parse_registry(get_wine_prefix() + "/system.reg").result;
			}
			return local_machine;
		case HKEY_CURRENT_USER:
			if(current_user.empty()) {
				current_user = parse_registry(get_wine_prefix() + "/user.reg").result;
			}
			return current_user;
		default: assert(false); return empty_sec;
	}
}

typedef boost::optional<section_value const&> section_value_opt;
section_value_opt get_value(section_list const& sec, std::string const& key, std::string const& val) {
	section_list::const_iterator sec_list_i = sec.find(key);
	if(sec_list_i == sec.end()) { return boost::none; }
	section::const_iterator sec_i = sec_list_i->second.find(val);
	if(sec_i == sec_list_i->second.end()) { return boost::none; }
	return section_value_opt(sec_i->second);
}
template<class T>
T const& get_value_with_type(HKEY hkey, std::string const& key, std::string const& val) {
	static T const err_val;
	section_value_opt const v = get_value(get_section(hkey), key, val);
	if(v == boost::none) {
		Output::Debug("registry not found: %s, %s", key.c_str(), val.c_str());
		return err_val;
	}
	if(not boost::get<T>(&*v)) {
		Output::Debug("type mismatch: %s, %s", key.c_str(), val.c_str());
		return err_val;
	}
	return boost::get<T const>(*v);
}
}

std::string Registry::ReadStrValue(HKEY hkey, std::string const& key, std::string const& val) {
	std::string const ret = get_value_with_type<std::string>(hkey, key, val);
	if(ret.size() < 3
	   or not std::isupper(*ret.begin())
	   or std::string(ret.begin() + 1, ret.begin() + 3) != ":\\")
	{ return ret; }

	std::string path;
	char const drive = std::tolower(*ret.begin());
	switch(drive) {
		default:
			path.assign(get_wine_prefix()).append("/drive_")
					.append(&drive, 1).append(ret.begin() + 2, ret.end());
			break;
		case 'Z': path.assign(ret.begin() + 2, ret.end()); break;
	}
	std::replace(path.begin(), path.end(), '\\', '/');

	Output::Debug("Path registry %s, %s: \"%s\"", key.c_str(), val.c_str(), path.c_str());

	return path;
}
int Registry::ReadBinValue(HKEY hkey, std::string const& key, std::string const& val, unsigned char* out) {
	binary_type const bin = get_value_with_type<binary_type>(hkey, key, val);
	std::copy(bin.begin(), bin.end(), out);
	return bin.size();
}
