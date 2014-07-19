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

// 2D Matrix class

struct Matrix {
	double xx, xy, x0;
	double yx, yy, y0;

	Matrix(double xx, double xy, double x0, double yx, double yy, double y0)
	    : xx(xx), xy(xy), x0(x0), yx(yx), yy(yy), y0(y0) {}

	static inline Matrix Rotation(double angle) {
		double c = cos(angle);
		double s = sin(angle);
		return Matrix(c, s, 0, -s, c, 0);
	}

	static inline Matrix Scale(double sx, double sy) { return Matrix(sx, 0, 0, 0, sy, 0); }

	static inline Matrix Translation(double x, double y) { return Matrix(1, 0, x, 0, 1, y); }

	static inline Matrix Multiply(const Matrix& a, const Matrix& b) {
		return Matrix(a.xy * b.yx + a.xx * b.xx, a.xy * b.yy + a.xx * b.xy,
		              a.xy * b.y0 + a.xx * b.x0 + a.x0, a.yy * b.yx + a.yx * b.xx,
		              a.yy * b.yy + a.yx * b.xy, a.yy * b.y0 + a.yx * b.x0 + a.y0);
	}

	inline Matrix PreMultiply(const Matrix& a) const { return Multiply(a, *this); }

	inline Matrix PostMultiply(const Matrix& b) const { return Multiply(*this, b); }

	inline Matrix Inverse() const {
		double det = xx * yy - xy * yx;
		return Matrix(yy / det, -xy / det, (y0 * xy - x0 * yy) / det, -yx / det, xx / det,
		              (x0 * yx - y0 * xx) / det);
	}

	inline void Transform(double x, double y, double& rx, double& ry) const {
		rx = xx * x + xy * y + x0;
		ry = yx * x + yy * y + y0;
	}

	// in bitmap_utils.cpp
	static Matrix Setup(double angle, double scale_x, double scale_y, int src_pos_x, int src_pos_y,
	                    int dst_pos_x, int dst_pos_y);
};

#endif
