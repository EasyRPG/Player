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
#include <fmt/core.h>

// Needed to allow building with fmt 5, older versions are untested.
#if FMT_VERSION < 60000
#  include <fmt/ostream.h>
#endif

using StringView = lcf::StringView;
using U32StringView = lcf::U32StringView;

using lcf::ToString;
using lcf::ToStringView;

// Version required to use the new formatting API
#define EP_FMT_MODERN_VERSION 80000

// FIXME: liblcf doesn't depend on fmt, so we need to add this here to enable fmtlib support for our StringView.
#if FMT_VERSION >= EP_FMT_MODERN_VERSION
template<>
struct fmt::formatter<lcf::StringView> : fmt::formatter<fmt::string_view> {
	auto format(const lcf::StringView& s, format_context& ctx) const -> decltype(ctx.out());
};

template<>
struct fmt::formatter<lcf::DBString> : formatter<string_view> {
	auto format(const lcf::DBString& s, format_context& ctx) const -> decltype(ctx.out());
};
#else
namespace nonstd { namespace sv_lite {
template <typename C, typename T>
inline fmt::basic_string_view<C> to_string_view(basic_string_view<C,T> s) {
	return fmt::basic_string_view<C>(s.data(), s.size());
}
} }

namespace lcf {
inline fmt::basic_string_view<char> to_string_view(const lcf::DBString& s) {
	return to_string_view(StringView(s));
}
}
#endif

#endif
