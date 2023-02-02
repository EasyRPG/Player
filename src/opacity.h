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

#ifndef EP_OPACITY_H
#define EP_OPACITY_H

#include <cassert>
#include <cstdint>
#include <climits>
#include <memory>

/** Opacity class.  */
struct Opacity {
	static constexpr Opacity Opaque() { return Opacity(); }

	int top = 255;
	int bottom = 255;
	int split = 0;

	constexpr Opacity() = default;

	constexpr Opacity(int opacity) :
		Opacity(opacity, opacity, 0) {}

	constexpr Opacity(int top_opacity, int bottom_opacity, int split) :
		top(top_opacity), bottom(bottom_opacity), split(split) {}

	int Value() const {
		assert(!IsSplit());
		return top;
	}

	constexpr bool IsSplit() const {
		return split > 0 && top != bottom;
	}

	constexpr bool IsTransparent() const {
		return IsSplit() ? top <= 0 && bottom <= 0 : top <= 0;
	}

	constexpr bool IsOpaque() const {
		return IsSplit() ? top >= 255 && bottom >= 255 : top >= 255;
	}

	private:
};

/**
 * Enum used to describe the opacity of an entire image.
 * This is used to decide whether we can shortcut fast blit
 * operations
 */
enum class ImageOpacity {
	/** Image uses the alpha channel, no optimisations possible */
	Alpha_8Bit,
	/** Image has pixels that are either full opaque or transparent (1 Bit Alpha) */
	Alpha_1Bit,
	/** Image is full opaque and can be blitted fast */
	Opaque,
	/** Image is completely transparent and blitting can be skipped entirely */
	Transparent
};

/**
 * Structure used to compactly store ImageOpacity values for a tiled bitmap
 */
class TileOpacity {
	public:
		/** Initialize with no tiles */
		constexpr TileOpacity() = default;

		/** Initialize with num_tiles tiles */
		explicit TileOpacity(int w, int h);

		/** Get ImageOpacity for tile at x, y */
		ImageOpacity Get(int x, int y) const;

		/** Set ImageOpacity for tile at x, y */
		void Set(int x, int y, ImageOpacity op);

		/** @return true if no tile opacities stored */
		bool Empty() const;

	private:
		std::unique_ptr<uint8_t[]> _p;
		int _w = 0;
		int _h = 0;
};

inline TileOpacity::TileOpacity(int w, int h)
	: _p(new uint8_t[w * h]), _w(w), _h(h)
{
	assert(_w >= 0);
	assert(_h >= 0);
}

inline ImageOpacity TileOpacity::Get(int x, int y) const {
	assert(x >= 0);
	assert(y >= 0);

	if (x >= _w || y >= _h) {
		return ImageOpacity::Alpha_8Bit;
	}

	return static_cast<ImageOpacity>(_p[x + y * _w]);
}

inline void TileOpacity::Set(int x, int y, ImageOpacity op) {
	assert(x >= 0 && x < _w);
	assert(y >= 0 && y < _h);

	_p[x + y * _w] = static_cast<uint8_t>(op);
}

inline bool TileOpacity::Empty() const {
	return _w * _h == 0;
}

#endif
