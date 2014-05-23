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
#include "filefinder.h"

#include <stdint.h>
#include <cassert>
#include <cctype>
#include <ciso646>
#include <cstdlib>

#include <algorithm>
#include <fstream>
#include <vector>

#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>
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
#include <boost/spirit/include/qi_no_skip.hpp>
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
			std::string();
}

using boost::container::vector;
using boost::format;

typedef vector<uint8_t> binary_type;
typedef boost::variant<std::string, binary_type, uint32_t> section_value;
typedef boost::unordered_map<std::string, section_value> section;
typedef boost::unordered_map<std::string, section> section_list;

namespace spirit = boost::spirit;
namespace qi = spirit::qi;
namespace phoenix = boost::phoenix;


struct parse_registry {
	std::string line;
	unsigned line_number;
	section_list result;
	std::string::const_iterator i, end;
	std::ifstream stream;

	void error(format const& fmt) {
		if(line_number > 0) {
			Output::Warning(
				"Wine registry error: %s\nline %d: \"%s\"",
				fmt.str().c_str(), line_number, line.c_str());
		} else {
			Output::Warning( "Wine registry error: %s", fmt.str().c_str());
		}
	}

	std::istream& getline() {
		line.clear();
		std::string tmp;
		do {
			if(not line.empty() and *line.rbegin() == '\\')
			{ line.resize(line.size() - 1); }
			std::getline(stream, tmp);
			++line_number;
			line += tmp;
		} while(stream and not tmp.empty() and *tmp.rbegin() == '\\');
		assert(line.empty() or *line.rbegin() != '\\');
		i = line.begin(), end = line.end();
		return stream;
	}

	vector<uint16_t> escaped;
	qi::symbols<char, uint16_t> escape;

	template<char TERM> std::string parse_str() {
		std::string::const_iterator str_beg = i, str_end = i - 1;
		bool has_escape_char = false;
		do {
			++str_end;
			std::string::const_iterator const next_str_end = std::find(str_end, end, TERM);
			if (not has_escape_char) {
				has_escape_char = std::find(str_end, next_str_end, '\\') != next_str_end;
			}
			str_end = next_str_end;
		} while (str_end < end and str_end[-1] == '\\' and str_end[-2] != '\\');
		if(str_end >= end) {
			error(format("string parse error"));
			return std::string();
		}
		i = str_end + 1; // skip term char

		if (not has_escape_char) { return std::string(str_beg, str_end); }

		typedef boost::u16_to_u32_iterator<vector<uint16_t>::const_iterator> u16_to_u32;
		typedef boost::u32_to_u8_iterator<u16_to_u32> u32_to_u8;

		// utf-8 -> utf-16
		escaped.clear();
		qi::uint_parser<uint16_t, 8, 1, 3> octal;
		qi::uint_parser<uint16_t, 16, 1, 4> hex;
		if(not qi::parse(
			   str_beg, str_end,
			   *(escape | ('\\' >> (('x' >> hex) | octal)) | qi::char_), escaped)
		   or str_beg != str_end) {
			error(format("escaping error"));
			return std::string();
		}

		// utf-16 -> utf-8
		return std::string(
				u32_to_u8(u16_to_u32(escaped.begin(), escaped.begin(), escaped.end())),
				u32_to_u8(u16_to_u32(escaped.end  (), escaped.begin(), escaped.end())));
	}

	section_value parse_value() {
		using qi::phrase_parse;
		using qi::_1;
		using qi::char_;
		using qi::uint_;
		using spirit::ascii::space;
		using phoenix::push_back;

		enum { STRING, BINARY, INTEGER };
		static struct prefix {
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
			if(size_t(i - end) < pre->pre.size() or
                           not std::equal(i, i + pre->pre.size(), pre->pre.begin())) { continue; }

			i += pre->pre.size();
			switch(pre->type) {
				case STRING:
					return (*i == '\'')? std::string() : parse_str<'\"'>();
				case BINARY: {
					binary_type ret;
					qi::uint_parser<uint8_t, 16, 2, 2> octed;
					if(not phrase_parse(i, end, -("(" >> uint_ >> ")") >> ":" >>
										octed[push_back(phoenix::ref(ret), _1)] % ',' >>
										-char_(')'), space)) {
						error(format("cannot parse %s") % pre->pre);
					}
					return ret;
				}
				case INTEGER: {
					uint32_t ret;
                                        qi::uint_parser<uint32_t, 16, 8, 8> dword;
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

	parse_registry(std::string const& name)
			: line_number(0)
			, stream(name.c_str(), std::ios_base::binary | std::ios_base::in)
	{
		if(not stream) {
			error(format("file open error: \"%s\"") % name);
			return;
		}

		escape.add
				("\\a", '\a')("\\b", '\b')("\\e", '\x1B')("\\f", '\f')
				("\\n", '\n')("\\r", '\r')("\\t", '\t')("\\v", '\v')
				("\\\\", '\\')("\\\"", '"');

		getline();
		if(line != "WINE REGISTRY Version 2") {
			error(format("file signature error"));
			return;
		}

		section current_section;
		std::string current_section_name;

		while(getline()) {
			skip_space();
			if(i >= end) { continue; } // empty line

			switch(*i) {
				case '[':
					if(not current_section_name.empty()) {
						assert(result.find(current_section_name) == result.end());
						result[current_section_name].swap(current_section);
					}
					++i; // skip '['
					parse_str<']'>().swap(current_section_name);
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
					if(i >= end or *i != '=') {
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
		current_section.swap(result[current_section_name]);

		Output::Debug("loaded registry: %s", name.c_str());
	}
};

section_list const& get_section(HKEY key) {
	static section_list const empty_sec;
	static section_list local_machine, current_user;

	std::string const prefix = get_wine_prefix();

	if(prefix.empty() or not FileFinder::Exists(prefix)) {
		Output::Debug("wine prefix not found: \"%s\"", prefix.c_str());
		return empty_sec;
	}

	switch(key) {
		case HKEY_LOCAL_MACHINE:
			if(local_machine.empty()) {
                        	parse_registry(prefix + "/system.reg").result.swap(local_machine);
			}
			return local_machine;
		case HKEY_CURRENT_USER:
			if(current_user.empty()) {
                        	parse_registry(prefix + "/user.reg").result.swap(current_user);
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
  T const* ptr;
	section_value_opt const v = get_value(get_section(hkey), key, val);
	if(v == boost::none) {
		Output::Debug("registry not found: %s, %s", key.c_str(), val.c_str());
		return err_val;
	}
	if(not (ptr = boost::get<T>(&*v))) {
		Output::Debug("type mismatch: %s, %s", key.c_str(), val.c_str());
		return err_val;
	}
	return *ptr;
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
		case 'z': path.assign(ret.begin() + 2, ret.end()); break;
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
