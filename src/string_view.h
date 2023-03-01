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

// FIXME: needed to allow building with fmt 5, older versions are untested.
#if FMT_VERSION < 60000
#include <fmt/ostream.h>
#endif

using StringView = lcf::StringView;
using U32StringView = lcf::U32StringView;

using lcf::ToString;
using lcf::ToStringView;

// FIXME: liblcf doesn't depend on fmt, so we need to add this here to enable fmtlib support for our StringView.
namespace nonstd { namespace sv_lite {
template <typename C, typename T>
inline fmt::basic_string_view<C> to_string_view(basic_string_view<C,T> s) {
    return fmt::basic_string_view<C>(s.data(), s.size());
}
} }

#endif
