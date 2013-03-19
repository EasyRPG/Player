#include "registry.h"

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

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

namespace {
std::string get_wine_prefix() {
	return
			getenv("WINEPREFIX")? getenv("WINEPREFIX"):
			getenv("HOME")? std::string(getenv("HOME")).append("/.wine"):
			"";
}
typedef boost::container::vector<uint8_t> binary_type;
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

	template<char term>
	std::string parse_str(std::string::const_iterator& i,
						  std::string::const_iterator const end)
	{
		using qi::phrase_parse;
		using qi::_1;
		using qi::char_;
		using qi::symbols;
		using phoenix::push_back;

		symbols<char, char> escape;
		escape.add
				("\\a", '\a')("\\b", '\b')("\\e", '\e')("\\f", '\f')
				("\\n", '\n')("\\r", '\r')("\\t", '\t')("\\v", '\v')
				("\\\\", '\\')("\\\"", '"');

		std::string ret;
		if(not phrase_parse(
			   i, end, *(escape | ~char_(term))[
				   push_back(phoenix::ref(ret), _1)] >> char_(term), ~char_)) {
			std::cerr << "string parse error: " << line << ", " << std::string(i, end) << std::endl;
			return std::string();
		}
		return ret;
	}

	std::istream& getline_ext(std::istream& ifs, std::string& ret) {
		ret.clear();
		std::string tmp;
		do {
			if(not ret.empty() and *ret.rbegin() == '\\') { ret.erase(ret.rbegin().base()); }
			std::getline(ifs, tmp);
			++line_number;
			ret += tmp;
		} while(ifs and not tmp.empty() and *tmp.rbegin() == '\\');
		if(not ret.empty()) { assert(*ret.rbegin() != '\\'); }
		return ifs;
	}

	section_value parse_value(std::string::const_iterator& i, std::string::const_iterator const end) {
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
					return (*i == '\'')? std::string() : parse_str<'\"'>(i, end);
				case BINARY: {
					binary_type ret;
					uint_parser<uint8_t, 16, 2, 2> octed;
					if(not phrase_parse(i, end, -("(" >> uint_ >> ")") >> ":" >>
										octed[push_back(phoenix::ref(ret), _1)] % ',' >>
										-char_(')'), space)) {
						std::cerr << std::string(i, end) << std::endl;
						std::cerr << "cannot parse " << pre->pre << std::endl;
					}
					return ret;
				}
				case INTEGER: {
					uint32_t ret;
					uint_parser<uint32_t, 16, 8, 8> dword;
					if(not phrase_parse(i, end, dword, space, ret)) {
						std::cerr << "cannot parse " << pre->pre << std::endl;
					}
					return ret;
				}
				default: assert(false);
			}
		}

		return std::string();
	}

	void skip_space(std::string::const_iterator& i, std::string::const_iterator end) {
		while(i < end and std::isspace(*i)) { ++i; }
	}

	parse_registry(std::string const& name) {
		std::ifstream ifs(name.c_str(), std::ios_base::binary | std::ios_base::in);
		if(not ifs) {
			std::cerr << "file open error: " << name << std::endl;
			return;
		}

		line_number = 1;

		getline_ext(ifs, line);
		if(line != "WINE REGISTRY Version 2") {
			std::cerr << "file signature error: " << line << std::endl;
			return;
		}

		section current_section;
		std::string current_section_name;

		while(getline_ext(ifs, line)) {
			std::string::const_iterator i = line.begin();

			skip_space(i, line.end());
			if(i >= line.end()) { continue; } // empty line

			switch(*i) {
				case '[':
					if(not current_section_name.empty()) {
						result[current_section_name] = current_section;
						current_section.clear();
					}
					current_section_name = parse_str<']'>(++i, line.end());
					break;
				case '@': break; // skip
				case '\"': {
					if(current_section_name.empty()) {
						std::cerr << "value without key" << std::endl;
						return;
					}
					std::string const val_name = parse_str<'\"'>(++i, line.end());
					skip_space(i, line.end());
					if(i >= line.end() or *i != '=') {
						std::cerr << "unexpected char or end of line:" << line << std::endl;
						return;
					}
					assert(*i == '=');
					++i; // skip =
					skip_space(i, line.end());
					current_section[val_name] = parse_value(i, line.end());
				} break;
				case '#': break; // skip
				case ';': break; // comment line
				default:
					std::cerr << "invalid line: " << line << std::endl;
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
		std::cerr << "registry not found: " << key << "\\" << val << std::endl;
		return err_val;
	}
	if(not boost::get<T>(&*v)) {
		std::cerr << "type mismatch: " << key << "\\" << val << std::endl;
		return err_val;
	}
	return boost::get<T const>(*v);
}
}

std::string Registry::ReadStrValue(HKEY hkey, std::string const& key, std::string const& val) {
	std::string const ret = get_value_with_type<std::string>(hkey, key, val);
	if(ret.size() >= 3 and std::isupper(*ret.begin()) and std::string(ret.begin() + 1, ret.begin() + 3) == ":\\") {
		switch(*ret.begin()) {
			case 'C': {
				std::string drive_c = get_wine_prefix();
				drive_c.append("/drive_c").append(ret.begin() + 2, ret.end());
				std::replace(drive_c.begin(), drive_c.end(), '\\', '/');
				std::cerr << drive_c << std::endl;
				return drive_c;
			}
			case 'Z': {
				std::string root_path(ret.begin() + 2, ret.end());
				std::replace(root_path.begin(), root_path.end(), '\\', '/');
				std::cerr << root_path << std::endl;
				return root_path;
			}
			default:
				std::cerr << "Unknown drive: " << *ret.begin() << std::endl;
		}
	} else { return ret; }
}
int Registry::ReadBinValue(HKEY hkey, std::string const& key, std::string const& val, unsigned char* out) {
	binary_type const bin = get_value_with_type<binary_type>(hkey, key, val);
	std::copy(bin.begin(), bin.end(), out);
	return bin.size();
}
