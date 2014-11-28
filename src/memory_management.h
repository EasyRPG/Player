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

#ifndef _MEMORY_MANAGEMENT_H_
#define _MEMORY_MANAGEMENT_H_

// Headers
#include <stdint.h>
#include <boost/config.hpp>

#ifdef BOOST_NO_CWCHAR
typedef uint32_t wchar_t;
#endif

#if defined(BOOST_NO_CXX11_SMART_PTR)
#  include <boost/shared_ptr.hpp>
#  include <boost/weak_ptr.hpp>
#  include <boost/make_shared.hpp>

#  define EASYRPG_SHARED_PTR boost::shared_ptr
#  define EASYRPG_WEAK_PTR boost::weak_ptr
#  define EASYRPG_MAKE_SHARED boost::make_shared
#else
#  include <memory>

#  define EASYRPG_SHARED_PTR std::shared_ptr
#  define EASYRPG_WEAK_PTR std::weak_ptr
#  define EASYRPG_MAKE_SHARED std::make_shared
#endif

#ifdef BOOST_NO_CXX11_HDR_ARRAY
#  include <boost/array.hpp>
#  define EASYRPG_ARRAY boost::array
#else
#  include <array>
#  define EASYRPG_ARRAY std::array
#endif

class Bitmap;
class Font;

typedef EASYRPG_SHARED_PTR<Bitmap> BitmapRef;
typedef EASYRPG_SHARED_PTR<Font> FontRef;

#endif // _MEMORY_MANAGEMENT_H_
