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

#ifndef EP_PIXMAN_IMAGE_PTR_H
#define EP_PIXMAN_IMAGE_PTR_H

#include <functional>
#include <pixman.h>

class PixmanImagePtr {
	public:
		/** Construct with nullptr */
		constexpr PixmanImagePtr() = default;

		/** Construct with nullptr */
		constexpr PixmanImagePtr(std::nullptr_t) noexcept;

		/** Take ownership of img */
		explicit constexpr PixmanImagePtr(pixman_image_t* img) noexcept;

		/** Increment ref count and share img with other */
		PixmanImagePtr(const PixmanImagePtr&);

		/** Increment ref count and share img with other */
		PixmanImagePtr& operator=(const PixmanImagePtr&);

		/** Take img from other */
		PixmanImagePtr(PixmanImagePtr&&) noexcept;

		/** Take img from other */
		PixmanImagePtr& operator=(PixmanImagePtr&&) noexcept;

		/** Unreference and possibly destroy image */
		~PixmanImagePtr();

		/** @return pointer to image */
		pixman_image_t* get() const noexcept;

		/** @return reference to image */
		pixman_image_t& operator*() const noexcept;

		/** @return pointer to image */
		pixman_image_t* operator->() const noexcept;

		/** @return true if image is not nullptr */
		explicit operator bool() const noexcept;

		/** 
		 * Release current image (if any) and take ownership of img
		 *
		 * @param img new image to take ownership of
		 */
		void reset(pixman_image_t* img = nullptr) noexcept;
	private:
		pixman_image_t* _img = nullptr;
};

inline bool operator==(const PixmanImagePtr& l, const PixmanImagePtr& r) { return l.get() == r.get(); }
inline bool operator!=(const PixmanImagePtr& l, const PixmanImagePtr& r) { return l.get() != r.get(); }
inline bool operator<(const PixmanImagePtr& l, const PixmanImagePtr& r) { return l.get() < r.get(); }
inline bool operator<=(const PixmanImagePtr& l, const PixmanImagePtr& r) { return l.get() <= r.get(); }
inline bool operator>(const PixmanImagePtr& l, const PixmanImagePtr& r) { return l.get() > r.get(); }
inline bool operator>=(const PixmanImagePtr& l, const PixmanImagePtr& r) { return l.get() >= r.get(); }

namespace std {
template <> struct hash<PixmanImagePtr> {
	size_t operator()(const PixmanImagePtr& p) {
		return std::hash<void*>()(p.get());
	}
};
}

inline constexpr PixmanImagePtr::PixmanImagePtr(std::nullptr_t) noexcept {}

inline constexpr PixmanImagePtr::PixmanImagePtr(pixman_image_t* img) noexcept
	: _img(img) {}

inline PixmanImagePtr::PixmanImagePtr(const PixmanImagePtr& o)
	: _img(o._img ? ::pixman_image_ref(o._img) : nullptr)
{ }

inline PixmanImagePtr& PixmanImagePtr::operator=(const PixmanImagePtr& o) {
	if (this != &o) {
		reset();
		_img = o._img ? ::pixman_image_ref(o._img) : nullptr;
	}
	return *this;
}
inline PixmanImagePtr::PixmanImagePtr(PixmanImagePtr&& o) noexcept
	:_img(o._img)
{
	o._img = nullptr;
}

inline PixmanImagePtr& PixmanImagePtr::operator=(PixmanImagePtr&& o) noexcept {
	if (this != &o) {
		reset();
		_img = o._img;
		o._img = nullptr;
	}
	return *this;
}

inline PixmanImagePtr::~PixmanImagePtr() {
	reset();
}

inline pixman_image_t* PixmanImagePtr::get() const noexcept {
	return _img;
}

inline pixman_image_t& PixmanImagePtr::operator*() const noexcept {
	return *_img;
}

inline pixman_image_t* PixmanImagePtr::operator->() const noexcept {
	return _img;
}

inline PixmanImagePtr::operator bool() const noexcept {
	return get() != nullptr;
}

inline void PixmanImagePtr::reset(pixman_image_t* img) noexcept {
	if (_img) {
		::pixman_image_unref(_img);
	}
	_img = img;
}

#endif
