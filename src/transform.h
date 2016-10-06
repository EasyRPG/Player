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

#ifndef _EASYRPG_MATRIX_H_
#define _EASYRPG_MATRIX_H_

// Headers
#include <cmath>
#include <pixman.h>

// pixman_transform interface class

struct Transform {
	pixman_transform_t matrix;

	Transform(const pixman_transform_t& m) : matrix(m) {}

	static inline Transform Rotation(double angle) {
		pixman_transform_t rotation;
		pixman_transform_init_rotate(&rotation,
			pixman_double_to_fixed(cos(angle)),
			pixman_double_to_fixed(sin(angle)));
		return rotation;
	}

	static inline Transform Scale(double sx, double sy) {
		pixman_transform_t scale;
		pixman_transform_init_scale(&scale,
			pixman_double_to_fixed(sx),
			pixman_double_to_fixed(sy));
		return scale;
	}

	static inline Transform Scale(int sx, int sy) {
		pixman_transform_t scale;
		pixman_transform_init_scale(&scale,
			pixman_int_to_fixed(sx),
			pixman_int_to_fixed(sy));
		return scale;
	}

	static inline Transform Translation(int x, int y) {
		pixman_transform_t translation;
		pixman_transform_init_translate(&translation,
			pixman_int_to_fixed(x),
			pixman_int_to_fixed(y));
		return translation;
	}

	Transform& operator*=(const Transform& rhs) {
		pixman_transform_multiply(&matrix, &matrix, &rhs.matrix);
		return *this;
	}

	inline Transform Inverse() const {
		pixman_transform_t inverse;
		pixman_transform_invert(&inverse, &matrix);
		return inverse;
	}
};

#endif

