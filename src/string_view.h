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

#ifndef EP_STRING_VIEW_H
#define EP_STRING_VIEW_H

#include <lcf/string_view.h>
#include <lcf/dbstring.h>
#include <fmt/format.h>

#if FMT_VERSION < 60000
// Remove after 0.8.1 in 2024: allow building with fmt 5
#  include <fmt/ostream.h>
#endif

using lcf::ToString;
using lcf::StartsWith;
using lcf::EndsWith;

// Version required to use the new formatting API
#define EP_FMT_MODERN_VERSION 80000

// FIXME: liblcf doesn't depend on fmt, so we need to add this here to enable fmtlib support for our std::string_view.
#if FMT_VERSION >= EP_FMT_MODERN_VERSION
template<>
struct fmt::formatter<lcf::DBString> : formatter<string_view> {
	auto format(const lcf::DBString& s, format_context& ctx) const -> decltype(ctx.out());
};
#else
namespace lcf {
inline fmt::basic_string_view<char> to_string_view(const lcf::DBString& s) {
	return to_string_view(std::string_view(s));
}
}
#endif

#endif
