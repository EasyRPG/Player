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

#include "string_view.h"

#if FMT_VERSION >= EP_FMT_MODERN_VERSION

#include <fmt/format.h>

auto fmt::formatter<lcf::DBString>::format(const lcf::DBString& s, format_context& ctx) const -> decltype(ctx.out()) {
	string_view sv(s.data(), s.size());
	return formatter<string_view>::format(sv, ctx);
}

auto fmt::formatter<lcf::StringView>::format(const lcf::StringView& s, format_context& ctx) const -> decltype(ctx.out()) {
	string_view sv(s.data(), s.size());
	return formatter<string_view>::format(sv, ctx);
}

#endif
