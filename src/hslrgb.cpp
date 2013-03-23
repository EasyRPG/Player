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

// Headers
#include "hslrgb.h"
#include "util_macro.h"

struct ColorHSL {
	double h;
	double s;
	double l;
};

/**
 ** RGB to HSL.
 */
ColorHSL RGB2HSL(Color col) {
	ColorHSL ncol;
	double vmin, vmax, delta;
	double r, g, b;
	r = col.red / 255.0;
	g = col.green / 255.0;
	b = col.blue / 255.0;
	vmin = min(min(r, g), b);
	vmax = max(max(r, g), b);
	delta = vmax - vmin;
	ncol.l = (vmax + vmin) / 2;
	if (delta == 0) {
		ncol.h = 0;
		ncol.s = 0;
	} else {
		double dr, dg, db;
		if (ncol.l < 0.5) {
			ncol.s = delta / (vmax + vmin);
		} else {
			ncol.s = delta / (2 - vmax - vmin);
		}
		dr = (((vmax - r) / 6) + (delta / 2)) / delta;
		dg = (((vmax - g) / 6) + (delta / 2)) / delta;
		db = (((vmax - b) / 6) + (delta / 2)) / delta;
		if (r == vmax) {
			ncol.h = db - dg;
		} else if (g == vmax) {
			ncol.h = (1.0 / 3) + dr - db;
		} else if (b == vmax) {
			ncol.h = (2.0 / 3) + dg - dr;
		}
	}
	return ncol;
}

/**
 * Hue to RGB.
 */
double Hue_2_RGB(double v1, double v2, double vH) {
	if (vH < 0) vH += 1;
	if (vH > 1) vH -= 1;
	if ((6 * vH) < 1) return (v1 + (v2 - v1) * 6 * vH);
	if ((2 * vH) < 1) return (v2);
	if ((3 * vH) < 2) return (v1 + (v2 - v1) * ((2.0 / 3) - vH ) * 6);
	return v1;
}

/**
 * HSL to RGB.
 */
Color HSL2RGB(ColorHSL col) {
	Color ncol(0, 0, 0, 0);
	if (col.s == 0) {
		ncol.red = (unsigned char)(col.l * 255);
		ncol.green = (unsigned char)(col.l * 255);
		ncol.blue = (unsigned char)(col.l * 255);
	} else {
		double v1, v2;
		if (col.l < 0.5) {
			v2 = col.l * (1 + col.s);
		} else {
			v2 = (col.l + col.s) - (col.s * col.l);
		}
		v1 = 2 * col.l - v2;
		ncol.red = (unsigned char)(255 * Hue_2_RGB(v1, v2, col.h + (1.0 / 3)));
		ncol.green = (unsigned char)(255 * Hue_2_RGB(v1, v2, col.h));
		ncol.blue = (unsigned char)(255 * Hue_2_RGB(v1, v2, col.h - (1.0 / 3)));
	}
	return ncol;
}

Color RGBAdjustHSL(Color col, double h, double s, double l) {
	ColorHSL hsl;
	Color rgb = col;
	hsl = RGB2HSL(rgb);
	hsl.h = hsl.h + h / 360.0;
	while (hsl.h > 1) hsl.h -= 1;
	while (hsl.h < 0) hsl.h += 1;
	hsl.s = hsl.s + s;
	if (hsl.s > 1) hsl.s = 1;
	if (hsl.s < 0) hsl.s = 0;
	hsl.l = hsl.l * l;
	if (hsl.l > 1) hsl.l = 1;
	if (hsl.l < 0) hsl.l = 0;
	rgb = HSL2RGB(hsl);
	rgb.alpha = col.alpha;
	return rgb;
}
